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

#include "cad_gas_pipeturn.h"

CAD_Gas_PipeTurn::CAD_Gas_PipeTurn() : CADitem(CADitemTypes::Gas_PipeTurn)
{
    wizardParams.insert("Position x", 0.0);
    wizardParams.insert("Position y", 0.0);
    wizardParams.insert("Position z", 0.0);
    wizardParams.insert("Angle x", 0.0);
    wizardParams.insert("Angle y", 0.0);
    wizardParams.insert("Angle z", 0.0);

    wizardParams.insert("r", 500.0);
    wizardParams.insert("alpha", 90.0);
    wizardParams.insert("d", 150.0);
    wizardParams.insert("iso", 15.0);
    wizardParams.insert("s", 10.0);
    wizardParams.insert("l1", 100.0);
    wizardParams.insert("l2", 100.0);

    turn = new CAD_basic_turn();
    left = new CAD_basic_pipe();
    right = new CAD_basic_pipe();
    this->subItems.append(turn);
    this->subItems.append(left);
    this->subItems.append(right);

    processWizardInput();
    calculate();
}

CAD_Gas_PipeTurn::~CAD_Gas_PipeTurn()
{
//    arrayBufVertices->destroy();
//    indexBufFaces->destroy();
//    indexBufLines->destroy();
//    delete arrayBufVertices;
//    delete indexBufFaces;
//    delete indexBufLines;
}

QList<CADitemTypes::ItemType> CAD_Gas_PipeTurn::flangable_items(int flangeIndex)
{
    Q_UNUSED(flangeIndex);
    QList<CADitemTypes::ItemType> flangable_items;
    
    return flangable_items;
}

QImage CAD_Gas_PipeTurn::wizardImage()
{
    QImage image;
    QFileInfo fileinfo(__FILE__);
    QString imageFileName = fileinfo.baseName();
    imageFileName.prepend(":/itemGraphic/");
    imageFileName.append(".png");
    image.load(imageFileName, "PNG");
                       
    return image;
}

QString CAD_Gas_PipeTurn::iconPath()
{
    return ":/icons/cad_gas/cad_gas_pipeturn.svg";
}

QString CAD_Gas_PipeTurn::domain()
{
    return "Gas";
}

QString CAD_Gas_PipeTurn::description()
{
    return "Gas|Pipe Turn";
}

void CAD_Gas_PipeTurn::calculate()
{                
    matrix_rotation.setToIdentity();
    matrix_rotation.rotate(angle_x, 1.0, 0.0, 0.0);
    matrix_rotation.rotate(angle_y, 0.0, 1.0, 0.0);
    matrix_rotation.rotate(angle_z, 0.0, 0.0, 1.0);

    boundingBox.reset();

    this->snap_flanges.clear();
    this->snap_center.clear();
    this->snap_vertices.clear();

    this->snap_basepoint = (position);

    left->wizardParams.insert("Position x", position.x());
    left->wizardParams.insert("Position y", position.y());
    left->wizardParams.insert("Position z", position.z());
    left->wizardParams.insert("Angle x", angle_x);
    left->wizardParams.insert("Angle y", angle_y);
    left->wizardParams.insert("Angle z", angle_z);
    left->wizardParams.insert("l", l1);
    left->wizardParams.insert("d", d + 2*iso);
    left->wizardParams.insert("s",  iso+s);
    left->layer = this->layer;
    left->processWizardInput();
    left->calculate();

    QVector3D position_right = position + matrix_rotation * QVector3D(l1 + sin(alpha / 180 * PI) * r, 0.0, -(1-cos(alpha / 180 * PI)) * r);
    QVector3D angles_right = MAngleCalculations().anglesFromMatrix(rotationOfFlange(2));
    right->wizardParams.insert("Position x", position_right.x());
    right->wizardParams.insert("Position y", position_right.y());
    right->wizardParams.insert("Position z", position_right.z());
    right->wizardParams.insert("Angle x", angles_right.x());
    right->wizardParams.insert("Angle y", angles_right.y());
    right->wizardParams.insert("Angle z", angles_right.z());
    right->wizardParams.insert("l", l2);
    right->wizardParams.insert("d", d + 2*iso);
    right->wizardParams.insert("s",  iso+s);
    right->layer = this->layer;
    right->processWizardInput();
    right->calculate();

    QVector3D position_turn = position + matrix_rotation * QVector3D(l1, 0.0, 0.0);
    turn->wizardParams.insert("Position x", position_turn.x());
    turn->wizardParams.insert("Position y", position_turn.y());
    turn->wizardParams.insert("Position z", position_turn.z());
    turn->wizardParams.insert("Angle x", 0.0);
    turn->wizardParams.insert("Angle y", 0.0);
    turn->wizardParams.insert("Angle z", 0.0);
    turn->wizardParams.insert("d", d + 2*iso);
    turn->wizardParams.insert("r", r);
    turn->wizardParams.insert("alpha", alpha);
    turn->wizardParams.insert("s", s + iso);
    turn->layer = this->layer;
    turn->processWizardInput();
    turn->rotateAroundAxis(90, QVector3D(1.0, 0.0, 0.0), angle_x, angle_y, angle_z);
    turn->calculate();

    this->boundingBox = left->boundingBox;
    this->boundingBox.enterVertices(right->boundingBox.getVertices());
    this->boundingBox.enterVertices(turn->boundingBox.getVertices());
    this->snap_flanges.append(position);
    this->snap_flanges.append(position + matrix_rotation * QVector3D(l1 + sin(alpha / 180 * PI) * r + cos(alpha / 180 * PI) * l2,
                                                                     0.0,
                                                                     -(1-cos(alpha / 180 * PI)) * r - sin(alpha / 180.0 * PI) * l2));
}

