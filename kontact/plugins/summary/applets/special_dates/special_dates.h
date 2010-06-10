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

#ifndef SPECIAL_DATES_H
#define SPECIAL_DATES_H

#include <Plasma/Applet>

namespace Plasma {
    class Svg;
}
#include <Plasma/DataEngine>

class QPainter;
#include <QGraphicsLinearLayout>
class QTimer;

class SpecialDatesApplet : public Plasma::Applet
{
Q_OBJECT
public:
    SpecialDatesApplet( QObject* parent, QVariantList args );
    
    void paintInterface(QPainter* p,
                        const QStyleOptionGraphicsItem* option,
                        const QRect& contentsRect);
    void init();

public Q_SLOTS:
    void updateSpecialDates();
    void configChanged();
    
    void dataUpdated(const QString& sourceName, const Plasma::DataEngine::Data& data);
    void addSource(QString sourceName);
    void updateUI();
    
    
private:
    Plasma::Svg m_svg;
    QGraphicsLinearLayout* m_layout;
    Plasma::DataEngine* m_calEngine;
    Plasma::DataEngine* m_akoEngine;
    QMap<QString,QVariant> m_specialDates; // date, data
    QTimer* m_updateTimer;
    
    int m_numDays; // How many days into the future do we show?
    QString m_locale; // What locale are the holidays we are looking for?
};

#endif