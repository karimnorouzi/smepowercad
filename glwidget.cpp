/**********************************************************************
** smepowercad
** Copyright (C) 2015 Smart Micro Engineering GmbH
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "glwidget.h"

Q_LOGGING_CATEGORY(glwidget, "powercad.glwidget")

GLWidget::GLWidget(QWidget *parent, ItemDB *itemDB, ItemWizard *itemWizard, ItemGripModifier *itemGripModifier) :
    QOpenGLWidget(parent)
{
    qCDebug(glwidget) << "Created GLWidget";
    this->itemDB = itemDB;
    this->itemWizard = itemWizard;
    this->itemGripModifier = itemGripModifier;
    this->mousePos = QPoint();
    mouse_lastMidPress_dateTime = QDateTime::fromMSecsSinceEpoch(0);

    this->translationOffset = QPoint();
    this->zoomFactor = 1.0;
    this->centerOfViewInScene = QVector3D();
    this->displayCenter = QPoint();
    this->cuttingplane = CuttingPlane_nZ;
    this->height_of_intersection = QVector3D(0.0, 0.0, 100000.0);
    this->depth_of_view = QVector3D(0.0, 0.0, 200000.0);
    this->render_perspective = false;
    this->render_solid = true;
    this->render_outline = true;
    this->render_maintenance_area = true;
    this->render_selection = false;
    this->cameraPosition = QVector3D(0.0f, 0.0f, 0.0f);
    this->lookAtPosition = QVector3D(0.0f, 0.0f, 0.0f);
    this->matrix_modelview.setToIdentity();
    this->matrix_projection.setToIdentity();
    this->matrix_rotation.setToIdentity();
    this->matrix_rotation_old.setToIdentity();
    this->matrix_arcball.setToIdentity();
    this->arcballRadius = 500.0;
    this->matrix_glSelect.setToIdentity();
    this->matrix_all.setToIdentity();
    this->rendertime_ns_per_frame = ULONG_MAX;

    this->pickActive = false;
    this->cursorShown = true;
    this->arcballShown = false;
    this->snapMode = SnapNo;
    this->item_lastHighlight = NULL;
    this->selectItemsByColor = false;
    this->aspectRatio = -1.0;

    slot_update_settings();

    this->setPalette(Qt::transparent);
    this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    this->setAttribute(Qt::WA_ForceUpdatesDisabled, true);
    this->setAttribute(Qt::WA_Hover, false);
    this->setAttribute(Qt::WA_MouseTracking, true);
    this->setAttribute(Qt::WA_NoMousePropagation, true);
}

GLWidget::~GLWidget()
{
    makeCurrent();
    delete fbo_select;
    delete fbo_renderImage;
    delete openGLTimerQuery;
    delete shader_1_vert;
    delete shader_1_lines_geom;
    delete shader_1_triangles_geom;
    delete shader_1_frag;
    delete shader_2_vert;
    delete shader_2_frag;
    delete shaderProgram_lines;
    delete shaderProgram_triangles;
    delete shaderProgram_overlay;
    delete texture_cube1;
    delete texture_cube2;
    delete texture_cube3;
    delete texture_cube4;
    delete texture_cube5;
    delete texture_cube6;
    qCDebug(glwidget) << "GLWidget destroyed";
}

QPointF GLWidget::mapFromScene(QVector3D &scenePoint)
{
    qreal x;
    qreal y;

//    QVector4D row0 = matrix_all.row(0);
//    QVector4D row1 = matrix_all.row(1);

//    x = row0.x() * scenePoint.x() + row0.y() * scenePoint.y() + row0.z() * scenePoint.z() + row0.w();
//    y = row1.x() * scenePoint.x() + row1.y() * scenePoint.y() + row1.z() * scenePoint.z() + row1.w();

    // This works, the version above doesn't work for perspective rendering... Why???
    QVector4D out = this->matrix_all * scenePoint;
    x = out.x();
    y = out.y();

    return QPointF(x / 2.0 * this->width(), y / 2.0 * this->height());
}

void GLWidget::updateMatrixAll()
{
    QMatrix4x4 perspective_projection = getMatrix_perspective_projection();

    matrix_all = matrix_projection * matrix_glSelect * matrix_modelview * perspective_projection * matrix_rotation;
}

void GLWidget::moveCursor(QPoint pos)
{
    this->mousePos = pos;
    this->cursorShown = true;
    slot_repaint();
}

void GLWidget::hideCursor()
{
    this->cursorShown = false;
    slot_repaint();
}

void GLWidget::pickStart()
{
    this->pickActive = true;
    this->pickStartPos = this->mousePos;
}

void GLWidget::pickEnd()
{
    this->pickActive = false;
    slot_repaint();
}

bool GLWidget::isPickActive()
{
    return this->pickActive;
}

QRect GLWidget::selection()
{
    // Selection rect must be from topleft to bottomright

    QPoint topLeft;
    topLeft.setX(qMin(this->pickStartPos.x(), this->mousePos.x()));
    topLeft.setY(qMin(this->pickStartPos.y(), this->mousePos.y()));

    QPoint bottomRight;
    bottomRight.setX(qMax(this->pickStartPos.x(), this->mousePos.x()));
    bottomRight.setY(qMax(this->pickStartPos.y(), this->mousePos.y()));

    return QRect(topLeft, bottomRight);
}

Qt::ItemSelectionMode GLWidget::selectionMode()
{
    if (this->mousePos.x() - this->pickStartPos.x() > 0)
        return Qt::ContainsItemShape;
    else
        return Qt::IntersectsItemShape;
}

void GLWidget::snap_enable(bool on)
{
    Q_UNUSED(on);
}

void GLWidget::set_snap_mode(SnapMode mode)
{
    this->snapMode = mode;
}

void GLWidget::set_snapPos(QVector3D snapPos)
{
    this->snapPos_screen = this->mapFromScene(snapPos).toPoint();
    this->snapPos_scene = snapPos;
}

void GLWidget::set_WorldRotation(float rot_x, float rot_y, float rot_z)
{
    matrix_rotation.setToIdentity();
    matrix_rotation.rotate(rot_x, 1.0, 0.0, 0.0);
    matrix_rotation.rotate(rot_y, 0.0, 1.0, 0.0);
    matrix_rotation.rotate(rot_z, 0.0, 0.0, 1.0);
    updateMatrixAll();
    slot_repaint();
}

void GLWidget::snap_calculation(bool set_snapMode, bool set_snapPos, bool emit_snapFired)
{
    if (item_lastHighlight != NULL)
    {
        // Basepoint snap
        QList<QVector3D> snap_basepoints;
        if ((mapFromScene(item_lastHighlight->snap_basepoint) - mousePos).manhattanLength() < (_snapIndicatorSize * 2))
            snap_basepoints.append(item_lastHighlight->snap_basepoint);

        // Flange snap
        QList<QVector3D> snap_flanges;
        foreach(QVector3D snap_flange, item_lastHighlight->snap_flanges)
        {
            if ((mapFromScene(snap_flange) - mousePos).manhattanLength() < _snapIndicatorSize)
                snap_flanges.append(snap_flange);
        }

        // Endpoint / Vertex snap
        QList<QVector3D> snap_vertex_points;
        foreach (QVector3D snap_vertex, item_lastHighlight->snap_vertices)
        {
            if ((mapFromScene(snap_vertex) - mousePos).manhattanLength() < _snapIndicatorSize)
                snap_vertex_points.append(snap_vertex);
        }

        // Center Snap
        QList<QVector3D> snap_center_points;
        foreach (QVector3D snap_center, item_lastHighlight->snap_center)
        {
            if ((mapFromScene(snap_center) - mousePos).manhattanLength() < _snapIndicatorSize)
                snap_center_points.append(snap_center);
        }

        if (!snap_flanges.isEmpty())
        {
            if (set_snapMode)
                this->set_snap_mode(GLWidget::SnapFlange);
            if (set_snapPos)
                this->set_snapPos(snap_flanges.at(0));
        }
        else if (!snap_basepoints.isEmpty())
        {
            if (set_snapMode)
                this->set_snap_mode(GLWidget::SnapBasepoint);
            if (set_snapPos)
                this->set_snapPos(snap_basepoints.at(0));
        }
        else if (!snap_vertex_points.isEmpty())
        {
            if (set_snapMode)
                this->set_snap_mode(GLWidget::SnapEndpoint);
            if (set_snapPos)
                this->set_snapPos(snap_vertex_points.at(0));
        }
        else if (!snap_center_points.isEmpty())
        {
            if (set_snapMode)
                this->set_snap_mode(GLWidget::SnapCenter);
            if (set_snapPos)
                this->set_snapPos(snap_center_points.at(0));
        }
        else
        {
            if (set_snapMode)
                this->set_snap_mode(GLWidget::SnapNo);
        }

        if (emit_snapFired)
            emit signal_snapFired(this->snapPos_scene, this->snapMode);
    }
    else
    {
        if (set_snapMode)
            this->set_snap_mode(GLWidget::SnapNo);
        if (emit_snapFired)
            emit signal_snapFired(this->snapPos_scene, this->snapMode);
    }
}

QMatrix4x4 GLWidget::getMatrix_all()
{
    return this->matrix_all;
}

QMatrix4x4 GLWidget::getMatrix_projection()
{
    return this->matrix_projection;
}

QMatrix4x4 GLWidget::getMatrix_glSelect()
{
    return this->matrix_glSelect;
}

QMatrix4x4 GLWidget::getMatrix_modelview()
{
    return this->matrix_modelview;
}

QMatrix4x4 GLWidget::getMatrix_rotation()
{
    return this->matrix_rotation;
}

QMatrix4x4 GLWidget::getMatrix_perspective_projection()
{
    QMatrix4x4 perspective_projection;
    if (this->render_perspective)
    {
        perspective_projection.perspective(90.0, 1.0, 0.9, 0.01);
        perspective_projection.translate(0.0, 0.0, -0.5);   // Position of camera; tbd: make this adjustable by a menu
        perspective_projection.scale(1.0, 1.0, 1.0 / 1000000.0);
    }
    else
        perspective_projection.scale(1.0, 1.0, 1.0 / 200000.0);

    return perspective_projection;
}

ItemGripModifier* GLWidget::getItemGripModifier()
{
    return this->itemGripModifier;
}

void GLWidget::setMatrices(QMatrix4x4 matrix_projection, QMatrix4x4 matrix_glSelect, QMatrix4x4 matrix_modelview, QMatrix4x4 matrix_rotation)
{
    this->matrix_projection = matrix_projection;
    this->matrix_glSelect = matrix_glSelect;
    this->matrix_modelview = matrix_modelview;
    this->zoomFactor = matrix_modelview.column(0).x();
    this->translationOffset.setX(matrix_modelview.column(3).x());
    this->translationOffset.setY(matrix_modelview.column(3).y());
    this->matrix_rotation = matrix_rotation;
// Why is that needed?
    QSize sizeShadow = this->size();
    this->resize(500, 500);
    this->resize(sizeShadow);
    this->updateMatrixAll();
    this->slot_repaint();
}

void GLWidget::setMatrices(QMatrix4x4 matrix_modelview, QMatrix4x4 matrix_rotation)
{
    this->matrix_modelview = matrix_modelview;
    this->zoomFactor = matrix_modelview.column(0).x();
    this->translationOffset.setX(matrix_modelview.column(3).x());
    this->translationOffset.setY(matrix_modelview.column(3).y());
    this->matrix_rotation = matrix_rotation;

    this->updateMatrixAll();
    this->slot_repaint();
}

void GLWidget::setLookAt(QVector3D lookAt)
{
    this->lookAtPosition = lookAt;
}

void GLWidget::render_image(QPainter* painter, int x, int y, int size_x, int size_y, QMatrix4x4 matrix_modelview, QMatrix4x4 matrix_rotation, bool showTiles)
{
    QMatrix4x4 matrix_tile;

    // Check if image size can be rendered in framebuffer or if tile rendering is needed
    int tile_pos_x;
    int tile_pos_y;
    int tile_size_x = this->width();
    int tile_size_y = this->height();
    int num_tiles_x = size_x / tile_size_x + 1;
    int num_tiles_y = size_y / tile_size_y + 1;

    qreal zoom = qMin((qreal)size_x / (qreal)this->width(), (qreal)size_y / (qreal)this->height());


    // Set cutting planes to requested state for image rendering - tbd.
//    this->height_of_intersection.setZ(height);
//    this->depth_of_view.setZ(depth);

    // Render it **************************************************************

    painter->save();
    painter->setClipRect(x, y, size_x, size_y);

    for (int current_tile_x = 0; current_tile_x < num_tiles_x; current_tile_x++)
    {
        for (int current_tile_y = 0; current_tile_y < num_tiles_y; current_tile_y++)
        {
            tile_pos_x = tile_size_x * current_tile_x;
            tile_pos_y = tile_size_y * current_tile_y;


            // Set matrix to requested state for tile image rendering
            matrix_tile.setToIdentity();
            matrix_tile.translate((-(qreal)current_tile_x) * (qreal)tile_size_x / zoom, ((qreal)current_tile_y + 1.0) * (qreal)tile_size_y / zoom, 0);

            QMatrix4x4 matrix_projection_tile;
            matrix_projection_tile.setToIdentity();
            matrix_projection_tile.translate(-1.0, -1.0);
            matrix_projection_tile.scale(zoom, zoom, 1.0);
            matrix_projection_tile.translate(+1.0, -1.0);
            matrix_projection_tile.scale(2.0 / (qreal)this->width(), 2.0 / (qreal)this->height(), 1.0);

//            this->matrix_all = matrix_projection_tile * matrix_tile * matrix_modelview * matrix_rotation;
            QMatrix4x4 perspective_projection = getMatrix_perspective_projection();
            this->matrix_all = matrix_projection_tile * matrix_tile * matrix_modelview * perspective_projection * matrix_rotation;

            makeCurrent();

            if (fbo_renderImage->size() != QSize(tile_size_x, tile_size_y))
            {
                QOpenGLFramebufferObjectFormat format = fbo_renderImage->format();
                delete fbo_renderImage;
                fbo_renderImage = new QOpenGLFramebufferObject(tile_size_x, tile_size_y, format);
            }

            fbo_renderImage->bind();

            glDepthFunc(GL_LEQUAL);
            glDepthRange(1,0);
            glDisable(GL_BLEND);
            glEnable(GL_MULTISAMPLE);
            glDisable(GL_CULL_FACE);
            glEnable(GL_DEPTH_TEST);
            glDisable(GL_ALPHA_TEST);
            glClearColor(1.0, 1.0, 1.0, 1.0);   // white background
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            LayerList l = LayerList();
            l.append(itemDB->getRootLayer());
            paintContent(l);
            QImage image_tile = fbo_renderImage->toImage(true);

            painter->drawImage(x + tile_pos_x, y + tile_pos_y, image_tile);
            fbo_renderImage->release();

            doneCurrent();
        }
    }

    if (showTiles)
    {
        painter->setPen(Qt::blue);
        for (int current_tile_x = 0; current_tile_x < num_tiles_x; current_tile_x++)
        {
            for (int current_tile_y = 0; current_tile_y < num_tiles_y; current_tile_y++)
            {
                tile_pos_x = tile_size_x * current_tile_x;
                tile_pos_y = tile_size_y * current_tile_y;


                painter->drawRect(x + tile_pos_x, y + tile_pos_y, tile_size_x, tile_size_y);
            }
        }
    }

    // Rendering done *********************************************************

    // Change cutting planes back to previous state - tbd.
//    this->height_of_intersection.setZ(height);
//    this->depth_of_view.setZ(depth);

    // Change painter back to previous state
    painter->restore();

    // Change matrix back back to previous state
    this->updateMatrixAll();
}

void GLWidget::setAspectRatio(qreal ratio)
{
    this->aspectRatio = ratio;
}

QStringList GLWidget::getOpenGLinfo()
{
    makeCurrent();

    // get OpenGL info
    QStringList ret;
    ret << tr("Vendor") << QString((const char*)glGetString(GL_VENDOR));
    ret << "Renderer" << QString((const char*)glGetString(GL_RENDERER));
    ret << "Version" << QString((const char*)glGetString(GL_VERSION));
    ret << "GLSL Version" << QString((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    return ret;
}

void GLWidget::slot_highlightItem(CADitem *item)
{
    if (this->item_lastHighlight != item)
    {
        this->item_lastHighlight = item;
        slot_repaint();
    }
}

void GLWidget::slot_snapTo(QVector3D snapPos_scene, int snapMode)
{
    bool repaint = false;

    if (this->snapMode != snapMode)
    {
        this->snapMode = (SnapMode)snapMode;
        repaint = true;
    }

    if (this->snapPos_scene != snapPos_scene)
    {
        this->set_snapPos(snapPos_scene);
        repaint = true;
    }

    if (repaint)
    {
        slot_repaint();
    }
}

void GLWidget::slot_changeSelection(QList<CADitem *> selectedItems)
{
    this->selection_itemList = selectedItems;
//    emit signal_selectionChanged(this->selection_itemList);
    slot_repaint();
}

void GLWidget::slot_itemDeleted(CADitem *item)
{
    if (item == item_lastHighlight)
    {
        item_lastHighlight = NULL;
        snapMode = SnapNo;
    }
}

void GLWidget::slot_mouse3Dmoved(int x, int y, int z, int a, int b, int c)
{
    Q_UNUSED(a)
    Q_UNUSED(b)
    Q_UNUSED(c)
    if (!cursorShown)
        return;

    // move
    translationOffset += QPoint(x/2, y/2);

    // zoom
    qreal zoomStep = 0.10;
    zoomStep = -(z * zoomStep / 8.0 / 20.0);
    if ((zoomFactor + zoomStep) <= 0)
    {
        zoomFactor += zoomStep / 100.0;
        if (zoomFactor < 0.0) zoomFactor = 0.0;
    }
    else
        zoomFactor += zoomStep;

    // rot tbd.
//    rot_x += -((float)a / 15.0);
//    rot_y += -((float)b / 15.0);
//    rot_z += -((float)c / 15.0);

    updateMatrixAll();
    emit signal_matrix_rotation_changed(matrix_rotation);
    slot_repaint();
}

void GLWidget::slot_update_settings()
{
    _backgroundColor = settings.value("Design_Colors_backgroundColor", QVariant::fromValue(QColor().black())).value<QColor>();

    _cursorSize = settings.value("Userinterface_Cursor_cursorSize", QVariant::fromValue(4500)).toInt();
    _cursorWidth = settings.value("Userinterface_Cursor_cursorLineWidth", QVariant::fromValue(1)).toInt();
    _cursorPickboxSize = settings.value("Userinterface_Cursor_cursorPickboxSize", QVariant::fromValue(11)).toInt() * 2 + 1;
    _cursorPickboxLineWidth = settings.value("Userinterface_Cursor_cursorPickboxLineWidth", QVariant::fromValue(1)).toInt();
    _cursorPickboxColor = settings.value("Userinterface_Cursor_cursorPickboxColor", QVariant::fromValue(QColor(200, 255, 200, 150))).value<QColor>();

    _snapIndicatorSize = settings.value("Userinterface_Snap_snapIndicatorSize", QVariant::fromValue(21)).toInt();

    _pickboxOutlineWidth = settings.value("Userinterface_pickbox_pickboxOutlineWidth", QVariant::fromValue(1)).toInt();
    _pickboxOutlineColorLeft = settings.value("Userinterface_pickbox_pickboxOutlineColorLeft", QVariant::fromValue(QColor(40, 255, 40, 255))).value<QColor>();
    _pickboxOutlineColorRight = settings.value("Userinterface_pickbox_pickboxOutlineColorRight", QVariant::fromValue(QColor(40, 40, 255, 255))).value<QColor>();
    _pickboxFillColorLeft = settings.value("Userinterface_pickbox_pickboxFillColorLeft", QVariant::fromValue(QColor(127, 255, 127, 127))).value<QColor>();
    _pickboxFillColorRight = settings.value("Userinterface_pickbox_pickboxFillColorRight", QVariant::fromValue(QColor(127, 127, 255, 127))).value<QColor>();

    slot_repaint();
}

void GLWidget::wheelEvent(QWheelEvent* event)
{
    qreal zoomStep = 1.15;

    int steps = abs(event->delta() / 8 / 15);

    // Scale the view
    if(event->delta() > 0)
    {

    }
    else
    {
        zoomStep = 1.0 / zoomStep;
    }

    zoomStep = qPow(zoomStep, steps);
    zoomFactor *= zoomStep;

    translationOffset += (mousePos - translationOffset) * (1.0 - zoomStep);

    event->accept();

    slot_repaint();
}

void GLWidget::mouseMoveEvent(QMouseEvent *event)
{
    //qCDebug(glwidget) << "mouseMove";

    mousePos = event->pos();
    mousePos.setY((this->height() - 1) - mousePos.y());
    mousePos -= QPoint(this->width() / 2, this->height() / 2);
    QPoint mouseMoveDelta = mousePos - mousePosOld;
    mousePosOld = mousePos;

    if (event->buttons() == Qt::LeftButton)
    {

    }

    if (event->buttons() == Qt::MidButton)
    {
        translationOffset += mouseMoveDelta;
        if (mouseMoveDelta.manhattanLength() > 20)  // Do not zoom_pan_all if double clicked and mouse was moved
        {
            QDateTime veryLongAgo = QDateTime::fromMSecsSinceEpoch(0);
            mouse_lastMidPress_dateTime = veryLongAgo;
        }
    }

    if (event->buttons() == Qt::RightButton)
    {
        QVector3D rotationEnd = pointOnSphere( mousePos );
        double angle = acos( QVector3D::dotProduct( rotationStart, rotationEnd ));
        QVector4D axis4D = matrix_rotation_old.transposed() * QVector4D(QVector3D::crossProduct(rotationStart, rotationEnd), 0.0);

        matrix_arcball.setToIdentity();
        matrix_arcball.translate(this->lookAtPosition);
        matrix_arcball.rotate(angle/PI*180,axis4D.toVector3D());
        matrix_arcball.translate(-1.0 * this->lookAtPosition);

        matrix_rotation = matrix_rotation_old * matrix_arcball;

        updateMatrixAll();
        emit signal_matrix_rotation_changed(matrix_rotation);
    }

    if ((event->buttons() == 0) && (this->pickActive == false))
    {
        // Item highlighting
        highlightClear();
        highlightItemAtPosition(mousePos);

        // Object Snap
        this->snap_calculation(true, true, true);
    }
    else
    {
        this->set_snap_mode(GLWidget::SnapNo);
    }

    this->cursorShown = true;

//    emit signal_mouseMoved(QVector3D(mousePos.x(), mousePos.y(), 0));

    slot_repaint();
    event->accept();
}



QVector3D GLWidget::pointOnSphere(QPoint pointOnScreen)
{
    QPoint lookAtScreenCoords = mapFromScene(lookAtPosition).toPoint();
    double x = pointOnScreen.x();
    double y = pointOnScreen.y();
    double center_x = lookAtScreenCoords.x();
    double center_y = lookAtScreenCoords.y();
    QVector3D v;
    v.setX((x - center_x) / arcballRadius);
    v.setY((y - center_y) / arcballRadius);
    double r = v.x() * v.x() + v.y() * v.y();
    if (r > 1.0d)
    {
        v.normalize();
    }
    else
    {
        v.setZ( sqrt(1.0d - r) );
    }
    return v;
}

void GLWidget::enterEvent(QEvent *event)
{
    this->setFocus();
    this->activateWindow();
    this->setCursor(Qt::BlankCursor);
    this->cursorShown = true;
    event->accept();
}

void GLWidget::leaveEvent(QEvent *event)
{
    this->hideCursor();

    event->accept();
}

void GLWidget::mousePressEvent(QMouseEvent *event)
{
    this->setFocus();

    if (event->buttons() == Qt::MidButton)
    {
        QDateTime now = QDateTime::currentDateTime();
        quint64 msecs_sinceLastMidPress = this->mouse_lastMidPress_dateTime.msecsTo(now);
        this->mouse_lastMidPress_dateTime = now;

        if (msecs_sinceLastMidPress < 500)
        {
            this->zoom_pan_showAll();
        }
        else
            this->setCursor(Qt::OpenHandCursor);
    }
    else
        this->setCursor(Qt::BlankCursor);


    if (event->buttons() == Qt::RightButton)
    {
        arcballPosOld = mousePos;
        rotationStart = pointOnSphere( mousePos );
        matrix_rotation_old = matrix_rotation;
        this->arcballShown = true;
        slot_repaint();
    }

    // Object drawing and manipulation
    if (event->buttons() == Qt::LeftButton)
    {
        if (event->modifiers() & Qt::ControlModifier)
        {
            if (this->snapMode != SnapNo)
            {
                this->lookAtPosition = this->snapPos_scene;
            }
            event->accept();
            return;
        }

        if (this->itemGripModifier == NULL)
        {
            event->accept();
            return;
        }

        // Check if there is a currently active command
        if (this->itemGripModifier->getActiveGrip() == ItemGripModifier::Grip_Move)
        {
            if (snapMode != SnapNo)
            {
                this->itemGripModifier->moveItemsTo(snapPos_scene);
                this->slot_repaint();
            }
        }
        else if (this->itemGripModifier->getActiveGrip() == ItemGripModifier::Grip_Copy)
        {
            if (snapMode != SnapNo)
            {
                this->itemGripModifier->copyItemsTo(snapPos_scene);
                this->slot_repaint();
            }
        }

        // Pickbox
        else if ((this->item_lastHighlight != NULL) && (!this->isPickActive()))   // There is an item beyond the cursor, so if it is clicked, select it.
        {
            if (event->modifiers() && Qt::ShiftModifier)
                selectionRemoveItem(item_lastHighlight);
            else if (snapMode == SnapFlange)
            {
                this->itemGripModifier->setItem(item_lastHighlight);
                this->itemGripModifier->activateGrip(ItemGripModifier::Grip_Append, QCursor::pos(), snapPos_scene);
            }
            else
                selectionAddItem(item_lastHighlight);
        }
        else if (!this->isPickActive())
            this->pickStart();
        else
        {
            QList<CADitem*> pickedItems = this->itemsAtPosition_v2(this->selection().center(), this->selection().width(), this->selection().height());
            if (this->selectionMode() == Qt::IntersectsItemShape)
                selectionAddItems(pickedItems);
            else if (this->selectionMode() == Qt::ContainsItemShape)
            {
                QSet<CADitem*> surroundingItems;
                surroundingItems.unite(this->itemsAtPosition_v2(((this->selection().topLeft() + this->selection().topRight()) / 2), this->selection().width(), 2).toSet());
                surroundingItems.unite(this->itemsAtPosition_v2(((this->selection().bottomLeft() + this->selection().bottomRight()) / 2), this->selection().width(), 2).toSet());
                surroundingItems.unite(this->itemsAtPosition_v2(((this->selection().topLeft() + this->selection().bottomLeft()) / 2), 2, this->selection().height()).toSet());
                surroundingItems.unite(this->itemsAtPosition_v2(((this->selection().topRight() + this->selection().bottomRight()) / 2), 2, this->selection().height()).toSet());
                selectionAddItems(pickedItems.toSet().subtract(surroundingItems).toList());
            }

            this->pickEnd();
            event->accept();
            return;
        }
    }
    event->accept();
}

void GLWidget::mouseReleaseEvent(QMouseEvent *event)
{
    this->setCursor(Qt::BlankCursor);

    if (event->button() == Qt::MidButton)
    {
        this->setCursor(Qt::BlankCursor);
        slot_repaint();
    }

    if (event->button() == Qt::RightButton)
    {
        this->arcballShown = false;
        slot_repaint();
    }

    event->accept();
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    if (this->itemGripModifier == NULL)
    {
        event->accept();
        return;
    }

    // Failure tests first

    quint64 itemsOff = 0;
    quint64 itemsLocked = 0;

    foreach (CADitem* item, this->selection_itemList)
    {
        if (!item->layer->isOn)
            itemsOff++;
        if (!item->layer->isWriteable)
            itemsLocked++;
    }

    switch (event->key())
    {
    case Qt::Key_Escape:
        break;
    case Qt::Key_A:
        break;
    case Qt::Key_C:
        if ((itemsOff > 0))
        {
            QMessageBox::critical(this, tr("Copy items"), tr("There are %1 item(s) to be copied from an inactive layer.\nCopy aborted.").arg(itemsOff));
            event->accept();
            return;
        }
        if (itemsLocked > 0)
        {
            QMessageBox::critical(this, tr("Copy items"), tr("There are %1 item(s) to be copied from a locked layer.\nCopy aborted.").arg(itemsLocked));
            event->accept();
            return;
        }
        break;
    case Qt::Key_E:
        break;
    case Qt::Key_F:
        break;
    case Qt::Key_L:
        break;
    case Qt::Key_M:
        if ((itemsOff > 0))
        {
            QMessageBox::critical(this, tr("Moving items"), tr("There are %1 item(s) to be moved on an inactive layer.\nMovement aborted.").arg(itemsOff));
            event->accept();
            return;
        }
        if (itemsLocked > 0)
        {
            QMessageBox::critical(this, tr("Moving items"), tr("There are %1 item(s) to be moved on a locked layer.\nMovement aborted.").arg(itemsLocked));
            event->accept();
            return;
        }
        break;
    case Qt::Key_R:
        if ((itemsOff > 0))
        {
            QMessageBox::critical(this, tr("Rotating items"), tr("There are %1 item(s) to be rotated on an inactive layer.\nRotation aborted.").arg(itemsOff));
            event->accept();
            return;
        }
        if (itemsLocked > 0)
        {
            QMessageBox::critical(this, tr("Rotating items"), tr("There are %1 item(s) to be rotated on a locked layer.\nRotation aborted.").arg(itemsLocked));
            event->accept();
            return;
        }
        break;
    case Qt::Key_X:
    case Qt::Key_Y:
    case Qt::Key_Z:
        if (!(event->modifiers() & Qt::ControlModifier))
        {
            if (!(item_lastHighlight == NULL))
            {
                if (!item_lastHighlight->layer->isOn)
                {
                    QMessageBox::critical(this, tr("Rotating item"), tr("The item to be rotated is on an inactive layer.\nRotation aborted."));
                    event->accept();
                    return;
                }
                if (!item_lastHighlight->layer->isWriteable)
                {
                    QMessageBox::critical(this, tr("Rotating item"), tr("The item to be rotated is on a locked layer.\nRotation aborted."));
                    event->accept();
                    return;
                }
            }
        }
        break;
    case Qt::Key_Delete:
        if ((itemsOff > 0))
        {
            QMessageBox::critical(this, tr("Deleting items"), tr("There are %1 item(s) to be deleted on an inactive layer.\nDeletion aborted.").arg(itemsOff));
            event->accept();
            return;
        }
        if (itemsLocked > 0)
        {
            QMessageBox::critical(this, tr("Deleting items"), tr("There are %1 item(s) to be deleted on a locked layer.\nDeletion aborted.").arg(itemsLocked));
            event->accept();
            return;
        }
        break;
    }

    // Actions
    switch (event->key())
    {
    case Qt::Key_Escape:
        if (this->pickActive)
        {
            this->pickEnd();
            break;
        }
        if (this->selection_itemList.count() > 0)
        {
            this->selectionClear();
        }
        if (this->itemGripModifier->getActiveGrip() != ItemGripModifier::Grip_None)
        {
            this->itemGripModifier->finishGrip();
            this->slot_repaint();
        }
        break;
    case Qt::Key_A:                         // Render a test image
    {
        QImage testImage(this->width() * 2, this->height() * 2, QImage::Format_ARGB32_Premultiplied);
        QPainter painter (&testImage);
        this->render_image(&painter, 0, 0, testImage.width(), testImage.height(), this->matrix_modelview, this->matrix_rotation);
        painter.end();
        QString filename = QFileDialog::getSaveFileName(this, tr("Save captured image"), "", "PNG image file (*.png)");
        testImage.save(filename, "PNG", -1);
        break;
    }
    case Qt::Key_C:                         // Copy item
        if ((this->selection_itemList.count() > 0) && (item_lastHighlight != NULL))   // more than one item
        {
            if (snapMode != SnapNo)
            {
                QList<CADitem*> itemsToDo = this->selection_itemList;
                if (QMessageBox::question(this, tr("Copy items"), tr("You are going to copy %1 item(s).").arg(itemsToDo.count()),
                                          tr("Abort"), tr("Proceed"), "", 1, 0)
                        == 1)
                {
                    this->itemGripModifier->setItems(itemsToDo);
                    if (event->modifiers() & Qt::ShiftModifier)
                        this->itemGripModifier->activateGrip(ItemGripModifier::Grip_CopyMulti, QCursor::pos(), snapPos_scene);
                    else
                        this->itemGripModifier->activateGrip(ItemGripModifier::Grip_Copy, QCursor::pos(), snapPos_scene);
                    this->slot_repaint();
                }
            }
        }
        else if (item_lastHighlight != NULL)        // only one item
        {
            if (snapMode != SnapNo)
            {
                this->itemGripModifier->setItem(item_lastHighlight);
                if (event->modifiers() & Qt::ShiftModifier)
                    this->itemGripModifier->activateGrip(ItemGripModifier::Grip_CopyMulti, QCursor::pos(), snapPos_scene);
                else
                    this->itemGripModifier->activateGrip(ItemGripModifier::Grip_Copy, QCursor::pos(), snapPos_scene);
                this->slot_repaint();
            }
        }
        break;
    case Qt::Key_E:                         // Edit item
        if (item_lastHighlight != NULL)
        {
            if (this->itemWizard != NULL)
            {
                this->itemWizard->showWizard(item_lastHighlight, itemDB);
            }
        }
        break;
    case Qt::Key_F:                         // Turn item around flange axis
        if (item_lastHighlight != NULL)
        {
            if (snapMode == SnapFlange)
            {
                int flangeIndex = item_lastHighlight->snap_flanges.indexOf(snapPos_scene) + 1;
                WizardParams newParams;
                qreal angle = 45.0;
                if (event->modifiers() & Qt::ShiftModifier)     // Hold shift to enter numerical value for angle
                    angle = QInputDialog::getDouble(this, tr("Rotate flanged item around flange"), tr("Angle [degrees]"), 45.0, -360.0, 360.0, 3);
                newParams = item_lastHighlight->rotateAroundFlange(snapPos_scene, flangeIndex, angle);
                itemDB->setRestorePoint();
                itemDB->modifyItem_withRestorePoint(item_lastHighlight, newParams);

                slot_repaint();
            }
        }
        break;
    case Qt::Key_L:                         // Change length of item
        if (item_lastHighlight != NULL)
        {
            this->itemGripModifier->setItem(item_lastHighlight);
            this->itemGripModifier->activateGrip(ItemGripModifier::Grip_Length, QCursor::pos(), snapPos_scene);
        }
        break;
    case Qt::Key_M:                         // Move item
        if ((this->selection_itemList.count() > 0) && (item_lastHighlight != NULL))   // more than one item
        {
            if (snapMode != SnapNo)
            {
                QList<CADitem*> itemsToDo = this->selection_itemList;
                if (QMessageBox::question(this, tr("Moving items"), tr("You are going to move %1 item(s).").arg(itemsToDo.count()),
                                          tr("Abort"), tr("Proceed"), "", 1, 0)
                        == 1)
                {
                    this->itemGripModifier->setItems(itemsToDo);
                    this->itemGripModifier->activateGrip(ItemGripModifier::Grip_Move, QCursor::pos(), snapPos_scene);
                    this->slot_repaint();
                }
            }
        }
        else if (item_lastHighlight != NULL)        // only one item
        {
            if (snapMode != SnapNo)
            {
                this->itemGripModifier->setItem(item_lastHighlight);
                this->itemGripModifier->activateGrip(ItemGripModifier::Grip_Move, QCursor::pos(), snapPos_scene);
                this->slot_repaint();
            }
        }
        break;
    case Qt::Key_R:
        if ((this->selection_itemList.count() > 0) && (item_lastHighlight != NULL))
        {
            if (snapMode != SnapNo)
            {
                QList<CADitem*> itemsToDo = this->selection_itemList;
                if (QMessageBox::question(this, tr("Rotating items"),
                                          tr("You are going to rotate %1 item(s).").arg(itemsToDo.count()),
                                          tr("Abort"), tr("Proceed"), "", 1, 0)
                        == 1)
                {
                    this->itemGripModifier->setItems(itemsToDo);
                    this->itemGripModifier->activateGrip(ItemGripModifier::Grip_Rotate_aroundPoint, QCursor::pos(), snapPos_scene);
                    this->slot_repaint();
                }
            }
        }
        break;
    case Qt::Key_X:
        if (event->modifiers() & Qt::ControlModifier)   // Ctrl + X (Reserved command)
        {
            ;
        }
        else if (item_lastHighlight != NULL)  // Turn item around x axis
        {
            QVector3D angles;
            QMatrix4x4 matrix_old = item_lastHighlight->matrix_rotation;
            QMatrix4x4 m;
            m.setToIdentity();
            if (event->modifiers() & Qt::ShiftModifier)
                m.rotate(-45.0, 1.0, 0.0, 0.0);
            else
                m.rotate(45.0, 1.0, 0.0, 0.0);
            angles = MAngleCalculations().anglesFromMatrix(m * matrix_old);
            WizardParams newParams;
            newParams.insert("Angle x", (angles.x()));
            newParams.insert("Angle y", (angles.y()));
            newParams.insert("Angle z", (angles.z()));
            itemDB->setRestorePoint();
            itemDB->modifyItem_withRestorePoint(item_lastHighlight, newParams);
            slot_repaint();
        }
        break;
    case Qt::Key_Y:
        if (event->modifiers() & Qt::ControlModifier)   // Ctrl + Y (Redo)
        {
            this->itemDB->restore_redo();
        }
        else if (item_lastHighlight != NULL)    // Turn item around y axis
        {
            QMatrix4x4 matrix_old = item_lastHighlight->matrix_rotation;
            QMatrix4x4 m;
            m.setToIdentity();
            if (event->modifiers() & Qt::ShiftModifier)
                m.rotate(-45.0, 0.0, 1.0, 0.0);
            else
                m.rotate(45.0, 0.0, 1.0, 0.0);
            QVector3D angles = MAngleCalculations().anglesFromMatrix(m * matrix_old);
            WizardParams newParams;
            newParams.insert("Angle x", (angles.x()));
            newParams.insert("Angle y", (angles.y()));
            newParams.insert("Angle z", (angles.z()));
            itemDB->setRestorePoint();
            itemDB->modifyItem_withRestorePoint(item_lastHighlight, newParams);
            slot_repaint();
        }
        break;
    case Qt::Key_Z:
        if (event->modifiers() & Qt::ControlModifier)   // Ctrl + Z (Undo)
        {
            this->itemDB->restore_undo();
        }
        else if (item_lastHighlight!= NULL)         // Turn item around z axis
        {
            QMatrix4x4 matrix_old = item_lastHighlight->matrix_rotation;
            QMatrix4x4 m;
            m.setToIdentity();
            if (event->modifiers() & Qt::ShiftModifier)
                m.rotate(-45.0, 0.0, 0.0, 1.0);
            else
                m.rotate(45.0, 0.0, 0.0, 1.0);
            QVector3D angles = MAngleCalculations().anglesFromMatrix(m * matrix_old);
            WizardParams newParams;
            newParams.insert("Angle x", (angles.x()));
            newParams.insert("Angle y", (angles.y()));
            newParams.insert("Angle z", (angles.z()));
            itemDB->setRestorePoint();
            itemDB->modifyItem_withRestorePoint(item_lastHighlight, newParams);
            slot_repaint();
        }
        break;
    case Qt::Key_Delete:
        if (this->selection_itemList.count() > 0)
        {
            QList<CADitem*> itemsToDelete = this->selection_itemList;

//            quint64 itemsOff = 0;
//            quint64 itemsLocked = 0;

//            foreach (CADitem* item, itemsToDelete)
//            {
//                if (!item->layer->on)
//                    itemsOff++;
//                if (!item->layer->writable)
//                    itemsLocked++;
//            }

//            if ((itemsOff > 0))
//            {
//                QMessageBox::critical(this, tr("Deleting items"), tr("There are %1 items to be deleted on an inactive layer.\nDeletion aborted.").arg(itemsOff));
//                event->accept();
//                return;
//            }
//            if (itemsLocked > 0)
//            {
//                QMessageBox::critical(this, tr("Deleting items"), tr("There are %1 items to be deleted on an locked layer.\nDeletion aborted.").arg(itemsLocked));
//                event->accept();
//                return;
//            }

            if (QMessageBox::question(this, tr("Deleting items"), tr("You are going to delete %1 item(s).").arg(itemsToDelete.count()),
                                      tr("Abort"), tr("Proceed"), "", 1, 0)
                    == 1)
            {
                selectionClear();
                itemDB->setRestorePoint();
                itemDB->deleteItems_withRestorePoint(itemsToDelete);
                slot_repaint();
            }
        }
        break;
    }

    event->accept();
}

void GLWidget::slot_set_cuttingplane_values_changed(qreal height, qreal depth)
{
    qCDebug(glwidget) << "GLWidget::slot_set_cuttingplane_values_changed";
    this->height_of_intersection.setZ(height);
    this->depth_of_view.setZ(depth);

    slot_repaint();
}

void GLWidget::paintGL()
{
    if (this->size().isNull())
        return;

    if (openGLTimerQuery->isResultAvailable())
    {
        rendertime_ns_per_frame = (quint64)openGLTimerQuery->waitForResult();
//        qCDebug(glwidget) << "rendertime [ns]:" << rendertime_ns_per_frame << "FPS:" << 1e9 / rendertime_ns_per_frame << "@" << QCursor::pos();
    }
    else
//        qCDebug(glwidget) << "render without time" << "@" << QCursor::pos();

        openGLTimerQuery->begin();

    matrix_modelview.setToIdentity();
    matrix_modelview.translate(translationOffset.x(), translationOffset.y(), 0.0);
    matrix_modelview.scale(this->zoomFactor, this->zoomFactor, 1.0);
    updateMatrixAll();

    glClearColor(_backgroundColor.redF(), _backgroundColor.greenF(), _backgroundColor.blueF(), _backgroundColor.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_CULL_FACE);
    glDepthFunc(GL_LEQUAL);
    glDepthRange(1,0);
    glPolygonOffset(0.0, 3.0);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_ALPHA_TEST);

    LayerList l =  LayerList();
    l.append(itemDB->getRootLayer());
    paintContent(l);   // After this: TRIANGLE SHADER IS ACTIVE!
    // Overlay
    shaderProgram = shaderProgram_overlay;
    shaderProgram->bind();

    glClear(GL_DEPTH_BUFFER_BIT);
    glDepthRange(1,0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set a matrix to the shader that does not rotate or scale, just transform to screen coordinate system
    QMatrix4x4 matrix_coordinateBoxScale;
    matrix_coordinateBoxScale.translate(90 - this->width() / 2, 90 - this->height() / 2);
    matrix_coordinateBoxScale.scale(40.0, 40.0, 0.1);
    QMatrix4x4 matrix_rotationOnly = matrix_rotation;
    matrix_rotationOnly.setColumn(3, QVector4D(0.0, 0.0, 0.0, 1.0));
    shaderProgram->setUniformValue(shader_matrixLocation,matrix_projection * matrix_coordinateBoxScale * matrix_rotationOnly);
    glEnable(GL_DEPTH_TEST);


    // Coordinate orientation display
    setUseTexture(true);

    // Bottom face
    texture_cube1->bind();
    setPaintingColor(QColor(50, 50, 255));
    glBegin(GL_QUADS);
    setTextureCoords(1.0, 1.0, 0.0);
    glVertex3f(-1, -1, -1);
    setTextureCoords(0.0, 1.0, 0.0);
    glVertex3f( 1, -1, -1);
    setTextureCoords(0.0, 0.0, 0.0);
    glVertex3f( 1,  1, -1);
    setTextureCoords(1.0, 0.0, 0.0);
    glVertex3f(-1,  1, -1);
    glEnd();
    texture_cube1->release();


    // Top face
    texture_cube2->bind();
    glBegin(GL_QUADS);
    setTextureCoords(1.0, 1.0, 0.0);
    glVertex3f( 1, -1, 1);
    setTextureCoords(0.0, 1.0, 0.0);
    glVertex3f(-1, -1, 1);
    setTextureCoords(0.0, 0.0, 0.0);
    glVertex3f(-1,  1, 1);
    setTextureCoords(1.0, 0.0, 0.0);
    glVertex3f( 1,  1, 1);
    glEnd();
    texture_cube2->release();


    // Front face
    texture_cube3->bind();
    setPaintingColor(QColor(10, 110, 10));
    glBegin(GL_QUADS);
    setTextureCoords(1.0, 1.0, 0.0);
    glVertex3i(-1,  1, -1);
    setTextureCoords(0.0, 1.0, 0.0);
    glVertex3i( 1,  1, -1);
    setTextureCoords(0.0, 0.0, 0.0);
    glVertex3i( 1,  1,  1);
    setTextureCoords(1.0, 0.0, 0.0);
    glVertex3i(-1,  1,  1);
    glEnd();
    texture_cube3->release();


    // Back face
    texture_cube4->bind();
    glBegin(GL_QUADS);
    setTextureCoords(0.0, 1.0, 0.0);
    glVertex3i(-1, -1, -1);
    setTextureCoords(1.0, 1.0, 0.0);
    glVertex3i( 1, -1, -1);
    setTextureCoords(1.0, 0.0, 0.0);
    glVertex3i( 1, -1,  1);
    setTextureCoords(0.0, 0.0, 0.0);
    glVertex3i(-1, -1,  1);
    glEnd();
    texture_cube4->release();


    // Left face
    texture_cube5->bind();
    setPaintingColor(QColor(150, 0, 0));
    glBegin(GL_QUADS);
    setTextureCoords(1.0, 1.0, 0.0);
    glVertex3i(-1, -1, -1);
    setTextureCoords(0.0, 1.0, 0.0);
    glVertex3i(-1,  1, -1);
    setTextureCoords(0.0, 0.0, 0.0);
    glVertex3i(-1,  1,  1);
    setTextureCoords(1.0, 0.0, 0.0);
    glVertex3i(-1, -1,  1);
    glEnd();
    texture_cube5->release();


    // Right face
    texture_cube6->bind();
    glBegin(GL_QUADS);
    setTextureCoords(0.0, 1.0, 0.0);
    glVertex3i( 1, -1, -1);
    setTextureCoords(1.0, 1.0, 0.0);
    glVertex3i( 1,  1, -1);
    setTextureCoords(1.0, 0.0, 0.0);
    glVertex3i( 1,  1,  1);
    setTextureCoords(0.0, 0.0, 0.0);
    glVertex3i( 1, -1,  1);
    glEnd();
    texture_cube6->release();


    setUseTexture(false);
    glDisable(GL_DEPTH_TEST);

    QPainter painter(this);
    QPen pen;
    QBrush brush;

    // Transform QPainter Coordinate System to work with openGL coordinates, which have 0,0 at the center of the screen and non inverted y-axis.
    painter.setTransform(QTransform());
    painter.setWorldTransform(QTransform());
    painter.setWindow(-this->width() / 2 - 1, this->height() / 2 - 1, this->width(), -this->height());

    // Set a matrix to the shader that does not rotate or scale, just transform to screen coordinate system
    shaderProgram->setUniformValue(shader_matrixLocation, matrix_projection);

    if (this->cursorShown)
    {
        // Cursor lines
/*
        glLineWidth((GLfloat)_cursorWidth);
        setPaintingColor(Qt::white);
        glBegin(GL_LINES);
        glVertex3i(mousePos.x() - _cursorSize, mousePos.y(), 0);
        glVertex3i(mousePos.x() + _cursorSize, mousePos.y(), 0);
        glVertex3i(mousePos.x(), mousePos.y() - _cursorSize, 0);
        glVertex3i(mousePos.x(), mousePos.y() + _cursorSize, 0);
        glEnd();
*/
        pen.setWidth(_cursorWidth);
        pen.setColor(Qt::white);
        painter.setPen(pen);
        painter.drawLine(mousePos.x() - _cursorSize, mousePos.y(),
                         mousePos.x() + _cursorSize, mousePos.y());
        painter.drawLine(mousePos.x(), mousePos.y() - _cursorSize,
                         mousePos.x(), mousePos.y() + _cursorSize);



        // Cursor Pickbox
