/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SPECIAL_DATES_WIDGET_H
#define SPECIAL_DATES_WIDGET_H

#include <Plasma/Label>

class QPainter;
#include <QGraphicsLinearLayout>

class SpecialDate : public Plasma::Label
{
Q_OBJECT
public:
    SpecialDate(QString date, QString name);

    void init();
    
private:
    QString m_dateString;
    QString m_name;
    QDate m_date;
};

#endif