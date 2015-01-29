/**********************************************************************
** smepowercad
** Copyright (C) 2015 Smart Micro Engineering GmbH
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
** You should have received a copy of the GNU General Public License
** along with this program. If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef COLLISIONDETECTION_H
#define COLLISIONDETECTION_H

#include <QThread>
#include <QList>

#include "itemdb.h"
#include "math/m3dboundingbox.h"

class CollisionDetection : public QThread
{
    Q_OBJECT
public:
    explicit CollisionDetection(ItemDB* itemDB, QObject *parent = 0);

    void run();
    void finished();

private:
    ItemDB* itemDB;
    QList<CADitem*> itemsToCheck;
    CADitem* currentItem;
    CADitem* currentReferenceItem;
    M3dBoundingBox currentBoundingBox;

    void testLayers(QList<Layer*> layers);
    void testItems(QList<CADitem *> items, Layer *layer, bool isSubItem = false);

private slots:
    void slot_checkNextItem();

signals:
    void signal_itemsDoCollide(CADitem* item_1, CADitem* item_2);

public slots:
    void slot_testModifiedItem(CADitem* item);
    void slot_itemDeleted(CADitem* item);
};

#endif // COLLISIONDETECTION_H