/*
        glLineWidth(_cursorPickboxLineWidth);
        setPaintingColor(_cursorPickboxColor);
        QRect pickRect = QRect(0, 0, _cursorPickboxSize, _cursorPickboxSize);
        pickRect.moveCenter(mousePos);
        glBegin(GL_LINE_LOOP);
        glVertex3i(pickRect.bottomLeft().x(), pickRect.bottomLeft().y(), 0);
        glVertex3i(pickRect.bottomRight().x(), pickRect.bottomRight().y(), 0);
        glVertex3i(pickRect.topRight().x(), pickRect.topRight().y(), 0);
        glVertex3i(pickRect.topLeft().x(), pickRect.topLeft().y(), 0);
        glEnd();
*/

        pen.setWidth(_cursorPickboxLineWidth);
        pen.setColor(_cursorPickboxColor);
        painter.setPen(pen);
        QRect pickRect = QRect(0, 0, _cursorPickboxSize, _cursorPickboxSize);
        pickRect.moveCenter(mousePos);
        painter.drawLine(pickRect.topLeft().x(), pickRect.topLeft().y(), pickRect.topRight().x(), pickRect.topRight().y());
        painter.drawLine(pickRect.bottomLeft().x(), pickRect.bottomLeft().y(), pickRect.bottomRight().x(), pickRect.bottomRight().y());
        painter.drawLine(pickRect.topLeft().x(), pickRect.topLeft().y(), pickRect.bottomLeft().x(), pickRect.bottomLeft().y());
        painter.drawLine(pickRect.topRight().x(), pickRect.topRight().y(), pickRect.bottomRight().x(), pickRect.bottomRight().y());


        if (this->pickActive)
        {
/*
            if (this->pickStartPos.x() < this->mousePos.x())
                setPaintingColor(_pickboxFillColorLeft);
            else
                setPaintingColor(_pickboxFillColorRight);

            glLineWidth(_pickboxOutlineWidth);

            QRect rect = this->selection();
            glBegin(GL_QUADS);
            glVertex3i(rect.bottomLeft().x(), rect.bottomLeft().y(), 0);
            glVertex3i(rect.bottomRight().x(), rect.bottomRight().y(), 0);
            glVertex3i(rect.topRight().x(), rect.topRight().y(), 0);
            glVertex3i(rect.topLeft().x(), rect.topLeft().y(), 0);
            glEnd();

            if (this->pickStartPos.x() < this->mousePos.x())
                setPaintingColor(_pickboxOutlineColorLeft);
            else
                setPaintingColor(_pickboxOutlineColorRight);
            glBegin(GL_LINE_LOOP);
            glVertex3i(rect.bottomLeft().x(), rect.bottomLeft().y(), 0);
            glVertex3i(rect.bottomRight().x(), rect.bottomRight().y(), 0);
            glVertex3i(rect.topRight().x(), rect.topRight().y(), 0);
            glVertex3i(rect.topLeft().x(), rect.topLeft().y(), 0);
            glEnd();
*/

            if (this->pickStartPos.x() < this->mousePos.x())
            {
                pen.setColor(_pickboxOutlineColorLeft);
                brush.setColor(_pickboxFillColorLeft);
                brush.setStyle(Qt::SolidPattern);
            }
            else
            {
                pen.setColor(_pickboxOutlineColorRight);
                brush.setColor(_pickboxFillColorRight);
                brush.setStyle(Qt::SolidPattern);
            }

            pen.setWidth(_pickboxOutlineWidth);
            painter.setPen(pen);
            painter.setBrush(brush);

            QRect rect = this->selection();
            painter.drawRect(rect.adjusted(0, 0, -1, -1));
        }



        // draw Arcball
        if(arcballShown)
        {
            QRect screenRect = QRect(0, 0, this->width(), this->height());
            screenRect.moveCenter(QPoint(0, 0));
            QPointF lookAtScreenCoords = mapFromScene(lookAtPosition);
/*
            setPaintingColor(QColor(255, 200, 0));
            glLineWidth(3.0);

            if (screenRect.contains(lookAtScreenCoords.toPoint()))    // Screen contains lookAtScreenCoords
            {
                glBegin(GL_LINES);
                glVertex3f(lookAtScreenCoords.x() - 15, lookAtScreenCoords.y()     , 0);
                glVertex3f(lookAtScreenCoords.x() + 15, lookAtScreenCoords.y()     , 0);
                glVertex3f(lookAtScreenCoords.x()     , lookAtScreenCoords.y() - 15, 0);
                glVertex3f(lookAtScreenCoords.x()     , lookAtScreenCoords.y() + 15, 0);
                glEnd();
            }
            else                                                        // lookAtScreenCoords is outside of screen, so show arrow to it
            {
                QVector3D center = QVector3D(0.0, 0.0, 0.0);
                QVector3D lookAt = QVector3D(lookAtScreenCoords);
                QVector3D direction = (lookAt - center).normalized() * 100.0;
                QVector3D tip = center + direction;
                QMatrix4x4 m;
                m.rotate(30.0, 0.0, 0.0, 1.0);
                QVector3D arrow1 = m * direction * 0.3;
                m.setToIdentity();
                m.rotate(-30.0, 0.0, 0.0, 1.0);
                QVector3D arrow2 = m * direction * 0.3;


                glBegin(GL_LINES);
                glVertex3f(center.x(), center.y(), 0.0);
                glVertex3f(tip.x(), tip.y(), 0.0);
                glVertex3f(tip.x(), tip.y(), 0.0);
                glVertex3f(tip.x() - arrow1.x(), tip.y() - arrow1.y(), 0.0);
                glVertex3f(tip.x(), tip.y(), 0.0);
                glVertex3f(tip.x() - arrow2.x(), tip.y() - arrow2.y(), 0.0);
                glEnd();
            }

            glLineWidth(2.0);
            glBegin(GL_LINE_LOOP);
            for(int i = 0; i < 60; i++ )
            {
                glVertex3f(arcballRadius * qSin(i * PI / 30.0) + lookAtScreenCoords.x(), arcballRadius * qCos(i * PI / 30.0) + lookAtScreenCoords.y(), 0.0);
            }
            glEnd();
*/
            pen.setColor(QColor(255, 200, 0));
            pen.setWidth(3);
            painter.setPen(pen);
            painter.setBrush(Qt::NoBrush);

            if (screenRect.contains(lookAtScreenCoords.toPoint()))    // Screen contains lookAtScreenCoords
            {
                painter.drawLine(lookAtScreenCoords.x() - 15, lookAtScreenCoords.y(), lookAtScreenCoords.x() + 15, lookAtScreenCoords.y());
                painter.drawLine(lookAtScreenCoords.x()     , lookAtScreenCoords.y() - 15, lookAtScreenCoords.x()     , lookAtScreenCoords.y() + 15);
            }
            else                                                      // lookAtScreenCoords is outside of screen, so show arrow to it
            {
                QVector3D center = QVector3D(0.0, 0.0, 0.0);
                QVector3D lookAt = QVector3D(lookAtScreenCoords);
                QVector3D direction = (lookAt - center).normalized() * 100.0;
                QVector3D tip = center + direction;
                QMatrix4x4 m;
                m.rotate(30.0, 0.0, 0.0, 1.0);
                QVector3D arrow1 = m * direction * 0.3;
                m.setToIdentity();
                m.rotate(-30.0, 0.0, 0.0, 1.0);
                QVector3D arrow2 = m * direction * 0.3;

                painter.drawLine(center.toPoint(), tip.toPoint());
                painter.drawLine(tip.toPoint(),    tip.toPoint() - arrow1.toPoint());
                painter.drawLine(tip.toPoint(),    tip.toPoint() - arrow2.toPoint());
            }

            painter.drawEllipse(lookAtScreenCoords, arcballRadius, arcballRadius);

        }
    }


