#include "itemdb.h"

ItemDB::ItemDB(QObject *parent) :
    QObject(parent)
{
    currentItemId = 0;
    topLevelLayer = new Layer(this);
    topLevelLayer->name = "$$ToplevelLayer";
    topLevelLayer->solo = true;
    layers.append(topLevelLayer);
    layerSoloActive = false;

    this->activeDrawCommand = CADitemTypes::None;

    this->deriveDomainsAndItemTypes();
}

ItemDB::~ItemDB()
{
    // TODO delete all layers incl. sublayers
}

void ItemDB::deriveDomainsAndItemTypes()
{
    CADitem* item;

    int type = (int)CADitemTypes::None + 1;

    for (;;)
    {
        if (type == CADitemTypes::LastItem)
            break;

        item = createItem((CADitemTypes::ItemType)type);
        if (item == NULL)
        {
            QString enumName = CADitemTypes().getEnumNameOfItemType((CADitemTypes::ItemType)type);
            qDebug() << "itemDB: createItem returned NULL; itemtype:" << type << enumName << "not implemented";
            type++;
            continue;
        }
        else
        {
            QString enumName = CADitemTypes().getEnumNameOfItemType((CADitemTypes::ItemType)type);
            qDebug() << "ItemDB::deriveDomainsAndItemTypes()" << enumName;
        }

        itemTypesByDomain.insertMulti(item->domain(), (int)type);
        iconPathByItemType.insert(type, item->iconPath());
        itemDescriptionByItemType.insert(type, item->description());
        delete item;
        type++;
    }

    this->domains = itemTypesByDomain.uniqueKeys();
}


QList<QString> ItemDB::getDomains()
{
    return domains;
}

QList<int> ItemDB::getItemTypesByDomain(QString domain)
{
    return itemTypesByDomain.values(domain);
}

QString ItemDB::getItemDescriptionByItemType(CADitemTypes::ItemType type)
{
    return itemDescriptionByItemType.value((int)type);
}

QString ItemDB::getIconPathByItemType(CADitemTypes::ItemType type)
{
    return iconPathByItemType.value((int)type);
}

QPixmap ItemDB::getIconByItemType(CADitemTypes::ItemType type, QSize size)
{
    QString filename = getIconPathByItemType(type);
    if (QFile(filename).exists())
    {
        QSvgRenderer svgRenderer(filename);
        QPixmap pixmap(size);
        QPainter painter(&pixmap);
        svgRenderer.render(&painter);
        return pixmap;
    }
    else
    {
        QPixmap pixmap(size);
        pixmap.fill(Qt::yellow);
        return pixmap;
    }
}

Layer* ItemDB::addLayer(QString layerName, QString parentLayerName)
{
    // First: check if layer already exists
    if (layerMap.contains(layerName))
        return layerMap.value(layerName);

    // Second: Find parent layer
    Layer* parentLayer = getLayerByName(parentLayerName);
    if (parentLayer == NULL)
        parentLayer = topLevelLayer;

    return addLayer(layerName, parentLayer);
}

Layer *ItemDB::addLayer(QString layerName, Layer *parentLayer)
{
    if (parentLayer == NULL)
        return NULL;

    // First: check if layer already exists
    if (layerMap.contains(layerName))
        return layerMap.value(layerName);

    // Insert Layer in quickfind-map
    Layer* newLayer = new Layer(this);
    newLayer->name = layerName;
    newLayer->parentLayer = parentLayer;
    parentLayer->subLayers.append(newLayer);
    layerMap.insert(layerName, newLayer);
    emit signal_layerAdded(newLayer, parentLayer);
    return newLayer;
}

