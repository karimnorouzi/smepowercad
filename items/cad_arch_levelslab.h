#ifndef CAD_ARCH_LEVELSLAB_H
#define CAD_ARCH_LEVELSLAB_H

#include "caditem.h"

class CAD_arch_levelSlab : public CADitem
{
public:
    CAD_arch_levelSlab();
    virtual ~CAD_arch_levelSlab();
    static QList<CADitem::ItemType> flangable_items();
    static QImage wizardImage();
    virtual void calculate();
    virtual void processWizardInput();
};

#endif // CAD_ARCH_LEVELSLAB_H