//    shaderProgram = shaderProgram_overlay;
//    shaderProgram->bind();
//    shaderProgram->setUniformValue(shader_matrixLocation, matrix_projection);

    // Render snap indicators
    QString infoText;
    QRect focusRect = QRect(0, 0, _snapIndicatorSize, _snapIndicatorSize);

    if ((this->itemGripModifier != NULL) && (this->itemGripModifier->getActiveGrip() == ItemGripModifier::Grip_Move))
    {
        QString itemDescription = "[" + this->itemGripModifier->getItemDescription() + "]";
        QVector3D pos = this->itemGripModifier->getScenePosSource();
        QString itemPosition_from = QString().sprintf(" @{%.3lf|%.3lf|%.3lf}", pos.x(), pos.y(), pos.z());
        infoText = tr("Move %1%2").arg(itemDescription).arg(itemPosition_from);
        if (snapMode != SnapNo)
            infoText += " " + tr("to") + "\n";
        focusRect.moveCenter(this->mousePos);
    }
    if ((this->itemGripModifier != NULL) && (this->itemGripModifier->getActiveGrip() == ItemGripModifier::Grip_Copy))
    {
        QString itemDescription = "[" + this->itemGripModifier->getItemDescription() + "]";
        QVector3D pos = this->itemGripModifier->getScenePosSource();
        QString itemPosition_from = QString().sprintf(" @{%.3lf|%.3lf|%.3lf}", pos.x(), pos.y(), pos.z());
        infoText = tr("Copy %1%2").arg(itemDescription).arg(itemPosition_from);
        if (snapMode != SnapNo)
            infoText += " " + tr("to") + "\n";
        focusRect.moveCenter(this->mousePos);
    }

    // Inactive snap grips
    if ((item_lastHighlight != NULL) && (snapMode == SnapNo))
    {
        focusRect.setRect(0, 0, _cursorPickboxSize, _cursorPickboxSize);
        focusRect.moveCenter(mapFromScene(item_lastHighlight->snap_basepoint).toPoint());
        paintSnapIndicator(&painter, focusRect, SnapBasepoint, false);

        QList<QVector3D> snap_center = item_lastHighlight->snap_center;
        foreach (QVector3D snap, snap_center)
        {
            focusRect.moveCenter(mapFromScene(snap).toPoint());
            paintSnapIndicator(&painter, focusRect, SnapCenter, false);
        }

        QList<QVector3D> snap_flanges = item_lastHighlight->snap_flanges;
        foreach (QVector3D snap, snap_flanges)
        {
            focusRect.moveCenter(mapFromScene(snap).toPoint());
            paintSnapIndicator(&painter, focusRect, SnapFlange, false);
        }

        QList<QVector3D> snap_vertices = item_lastHighlight->snap_vertices;
        foreach (QVector3D snap, snap_vertices)
        {
            focusRect.moveCenter(mapFromScene(snap).toPoint());
            paintSnapIndicator(&painter, focusRect, SnapEndpoint, false);
        }
        focusRect.setRect(0, 0, _snapIndicatorSize, _snapIndicatorSize);
        focusRect.moveCenter(this->mousePos);
    }

    // Active Snap grips
    if ((snapMode != SnapNo) && (item_lastHighlight != NULL))
    {
        focusRect.moveCenter(this->snapPos_screen);

        paintSnapIndicator(&painter, focusRect, snapMode, true);

        switch (snapMode)
        {
        case SnapBasepoint:
            infoText.append(tr("Basepoint"));
            break;
        case SnapFlange:
        {
            infoText.append(tr("Flange") + " ");
            int flangeIndex = item_lastHighlight->snap_flanges.indexOf(snapPos_scene) + 1;
            infoText.append(QString().setNum(flangeIndex));
            break;
        }
        case SnapEndpoint:
            infoText.append(tr("Endpoint/Vertex"));
            break;
        case SnapCenter:
            infoText.append(tr("Center"));
            break;
        case SnapNo:
            break;
        }

        QString itemDescription = "[" + item_lastHighlight->description() + "]";
        QString itemPosition = QString().sprintf(" @{%.3lf|%.3lf|%.3lf}", this->snapPos_scene.x(), this->snapPos_scene.y(), this->snapPos_scene.z());
        infoText += " " + tr("of") + " " + itemDescription + itemPosition;
    }

    if (!infoText.isEmpty())
    {
        BoxVertex textAnchorType;
        QPoint textAnchorPos;
        if      (this->snapPos_screen.x() > 0.0 && this->snapPos_screen.y() > 0.0)
        {
            textAnchorPos = focusRect.topLeft();
            textAnchorType = topRight;
        }
        else if (this->snapPos_screen.x() > 0.0 && this->snapPos_screen.y() < 0.0)
        {
            textAnchorPos = focusRect.bottomLeft();
            textAnchorType = bottomRight;
        }
        else if (this->snapPos_screen.x() < 0.0 && this->snapPos_screen.y() > 0.0)
        {
            textAnchorPos = focusRect.topRight();
            textAnchorType = topLeft;
        }
        else if (this->snapPos_screen.x() < 0.0 && this->snapPos_screen.y() < 0.0)
        {
            textAnchorPos = focusRect.bottomRight();
            textAnchorType = bottomLeft;
        }
        else
        {
            textAnchorPos = focusRect.bottomRight();
            textAnchorType = bottomLeft;
        }

        paintTextInfoBox(&painter, textAnchorPos, infoText, textAnchorType);
    }

    painter.end();
    openGLTimerQuery->end();
}

