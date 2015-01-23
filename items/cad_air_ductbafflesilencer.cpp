#include "cad_air_ductbafflesilencer.h"
#include "itemdb.h"

CAD_air_ductBaffleSilencer::CAD_air_ductBaffleSilencer() : CADitem(CADitemTypes::Air_DuctBaffleSilencer)
{
    main_duct = new CAD_basic_duct();
    flange_duct_left = new CAD_basic_duct();
    flange_duct_right = new CAD_basic_duct();
    subItems.append(main_duct);
    subItems.append(flange_duct_left);
    subItems.append(flange_duct_right);

    wizardParams.insert("Position x", (0.0));
    wizardParams.insert("Position y", (0.0));
    wizardParams.insert("Position z", (0.0));
    wizardParams.insert("Angle x", (0.0));
    wizardParams.insert("Angle y", (0.0));


    wizardParams.insert("l", (100));
    wizardParams.insert("a", (20));
    wizardParams.insert("b", (30.0));
    wizardParams.insert("d", (5.0));
    wizardParams.insert("n", (3.0));
    wizardParams.insert("fe", (1.0));
    wizardParams.insert("ff", (1.0));
    wizardParams.insert("s", (1.0));

    processWizardInput();
    calculate();
}

CAD_air_ductBaffleSilencer::~CAD_air_ductBaffleSilencer()
{

}

QList<CADitemTypes::ItemType> CAD_air_ductBaffleSilencer::flangable_items()
{
    QList<CADitemTypes::ItemType> flangable_items;
    flangable_items.append(CADitemTypes::Air_Duct);
    flangable_items.append(CADitemTypes::Air_DuctEndPlate);
    flangable_items.append(CADitemTypes::Air_DuctFireDamper);
    flangable_items.append(CADitemTypes::Air_DuctTeeConnector);
    flangable_items.append(CADitemTypes::Air_DuctTransition);
    flangable_items.append(CADitemTypes::Air_DuctTransitionRectRound);
    flangable_items.append(CADitemTypes::Air_DuctTurn);
    flangable_items.append(CADitemTypes::Air_DuctVolumetricFlowController);
    flangable_items.append(CADitemTypes::Air_DuctYpiece);
    flangable_items.append(CADitemTypes::Air_Filter);
    flangable_items.append(CADitemTypes::Air_HeatExchangerAirAir);
    flangable_items.append(CADitemTypes::Air_HeatExchangerWaterAir);
    flangable_items.append(CADitemTypes::Air_MultiLeafDamper);
    return flangable_items;
}

QImage CAD_air_ductBaffleSilencer::wizardImage()
{
    QImage image;
    QFileInfo fileinfo(__FILE__);
    QString imageFileName = fileinfo.baseName();
    imageFileName.prepend(":/itemGraphic/");
    imageFileName.append(".png");

    image.load(imageFileName, "PNG");

    return image;
}

QString CAD_air_ductBaffleSilencer::iconPath()
{
    return ":/icons/cad_air/cad_air_ductbafflesilencer.svg";
}

QString CAD_air_ductBaffleSilencer::domain()
{
    return "Air";
}

QString CAD_air_ductBaffleSilencer::description()
{
    return "Air|Duct baffle silencer";
}

