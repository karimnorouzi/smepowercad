#ifndef CAD_AIR_CANVASFLANGE_H
#define CAD_AIR_CANVASFLANGE_H

#include "caditem.h"

class CAD_air_canvasFlange : public CADitem
{
public:
    CAD_air_canvasFlange();
    virtual ~CAD_air_canvasFlange();
    static QList<CADitem::ItemType> flangable_items();
    static QImage wizardImage();
    virtual void calculate();
    virtual void processWizardInput();
};

#endif // CAD_AIR_CANVASFLANGE_H