void GLWidget::slot_repaint()
{
    snap_calculation(false, true, false); // Recalculate snap positions to get correct index of flange next time (due to rounding errors)
    update();
}

void GLWidget::slot_perspective(bool on)
{
    this->render_perspective = on;
    updateMatrixAll();
    slot_repaint();
}

void GLWidget::slot_wireframe(bool on)
{
    this->render_outline = on;
    slot_repaint();
}

void GLWidget::slot_solid(bool on)
{
    this->render_solid = on;
    slot_repaint();
}

void GLWidget::setVertex(QVector3D pos)
{
    shaderProgram->setAttributeValue(shader_vertexLocation, pos);
}

void GLWidget::setVertex(QPoint pos)
{
    glVertex2i((GLint)pos.x(), (GLint)pos.y());
}

void GLWidget::setPaintingColor(QColor color)
{
    if (selectItemsByColor)
    {
        color.setRed(  (glName >> 16) & 0xff);
        color.setGreen((glName >>  8) & 0xff);
        color.setBlue( (glName >>  0) & 0xff);
//        color.setRed(  ((glName >> 10) & 0xfc) | 0x01);
//        color.setGreen(((glName >>  4) & 0xfc) | 0x01);
//        color.setBlue( ((glName <<  2) & 0xfc) | 0x01);
        color.setAlpha(255);
        shaderProgram->setAttributeValue(shader_colorLocation, color.red() / 255.0f, color.green() / 255.0f, color.blue() / 255.0f);
    }
    else
    {
        QVector4D vertex_color_new = QVector4D(color.redF(), color.greenF(), color.blueF(), color.alphaF());
        vertex_color = vertex_color_new;
        shaderProgram->setAttributeValue(shader_colorLocation, vertex_color);
    }
}