bool ItemDB::moveLayer(QString layerName, QString newParentLayerName, quint32 position)
{
    Layer* layer = getLayerByName(layerName);
    if (layer == NULL)
        return false;
    if (layer == topLevelLayer)
        return false;

    Layer* oldParentLayer = layer->parentLayer;
    if (oldParentLayer == NULL)
        return false;

    Layer* newParentLayer = getLayerByName(newParentLayerName);
    if (newParentLayer == NULL)
        return false;




    oldParentLayer->subLayers.removeOne(layer);
    newParentLayer->subLayers.insert(position, layer);



    emit signal_layerMoved(layer);
    return true;
}

bool ItemDB::renameLayer(QString layerName, QString newLayerName)
{
    Layer* layer = getLayerByName(layerName);
    if (layer == NULL)
        return false;
    if (layer == topLevelLayer)
        return false;

    return renameLayer(layer, newLayerName);
}

bool ItemDB::renameLayer(Layer *layer, QString newLayerName)
{
    if (layer == NULL)
        return false;
    if (newLayerName.isEmpty())
        return false;

    layerMap.remove(layer->name);
    layer->name = newLayerName;
    layerMap.insert(layer->name, layer);
    emit signal_layerChanged(layer);
    return true;
}

bool ItemDB::deleteLayer(Layer *layer)
{
    Layer* parentLayer = layer->parentLayer;
    if (parentLayer == NULL)
        return false;

    if (!layer->isEmpty())
        return false;

    if (parentLayer->subLayers.removeOne(layer))
    {
        layerMap.remove(layer->name);
        delete layer;
        emit signal_layerDeleted(layer);
        return true;
    }
    else
        return false;
}

Layer* ItemDB::getLayerByName(QString layerName)
{
    if (layerName.isEmpty())
        return topLevelLayer;
    else
        return layerMap.value(layerName, NULL);
}

Layer* ItemDB::getTopLevelLayer()
{
    return topLevelLayer;
}

bool ItemDB::isLayerValid(Layer *layer)
{
    if (layerMap.values().contains(layer))
        return true;
    else
        return false;
}

void ItemDB::addItem(CADitem* item, QString LayerName)
{
    Layer* layer = getLayerByName(LayerName);
    if (layer == NULL)
        layer = topLevelLayer;

    this->addItem(item, layer);
}

void ItemDB::addItem(CADitem *item, Layer *layer)
{
    if (layer == NULL)
    {
        qDebug() << "ItemDB::addItem(): layer is NULL.";
        return;
    }

//    item->layerName = layer->name;
    item->setLayer(layer);
    item->setID(currentItemId);
    itemMap.insert(item->id, item);
    currentItemId++;
    layer->items.append(item);
    emit signal_itemAdded(item, layer);
}

void ItemDB::deleteItem(CADitem *item)
{
//    Layer* layer = getLayerByName(item->layerName);
    Layer* layer = item->layer;

    layer->items.removeOne(item);
    itemMap.remove(item->id);

    foreach (CADitem* subItem, item->subItems)
    {
        deleteItem(subItem);
    }

    signal_itemDeleted(item);
    delete item;
}

bool ItemDB::deleteItem(quint64 id)
{
    CADitem* item = getItemById(id);

    if (item == NULL)
        return false;

    deleteItem(item);
    return true;
}

void ItemDB::deleteItems(QList<CADitem *> items)
{
    foreach (CADitem* item, items)
        deleteItem(item);
}

bool ItemDB::changeLayerOfItem(CADitem *item, Layer *newLayer)
{
    if (item == NULL)
        return false;
    if (newLayer == NULL)
        return false;

//    Layer* oldLayer = getLayerByName(item->layerName);
    Layer* oldLayer = item->layer;
    if (oldLayer == NULL)
        return false;

    oldLayer->items.removeOne(item);
//    item->layerName = newLayer->name;
    item->setLayer(newLayer);
    newLayer->items.append(item);
    emit signal_repaintNeeded();
    return true;
}

bool ItemDB::changeLayerOfItem(quint64 id, QString newLayerName)
{
    CADitem* item = getItemById(id);
    Layer* newLayer = getLayerByName(newLayerName);
    return changeLayerOfItem(item, newLayer);
}

