#ifndef CAD_BASIC_PIPE_H
#define CAD_BASIC_PIPE_H

#include "caditem.h"

#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>

class CAD_basic_pipe : public CADitem
{
public:
    CAD_basic_pipe();
    virtual void calculate();
    virtual void processWizardInput();

   QMatrix4x4 matrix_rotation;

    qreal radius;
    qreal length;
    qreal wallThickness;
    QVector3D direction;
};

#endif // CAD_BASIC_PIPE_H