void GLWidget::setTextureCoords(QPoint coord)
{
    shaderProgram->setAttributeValue(shader_textureCoordLocation, QVector4D((qreal)coord.x(), (qreal)coord.y(), 0.0, 0.0));
}

void GLWidget::setTextureCoords(qreal x, qreal y, qreal z)
{
    shaderProgram->setAttributeValue(shader_textureCoordLocation, x, y, z, 0.0);
}

void GLWidget::setUseTexture(bool on)
{
    GLint use;
    if (on)
        use = 1;
    else use = 0;
    shaderProgram->setUniformValue(shader_useTextureLocation, use);
}

void GLWidget::paintTextInfoBox(QPainter* painter, QPoint pos, QString text, BoxVertex anchor, QFont font, QColor colorText, QColor colorBackground, QColor colorOutline)
{
    // Calculate text box size
    QFontMetrics fm(font);
    int textHeight = 0;
    int textWidth = 0;
    foreach (QString line, text.split('\n'))
    {
        QRect lineRect = fm.boundingRect(line);
        textHeight += lineRect.height();
        if (lineRect.width() > textWidth)
            textWidth = lineRect.width();
    }
    QRect boundingRect;
    boundingRect.setWidth(textWidth);
    boundingRect.setHeight(textHeight);
    boundingRect.adjust(-5, -5, 5, 5);
/*
    boundingRect.moveTopLeft(QPoint(0, 0));
    // Text as texture
    QImage textImage(boundingRect.width(), boundingRect.height(), QImage::Format_ARGB32);
    textImage.fill(colorBackground);
    QPainter painter(&textImage);
    painter.setPen(colorText);
    painter.setFont(font);
    painter.drawText(boundingRect, Qt::AlignCenter, text);
    painter.end();
    switch(anchor)
    {
    case topLeft:
        boundingRect.moveBottomLeft(pos);
        break;
    case topRight:
        boundingRect.moveBottomRight(pos);
        break;
    case bottomLeft:
        boundingRect.moveTopLeft(pos);
        break;
    case bottomRight:
        boundingRect.moveTopRight(pos);
        break;
    }

    QOpenGLTexture* texture = new QOpenGLTexture(textImage, QOpenGLTexture::DontGenerateMipMaps);
    texture->setMinMagFilters(QOpenGLTexture::Nearest, QOpenGLTexture::Nearest);
    texture->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture->bind();
    setUseTexture(true);


    // Draw background
    setPaintingColor(Qt::transparent);
    glBegin(GL_QUADS);
    setTextureCoords(0.0, 0.001, 0.0);
    setVertex(boundingRect.bottomLeft());
    setTextureCoords(1.0, 0.001, 0.0);
    setVertex(boundingRect.bottomRight());
    setTextureCoords(1.0, 1.0, 0.0);
    setVertex(boundingRect.topRight());
    setTextureCoords(0.0, 1.0, 0.0);
    setVertex(boundingRect.topLeft());
    glEnd();
    setUseTexture(false);
    texture->release();
    delete texture;


    // Draw outline
    setPaintingColor(colorOutline);
    glLineWidth(1.0);
    glBegin(GL_LINE_LOOP);
    glVertex2i(boundingRect.bottomLeft().x(), boundingRect.bottomLeft().y());
    glVertex2i(boundingRect.bottomRight().x(), boundingRect.bottomRight().y());
    glVertex2i(boundingRect.topRight().x(), boundingRect.topRight().y());
    glVertex2i(boundingRect.topLeft().x(), boundingRect.topLeft().y());
    glEnd();
*/

    switch(anchor)
    {
    case topLeft:
        boundingRect.moveTopLeft(pos);
        break;
    case topRight:
        boundingRect.moveTopRight(pos);
        break;
    case bottomLeft:
        boundingRect.moveBottomLeft(pos);
        break;
    case bottomRight:
        boundingRect.moveBottomRight(pos);
        break;
    }

    QTransform trafo;
    trafo.scale(1.0, -1.0);
    trafo.translate(0.0, 2.0 * -pos.y());
    painter->save();
    painter->setTransform(trafo, true);
    painter->setPen(colorOutline);
    painter->setBrush(colorBackground);
    painter->drawRect(boundingRect);
    painter->setPen(colorText);
    painter->setFont(font);
    painter->drawText(boundingRect, Qt::AlignCenter, text);
    painter->restore();
}