CADitem *ItemDB::createItem(CADitemTypes::ItemType type)
{
    CADitem* newItem = NULL;

    switch (type)
    {
    case CADitemTypes::Basic_Arc:
        newItem = new CAD_basic_arc();
        break;
    case CADitemTypes::Basic_Box:
        newItem = new CAD_basic_box();
        break;
    case CADitemTypes::Basic_Cylinder:
        newItem = new CAD_basic_cylinder();
        break;
    case CADitemTypes::Basic_Circle:
        newItem = new CAD_basic_circle();
        break;
    case CADitemTypes::Basic_Duct:
        newItem = new CAD_basic_duct();
        break;
    case CADitemTypes::Basic_Face:
        newItem = new CAD_basic_3Dface();
        break;
    case CADitemTypes::Basic_Line:
        newItem = new CAD_basic_line();
        break;
    case CADitemTypes::Basic_Pipe:
        newItem = new CAD_basic_pipe();
        break;
    case CADitemTypes::Basic_Plane:
        newItem = new CAD_basic_plane();
        break;
    case CADitemTypes::Basic_Point:
        newItem = new CAD_basic_point();
        break;
    case CADitemTypes::Basic_Polyline:
        newItem = new CAD_basic_polyline();
        break;
    case CADitemTypes::Basic_Sphere:
        newItem = new CAD_basic_sphere();
        break;
    case CADitemTypes::Basic_Turn:
        newItem = new CAD_basic_turn();
        break;




    case CADitemTypes::Arch_Beam:
        newItem = new CAD_arch_beam();
        break;
    case CADitemTypes::Arch_BlockOut:
        newItem = new CAD_arch_blockOut();
        break;
    case CADitemTypes::Arch_BoredPile:
        newItem = new CAD_arch_boredPile();
        break;
    case CADitemTypes::Arch_Door:
        newItem = new CAD_arch_door();
        break;
    case CADitemTypes::Arch_Foundation:
        newItem = new CAD_arch_foundation();
        break;
    case CADitemTypes::Arch_Grating:
        newItem = new CAD_arch_grating();
        break;
    case CADitemTypes::Arch_LevelSlab:
        newItem = new CAD_arch_levelSlab();
        break;
    case CADitemTypes::Arch_Support:
        newItem = new CAD_arch_support();
        break;
    case CADitemTypes::Arch_Wall_loadBearing:
        newItem = new CAD_arch_wall_loadBearing();
        break;
    case CADitemTypes::Arch_Wall_nonLoadBearing:
        newItem = new CAD_arch_wall_nonLoadBearing();
        break;
    case CADitemTypes::Arch_Window:
        newItem = new CAD_arch_window();
        break;

    case CADitemTypes::Air_Duct:
        newItem = new CAD_air_duct();
        break;
    case CADitemTypes::Air_Pipe:
        newItem = new CAD_air_pipe();
        break;
    case CADitemTypes::Air_DuctFireResistant:
        newItem = new CAD_air_ductFireResistant();
        break;
    case CADitemTypes::Air_DuctTurn:
        newItem = new CAD_air_ductTurn();
        break;
    case CADitemTypes::Air_PipeTurn:
        newItem = new CAD_air_pipeTurn();
        break;
    case CADitemTypes::Air_PipeReducer:
        newItem = new CAD_air_pipeReducer();
        break;
    case CADitemTypes::Air_PipeTeeConnector:
        newItem = new CAD_air_pipeTeeConnector();
        break;
    case CADitemTypes::Air_DuctTeeConnector:
        newItem = new CAD_air_ductTeeConnector();
        break;
    case CADitemTypes::Air_DuctTransition:
        newItem = new CAD_air_ductTransition();
        break;
    case CADitemTypes::Air_DuctTransitionRectRound:
        newItem = new CAD_air_ductTransitionRectRound();
        break;
    case CADitemTypes::Air_DuctYpiece:
        newItem = new CAD_air_ductYpiece();
        break;
    case CADitemTypes::Air_DuctEndPlate:
        newItem = new CAD_air_ductEndPlate();
        break;
    case CADitemTypes::Air_LinearDiffuser:
        newItem = new CAD_air_lineardiffuser();
        break;
    case CADitemTypes::Air_PipeEndCap:
        newItem = new CAD_air_pipeEndCap();
        break;
    case CADitemTypes::Air_ThrottleValve:
        newItem = new CAD_air_throttleValve();
        break;
    case CADitemTypes::Air_MultiLeafDamper:
        newItem = new CAD_air_multiLeafDamper();
        break;
    case CADitemTypes::Air_PressureReliefDamper:
        newItem = new CAD_air_pressureReliefDamper();
        break;
    case CADitemTypes::Air_PipeFireDamper:
        newItem = new CAD_air_pipeFireDamper();
        break;
    case CADitemTypes::Air_PipeBranch:
        newItem = new CAD_air_pipeBranch();
        break;
    case CADitemTypes::Air_DuctFireDamper:
        newItem = new CAD_air_ductFireDamper();
        break;
    case CADitemTypes::Air_DuctVolumetricFlowController:
        newItem = new CAD_air_ductVolumetricFlowController();
        break;
    case CADitemTypes::Air_PipeVolumetricFlowController:
        newItem = new CAD_air_pipeVolumetricFlowController();
        break;
    case CADitemTypes::Air_HeatExchangerWaterAir:
        newItem = new CAD_air_heatExchangerWaterAir();
        break;
    case CADitemTypes::Air_HeatExchangerAirAir:
        newItem = new CAD_air_heatExchangerAirAir();
        break;
    case CADitemTypes::Air_CanvasFlange:
        newItem = new CAD_air_canvasFlange();
        break;
    case CADitemTypes::Air_Filter:
        newItem = new CAD_air_filter();
        break;
    case CADitemTypes::Air_PipeSilencer:
        newItem = new CAD_air_pipeSilencer();
        break;
    case CADitemTypes::Air_DuctBaffleSilencer:
        newItem = new CAD_air_ductBaffleSilencer();
        break;
    case CADitemTypes::Air_Fan:
        newItem = new CAD_air_fan();
        break;
    case CADitemTypes::Air_Humidifier:
        newItem = new CAD_air_humidifier();
        break;
    case CADitemTypes::Air_EmptyCabinet:
        newItem = new CAD_air_emptyCabinet();
        break;
    case CADitemTypes::Air_EquipmentFrame:
        newItem = new CAD_air_equipmentFrame();
        break;

    case CADitemTypes::HeatCool_Adjustvalve:
        newItem = new CAD_heatcool_adjustvalve();
        break;
    case CADitemTypes::HeatCool_Chiller:
        newItem = new CAD_heatcool_chiller();
        break;
    case CADitemTypes::HeatCool_Controlvalve:
        newItem = new CAD_heatcool_controlvalve();
        break;
    case CADitemTypes::HeatCool_CoolingTower:
        newItem = new CAD_heatcool_coolingTower();
        break;
    case CADitemTypes::HeatCool_HeatExchanger:
        newItem = new CAD_heatcool_heatExchanger();
        break;
    case CADitemTypes::HeatCool_Pipe:
        newItem = new CAD_heatcool_pipe();
        break;
    case CADitemTypes::HeatCool_Pump:
        newItem = new CAD_heatcool_pump();
        break;
    case CADitemTypes::HeatCool_Sensor:
        newItem = new CAD_heatcool_sensor();
        break;
    case CADitemTypes::HeatCool_PipeTurn:
        newItem = new CAD_heatcool_pipeTurn();
        break;
    case CADitemTypes::HeatCool_PipeReducer:
        newItem = new CAD_heatcool_pipeReducer();
        break;
    case CADitemTypes::HeatCool_PipeTeeConnector:
        newItem = new CAD_heatcool_pipeTeeConnector();
        break;
    case CADitemTypes::HeatCool_PipeEndCap:
        newItem = new CAD_heatcool_pipeEndCap();
        break;
    case CADitemTypes::HeatCool_Flange:
        newItem = new CAD_heatcool_flange();
        break;
    case CADitemTypes::HeatCool_ExpansionChamber:
        newItem = new CAD_heatcool_expansionChamber();
        break;
    case CADitemTypes::HeatCool_Boiler:
        newItem = new CAD_heatcool_boiler();
        break;
    case CADitemTypes::HeatCool_WaterHeater:
        newItem = new CAD_heatcool_waterHeater();
        break;
    case CADitemTypes::HeatCool_StorageBoiler:
        newItem = new CAD_heatcool_storageBoiler();
        break;
    case CADitemTypes::HeatCool_Radiator:
        newItem = new CAD_heatcool_radiator();
        break;
    case CADitemTypes::HeatCool_Filter:
        newItem = new CAD_heatcool_filter();
        break;
    case CADitemTypes::HeatCool_BallValve:
        newItem = new CAD_heatcool_ballValve();
        break;
    case CADitemTypes::HeatCool_ButterflyValve:
        newItem = new CAD_heatcool_butterflyValve();
        break;
    case CADitemTypes::HeatCool_SafetyValve:
        newItem = new CAD_heatcool_safetyValve();
        break;
    case CADitemTypes::HeatCool_Flowmeter:
        newItem = new CAD_heatcool_flowmeter();
        break;

    case CADitemTypes::Sprinkler_CompressedAirWaterContainer:
        newItem = new CAD_sprinkler_compressedAirWaterContainer();
        break;
    case CADitemTypes::Sprinkler_Distribution:
        newItem = new CAD_sprinkler_distribution();
        break;
    case CADitemTypes::Sprinkler_Head:
        newItem = new CAD_sprinkler_head();
        break;
    case CADitemTypes::Sprinkler_Pipe:
        newItem = new CAD_sprinkler_pipe();
        break;
    case CADitemTypes::Sprinkler_Pump:
        newItem = new CAD_sprinkler_pump();
        break;
    case CADitemTypes::Sprinkler_TeeConnector:
        newItem = new CAD_sprinkler_teeConnector();
        break;
    case CADitemTypes::Sprinkler_Valve:
        newItem = new CAD_sprinkler_valve();
        break;
    case CADitemTypes::Sprinkler_WetAlarmValve:
        newItem = new CAD_sprinkler_wetAlarmValve();
        break;
    case CADitemTypes::Sprinkler_ZoneCheck:
        newItem = new CAD_sprinkler_zoneCheck();
        break;
    case CADitemTypes::Sprinkler_PipeTurn:
        newItem = new CAD_sprinkler_pipeTurn();
        break;
    case CADitemTypes::Sprinkler_PipeEndCap:
        newItem = new CAD_sprinkler_pipeEndCap();
        break;
    case CADitemTypes::Sprinkler_PipeReducer:
        newItem = new CAD_sprinkler_pipeReducer();
        break;

    case CADitemTypes::Electrical_Cabinet:
        newItem = new CAD_electrical_cabinet();
        break;
    case CADitemTypes::Electrical_CableTray:
        newItem = new CAD_electrical_cableTray();
        break;
    default:
    {
        qDebug() << "ItemDB::drawItem(): unknown item type.";
        return NULL;
    }
        break;
    }

    return newItem;
}

