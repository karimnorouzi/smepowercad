#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QDebug>
#include <QEvent>
#include <QPaintEvent>
#include <QMatrix4x4>
#include <QMessageBox>

#include <qmath.h>
#include <qgl.h>
#include <GL/glu.h>

#include "itemdb.h"
#include "snapengine.h"


class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    enum CuttingPlane{CuttingPlane_pX = 0, CuttingPlane_nX = 1, CuttingPlane_pY = 2, CuttingPlane_nY = 3, CuttingPlane_pZ = 4, CuttingPlane_nZ = 5, CuttingPlane_ISO_001 = 6};


    explicit GLWidget(QWidget *parent, ItemDB *itemDB);
    ~GLWidget();

    QPointF mapFromScene(QVector3D scenePoint);

    // Overlay
    void moveCursor(QPoint pos);
    void hideCursor();
    void pickStart();
    void pickEnd();
    bool isPickActive();
    QRect selection();
    Qt::ItemSelectionMode selectionMode();

    enum SnapMode{SnapNo, SnapEndpoint, SnapCenter, SnapBasepoint};
    void snap_enable(bool on);
    void set_snap_mode(SnapMode mode);
    void set_snapPos(QPoint snapPos);
    void set_WorldRotation(float rot_x, float rot_y, float rot_z);

    QMap<QString, QString> getOpenGLinfo();

private:
    ItemDB* itemDB;
    QPoint mousePosOld;
    CuttingPlane cuttingplane;
    qreal height_of_intersection;
    qreal depth_of_view;