void GLWidget::paintContent(LayerList layers)
{
    bool render_outline_shadow = render_outline;
    bool render_solid_shadow = render_solid;

    // ********** LINES SHADER ON ********
    shaderProgram = shaderProgram_lines;
    shaderProgram->bind();

    shaderProgram->setUniformValue(shader_useClippingXLocation, (GLint)0);   // Enable X-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingYLocation, (GLint)0);   // Enable Y-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingZLocation, (GLint)0);   // Enable Z-Clipping Plane
    shaderProgram->setUniformValue(shader_Height_of_intersection_location, this->height_of_intersection);
    shaderProgram->setUniformValue(shader_Depth_of_view_location,  this->height_of_intersection - this->depth_of_view);
    shaderProgram->setUniformValue(shader_matrixLocation, matrix_all);
    this->glEnable(GL_PRIMITIVE_RESTART);
    this->glPrimitiveRestartIndex(0xABCD);

    glName = 1;

    if (render_outline)
    {
        render_solid = false;
        paintLayers(layers);
    }
    render_solid = render_solid_shadow;

    shaderProgram->setUniformValue(shader_useClippingXLocation, (GLint)0);   // Enable X-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingYLocation, (GLint)0);   // Enable Y-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingZLocation, (GLint)0);   // Enable Z-Clipping Plane

    setUseTexture(false);

    // ********** TRIANGLES SHADER ON ********
    shaderProgram = shaderProgram_triangles;
    shaderProgram->bind();

    shaderProgram->setUniformValue(shader_useClippingXLocation, (GLint)0);   // Enable X-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingYLocation, (GLint)0);   // Enable Y-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingZLocation, (GLint)1);   // Enable Z-Clipping Plane
    shaderProgram->setUniformValue(shader_Height_of_intersection_location, this->height_of_intersection);
    shaderProgram->setUniformValue(shader_Depth_of_view_location,  this->height_of_intersection - this->depth_of_view);
    shaderProgram->setUniformValue(shader_matrixLocation, matrix_all);
    shaderProgram->setUniformValue(shader_is_Selection_location, render_selection);
    this->glEnable(GL_PRIMITIVE_RESTART);
    this->glPrimitiveRestartIndex(0xABCD);

    setUseTexture(false);

    glName = 0;

    if (render_solid)
    {
        render_outline = false;
        paintLayers(layers);
    }
    render_outline = render_outline_shadow;

    shaderProgram->setUniformValue(shader_useClippingXLocation, (GLint)0);   // Enable X-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingYLocation, (GLint)0);   // Enable Y-Clipping Plane
    shaderProgram->setUniformValue(shader_useClippingZLocation, (GLint)0);   // Enable Z-Clipping Plane
}

void GLWidget::paintLayers(LayerList layers)
{
    foreach (Layer* layer, layers)
    {
        // Set line width
        if (render_outline)
            glLineWidth(layer->lineWidth);
        else
            glLineWidth(1);
        //tbd: set line type

        if(itemDB->layerSoloActive)
        {
            if(layer->solo)
            {
                paintItems(layer->getItems(), layer);
                paintLayersSoloActive(layer->getChildLayers());
            }
            else
                paintLayers(layer->getChildLayers());
        }
        else
        {
            if (!layer->isOn)
                continue;

            paintItems(layer->getItems(), layer);
            paintLayers(layer->getChildLayers());
        }
    }
}

//once we have reached a layer in the hirarchy, that is active, all sublayers are consequently painted.
void GLWidget::paintLayersSoloActive(LayerList layers)
{
    foreach (Layer* layer, layers)
    {
        // Set line width
        if (render_outline)
            glLineWidth(layer->lineWidth);
        else
            glLineWidth(1);

        // tbd.: Set line type
        paintItems(layer->getItems(), layer);
        paintLayersSoloActive(layer->getChildLayers());
    }
}


void GLWidget::paintItems(QList<CADitem*> items, Layer* layer, bool checkBoundingBox, bool isSubItem)
{
    foreach (CADitem* item, items)
    {
        if(!isSubItem)
            glName ++;

        if(checkBoundingBox)
        {
            //Global culling performance test
            //Exclude all items from painting that do not reach the canvas with their boundingRect
            int screen_x_min = -this->width() / 2;
//            int screen_x_max =  this->width() / 2;
            int screen_y_min = -this->height() / 2;
//            int screen_y_max =  this->height() / 2;

            int p_x_min =  100000;
            int p_x_max = -100000;
            int p_y_min =  100000;
            int p_y_max = -100000;


            for (int i=0; i < 8; i++)
            {
                QVector3D boxPoint = item->boundingBox.p(i);
                QPointF screen_p = mapFromScene(boxPoint);

                if (screen_p.x() < p_x_min)     p_x_min = screen_p.x();
                if (screen_p.x() > p_x_max)     p_x_max = screen_p.x();
                if (screen_p.y() < p_y_min)     p_y_min = screen_p.y();
                if (screen_p.y() > p_y_max)     p_y_max = screen_p.y();
            }

            QRect screenRect;
            QRect itemRect;

            screenRect = QRect(screen_x_min, screen_y_min, this->width(), this->height());
            itemRect = QRect(p_x_min, p_y_min, (p_x_max - p_x_min), (p_y_max - p_y_min));

            if (!screenRect.intersects(itemRect))
            {
                continue;
            }
        }

        item->index = glName;
        if (item->isMaintenanceArea)
        {
            bool renderSolidShadow = this->render_solid;
            this->render_solid = false;
            item->paint(this);
            this->render_solid = renderSolidShadow;
        }
        else
            item->paint(this);
        if (item->subItems.count() > 0)
            paintItems(item->subItems, layer, false, true);
    }
}

void GLWidget::paintSnapIndicator(QPainter* painter, QRect focusRect, GLWidget::SnapMode snapMode, bool active)
{
    QPen pen;
    painter->setBrush(Qt::NoBrush);

    if (!active)
        focusRect.adjust(focusRect.width() / 4, focusRect.height() / 4, -focusRect.width() / 4, -focusRect.height() / 4);

    // Double render pass for glow effect
    for (int i = 0; i < 2; i++)
    {
        switch (snapMode)
        {
        case SnapBasepoint:
        {
            if (active)
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(170, 170, 255, 100));
                    pen.setWidth(3);
                    pen.setColor(QColor(170, 170, 255, 100));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(0, 0, 170));
                    pen.setWidth(1);
                    pen.setColor(QColor(0, 0, 170));
                }
            }
            else
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(170, 170, 255, 40));
                    pen.setWidth(3);
                    pen.setColor(QColor(170, 170, 255, 40));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(100, 100, 255));
                    pen.setWidth(1);
                    pen.setColor(QColor(100, 100, 255));
                }
            }

//            glBegin(GL_LINE_LOOP);
//            glVertex2i(focusRect.bottomLeft().x(), focusRect.bottomLeft().y());
//            glVertex2i(focusRect.bottomRight().x(), focusRect.bottomRight().y());
//            glVertex2i(focusRect.topLeft().x(), focusRect.topLeft().y());
//            glVertex2i(focusRect.topRight().x(), focusRect.topRight().y());
//            glEnd();

            QPolygon poly;
            poly.append(focusRect.bottomLeft());
            poly.append(focusRect.bottomRight());
            poly.append(focusRect.topLeft());
            poly.append(focusRect.topRight());
            poly.append(focusRect.bottomLeft());

            painter->setPen(pen);
            painter->drawPolyline(poly);

            break;
        }
        case SnapFlange:
        {
            if (active)
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(0, 170, 0, 100));
                    pen.setWidth(3);
                    pen.setColor(QColor(0, 170, 0, 100));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(170, 255, 170));
                    pen.setWidth(1);
                    pen.setColor(QColor(170, 255, 170));
                }
            }
            else
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(170, 255, 170, 40));
                    pen.setWidth(3);
                    pen.setColor(QColor(170, 255, 170, 40));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(100, 255, 100));
                    pen.setWidth(1);
                    pen.setColor(QColor(100, 255, 100));
                }
            }