CADitem* ItemDB::drawItem(Layer* layer, CADitemTypes::ItemType type)
{
    if (layer == NULL)
    {
        qDebug() << "ItemDB::drawItem(): layer is NULL.";
        return NULL;
    }

    this->activeDrawCommand = type;

    CADitem* newItem = this->createItem(type);

    this->addItem(newItem, layer);

    return newItem;
}

CADitem *ItemDB::drawItem(QString layerName, CADitemTypes::ItemType type)
{
    Layer* layer = getLayerByName(layerName);
    if (layer == NULL)
        layer = topLevelLayer;
    return this->drawItem(layer, type);
}

CADitem *ItemDB::getItemById(quint64 id)
{
    return itemMap.value(id, NULL);
}

bool ItemDB::modifyItem(quint64 &id, QString &key, QString &value)
{
    CADitem* item = getItemById(id);
    if (item == NULL)
        return false;

    QVariant oldValue = item->wizardParams.value(key);

    if (!oldValue.isValid())
        return false;

    switch (oldValue.type())
    {
    case QVariant::Double:
        item->wizardParams.insert(key, QVariant(value.toDouble()));
        break;
    case QVariant::String:
        item->wizardParams.insert(key, value);
        break;
    default:
        return false;
        break;
    }

    item->processWizardInput();
    item->calculate();
    return true;
}

