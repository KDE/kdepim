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

#include <plasma/applet.h>

#include <akonadi/collection.h>

class KJob;

class EventsApplet : public Plasma::Applet
{
Q_OBJECT
public:
    EventsApplet(  QObject* parent = 0, QVariantList args = QVariantList() );

    void init();

public slots:
    void configChanged();
    void collectionsReceived(  const Akonadi::Collection::List& collections );
    void collectionFetchResult( KJob* job );

protected:
    void getCollectionFromKorg();

private:
    int m_collectionJobsInProgress;
    QString m_incidenceType;
    int m_numDays;
    QStringList m_collectionIds;
    Akonadi::Collection::List m_collections;
    bool m_noCollections;
};

#endif