//            glBegin(GL_LINES);
//            glVertex2i(focusRect.left(), focusRect.top());
//            glVertex2i(focusRect.left(), focusRect.bottom());
//            glVertex2i(focusRect.left(), (focusRect.center().y() + focusRect.top()) / 2);
//            glVertex2i(focusRect.right(), (focusRect.center().y() + focusRect.top()) / 2);
//            glVertex2i(focusRect.left(), (focusRect.center().y() + focusRect.bottom()) / 2);
//            glVertex2i(focusRect.right(), (focusRect.center().y() + focusRect.bottom()) / 2);
//            glEnd();

            painter->setPen(pen);
            painter->drawLine(focusRect.topLeft(), focusRect.bottomLeft());
            painter->drawLine(focusRect.left(), (focusRect.center().y() + focusRect.top()) / 2,
                              focusRect.right(), (focusRect.center().y() + focusRect.top()) / 2);
            painter->drawLine(focusRect.left(), (focusRect.center().y() + focusRect.bottom()) / 2,
                              focusRect.right(), (focusRect.center().y() + focusRect.bottom()) / 2);

            break;
        }
        case SnapEndpoint:
        {
            if (active)
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(255, 255, 100, 180));
                    pen.setWidth(3);
                    pen.setColor(QColor(255, 255, 100, 180));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(255, 255, 255));
                    pen.setWidth(1);
                    pen.setColor(QColor(255, 255, 255));
                }
            }
            else
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(255, 255, 170, 40));
                    pen.setWidth(3);
                    pen.setColor(QColor(255, 255, 170, 40));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(255, 255, 100));
                    pen.setWidth(1);
                    pen.setColor(QColor(255, 255, 100));
                }
            }

//            glBegin(GL_LINE_LOOP);
//            glVertex2i(focusRect.bottomLeft().x(), focusRect.bottomLeft().y());
//            glVertex2i(focusRect.bottomRight().x(), focusRect.bottomRight().y());
//            glVertex2i(focusRect.topRight().x(), focusRect.topRight().y());
//            glVertex2i(focusRect.topLeft().x(), focusRect.topLeft().y());
//            glEnd();

            painter->setPen(pen);
            painter->drawRect(focusRect.adjusted(0, 0, -1, -1));

            break;
        }
        case SnapCenter:
        {
            if (active)
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(170, 255, 255, 100));
                    pen.setWidth(3);
                    pen.setColor(QColor(170, 255, 255, 100));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(0, 170, 170));
                    pen.setWidth(1);
                    pen.setColor(QColor(0, 170, 170));
                }
            }
            else
            {
                if (i == 0)
                {
//                    glLineWidth(3);
//                    setPaintingColor(QColor(170, 255, 255, 40));
                    pen.setWidth(3);
                    pen.setColor(QColor(170, 255, 255, 40));
                }
                else
                {
//                    glLineWidth(1);
//                    setPaintingColor(QColor(100, 255, 255));
                    pen.setWidth(1);
                    pen.setColor(QColor(100, 255, 255));
                }
            }

//            glBegin(GL_LINES);
//            glVertex2i(focusRect.left(), focusRect.top());
//            glVertex2i(focusRect.right(), focusRect.bottom());
//            glVertex2i(focusRect.center().x() - 5, focusRect.center().y() + 5);
//            glVertex2i(focusRect.center().x() + 5, focusRect.center().y() - 5);
//            glEnd();

            painter->setPen(pen);
            painter->drawLine(focusRect.topLeft(), focusRect.bottomRight());
            painter->drawLine(focusRect.center().x() - 5, focusRect.center().y() + 5,
                              focusRect.center().x() + 5, focusRect.center().y() - 5);

            break;
        }
        case SnapNo:
        {
            break;
        }
        }
    }

    glLineWidth(1);
}

QList<CADitem*> GLWidget::itemsAtPosition_v2(QPoint pos, int size_x, int size_y)
{
    makeCurrent();

    if (fbo_select->size() != QSize(size_x, size_y))
    {
//        qCDebug(glwidget) << "fbo resize" << QSize(size_x, size_y);
        QOpenGLFramebufferObjectFormat format = fbo_select->format();
        delete fbo_select;
        fbo_select = new QOpenGLFramebufferObject(size_x, size_y, format);
    }

    matrix_glSelect.setToIdentity();
    matrix_glSelect.translate(-(qreal)(pos.x() + (this->width() - size_x) / 2), -(qreal)(pos.y() + (this->height() - size_y) / 2), 0.0);
    updateMatrixAll();

    fbo_select->bind();

    glDepthFunc(GL_LEQUAL);
    glDepthRange(1,0);
    glDisable(GL_BLEND);
    glDisable(GL_MULTISAMPLE);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_ALPHA_TEST);
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    bool render_outline_shadow = this->render_outline;
    if (this->render_solid == true)
        render_outline = false;
    render_selection = true;
    selectItemsByColor = true;
    paintContent(itemDB->getLayerList());
    selectItemsByColor = false;
    render_selection = false;
    this->render_outline = render_outline_shadow;

    QImage image = fbo_select->toImage(false);

    fbo_select->release();


    matrix_glSelect.setToIdentity();
    updateMatrixAll();
    doneCurrent();

    QList<CADitem*> foundItems;
    QMap<quint32,quint32> itemsDistMap;

    for (int x = 0; x < size_x; x++)
    {
        for (int y = 0; y < size_y; y++)
        {
            QRgb pixel = image.pixel(x, y);
            if ((pixel & 0xffffff) != 0)
            {
                quint32 itemName;
                itemName = (quint32)pixel & 0xffffff;
//                itemName =  ((quint32)pixel & 0x0000fc) >> 2;
//                itemName |= ((quint32)pixel & 0x00fc00) >> 4;
//                itemName |= ((quint32)pixel & 0xfc0000) >> 6;

                itemsDistMap.insertMulti(qAbs(size_x/2 - x) + qAbs(size_y/2 - y), itemName);
            }
        }
    }

    QList<quint32> processedNames;
    foreach(quint32 itemName, itemsDistMap.values())
    {
        if (!processedNames.contains(itemName))
        {
            CADitem* item = itemsAtPosition_processLayers(itemDB->getLayerList(), itemName);
            foundItems.append(item);
            processedNames.append(itemName);
        }
    }

    return foundItems;
}

CADitem *GLWidget::itemsAtPosition_processLayers(LayerList layers, GLuint glName)
{
    foreach (Layer* layer, layers)
    {
        if ((!layer->isOn) && (!layer->solo))
            continue;

        if (!layer->isWriteable)
            continue;

        CADitem* item = itemsAtPosition_processItems(layer->getItems(), glName);
        if (item)
            return item;


        item = itemsAtPosition_processLayers(layer->getChildLayers(), glName);
        if (item)
            return item;
    }

    return NULL;
}

CADitem *GLWidget::itemsAtPosition_processItems(QList<CADitem *> items, GLuint glName)
{
    foreach (CADitem* item, items)
    {
        if (item->index == glName)
        {
            return item;
        }

//        item = itemsAtPosition_processItems(item->subItems, glName);
//        if (item)
//            return item;
    }

    return NULL;
}

void GLWidget::highlightItemAtPosition(QPoint pos)
{
    QList<CADitem*> itemList = this->itemsAtPosition_v2(pos, _cursorPickboxSize, _cursorPickboxSize);
    CADitem* item;
    if (itemList.isEmpty())
        item = NULL;
    else
        item = itemList.at(0);

    // tst
    this->item_lastHighlight = item;

    if (item != NULL)
    {
        item->highlight = true;
        highlightItems(item->subItems);
    }

    emit signal_highlightItem(item);
}

void GLWidget::highlightItems(QList<CADitem *> items)
{
    foreach(CADitem* item, items)
    {
        item->highlight = true;
        highlightItems(item->subItems);
    }
}

void GLWidget::highlightClear()
{
    this->item_lastHighlight = NULL;
    highlightClear_processLayers(itemDB->getLayerList());
}

void GLWidget::highlightClear_processLayers(LayerList layers)
{
    foreach (Layer* layer, layers)
    {
        highlightClear_processItems(layer->getItems());
        highlightClear_processLayers(layer->getChildLayers());
    }
}

void GLWidget::highlightClear_processItems(QList<CADitem *> items)
{
    foreach (CADitem* item, items)
    {
        item->highlight = false;
        highlightClear_processItems(item->subItems);
    }
}

void GLWidget::selectionAddItem(CADitem *item)
{
    if (item != NULL)
    {
        if (selection_itemList.contains(item))
            return;
        this->selection_itemList.append(item);
        item->selected = true;
        selectionAddSubItems(item->subItems);
        emit signal_selectionChanged(selection_itemList);
    }
}

void GLWidget::selectionAddItems(QList<CADitem *> items)
{
    foreach (CADitem* item, items)
    {
        if (item != NULL)
        {
            if (selection_itemList.contains(item))
                continue;
            this->selection_itemList.append(item);
            item->selected = true;
            selectionAddSubItems(item->subItems);
        }
    }
    emit signal_selectionChanged(selection_itemList);
}

void GLWidget::selectionAddSubItems(QList<CADitem *> items)
{
    foreach(CADitem * item, items) {
        item->selected = true;
        selectionAddSubItems(item->subItems);
    }
}

void GLWidget::selectionRemoveItem(CADitem *item)
{
    if (item != NULL)
    {
        if (!selection_itemList.contains(item))
            return;
        this->selection_itemList.removeOne(item);
        item->selected = false;
        selectionRemoveSubItems(item->subItems);
        emit signal_selectionChanged(selection_itemList);
    }
}

void GLWidget::selectionRemoveSubItems(QList<CADitem *> items)
{
    foreach(CADitem * item, items) {
        item->selected = false;
        selectionRemoveSubItems(item->subItems);
    }
}

void GLWidget::selectionClear()
{
    selectionClear_processLayers(itemDB->getLayerList());
    this->selection_itemList.clear();
    emit signal_selectionChanged(this->selection_itemList);
}

void GLWidget::selectionClear_processLayers(LayerList layers)
{
    foreach (Layer* layer, layers)
    {
        selectionClear_processItems(layer->getItems());
        selectionClear_processLayers(layer->getChildLayers());
    }
}

void GLWidget::selectionClear_processItems(QList<CADitem *> items)
{
    foreach (CADitem* item, items)
    {
        item->selected = false;
        selectionClear_processItems(item->subItems);
    }
}

void GLWidget::zoom_pan_showAll()
{
    M3dBoundingBox boundingBox_scene;
    boundingBox_scene.reset();
    zoom_pan_showAll_processLayers(itemDB->getLayerList(), &boundingBox_scene);

    QList<QVector3D> vertices = boundingBox_scene.getVertices();

    M3dBoundingBox boundingBox_screen;
    boundingBox_screen.reset();

    foreach (QVector3D vertex, vertices)
    {
        QPointF screenPoint = this->mapFromScene(vertex);
        boundingBox_screen.enterVertex(QVector3D(screenPoint));
    }


    // Set zoom to fit all items in screen
    qreal current_width  = boundingBox_screen.x_max - boundingBox_screen.x_min;
    qreal current_height = boundingBox_screen.y_max - boundingBox_screen.y_min;

    qreal zoom_x = (qreal)this->width()  / current_width;
    qreal zoom_y = (qreal)this->height() / current_height;

    qreal zoom_final = qMin(zoom_x, zoom_y) * 0.99;

    this->zoomFactor *= zoom_final;

    // Calculate new center after zooming
    matrix_modelview.setToIdentity();
    matrix_modelview.translate(translationOffset.x(), translationOffset.y(), 0.0);
//    matrix_modelview.scale(this->zoomFactor, this->zoomFactor, 1.0 / 100000.0);
    matrix_modelview.scale(this->zoomFactor, this->zoomFactor, 1.0);
    updateMatrixAll();

    boundingBox_screen.reset();
    foreach (QVector3D vertex, vertices)
    {
        QPointF screenPoint = this->mapFromScene(vertex);
        boundingBox_screen.enterVertex(QVector3D(screenPoint));
    }

    // Move center of scene to center of screen
    QPointF center_scene = QPointF((boundingBox_screen.x_min + boundingBox_screen.x_max) / 2.0, (boundingBox_screen.y_min + boundingBox_screen.y_max) / 2.0);
    QPointF center_screen = QPointF(0.0, 0.0);
    QPointF center_delta = center_screen - center_scene;

    this->translationOffset += center_delta.toPoint();

}

void GLWidget::zoom_pan_showAll_processLayers(LayerList layers, M3dBoundingBox* boundingBox)
{
    foreach (Layer* layer, layers)
    {
        zoom_pan_showAll_processItems(layer->getItems(), boundingBox);
        zoom_pan_showAll_processLayers(layer->getChildLayers(), boundingBox);
    }
}

void GLWidget::zoom_pan_showAll_processItems(QList<CADitem *> items, M3dBoundingBox* boundingBox)
{
    foreach (CADitem* item, items)
    {
        if (item->boundingBox.isEmpty())
            continue;
        boundingBox->enterVertices(item->boundingBox.getVertices());
    }
}

