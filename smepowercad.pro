#**********************************************************************
#* smepowercad
#* Copyright (C) 2015 Smart Micro Engineering GmbH
#* This program is free software: you can redistribute it and/or modify
#* it under the terms of the GNU General Public License as published by
#* the Free Software Foundation, either version 3 of the License, or
#* (at your option) any later version.
#* This program is distributed in the hope that it will be useful,
#* but WITHOUT ANY WARRANTY; without even the implied warranty of
#* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#* GNU General Public License for more details.
#* You should have received a copy of the GNU General Public License
#* along with this program. If not, see <http://www.gnu.org/licenses/>.
#*********************************************************************/

#-------------------------------------------------
#
# Project created by QtCreator 2013-12-03T16:47:52
#
#-------------------------------------------------

QT       += core gui opengl svg xml network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
#greaterThan(QT_MAJOR_VERSION, 4): QT -= opengl

TARGET = smepowercad
TEMPLATE = app

OBJECTS_DIR = .obj/
MOC_DIR = .moc/
UI_DIR = .ui/
RCC_DIR = .rcc/

LIBS += -lGLU -lX11

TRANSLATIONS =  lang/powercad-de_DE.ts \
                lang/powercad-ru_RU.ts

# Uncomment to enable 3D Mouse Driver Compilation
#CONFIG += 3D_MOUSE

3D_MOUSE {
DEFINES += USE_3D_MOUSE
SOURCES +=
HEADERS +=
}