//    QList<Layer*> layers;
    QPoint translationOffset;
    qreal zoomFactor;
    QVector3D centerOfViewInScene;  // in coordsOnScene
    QPoint displayCenter;           // The Center of the widget in PixelsOnScreen, related to bottomLeft of Widget
    QVector3D cameraPosition;
    float rot_x, rot_y, rot_z;

    QMatrix4x4 matrix_projection;
    QMatrix4x4 matrix_modelview;

    bool render_solid;
    bool render_outline;

    GLuint tile_list;

    QMap<GLuint, CADitem*> glNameMap;

    // Overlay
    QPoint mousePos;
    QPoint pickStartPos;
    bool pickActive;
    bool cursorShown;
    SnapMode snapMode;
    QPoint snapPos;

    SnapEngine* snapEngine;
    CADitem* item_lastHighlight;


    // OpenGL
    void saveGLState();
    void restoreGLState();

    void paintContent(QList<Layer*> layers);


    void paintLine(Layer* layer, CAD_basic_line* item);
    void paintPolyLine(Layer *layer, CAD_basic_polyline *item);
    void paintFace(Layer *layer, CAD_basic_3Dface *item);
    void paintBasicCircle(Layer *layer, CAD_basic_circle *item);
    void paintBasicBox(Layer *layer, CAD_basic_box *item);
    void paintBasicCylinder(Layer *layer, CAD_basic_cylinder *item);
    void paintBasicSphere(Layer *layer, CAD_basic_sphere *item);

    void paintArchLevelSlab(Layer *layer, CAD_arch_levelSlab *item);
    void paintArchWallLoadBearing(Layer *layer, CAD_arch_wall_loadBearing *item);
    void paintArchWallNonLoadBearing(Layer *layer, CAD_arch_wall_nonLoadBearing *item);
    void paintArchBlockOut(Layer *layer, CAD_arch_blockOut *item);
    void paintArchDoor(Layer *layer, CAD_arch_door *item);
    void paintArchWindow(Layer *layer, CAD_arch_window *item);

    void paintAirDuct(Layer *layer, CAD_air_duct *item);
    void paintAirPipe(Layer *layer, CAD_air_pipe *item);
    void paintAirDuctTurn(Layer *layer, CAD_air_ductTurn *item);
    void paintAirPipeTurn(Layer *layer, CAD_air_pipeTurn *item);
    void paintAirPipeReducer(Layer *layer, CAD_air_pipeReducer *item);
    void paintAirPipeTeeConnector(Layer *layer, CAD_air_pipeTeeConnector *item);
    void paintAirDuctTeeConnector(Layer *layer, CAD_air_ductTeeConnector *item);
    void paintAirDuctTransiton(Layer *layer, CAD_air_ductTransition *item);
    void paintAirDuctTransitionRectRound(Layer *layer, CAD_air_ductTransitionRectRound *item);
    void paintAirDuctYpiece(Layer *layer, CAD_air_ductYpiece *item);
    void paintAirDuctEndPlate(Layer *layer, CAD_air_ductEndPlate *item);
    void paintAirPipeEndCap(Layer *layer, CAD_air_pipeEndCap *item);
    void paintAirThrottleValve(Layer *layer, CAD_air_throttleValve *item);
    void paintAirMultiLeafDamper(Layer *layer, CAD_air_multiLeafDamper *item);
    void paintAirPressureReliefDamper(Layer *layer, CAD_air_pressureReliefDamper *item);
    void paintAirPipeFireDamper(Layer *layer, CAD_air_pipeFireDamper *item);
    void paintAirDuctFireDamper(Layer *layer, CAD_air_ductFireDamper *item);
    void paintAirDuctVolumetricFlowController(Layer *layer, CAD_air_ductVolumetricFlowController *item);
    void paintAirPipeVolumetricFlowController(Layer *layer, CAD_air_pipeVolumetricFlowController *item);
    void paintAirHeatExchangerWaterAir(Layer *layer, CAD_air_heatExchangerWaterAir *item);
    void paintAirHeatExchangerAirAir(Layer *layer, CAD_air_heatExchangerAirAir *item);
    void paintAirCanvasFlange(Layer *layer, CAD_air_canvasFlange *item);
    void paintAirFilter(Layer *layer, CAD_air_filter *item);
    void paintAirPipeSilencer(Layer *layer, CAD_air_pipeSilencer *item);
    void paintAirDuctBaffleSilencer(Layer *layer, CAD_air_ductBaffleSilencer *item);
    void paintAirFan(Layer *layer, CAD_air_fan *item);
    void paintAirHumidifier(Layer *layer, CAD_air_humidifier *item);
    void paintAirEmptyCabinet(Layer *layer, CAD_air_emptyCabinet *item);
    void paintAirEquipmentFrame(Layer *layer, CAD_air_equipmentFrame *item);

    void paintHeatCoolAdjustvalve(Layer *layer, CAD_heatcool_adjustvalve *item);
    void paintHeatCoolChiller(Layer *layer, CAD_heatcool_chiller *item);
    void paintHeatCoolControlvalve(Layer *layer, CAD_heatcool_controlvalve *item);
    void paintHeatCoolCoolingTower(Layer *layer, CAD_heatcool_coolingTower *item);
    void paintHeatCoolHeatExchanger(Layer *layer, CAD_heatcool_heatExchanger *item);
    void paintHeatCoolPipe(Layer *layer, CAD_heatcool_pipe *item);
    void paintHeatCoolPump(Layer *layer, CAD_heatcool_pump *item);
    void paintHeatCoolSensor(Layer *layer, CAD_heatcool_sensor *item);
    void paintHeatCoolPipeTurn(Layer *layer, CAD_heatcool_pipeTurn *item);
    void paintHeatCoolPipeReducer(Layer *layer, CAD_heatcool_pipeReducer *item);
    void paintHeatCoolPipeTeeConnector(Layer *layer, CAD_heatcool_pipeTeeConnector *item);
    void paintHeatCoolPipeEndCap(Layer *layer, CAD_heatcool_pipeEndCap *item);
    void paintHeatCoolFlange(Layer *layer, CAD_heatcool_flange *item);
    void paintHeatCoolExpansionChamber(Layer *layer, CAD_heatcool_expansionChamber *item);
    void paintHeatCoolBoiler(Layer *layer, CAD_heatcool_boiler *item);
    void paintHeatCoolWaterHeater(Layer *layer, CAD_heatcool_waterHeater *item);
    void paintHeatCoolStorageBoiler(Layer *layer, CAD_heatcool_storageBoiler *item);
    void paintHeatCoolRadiator(Layer *layer, CAD_heatcool_radiator *item);
    void paintHeatCoolFilter(Layer *layer, CAD_heatcool_filter *item);
    void paintHeatCoolBallValve(Layer *layer, CAD_heatcool_ballValve *item);
    void paintHeatCoolButterflyValve(Layer *layer, CAD_heatcool_butterflyValve *item);
    void paintHeatCoolSafteyValve(Layer *layer, CAD_heatcool_safetyValve *item);
    void paintHeatCoolFlowmeter(Layer *layer, CAD_heatcool_flowmeter *item);

    void paintSprinklerCompressedAirWaterContainer(Layer *layer, CAD_sprinkler_compressedAirWaterContainer *item);
    void paintSprinklerDistribution(Layer *layer, CAD_sprinkler_distribution *item);
    void paintSprinklerHead(Layer *layer, CAD_sprinkler_head *item);
    void paintSprinklerPipe(Layer *layer, CAD_sprinkler_pipe *item);
    void paintSprinklerPump(Layer *layer, CAD_sprinkler_pump *item);
    void paintSprinklerTeeConnector(Layer *layer, CAD_sprinkler_teeConnector *item);
    void paintSprinklerValve(Layer *layer, CAD_sprinkler_valve *item);
    void paintSprinklerWetAlarmValve(Layer *layer, CAD_sprinkler_wetAlarmValve *item);
    void paintSprinklerZoneCheck(Layer *layer, CAD_sprinkler_zoneCheck *item);

    void paintElectricalCabinet(Layer *layer, CAD_electrical_cabinet *item);
    void paintElectricalCabletray(Layer *layer, CAD_electrical_cableTray *item);



    CADitem *itemAtPosition(QPoint pos);
    CADitem *itemAtPosition_processLayers(QList<Layer*> layers, GLuint glName);
    CADitem *itemAtPosition_processItems(QList<CADitem*> items, GLuint glName);
    void highlightItemAtPosition(QPoint pos);
    void highlightClear();
    void highlightClear_processLayers(QList<Layer*> layers);
    void highlightClear_processItems(QList<CADitem*> items);

protected:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void enterEvent(QEvent *event);
    virtual void leaveEvent(QEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);

    virtual void resizeEvent(QResizeEvent *event);
    virtual void paintEvent(QPaintEvent *event);


signals:
    void signal_mouseMoved(QVector3D coords);

public slots:
    void slot_repaint();
    void slot_wireframe(bool on);
    void slot_solid(bool on);
};

#endif // GLWIDGET_H