void GLWidget::initializeGL()
{
    qCDebug(glwidget) << "initializeGL()";

    makeCurrent();
    bool initializedOpenGLFunktions = initializeOpenGLFunctions();
    if (!initializedOpenGLFunktions)
    {
        QMessageBox::critical(this, tr("Critical error in GLWidget::initializeGL()"), tr("initializeOpenGLFunctions() returned false.\n"
                                                                                         "Running the correct Qt-version? Check env variable LD_LIBRARY_PATH!\n"
                                                                                         "Check openGL Version against system requirements: \nglxinfo | grep version"));
        return;
    }
//    makeCurrent();

    openGLTimerQuery = new QOpenGLTimerQuery(this);
    openGLTimerQuery->create();

    glEnable(GL_FRAMEBUFFER_SRGB);

    shader_1_vert = new QOpenGLShader(QOpenGLShader::Vertex);
    if (!shader_1_vert->compileSourceFile(":/shaders/shader_1.vert"))
        QMessageBox::critical(this, "Shader compiler", "Vertex shader failed to compile!");

    shader_1_triangles_geom = new QOpenGLShader(QOpenGLShader::Geometry);
    if (!shader_1_triangles_geom->compileSourceFile(":/shaders/shader_1_triangles.geom"))
        QMessageBox::critical(this, "Shader compiler", "Triangle geometry shader failed to compile!");

    shader_1_lines_geom = new QOpenGLShader(QOpenGLShader::Geometry);
    if (!shader_1_lines_geom->compileSourceFile(":/shaders/shader_1_lines.geom"))
        QMessageBox::critical(this, "Shader compiler", "Lines geometry shader failed to compile!");

    shader_1_frag = new QOpenGLShader(QOpenGLShader::Fragment);
    if (!shader_1_frag->compileSourceFile(":/shaders/shader_1.frag"))
        QMessageBox::critical(this, "Shader compiler", "Fragment shader failed to compile!");

    shader_2_vert = new QOpenGLShader(QOpenGLShader::Vertex);
    if (!shader_2_vert->compileSourceFile(":/shaders/shader_2.vert"))
        QMessageBox::critical(this, "Shader compiler", "Vertex shader 2 failed to compile!");

    shader_2_frag = new QOpenGLShader(QOpenGLShader::Fragment);
    if (!shader_2_frag->compileSourceFile(":/shaders/shader_2.frag"))
        QMessageBox::critical(this, "Shader compiler", "Fragment shader 2 failed to compile!");

    // ********** LINE SHADER **********
    shaderProgram_lines = new QOpenGLShaderProgram(this);
    shaderProgram_lines->addShader(shader_1_vert);
    shaderProgram_lines->addShader(shader_1_lines_geom);
    shaderProgram_lines->addShader(shader_1_frag);
    if (!shaderProgram_lines->link())
    {
        QMessageBox::critical(this, "Shader linker", QString("Line Shader failed to link!\n\n") + shaderProgram->log());
    }
    if (!shaderProgram_lines->bind())
    {
        QMessageBox::critical(this, "Shader program", "Line Shaderprogram could not be bound to gl context!");
    }

    // ********** TRIANGLE SHADER **********
    shaderProgram_triangles = new QOpenGLShaderProgram(this);
    shaderProgram_triangles->addShader(shader_1_vert);
    shaderProgram_triangles->addShader(shader_1_triangles_geom);
    shaderProgram_triangles->addShader(shader_1_frag);
    if (!shaderProgram_triangles->link())
    {
        QMessageBox::critical(this, "Shader linker", QString("Triangle Shader failed to link!\n\n") + shaderProgram->log());
    }
    if (!shaderProgram_triangles->bind())
    {
        QMessageBox::critical(this, "Shader program", "Triangle Shaderprogram could not be bound to gl context!");
    }

    // ********** OVERLAY SHADER **********
    shaderProgram_overlay = new QOpenGLShaderProgram(this);
    shaderProgram_overlay->addShader(shader_2_vert);
    shaderProgram_overlay->addShader(shader_2_frag);
    if (!shaderProgram_overlay->link())
    {
        QMessageBox::critical(this, "Shader linker", QString("Overlay Shader failed to link!\n\n") + shaderProgram->log());
    }
    if (!shaderProgram_overlay->bind())
    {
        QMessageBox::critical(this, "Shader program", "Overlay Shaderprogram could not be bound to gl context!");
    }


    shaderProgram = shaderProgram_triangles;


    shader_vertexLocation = shaderProgram->attributeLocation("VertexPosition");
    shader_matrixLocation = shaderProgram->uniformLocation("Matrix");
    shader_colorLocation = shaderProgram->attributeLocation("VertexColor");
    shader_textureCoordLocation = shaderProgram->attributeLocation("TexCoord");
    shader_textureSamplerLocation = shaderProgram->uniformLocation("uTexUnit0");
    shader_useTextureLocation = shaderProgram->uniformLocation("UseTexture");
    shader_useClippingXLocation = shaderProgram->uniformLocation("UseClippingX");
    shader_useClippingYLocation = shaderProgram->uniformLocation("UseClippingY");
    shader_useClippingZLocation = shaderProgram->uniformLocation("UseClippingZ");
    shader_Depth_of_view_location = shaderProgram->uniformLocation("Depth_of_view");
    shader_Height_of_intersection_location = shaderProgram->uniformLocation("Height_of_intersection");
    shader_is_Selection_location = shaderProgram->uniformLocation("is_Selection");

    if (shader_vertexLocation < 0)
        QMessageBox::information(this, "Vertex Location invalid", QString().setNum(shader_vertexLocation));
    if (shader_colorLocation < 0)
        QMessageBox::information(this, "Color Location invalid", QString().setNum(shader_colorLocation));
    if (shader_textureCoordLocation < 0)
        QMessageBox::information(this, "Texture Coordinate Location invalid", QString().setNum(shader_textureCoordLocation));
    if (shader_matrixLocation < 0)
        QMessageBox::information(this, "Matrix Location invalid", QString().setNum(shader_matrixLocation));
    if (shader_useTextureLocation < 0)
        QMessageBox::information(this, "Use Texture Location invalid", QString().setNum(shader_useTextureLocation));
    if (shader_useClippingXLocation < 0)
        QMessageBox::information(this, "Use ClippingX Location invalid", QString().setNum(shader_useClippingXLocation));
    if (shader_useClippingYLocation < 0)
        QMessageBox::information(this, "Use ClippingY Location invalid", QString().setNum(shader_useClippingYLocation));
    if (shader_useClippingZLocation < 0)
        QMessageBox::information(this, "Use ClippingZ Location invalid", QString().setNum(shader_useClippingZLocation));
    if (shader_Depth_of_view_location < 0)
        QMessageBox::information(this, "Depth of View Location invalid", QString().setNum(shader_Depth_of_view_location));
    if (shader_Height_of_intersection_location < 0)
        QMessageBox::information(this, "Height of Intersection Location invalid", QString().setNum(shader_Height_of_intersection_location));

//    qCDebug(glwidget) << "vertex location" << shader_vertexLocation;
//    qCDebug(glwidget) << "matrix location" << shader_matrixLocation;
//    qCDebug(glwidget) << "color location" << shader_colorLocation;
//    qCDebug(glwidget) << "texture coord location" << shader_textureCoordLocation;
//    qCDebug(glwidget) << "use texture location" << shader_useTextureLocation;
//    qCDebug(glwidget) << "use clippingX location" << shader_useClippingXLocation;
//    qCDebug(glwidget) << "use clippingY location" << shader_useClippingYLocation;
//    qCDebug(glwidget) << "use clippingZ location" << shader_useClippingZLocation;
//    qCDebug(glwidget) << "depth of view location" << shader_Depth_of_view_location;
//    qCDebug(glwidget) << "height of intersection location" << shader_Height_of_intersection_location;

    texture_cube1 = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture_cube1->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_cube1->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture_cube2 = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture_cube2->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_cube2->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture_cube3 = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture_cube3->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_cube3->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture_cube4 = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture_cube4->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_cube4->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture_cube5 = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture_cube5->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_cube5->setWrapMode(QOpenGLTexture::ClampToEdge);
    texture_cube6 = new QOpenGLTexture(QOpenGLTexture::Target2D);
    texture_cube6->setMinMagFilters(QOpenGLTexture::Linear, QOpenGLTexture::Linear);
    texture_cube6->setWrapMode(QOpenGLTexture::ClampToEdge);

    QImage textImage(80, 80, QImage::Format_ARGB32);
    QPainter painter(&textImage);
    painter.setPen(Qt::white);
    QFont font_big;
    font_big.setPixelSize(25);
    QFont font_small;
    font_small.setPixelSize(12);

    textImage.fill(Qt::black);
    painter.setFont(font_big);
    painter.drawText(textImage.rect(), Qt::AlignCenter, "Z+");
    painter.setFont(font_small);
    painter.drawText(textImage.rect(), Qt::AlignHCenter | Qt::AlignBottom, tr("looking up"));
    texture_cube1->setData(textImage, QOpenGLTexture::DontGenerateMipMaps);

    textImage.fill(Qt::black);
    painter.setFont(font_big);
    painter.drawText(textImage.rect(), Qt::AlignCenter, "Z-");
    painter.setFont(font_small);
    painter.drawText(textImage.rect(), Qt::AlignHCenter | Qt::AlignBottom, tr("looking down"));
    texture_cube2->setData(textImage, QOpenGLTexture::DontGenerateMipMaps);

    textImage.fill(Qt::black);
    painter.setFont(font_big);
    painter.drawText(textImage.rect(), Qt::AlignCenter, "Y-");
    painter.setFont(font_small);
    painter.drawText(textImage.rect(), Qt::AlignHCenter | Qt::AlignBottom, tr("looking south"));
    texture_cube3->setData(textImage, QOpenGLTexture::DontGenerateMipMaps);

    textImage.fill(Qt::black);
    painter.setFont(font_big);
    painter.drawText(textImage.rect(), Qt::AlignCenter, "Y+");
    painter.setFont(font_small);
    painter.drawText(textImage.rect(), Qt::AlignHCenter | Qt::AlignBottom, tr("looking north"));
    texture_cube4->setData(textImage, QOpenGLTexture::DontGenerateMipMaps);

    textImage.fill(Qt::black);
    painter.setFont(font_big);
    painter.drawText(textImage.rect(), Qt::AlignCenter, "X+");
    painter.setFont(font_small);
    painter.drawText(textImage.rect(), Qt::AlignHCenter | Qt::AlignBottom, tr("looking east"));
    texture_cube5->setData(textImage, QOpenGLTexture::DontGenerateMipMaps);

    textImage.fill(Qt::black);
    painter.setFont(font_big);
    painter.drawText(textImage.rect(), Qt::AlignCenter, "X-");
    painter.setFont(font_small);
    painter.drawText(textImage.rect(), Qt::AlignHCenter | Qt::AlignBottom, tr("looking west"));
    texture_cube6->setData(textImage, QOpenGLTexture::DontGenerateMipMaps);

    painter.end();

    QOpenGLFramebufferObjectFormat format;
    format.setSamples(0);
    format.setAttachment(QOpenGLFramebufferObject::Depth);
    format.setInternalTextureFormat(GL_RGBA);
    format.setMipmap(false);
    format.setTextureTarget(GL_TEXTURE_2D);

    fbo_select = new QOpenGLFramebufferObject(_cursorPickboxSize, _cursorPickboxSize, format);

    format.setSamples(8);
    fbo_renderImage = new QOpenGLFramebufferObject(this->width(), this->height(), format);
}

void GLWidget::resizeGL(int w, int h)
{
    qCDebug(glwidget) << "resizeGL()";

    if (this->aspectRatio > 0.0)
    {
        QSize newSize = QSize(w, h);
        if (fabs((qreal)newSize.width() / (qreal)newSize.height() - this->aspectRatio) > 0.01)
        {
            if ((qreal)newSize.width() / (qreal)newSize.height() < this->aspectRatio)
            {
                w = newSize.width();
                h = (qreal)newSize.width() / this->aspectRatio;
            }
            else
            {
                w = (qreal)newSize.height() * this->aspectRatio;
                h = newSize.height();
            }
            this->resize(w, h);
        }
    }

    displayCenter = QPoint(w, h) / 2;

    matrix_projection.setToIdentity();
    matrix_projection.scale(2.0 / (qreal)w, 2.0 / (qreal)h, 1.0);
    matrix_projection.translate(0.5, 0.5, 0.0);    // Half of a pixel to hit pixels with cursor lines

    updateMatrixAll();
    slot_repaint();
}