void CAD_Gas_PipeTurn::processWizardInput()
{
    position.setX(wizardParams.value("Position x").toDouble());
    position.setY(wizardParams.value("Position y").toDouble());
    position.setZ(wizardParams.value("Position z").toDouble());
    angle_x = wizardParams.value("Angle x").toDouble();
    angle_y = wizardParams.value("Angle y").toDouble();
    angle_z = wizardParams.value("Angle z").toDouble();

    l1 = wizardParams.value("l1").toDouble();
    l2 = wizardParams.value("l2").toDouble();
    r = wizardParams.value("r").toDouble();
    alpha = wizardParams.value("alpha").toDouble();
    d = wizardParams.value("d").toDouble();
    iso = wizardParams.value("iso").toDouble();
    s = wizardParams.value("s").toDouble();
}

QMatrix4x4 CAD_Gas_PipeTurn::rotationOfFlange(quint8 num)
{
    if(num == 1)
    {
        QMatrix4x4 m;
        m.setToIdentity();
        m.rotate(180.0, 0.0, 0.0, 1.0);
        return matrix_rotation * m;
    }
    else if(num == 2)
    {
        QMatrix4x4 m;
        m.setToIdentity();
        m.rotate(alpha, 0.0, 1.0, 0.0);
        return matrix_rotation * m;
    }
    else
        return matrix_rotation;
}

//void CAD_Gas_PipeTurn::paint(GLWidget *glwidget)
//{
//    QColor color_pen_tmp = getColorPen();
//    QColor color_brush_tmp = getColorBrush();

//    arrayBufVertices->bind();
//    glwidget->shaderProgram->enableAttributeArray(glwidget->shader_vertexLocation);
//    glwidget->shaderProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, sizeof(QVector3D));

//    if (glwidget->render_solid)
//    {
//        glwidget->setPaintingColor(color_brush_tmp);

//        indexBufFaces->bind();
//        glwidget->glDrawElements(GL_TRIANGLE_STRIP, indexBufFaces->size(), GL_UNSIGNED_SHORT, 0);

//        indexBufFaces->release();
//    }

//    if (glwidget->render_outline)
//    {
//        glwidget->setPaintingColor(color_pen_tmp);
                                      
//        indexBufLines->bind();
//        glwidget->glDrawElements(GL_LINES, indexBufLines->size(), GL_UNSIGNED_SHORT, 0);
//        indexBufLines->release();
//     }                          
                                                                                           
//     arrayBufVertices->release();
//}
