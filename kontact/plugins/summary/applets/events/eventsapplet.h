/*
 *   Copyright 2010 Ryan Rix <ry@n.rix.si>
 * 
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

#ifndef EVENTSAPPLET_H
#define EVENTSAPPLET_H

#include "ui_configui.h"

class EventWidget;

#include <plasma/applet.h>
#include <plasma/dataengine.h>

namespace Plasma
{
    class ScrollWidget;
} // namespace Plasma

class KJob;

class QTimer;
class QGraphicsLinearLayout;

class EventsApplet : public Plasma::Applet
{
Q_OBJECT
public:
    EventsApplet(  QObject* parent = 0, QVariantList args = QVariantList() );

    void init();
    void ui();

public slots:
    void configChanged();
    void updateUI();
    void updateEvents();
    void dataUpdated( QString source, Plasma::DataEngine::Data data );

    void createConfigurationInterface( KConfigDialog* parent );
    void configAccepted();

private:
    QString m_source;
    QString m_incidenceType;
    int m_numDays;
    bool m_noCollections;
    QTimer* m_timer;
    QMap<QString,EventWidget*> m_incidences; // It's a map so that things are ordered magically
    Plasma::ScrollWidget* m_scrollWidget;
    QGraphicsWidget* m_scroller;
    QGraphicsLinearLayout* m_scrollerLayout;
    QStringList m_categories;

    QWidget* m_configWidget;
    Ui::EventsConfigUI m_configUi;
};

#endif
