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
#ifndef PRINTSCRIPT_H
#define PRINTSCRIPT_H

#include <QString>
#include <QMap>
#include "printscripttreeitem.h"

class Printscript : public PrintscriptTreeItem
{
    Q_OBJECT
public:
    Printscript(const QString &name, const QString &script, PrintscriptTreeItem *parentItem = Q_NULLPTR, QObject *parent = Q_NULLPTR);
    Printscript(Printscript *item);

    QString script;

    void insertVariable(const QString &key, const QString &value);
    void insertVariables(const QMap<QString,QString> variables);
    QMap<QString,QString> getVariables() const;
    void removeVariable(const QString &key);

private:
    QMap<QString,QString> m_variables;
};

#endif // PRINTSCRIPT_H
