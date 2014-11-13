#include "cad_air_duct.h"

CAD_air_duct::CAD_air_duct() : CADitem(CADitem::Air_Duct)
{
    this->description = "Air|Duct";
    wizardParams.insert("Position x", QVariant::fromValue(0.0));
    wizardParams.insert("Position y", QVariant::fromValue(0.0));
    wizardParams.insert("Position z", QVariant::fromValue(0.0));

    wizardParams.insert("Angle x", QVariant::fromValue(0.0));
    wizardParams.insert("Angle y", QVariant::fromValue(0.0));
    wizardParams.insert("Angle z", QVariant::fromValue(0.0));

    wizardParams.insert("Wall thickness", QVariant::fromValue(1.0));
    wizardParams.insert("Length (l)", QVariant::fromValue(10.0));
    wizardParams.insert("Width (b)", QVariant::fromValue(5.0));
    wizardParams.insert("Height (a)", QVariant::fromValue(5.0));
    wizardParams.insert("Flange size", QVariant::fromValue(1.0));


    main_duct = new CAD_basic_duct();
    flange_duct_left = new CAD_basic_duct();
    flange_duct_right = new CAD_basic_duct();

    subItems.append(main_duct);
    subItems.append(flange_duct_left);
    subItems.append(flange_duct_right);

    processWizardInput();
    calculate();
}

CAD_air_duct::~CAD_air_duct()
{
}

QList<CADitem::ItemType> CAD_air_duct::flangable_items()
{
    QList<CADitem::ItemType> flangable_items;
    flangable_items.append(CADitem::Air_Duct);
    flangable_items.append(CADitem::Air_DuctEndPlate);
    flangable_items.append(CADitem::Air_DuctBaffleSilencer);
    flangable_items.append(CADitem::Air_DuctFireDamper);
    flangable_items.append(CADitem::Air_DuctTeeConnector);
    flangable_items.append(CADitem::Air_DuctTransition);
    flangable_items.append(CADitem::Air_DuctTransitionRectRound);
    flangable_items.append(CADitem::Air_DuctTurn);
    flangable_items.append(CADitem::Air_DuctVolumetricFlowController);
    flangable_items.append(CADitem::Air_DuctYpiece);
    flangable_items.append(CADitem::Air_Filter);
    flangable_items.append(CADitem::Air_HeatExchangerAirAir);
    flangable_items.append(CADitem::Air_HeatExchangerWaterAir);
    flangable_items.append(CADitem::Air_MultiLeafDamper);
    return flangable_items;
}

QImage CAD_air_duct::wizardImage()
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

void CAD_air_duct::calculate()
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



    main_duct->wizardParams.insert("Position x", QVariant::fromValue(position.x()));
    main_duct->wizardParams.insert("Position y", QVariant::fromValue(position.y()));
    main_duct->wizardParams.insert("Position z", QVariant::fromValue(position.z()));
    main_duct->wizardParams.insert("Angle x", QVariant::fromValue(angle_x));
    main_duct->wizardParams.insert("Angle y", QVariant::fromValue(angle_y));
    main_duct->wizardParams.insert("Angle z", QVariant::fromValue(angle_z));
    main_duct->wizardParams.insert("Length (l)", QVariant::fromValue(length));
    main_duct->wizardParams.insert("Width (b)", QVariant::fromValue(width));
    main_duct->wizardParams.insert("Height (a)", QVariant::fromValue(height));
    main_duct->wizardParams.insert("Wall thickness", QVariant::fromValue(wall_thickness));
    main_duct->processWizardInput();
    main_duct->calculate();


    QVector3D pos_rot = position + (QVector3D(-(length - flange_size), 0.0, 0.0));
    flange_duct_left->wizardParams.insert("Position x", QVariant::fromValue(pos_rot.x()));
    flange_duct_left->wizardParams.insert("Position y", QVariant::fromValue(pos_rot.y()));
    flange_duct_left->wizardParams.insert("Position z", QVariant::fromValue(pos_rot.z()));
    flange_duct_left->wizardParams.insert("Angle x", QVariant::fromValue(angle_x));
    flange_duct_left->wizardParams.insert("Angle y", QVariant::fromValue(angle_y));
    flange_duct_left->wizardParams.insert("Angle z", QVariant::fromValue(angle_z));
    flange_duct_left->wizardParams.insert("Length (l)", QVariant::fromValue(flange_size));
    flange_duct_left->wizardParams.insert("Width (b)", QVariant::fromValue(width + 2 * flange_size));
    flange_duct_left->wizardParams.insert("Height (a)", QVariant::fromValue(height + 2 * flange_size));
    flange_duct_left->wizardParams.insert("Wall thickness", QVariant::fromValue(flange_size));
    flange_duct_left->processWizardInput();
    flange_duct_left->calculate();

    flange_duct_right->wizardParams.insert("Position x", QVariant::fromValue(position.x()));
    flange_duct_right->wizardParams.insert("Position y", QVariant::fromValue(position.y()));
    flange_duct_right->wizardParams.insert("Position z", QVariant::fromValue(position.z()));
    flange_duct_right->wizardParams.insert("Angle x", QVariant::fromValue(angle_x));
    flange_duct_right->wizardParams.insert("Angle y", QVariant::fromValue(angle_y));
    flange_duct_right->wizardParams.insert("Angle z", QVariant::fromValue(angle_z));
    flange_duct_right->wizardParams.insert("Length (l)", QVariant::fromValue(flange_size));
    flange_duct_right->wizardParams.insert("Width (b)", QVariant::fromValue(width + 2 * flange_size));
    flange_duct_right->wizardParams.insert("Height (a)", QVariant::fromValue(height + 2 * flange_size));
    flange_duct_right->wizardParams.insert("Wall thickness", QVariant::fromValue(flange_size));
    flange_duct_right->processWizardInput();
    flange_duct_right->calculate();
    qDebug() << "angle x" << angle_x;


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

void CAD_air_duct::processWizardInput()
{
    position.setX(wizardParams.value("Position x").toDouble());
    position.setY(wizardParams.value("Position y").toDouble());
    position.setZ(wizardParams.value("Position z").toDouble());
    angle_x = wizardParams.value("Angle x").toDouble();
    angle_y = wizardParams.value("Angle y").toDouble();
    angle_z = wizardParams.value("Angle z").toDouble();

    wall_thickness = wizardParams.value("Wall thickness").toDouble();
    length = wizardParams.value("Length (l)").toDouble();
    width = wizardParams.value("Width (b)").toDouble();
    height = wizardParams.value("Height (a)").toDouble();
    flange_size = wizardParams.value("Flange size").toDouble();


//    size.setX(wizardParams.value("Length (l)").toDouble());
//    size.setY(wizardParams.value("Width (b)").toDouble());
//    size.setZ(wizardParams.value("Height (a)").toDouble());
}