SOURCES += main.cpp\
        mainwindow.cpp \
    layer.cpp \
    layermanager.cpp \
    creationinterface.cpp \
    dxflib/src/dl_writer_ascii.cpp \
    dxflib/src/dl_dxf.cpp \
    geometrydisplay.cpp \
    itemdb.cpp \
    caditem.cpp \
    math/m3dbox.cpp \
    glwidget.cpp \
    geometrydisplaytitle.cpp \
    items/cad_basic_point.cpp \
    items/cad_basic_line.cpp \
    items/cad_basic_polyline.cpp \
    items/cad_basic_circle.cpp \
    items/cad_basic_arc.cpp \
    items/cad_basic_3Dface.cpp \
    items/cad_basic_box.cpp \
    items/cad_basic_cylinder.cpp \
    items/cad_basic_sphere.cpp \
    items/cad_arch_door.cpp \
    items/cad_arch_levelslab.cpp \
    items/cad_arch_wall_loadbearing.cpp \
    items/cad_arch_wall_nonloadbearing.cpp \
    items/cad_arch_window.cpp \
    items/cad_basic_plane.cpp \
    items/cad_heatcool_pipe.cpp \
    items/cad_heatcool_chiller.cpp \
    items/cad_heatcool_coolingtower.cpp \
    items/cad_heatcool_heatexchanger.cpp \
    items/cad_heatcool_pump.cpp \
    items/cad_heatcool_controlvalve.cpp \
    items/cad_heatcool_adjustvalve.cpp \
    items/cad_heatcool_sensor.cpp \
    items/cad_air_duct.cpp \
    items/cad_air_pipe.cpp \
    items/cad_arch_blockout.cpp \
    items/cad_air_ductturn.cpp \
    items/cad_air_pipeturn.cpp \
    items/cad_air_pipereducer.cpp \
    items/cad_air_pipeteeconnector.cpp \
    items/cad_sprinkler_pipe.cpp \
    items/cad_sprinkler_head.cpp \
    items/cad_sprinkler_pump.cpp \
    items/cad_sprinkler_valve.cpp \
    items/cad_sprinkler_distribution.cpp \
    items/cad_sprinkler_teeconnector.cpp \
    items/cad_sprinkler_wetalarmvalve.cpp \
    items/cad_sprinkler_compressedairwatercontainer.cpp \
    items/cad_electrical_cabletray.cpp \
    items/cad_electrical_cabinet.cpp \
    items/cad_sprinkler_zonecheck.cpp \
    items/cad_air_ductfireresistant.cpp \
    items/cad_air_ductteeconnector.cpp \
    items/cad_air_ducttransition.cpp \
    items/cad_air_ducttransitionrectround.cpp \
    items/cad_air_ductypiece.cpp \
    items/cad_air_ductendplate.cpp \
    items/cad_air_throttlevalve.cpp \
    items/cad_air_multileafdamper.cpp \
    items/cad_air_pressurereliefdamper.cpp \
    items/cad_air_pipefiredamper.cpp \
    items/cad_air_ductfiredamper.cpp \
    items/cad_air_pipevolumetricflowcontroller.cpp \
    items/cad_air_heatexchangerwaterair.cpp \
    items/cad_air_heatexchangerairair.cpp \
    items/cad_air_canvasflange.cpp \
    items/cad_air_filter.cpp \
    items/cad_air_pipesilencer.cpp \
    items/cad_air_ductbafflesilencer.cpp \
    items/cad_air_fan.cpp \
    items/cad_air_humidifier.cpp \
    items/cad_air_emptycabinet.cpp \
    items/cad_air_equipmentframe.cpp \
    items/cad_air_pipeendcap.cpp \
    items/cad_heatcool_expansionchamber.cpp \
    items/cad_heatcool_boiler.cpp \
    items/cad_heatcool_waterheater.cpp \
    items/cad_heatcool_storageboiler.cpp \
    items/cad_air_ductvolumetricflowcontroller.cpp \
    items/cad_heatcool_pipeturn.cpp \
    items/cad_heatcool_pipereducer.cpp \
    items/cad_heatcool_pipeteeconnector.cpp \
    items/cad_heatcool_pipeendcap.cpp \
    items/cad_heatcool_flange.cpp \
    items/cad_heatcool_filter.cpp \
    items/cad_heatcool_ballvalve.cpp \
    items/cad_heatcool_butterflyvalve.cpp \
    items/cad_heatcool_safetyvalve.cpp \
    items/cad_arch_support.cpp \
    items/cad_arch_beam.cpp \
    items/cad_heatcool_radiator.cpp \
    items/cad_heatcool_flowmeter.cpp \
    modaldialog.cpp \
    settingsdialog.cpp \
    itemwizard.cpp \
    items/cad_sprinkler_pipeturn.cpp \
    items/cad_sprinkler_pipereducer.cpp \
    items/cad_sprinkler_pipeendcap.cpp \
    items/cad_sanitary_washbasin.cpp \
    items/cad_sanitary_sink.cpp \
    items/cad_sanitary_shower.cpp \
    items/cad_sanitary_pipeturn.cpp \
    items/cad_sanitary_pipeteeconnector.cpp \
    items/cad_sanitary_pipereducer.cpp \
    items/cad_sanitary_pipeendcap.cpp \
    items/cad_sanitary_pipe.cpp \
    items/cad_sanitary_liftingunit.cpp \
    items/cad_sanitary_flange.cpp \
    items/cad_sanitary_emergencyshower.cpp \
    items/cad_sanitary_emergencyeyeshower.cpp \
    items/cad_sanitary_electricwaterheater.cpp \
    items/cad_basic_pipe.cpp \
    items/cad_basic_turn.cpp \
    math/m3dboundingbox.cpp \
    network/server.cpp \
    network/clienthandler.cpp \
    items/cad_basic_duct.cpp \
    items/cad_arch_boredPile.cpp \
    items/cad_arch_grating.cpp \
    items/cad_arch_foundation.cpp \
    items/cad_basic_face.cpp \
    itemgripmodifier.cpp \
    items/cad_air_pipebranch.cpp \
    toolwidget.cpp \
    caditemtypes.cpp \
    wizardparams.cpp \
    items/cad_air_lineardiffuser.cpp \
    collisiondetection.cpp \
    items/cad_electrical_busbarwithouttapoffpoints.cpp \
    items/cad_electrical_busbarwithtapoffpoints1row.cpp \
    items/cad_electrical_busbarwithtapoffpoints2row.cpp \
    items/cad_electrical_cabletrayreducer.cpp \
    items/cad_electrical_cabletrayteeconnector.cpp \
    items/cad_electrical_cabletraytransition.cpp\
    items/cad_cleanroom_wallsmokeextractflap.cpp \
    items/cad_cleanroom_wallpost.cpp \
    items/cad_cleanroom_wallpanel.cpp \
    items/cad_cleanroom_walloverflowgrate.cpp \
    items/cad_cleanroom_wallmountingprofile.cpp \
    items/cad_cleanroom_wallstiffenerdiagonal.cpp \
    items/cad_cleanroom_vacuumcleanersocket.cpp \
    items/cad_cleanroom_tagsmokedetector.cpp \
    items/cad_cleanroom_tagleakagedetector.cpp \
    items/cad_cleanroom_floorsupport.cpp \
    items/cad_cleanroom_floorpanelperforated.cpp \
    items/cad_cleanroom_floorpanelwithtank.cpp \
    items/cad_cleanroom_floorpanelwithbushing.cpp \
    items/cad_cleanroom_floorpanel.cpp \
    items/cad_cleanroom_floorgrating.cpp \
    items/cad_cleanroom_floorstiffenerhorizontal.cpp \
    items/cad_cleanroom_floorstiffenerdiagonal.cpp \
    items/cad_cleanroom_doorswingingsingle.cpp \
    items/cad_cleanroom_doorswingingdouble.cpp \
    items/cad_cleanroom_doorslidingdouble.cpp \
    items/cad_cleanroom_controlswitch.cpp \
    items/cad_cleanroom_controlradarsensor.cpp \
    items/cad_cleanroom_controlledtouchkey.cpp \
    items/cad_cleanroom_controlemergencyswitch.cpp \
    items/cad_cleanroom_ceilingverticalladder.cpp \
    items/cad_cleanroom_ceilingteejoiningpiece.cpp \
    items/cad_cleanroom_ceilingsuspension.cpp \
    items/cad_cleanroom_ceilingsmokeextractflap.cpp \
    items/cad_cleanroom_ceilingpanel.cpp \
    items/cad_cleanroom_ceilingmountingrails.cpp \
    items/cad_cleanroom_ceilingmaintenanceflap.cpp \
    items/cad_cleanroom_ceilingjoiningknot.cpp \
    items/cad_cleanroom_ceilinggrating.cpp \
    items/cad_cleanroom_ceilingframe.cpp \
    items/cad_cleanroom_ceilingframefeedthrough.cpp \
    items/cad_cleanroom_ceilingfilterfanunit.cpp \
    items/cad_cleanroom_ceilingcornerpiece.cpp \
    items/cad_cleanroom_doorslidingsingle.cpp \
    items/cad_electrical_cabletrayturn.cpp \
    items/cad_electrical_cabletrayverticalladder.cpp \
    items/cad_electrical_cabinetwithoutdoor.cpp \
    items/cad_electrical_cabinetwithdoorleftandright.cpp \
    items/cad_electrical_cabinetwithdoorfrontandback.cpp \
    items/cad_basic_hemisphere.cpp \
    items/cad_electrical_luminairesurfacemounted.cpp \
    items/cad_electrical_luminairesemicircular.cpp \
    items/cad_electrical_luminairerecessedmounted.cpp \
    items/cad_electrical_luminaireescapelighting.cpp \
    items/cad_electrical_cabletraycross.cpp \
    items/cad_electrical_busbarendfeederunitsinglesided.cpp \
    items/cad_electrical_busbarendfeederunitdoublesided.cpp \
    items/cad_electrical_equipmentswitchorsocket.cpp \
    items/cad_cleanroom_controlbutton.cpp \
    items/cad_cleanroom_tagfiredetector.cpp \
    items/cad_cleanroom_tagelectricalgrounding.cpp \
    items/cad_electrical_busbartapoffunit.cpp \
    items/cad_electrical_luminairerailmounted.cpp \
    items/cad_gas_cdaballvalve.cpp \
    items/cad_gas_cdacompressor.cpp \
    items/cad_gas_cdadesiccantdryer.cpp \
    items/cad_gas_cdadiaphragmvalve.cpp \
    items/cad_gas_cdafilter.cpp \
    items/cad_gas_cdaflowmeter.cpp \
    items/cad_gas_cdahose.cpp \
    items/cad_gas_cdamanometer.cpp \
    items/cad_gas_cdamoisturesensor.cpp \
    items/cad_gas_cdanonreturnvalve.cpp \
    items/cad_gas_cdapipe.cpp \
    items/cad_gas_cdapipearc.cpp \
    items/cad_gas_cdapressureregulator.cpp \
    items/cad_gas_cdaquicklockcoupling.cpp \
    items/cad_gas_cdarefrigerantdryer.cpp \
    items/cad_gas_cdatank.cpp \
    items/cad_gas_vacballvalve.cpp \
    items/cad_gas_vacdiaphragmvalve.cpp \
    items/cad_gas_vacfilter.cpp \
    items/cad_gas_vacflowmeter.cpp \
    items/cad_gas_vachose.cpp \
    items/cad_gas_vacliquidseparator.cpp \
    items/cad_gas_vacmanometer.cpp \
    items/cad_gas_vacnonreturnvalve.cpp \
    items/cad_gas_vacpipe.cpp \
    items/cad_gas_vacpipearc.cpp \
    items/cad_gas_vacpump.cpp \
    items/cad_gas_vacquicklockcoupling.cpp \
    items/cad_gas_vactank.cpp \
    items/cad_gas_cdapipetfitting.cpp \
    items/cad_gas_vacpipetfitting.cpp \
    items/cad_sanitary_cleaningpiece.cpp \
    printwidget.cpp \
    items/cad_basic_torisphericalheaddeepdisheddin28013.cpp \
    items/cad_basic_torisphericalheaddin28011.cpp