void ItemDB::itemAdded(CADitem *item)
{

}

void ItemDB::itemModified(CADitem *item)
{

}

void ItemDB::itemDeleted(CADitem *item)
{

}

QByteArray ItemDB::network_newLayer(QMap<QString, QString> data)
{
    QString newLayerName = data.value("newLayer");
    if (getLayerByName(newLayerName) != NULL)
        return "Error: Layer already exists.\n";

    QString parentLayerName = data.value("parentLayer");

    Layer* newLayer = addLayer(newLayerName, parentLayerName);
    // tbd: set layer properties
    return "Ok\n";  // tbd: Broadcast response
}

QByteArray ItemDB::network_modifyLayer(QMap<QString, QString> data)
{
    QString layerName = data.value("Layer");
    Layer* layer = getLayerByName(layerName);
    if ((layer == NULL) || (layer == topLevelLayer))
        return "Error: Layer does not exist. Unable to modify it.\n";

    QByteArray answer;
    bool repaintNeeded = false;

    if (data.contains("pen"))
    {
        QColor color_pen;
        color_pen = QColor(data.value("pen"));
        layer->pen.setColor(color_pen);
        repaintNeeded = true;
    }
    if (data.contains("brush"))
    {
        QColor color_brush;
        color_brush = QColor(data.value("brush"));
        layer->brush.setColor(color_brush);
        repaintNeeded = true;
    }
    if (data.contains("lineWidth"))
    {
        layer->width = data.value("lineWidth").toInt();
        repaintNeeded = true;
    }
    if (data.contains("lineType"))
    {
        layer->lineType = data.value("lineType");
        repaintNeeded = true;
    }
    if (data.contains("name"))
    {
        bool result = renameLayer(layer, data.value("name"));
        if (result == false)
            answer += "Error: Unable to rename Layer.\n";
    }

    emit signal_layerChanged(layer);
    if (repaintNeeded)
        emit signal_repaintNeeded();

    if (answer.isEmpty())
        answer = "Ok\n";

    return answer;
}

