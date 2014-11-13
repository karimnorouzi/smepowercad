#ifndef CAD_ARCH_DOOR_H
#define CAD_ARCH_DOOR_H

#include "caditem.h"

class CAD_arch_door : public CADitem
{
public:
    CAD_arch_door();
    virtual ~CAD_arch_door();
    static QList<CADitem::ItemType> flangable_items();
    static QImage wizardImage();
    virtual void calculate();
    virtual void processWizardInput();
};

#endif // CAD_ARCH_DOOR_H