HEADERS  += mainwindow.h \
    layer.h \
    layermanager.h \
    creationinterface.h \
    dxflib/src/dl_writer_ascii.h \
    dxflib/src/dl_writer.h \
    dxflib/src/dl_global.h \
    dxflib/src/dl_extrusion.h \
    dxflib/src/dl_exception.h \
    dxflib/src/dl_entities.h \
    dxflib/src/dl_dxf.h \
    dxflib/src/dl_creationinterface.h \
    dxflib/src/dl_creationadapter.h \
    dxflib/src/dl_codes.h \
    dxflib/src/dl_attributes.h \
    geometrydisplay.h \
    itemdb.h \
    caditem.h \
    math/m3dbox.h \
    glwidget.h \
    geometrydisplaytitle.h \
    items/cad_basic_point.h \
    items/cad_basic_line.h \
    items/cad_basic_polyline.h \
    items/cad_basic_arc.h \
    items/cad_basic_box.h \
    items/cad_basic_circle.h \
    items/cad_basic_cylinder.h \
    items/cad_basic_sphere.h \
    items/cad_basic_3Dface.h \
    items/cad_arch_door.h \
    items/cad_arch_levelslab.h \
    items/cad_arch_wall_loadbearing.h \
    items/cad_arch_wall_nonloadbearing.h \
    items/cad_arch_window.h \
    items/cad_basic_plane.h \
    items/cad_heatcool_pipe.h \
    items/cad_heatcool_chiller.h \
    items/cad_heatcool_coolingtower.h \
    items/cad_heatcool_heatexchanger.h \
    items/cad_heatcool_pump.h \
    items/cad_heatcool_controlvalve.h \
    items/cad_heatcool_adjustvalve.h \
    items/cad_heatcool_sensor.h \
    items/cad_air_duct.h \
    items/cad_air_pipe.h \
    items/cad_arch_blockout.h \
    items/cad_air_ductturn.h \
    items/cad_air_pipeturn.h \
    items/cad_air_pipereducer.h \
    items/cad_air_pipeteeconnector.h \
    items/cad_sprinkler_pipe.h \
    items/cad_sprinkler_head.h \
    items/cad_sprinkler_pump.h \
    items/cad_sprinkler_valve.h \
    items/cad_sprinkler_distribution.h \
    items/cad_sprinkler_teeconnector.h \
    items/cad_sprinkler_wetalarmvalve.h \
    items/cad_sprinkler_compressedairwatercontainer.h \
    items/cad_electrical_cabletray.h \
    items/cad_electrical_cabinet.h \
    items/cad_sprinkler_zonecheck.h \
    items/cad_air_ductfireresistant.h \
    items/cad_air_ductteeconnector.h \
    items/cad_air_ducttransition.h \
    items/cad_air_ducttransitionrectround.h \
    items/cad_air_ductypiece.h \
    items/cad_air_ductendplate.h \
    items/cad_air_throttlevalve.h \
    items/cad_air_multileafdamper.h \
    items/cad_air_pressurereliefdamper.h \
    items/cad_air_pipefiredamper.h \
    items/cad_air_ductfiredamper.h \
    items/cad_air_pipevolumetricflowcontroller.h \
    items/cad_air_heatexchangerwaterair.h \
    items/cad_air_heatexchangerairair.h \
    items/cad_air_canvasflange.h \
    items/cad_air_filter.h \
    items/cad_air_pipesilencer.h \
    items/cad_air_ductbafflesilencer.h \
    items/cad_air_fan.h \
    items/cad_air_humidifier.h \
    items/cad_air_emptycabinet.h \
    items/cad_air_equipmentframe.h \
    items/cad_air_pipeendcap.h \
    items/cad_heatcool_expansionchamber.h \
    items/cad_heatcool_boiler.h \
    items/cad_heatcool_waterheater.h \
    items/cad_heatcool_storageboiler.h \
    items/cad_air_ductvolumetricflowcontroller.h \
    items/cad_heatcool_pipeturn.h \
    items/cad_heatcool_pipereducer.h \
    items/cad_heatcool_pipeteeconnector.h \
    items/cad_heatcool_pipeendcap.h \
    items/cad_heatcool_flange.h \
    items/cad_heatcool_filter.h \
    items/cad_heatcool_ballvalve.h \
    items/cad_heatcool_butterflyvalve.h \
    items/cad_heatcool_safetyvalve.h \
    items/cad_arch_support.h \
    items/cad_arch_beam.h \
    items/cad_heatcool_radiator.h \
    items/cad_heatcool_flowmeter.h \
    modaldialog.h \
    settingsdialog.h \
    itemwizard.h \
    items/cad_sprinkler_pipeturn.h \
    items/cad_sprinkler_pipereducer.h \
    items/cad_sprinkler_pipeendcap.h \
    items/cad_sanitary_washbasin.h \
    items/cad_sanitary_sink.h \
    items/cad_sanitary_shower.h \
    items/cad_sanitary_pipeturn.h \
    items/cad_sanitary_pipeteeconnector.h \
    items/cad_sanitary_pipereducer.h \
    items/cad_sanitary_pipeendcap.h \
    items/cad_sanitary_pipe.h \
    items/cad_sanitary_liftingunit.h \
    items/cad_sanitary_flange.h \
    items/cad_sanitary_emergencyshower.h \
    items/cad_sanitary_emergencyeyeshower.h \
    items/cad_sanitary_electricwaterheater.h \
    items/cad_basic_pipe.h \
    items/cad_basic_turn.h \
    math/m3dboundingbox.h \
    network/server.h \
    network/clienthandler.h \
    items/cad_basic_duct.h \
    items/cad_arch_boredPile.h \
    items/cad_arch_grating.h \
    items/cad_arch_foundation.h \
    items/cad_basic_face.h \
    itemgripmodifier.h \
    items/cad_air_pipebranch.h \
    toolwidget.h \
    caditemheaderincludes.h \
    caditemtypes.h \
    wizardparams.h \
    items/cad_air_lineardiffuser.h\
    collisiondetection.h \
    items/cad_electrical_busbarwithouttapoffpoints.h \
    items/cad_electrical_busbarwithtapoffpoints1row.h \
    items/cad_electrical_busbarwithtapoffpoints2row.h \
    items/cad_electrical_cabletrayreducer.h \
    items/cad_electrical_cabletrayteeconnector.h \
    items/cad_electrical_cabletraytransition.h\
    items/cad_cleanroom_wallsmokeextractflap.h \
    items/cad_cleanroom_wallpost.h \
    items/cad_cleanroom_wallpanel.h \
    items/cad_cleanroom_walloverflowgrate.h \
    items/cad_cleanroom_wallmountingprofile.h \
    items/cad_cleanroom_wallstiffenerdiagonal.h \
    items/cad_cleanroom_vacuumcleanersocket.h \
    items/cad_cleanroom_tagsmokedetector.h \
    items/cad_cleanroom_tagleakagedetector.h \
    items/cad_cleanroom_floorsupport.h \
    items/cad_cleanroom_floorpanelperforated.h \
    items/cad_cleanroom_floorpanelwithtank.h \
    items/cad_cleanroom_floorpanelwithbushing.h \
    items/cad_cleanroom_floorpanel.h \
    items/cad_cleanroom_floorgrating.h \
    items/cad_cleanroom_floorstiffenerhorizontal.h \
    items/cad_cleanroom_floorstiffenerdiagonal.h \
    items/cad_cleanroom_doorswingingsingle.h \
    items/cad_cleanroom_doorswingingdouble.h \
    items/cad_cleanroom_doorslidingdouble.h \
    items/cad_cleanroom_controlswitch.h \
    items/cad_cleanroom_controlradarsensor.h \
    items/cad_cleanroom_controlledtouchkey.h \
    items/cad_cleanroom_controlemergencyswitch.h \
    items/cad_cleanroom_ceilingverticalladder.h \
    items/cad_cleanroom_ceilingteejoiningpiece.h \
    items/cad_cleanroom_ceilingsuspension.h \
    items/cad_cleanroom_ceilingsmokeextractflap.h \
    items/cad_cleanroom_ceilingpanel.h \
    items/cad_cleanroom_ceilingmountingrails.h \
    items/cad_cleanroom_ceilingmaintenanceflap.h \
    items/cad_cleanroom_ceilingjoiningknot.h \
    items/cad_cleanroom_ceilinggrating.h \
    items/cad_cleanroom_ceilingframe.h \
    items/cad_cleanroom_ceilingframefeedthrough.h \
    items/cad_cleanroom_ceilingfilterfanunit.h \
    items/cad_cleanroom_ceilingcornerpiece.h \
    items/cad_cleanroom_doorslidingsingle.h \
    items/cad_electrical_cabletrayturn.h \
    items/cad_electrical_cabletrayverticalladder.h \
    items/cad_electrical_cabinetwithoutdoor.h \
    items/cad_electrical_cabinetwithdoorleftandright.h \
    items/cad_electrical_cabinetwithdoorfrontandback.h \
    items/cad_basic_hemisphere.h \
    items/cad_electrical_luminairesurfacemounted.h \
    items/cad_electrical_luminairesemicircular.h \
    items/cad_electrical_luminairerecessedmounted.h \
    items/cad_electrical_luminaireescapelighting.h \
    items/cad_electrical_cabletraycross.h \
    items/cad_electrical_busbarendfeederunitsinglesided.h \
    items/cad_electrical_busbarendfeederunitdoublesided.h \
    items/cad_electrical_equipmentswitchorsocket.h \
    items/cad_cleanroom_controlbutton.h \
    items/cad_cleanroom_tagfiredetector.h \
    items/cad_cleanroom_tagelectricalgrounding.h \
    items/cad_electrical_busbartapoffunit.h \
    items/cad_electrical_luminairerailmounted.h \
    items/cad_gas_cdaballvalve.h \
    items/cad_gas_cdacompressor.h \
    items/cad_gas_cdadesiccantdryer.h \
    items/cad_gas_cdadiaphragmvalve.h \
    items/cad_gas_cdafilter.h \
    items/cad_gas_cdaflowmeter.h \
    items/cad_gas_cdahose.h \
    items/cad_gas_cdamanometer.h \
    items/cad_gas_cdamoisturesensor.h \
    items/cad_gas_cdanonreturnvalve.h \
    items/cad_gas_cdapipe.h \
    items/cad_gas_cdapipearc.h \
    items/cad_gas_cdapressureregulator.h \
    items/cad_gas_cdaquicklockcoupling.h \
    items/cad_gas_cdarefrigerantdryer.h \
    items/cad_gas_cdatank.h \
    items/cad_gas_vacballvalve.h \
    items/cad_gas_vacdiaphragmvalve.h \
    items/cad_gas_vacfilter.h \
    items/cad_gas_vacflowmeter.h \
    items/cad_gas_vachose.h \
    items/cad_gas_vacliquidseparator.h \
    items/cad_gas_vacmanometer.h \
    items/cad_gas_vacnonreturnvalve.h \
    items/cad_gas_vacpipe.h \
    items/cad_gas_vacpipearc.h \
    items/cad_gas_vacpump.h \
    items/cad_gas_vacquicklockcoupling.h \
    items/cad_gas_vactank.h \
    items/cad_gas_cdapipetfitting.h \
    items/cad_gas_vacpipetfitting.h \
    items/cad_sanitary_cleaningpiece.h \
    printwidget.h \
    items/cad_basic_torisphericalheaddeepdisheddin28013.h \
    items/cad_basic_torisphericalheaddin28011.h

FORMS    += mainwindow.ui \
    layermanager.ui \
    geometrydisplaytitle.ui \
    modaldialog.ui \
    settingsdialog.ui \
    itemwizard.ui \
    itemgripmodifier.ui \
    toolwidget.ui \
    printwidget.ui

RESOURCES += \
    icons.qrc

OTHER_FILES += \
    todo.txt \
    lang/powercad-de_DE.ts \
    lang/powercad-ru_RU.ts \
    lang/powercad-de_DE.qm \
    lang/powercad-ru_RU.qm \
    settings.xml \
    shaders/test.frag \
    shaders/shader_1.vert \
    shaders/shader_1_triangles.geom \
    shaders/shader_1_lines.geom \
    shaders/shader_2.frag \
    shaders/shader_2.vert