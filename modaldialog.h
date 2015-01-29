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

#ifndef MODALDIALOG_H
#define MODALDIALOG_H

#include <QDialog>
#include <QPainter>
#include <QPaintEvent>

namespace Ui {
class ModalDialog;
}

class ModalDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ModalDialog(QString title, QStringList data, QWidget *parent = 0);
    ~ModalDialog();
    virtual void paintEvent(QPaintEvent *event);

private:
    Ui::ModalDialog *ui;
};

#endif // MODALDIALOG_H
