#include "cad_basic_arc.h"

#define PI 3.1415926535897

CAD_basic_arc::CAD_basic_arc() : CADitem(CADitem::Basic_Arc)
{
    this->description = "Basic|Arc";

    wizardParams.insert("Center x", QVariant::fromValue(0.0));
    wizardParams.insert("Center y", QVariant::fromValue(0.0));
    wizardParams.insert("Center z", QVariant::fromValue(0.0));

    wizardParams.insert("Angle x", QVariant::fromValue(0.0));
    wizardParams.insert("Angle y", QVariant::fromValue(0.0));
    wizardParams.insert("Angle z", QVariant::fromValue(0.0));

    wizardParams.insert("Radius", QVariant::fromValue(1.0));
    wizardParams.insert("Central Angle", QVariant::fromValue(90.0));

    processWizardInput();
    calculate();
}

CAD_basic_arc::~CAD_basic_arc()
{

}

QList<CADitem::ItemType> CAD_basic_arc::flangable_items()
{
    QList<CADitem::ItemType> flangable_items;

    return flangable_items;
}

QImage CAD_basic_arc::wizardImage()
{
    QImage image;
    QFileInfo fileinfo(__FILE__);
    QString imageFileName = fileinfo.baseName();
    imageFileName.prepend(":/itemGraphic/");
    imageFileName.append(".png");

    qDebug() << imageFileName;

    image.load(imageFileName, "PNG");

    return image;
}

void CAD_basic_arc::calculate()
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

    this->snap_vertices.append(QVector3D(position.x()+radius, position.y(), position.z()));
    this->snap_vertices.append(QVector3D(position.x()+radius*qCos(centralAngle/180.0f*PI), position.y()+radius*qSin(centralAngle/180.0f*PI), position.z()));
    this->snap_vertices.append(QVector3D(position.x()+radius*qCos(centralAngle/360.0f*PI), position.y()+radius*qSin(centralAngle/360.0f*PI), position.z()));

}

void CAD_basic_arc::processWizardInput()
{
    this->position.setX(wizardParams.value("Center x").toDouble());
    this->position.setY(wizardParams.value("Center y").toDouble());
    this->position.setZ(wizardParams.value("Center z").toDouble());
    this->center = QVector3D(position.x()+radius*qCos(centralAngle/360.0f*PI), position.y()+radius*qSin(centralAngle/360.0f*PI), position.z());

    this->radius = wizardParams.value("Radius").toDouble();
    this->centralAngle = wizardParams.value("Central Angle").toDouble();

    this->angle_x = wizardParams.value("Angle x").toDouble();
    this->angle_y = wizardParams.value("Angle y").toDouble();
    this->angle_z = wizardParams.value("Angle z").toDouble();

}