void CAD_air_ductBaffleSilencer::calculate()
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

    main_duct->wizardParams.insert("Position x", (position.x()));
    main_duct->wizardParams.insert("Position y", (position.y()));
    main_duct->wizardParams.insert("Position z", (position.z()));
    main_duct->wizardParams.insert("Angle x", (angle_x));
    main_duct->wizardParams.insert("Angle y", (angle_y));
    main_duct->wizardParams.insert("Angle z", (angle_z));
    main_duct->wizardParams.insert("l", (l));
    main_duct->wizardParams.insert("b", (b));
    main_duct->wizardParams.insert("a", (a));
    main_duct->wizardParams.insert("s", (s));
    main_duct->layer = this->layer;
    main_duct->processWizardInput();
    main_duct->calculate();


    QVector3D pos_rot = position + matrix_rotation * (QVector3D((l - fe), 0.0, 0.0));
    flange_duct_left->wizardParams.insert("Position x", (pos_rot.x()));
    flange_duct_left->wizardParams.insert("Position y", (pos_rot.y()));
    flange_duct_left->wizardParams.insert("Position z", (pos_rot.z()));
    flange_duct_left->wizardParams.insert("Angle x", (angle_x));
    flange_duct_left->wizardParams.insert("Angle y", (angle_y));
    flange_duct_left->wizardParams.insert("Angle z", (angle_z));
    flange_duct_left->wizardParams.insert("l", (fe));
    flange_duct_left->wizardParams.insert("b", (b + 2 * ff));
    flange_duct_left->wizardParams.insert("a", (a + 2 * ff));
    flange_duct_left->wizardParams.insert("s", (ff));
    flange_duct_left->layer = this->layer;
    flange_duct_left->processWizardInput();
    flange_duct_left->calculate();

    flange_duct_right->wizardParams.insert("Position x", (position.x()));
    flange_duct_right->wizardParams.insert("Position y", (position.y()));
    flange_duct_right->wizardParams.insert("Position z", (position.z()));
    flange_duct_right->wizardParams.insert("Angle x", (angle_x));
    flange_duct_right->wizardParams.insert("Angle y", (angle_y));
    flange_duct_right->wizardParams.insert("Angle z", (angle_z));
    flange_duct_right->wizardParams.insert("l", (fe));
    flange_duct_right->wizardParams.insert("b", (b + 2 * ff));
    flange_duct_right->wizardParams.insert("a", (a + 2 * ff));
    flange_duct_right->wizardParams.insert("s", (ff));
    flange_duct_right->layer = this->layer;
    flange_duct_right->processWizardInput();
    flange_duct_right->calculate();

    foreach(CADitem* item, subItems)
    {
        if(item == main_duct || item == flange_duct_left || item == flange_duct_right)
            continue;
        delete item;
    }
    subItems.clear();
    subItems.append(main_duct);
    subItems.append(flange_duct_left);
    subItems.append(flange_duct_right);

    for(int i = 0; i < n; i++)
    {
        CAD_basic_box* plate = new CAD_basic_box();
        subItems.append(plate);
        qreal offset = (b - 2 * s) / (2 * n);
        QVector3D position_sl = position + matrix_rotation * QVector3D(l / 2, b/2 - (2 * i + 1) * offset - s, 0.0);
        plate->wizardParams.insert("Position x", (position_sl.x()));
        plate->wizardParams.insert("Position y", (position_sl.y()));
        plate->wizardParams.insert("Position z", (position_sl.z()));
        plate->wizardParams.insert("Angle x", (angle_x));
        plate->wizardParams.insert("Angle y", (angle_y));
        plate->wizardParams.insert("Angle z", (angle_z));
        plate->wizardParams.insert("Size x", (l));
        plate->wizardParams.insert("Size y", (d));
        plate->wizardParams.insert("Size z", (a - 2 * s));
        plate->layer = this->layer;
        plate->processWizardInput();
        plate->calculate();

    }


    // tbd: not all vertices are needed here!
    boundingBox.enterVertex(flange_duct_left->pos_top_1);
    boundingBox.enterVertex(flange_duct_left->pos_top_2);
    boundingBox.enterVertex(flange_duct_left->pos_top_3);
    boundingBox.enterVertex(flange_duct_left->pos_top_4);
    boundingBox.enterVertex(flange_duct_left->pos_bot_1);
    boundingBox.enterVertex(flange_duct_left->pos_bot_2);
    boundingBox.enterVertex(flange_duct_left->pos_bot_3);
    boundingBox.enterVertex(flange_duct_left->pos_bot_4);

    boundingBox.enterVertex(flange_duct_right->pos_top_1);
    boundingBox.enterVertex(flange_duct_right->pos_top_2);
    boundingBox.enterVertex(flange_duct_right->pos_top_3);
    boundingBox.enterVertex(flange_duct_right->pos_top_4);
    boundingBox.enterVertex(flange_duct_right->pos_bot_1);
    boundingBox.enterVertex(flange_duct_right->pos_bot_2);
    boundingBox.enterVertex(flange_duct_right->pos_bot_3);
    boundingBox.enterVertex(flange_duct_right->pos_bot_4);



    this->snap_vertices.append(main_duct->snap_vertices);
    this->snap_flanges.append(main_duct->snap_flanges);
}

void CAD_air_ductBaffleSilencer::processWizardInput()
{
    position.setX(wizardParams.value("Position x").toDouble());
    position.setY(wizardParams.value("Position y").toDouble());
    position.setZ(wizardParams.value("Position z").toDouble());
    angle_x = wizardParams.value("Angle x").toDouble();
    angle_y = wizardParams.value("Angle y").toDouble();
    angle_z = wizardParams.value("Angle z").toDouble();


    l = wizardParams.value("l").toDouble();
    a = wizardParams.value("a").toDouble();
    b = wizardParams.value("b").toDouble();
    d = wizardParams.value("d").toDouble();
    n = wizardParams.value("n").toDouble();
    fe = wizardParams.value("fe").toDouble();
    ff = wizardParams.value("ff").toDouble();
    s = wizardParams.value("s").toDouble();

}