QByteArray ItemDB::network_moveLayer(QMap<QString, QString> data)
{
    QString layerName = data.value("Layer");
    Layer* layer = getLayerByName(layerName);
    if (layer == NULL)
        return "Error: Layer does not exist. Unable to delete it.\n";

    QString newParentLayerName = data.value("newParent");
    quint32 pos = data.value("Pos").toUInt();

    bool result = moveLayer(layerName, newParentLayerName, pos);
    if (result == false)
        return "Error: Unable to move layer.\n";
    else
        return "Ok\n";  // tbd: Broadcast response
}

QByteArray ItemDB::network_deleteLayer(QMap<QString, QString> data)
{
    QString layerName = data.value("Layer");
    Layer* layer = getLayerByName(layerName);
    if (layer == NULL)
        return "Error: Layer does not exist. Unable to delete it.\n";

    bool result = deleteLayer(layer);
    if (result == false)
        return "Error: Unable to delete layer. May be it is not empty.\n";
    else
        return "Ok\n";  // tbd: Broadcast response
}

QByteArray ItemDB::network_getAll()
{
    QByteArray answer;

    network_getAll_processLayers(this->layers, &answer);

    return answer;
}

QByteArray ItemDB::network_getItem(quint64 id)
{
    CADitem* item = getItemById(id);
    if (item == NULL)
        return "Error in network_getItem(" + QByteArray().setNum(id) + ")\n";

    QByteArray answer;

    QList<CADitem*> items;
    items.append(item);

    network_getAll_processItems(items, &answer);

    return answer;
}

