#include "cad_electrical_cabinet.h"
#include "itemdb.h"

CAD_electrical_cabinet::CAD_electrical_cabinet() : CADitem(CADitemTypes::Electrical_Cabinet)
{
    wizardParams.insert("Position x", (0.0));
    wizardParams.insert("Position y", (0.0));
    wizardParams.insert("Position z", (0.0));
    wizardParams.insert("Angle x", (0.0));
    wizardParams.insert("Angle y", (0.0));
    wizardParams.insert("Angle z", (0.0));

    processWizardInput();
    calculate();
}

CAD_electrical_cabinet::~CAD_electrical_cabinet()
{

}

QList<CADitemTypes::ItemType> CAD_electrical_cabinet::flangable_items()
{
    QList<CADitemTypes::ItemType> flangable_items;
    flangable_items.append(CADitemTypes::Electrical_Cabinet);
    return flangable_items;
}

QImage CAD_electrical_cabinet::wizardImage()
{
    QImage image;
    QFileInfo fileinfo(__FILE__);
    QString imageFileName = fileinfo.baseName();
    imageFileName.prepend(":/itemGraphic/");
    imageFileName.append(".png");

    ;

    image.load(imageFileName, "PNG");

    return image;
}

QString CAD_electrical_cabinet::iconPath()
{
    return ":/icons/cad_electrical/cad_electrical_cabinet.svg";
}

QString CAD_electrical_cabinet::domain()
{
    return "Electrical";
}

QString CAD_electrical_cabinet::description()
{
    return "Electrical|Cabinet";
}

void CAD_electrical_cabinet::calculate()
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
}

void CAD_electrical_cabinet::processWizardInput()
{
    position.setX(wizardParams.value("Position x").toDouble());
    position.setY(wizardParams.value("Position y").toDouble());
    position.setZ(wizardParams.value("Position z").toDouble());
    angle_x = wizardParams.value("Angle x").toDouble();
    angle_y = wizardParams.value("Angle y").toDouble();
    angle_z = wizardParams.value("Angle z").toDouble();

}