QByteArray ItemDB::network_newItem(quint32 type, QMap<QString, QString> data)
{
    QString layerName;
    layerName = data.value("Layer");
    CADitem* newItem = drawItem(layerName, (CADitemTypes::ItemType) type);

    if (newItem == NULL)
        return "Error in network_newItem()\n";

    data.remove("Layer");
    network_modifyItem(newItem->id, data);
    QByteArray answer;
    answer = "N id " + QByteArray().setNum(newItem->id) + "\n";
    return answer;
}

QByteArray ItemDB::network_modifyItem(quint64 id, QMap<QString, QString> data)
{
    QByteArray answer;

    QList<QString> keys = data.keys();

    foreach (QString key, keys)
    {
        QString value = data.value(key);
        bool result = modifyItem(id, key, value);
        if (result == false)
            answer += "Error in modifyItem(" + QByteArray().setNum(id) + " " + key.toUtf8() + " " + value.toUtf8() + ")\n";
    }

    emit signal_repaintNeeded();

    if (answer.isEmpty())
        answer = "Ok\n";

    return answer;
}

QByteArray ItemDB::network_changeLayerOfItem(quint64 id, QMap<QString, QString> data)
{
    QString newLayerName = data.value("newLayer");
    bool result = changeLayerOfItem(id, newLayerName);
    if (result == true)
        return "Ok\n";  // tbd: Broadcast response
    else
        return QByteArray("Error in changeLayerOfItem(" + QByteArray().setNum(id) + ", " + newLayerName.toUtf8() + ")\n");
}

QByteArray ItemDB::network_deleteItem(quint64 id)
{
    bool result = deleteItem(id);
    if (result == true)
    {
        emit signal_repaintNeeded();
        return "Ok\n";
    }
    else
        return QByteArray("Error while deleting item ") + QByteArray().setNum(id) + "\n";
}

bool ItemDB::file_storeDB(QString filename)
{
    QDomDocument document;
    QDomElement root = document.createElement("SmePowerCadProject");
    document.appendChild(root);
    root.setAttribute("Version", QString());    //tbd.

    file_storeDB_processLayers(document, root, this->topLevelLayer->subLayers);

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&file);
    document.save(stream, 1);   // Indent = 1
    file.close();
    return true;
}

void ItemDB::file_storeDB_processLayers(QDomDocument document, QDomElement parentElement, QList<Layer *> layers)
{
    foreach (Layer* layer, layers)
    {
        QDomElement element = document.createElement("L");
        parentElement.appendChild(element);

        element.setAttribute("Name", layer->name);
        element.setAttribute("FillColor", layer->brush.color().name());
        element.setAttribute("OutlineColor", layer->pen.color().name());
        element.setAttribute("LineWidth", layer->width);
        element.setAttribute("LineType", layer->lineType);

        file_storeDB_processLayers(document, element, layer->subLayers);
        file_storeDB_processItems(document, element, layer->items);
    }
}

void ItemDB::file_storeDB_processItems(QDomDocument document, QDomElement parentElement, QList<CADitem *> items)
{
    foreach (CADitem* item, items)
    {
        QDomElement element = document.createElement(QString().sprintf("I%d", (unsigned int)item->getType()));
        parentElement.appendChild(element);

        foreach (QString key, item->wizardParams.keys())
        {
            element.setAttribute(key.replace(' ', '_'), item->wizardParams.value(key).toString());
        }

        // Do not store subitems as they are recovered automatically when loading the parent item
//        file_storeDB_processItems(document, element, item->subItems);
    }
}

bool ItemDB::file_loadDB(QString filename)
{
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    QString errorStr;
    int errorLine;
    int errorColumn;

    QDomDocument document;
    if (!document.setContent(&file, true, &errorStr, &errorLine, &errorColumn))
    {
        file.close();
//        QMessageBox::information(this, tr("Error while reading xml-file"),
//                                 tr("line %1, column %2:\n%3")
//                                 .arg(errorLine)
//                                 .arg(errorColumn)
//                                 .arg(errorStr));
        return false;
    }

    QDomElement root = document.documentElement();
    if (root.tagName() != "SmePowerCadProject")
    {
        file.close();
//        QMessageBox::information(this, tr("Error"),
//                                 tr("Root-Node has wrong tagName."));
        return false;
    }
    else if (root.hasAttribute("Version") && root.attribute("Version") != "")
    {
        file.close();
//        QMessageBox::information(this, tr("Error"),
//                                 tr("Invalid file version"));
        return false;
    }

    QDomElement child = root.firstChildElement("");
    while (!child.isNull())
    {
        this->file_loadDB_parseDomElement(child, this->topLevelLayer);  // tbd. toplevellayer may be wrong here...
        child = child.nextSiblingElement();
    }

    file.close();
    emit signal_layerManagerUpdateNeeded();
    return true;
}

void ItemDB::file_loadDB_parseDomElement(QDomElement element, Layer *currentLayer)
{
    QString tagName = element.tagName();
    if (tagName == "L")
    {
        Layer* newLayer = this->addLayer(element.attribute("Name"), currentLayer);
        newLayer->brush.setColor(QColor(element.attribute("FillColor")));
        newLayer->pen.setColor(QColor(element.attribute("OutlineColor")));
        newLayer->width = element.attribute("LineWidth").toDouble();
        newLayer->lineType = element.attribute("LineType");
        currentLayer = newLayer;
    }
    else if (tagName.startsWith('I'))
    {
        tagName.remove(0, 1);   // Strip "I"
        int itemType = tagName.toInt();
        CADitem* item = this->drawItem(currentLayer, (CADitemTypes::ItemType)itemType);
        foreach (QString key, item->wizardParams.keys())
        {
            QString elementKey = key;
            elementKey.replace(' ', '_');

            switch (item->wizardParams.value(key).type())
            {
            case QVariant::String:
                item->wizardParams.insert(key, QString(element.attribute(elementKey)));
                break;
            case QVariant::Int:
                item->wizardParams.insert(key, QString(element.attribute(elementKey)).toInt());
                break;
            case QVariant::Double:
                item->wizardParams.insert(key, QString(element.attribute(elementKey)).toDouble());
                break;
            default:
                qDebug() << "ItemDB::file_loadDB_parseDomElement() Unhandled value type:" << item->wizardParams.value(key).type();
                break;
            }
        }

        item->processWizardInput();
        item->calculate();
    }


    QDomElement child = element.firstChildElement();
    while (!child.isNull())
    {
        this->file_loadDB_parseDomElement(child, currentLayer);
        child = child.nextSiblingElement();
    }
}

void ItemDB::network_getAll_processLayers(QList<Layer *> layers, QByteArray* answer)
{
    foreach (Layer* layer, layers)
    {
        layer->serialOut(answer);
        network_getAll_processItems(layer->items, answer);
        network_getAll_processLayers(layer->subLayers, answer);
    }
}

void ItemDB::network_getAll_processItems(QList<CADitem *> items, QByteArray* answer)
{
    foreach (CADitem* item, items)
    {
        item->serialOut(answer);
        network_getAll_processItems(item->subItems, answer);
    }
}
