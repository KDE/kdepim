/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef JOBSIGNALMAPPER_H
#define JOBSIGNALMAPPER_H

#include <QtCore/QHash>

#include <libkmobiletools/jobxp.h>

/**
 * This class maps a signal emitted from the job interface
 * to the name of the device from which the signals originates
 *
 * @todo make this class more generic by abstracting the job argument (use templates)
 *
 * @author Matthias Lechner <matthias@lmme.de>
 */
class JobSignalMapper : public QObject {
    Q_OBJECT
public:
    /**
     * Creates a new signal mapper with @p parent
     *
     * @param parent the parent
     */
    JobSignalMapper( QObject* parent = 0 );
    ~JobSignalMapper();

    /**
     * Defines the mapping between @p signalOrigin and @p deviceName
     * If a signal from the object @p signalOrigin is emitted, it will
     * be reemitted with the additional @param deviceName parameter
     *
     * @param signalOrigin the object the signal originates
     * @param deviceName the name of the device which is connected with the signal origin
     */
    void setMapping( QObject* signalOrigin, const QString& deviceName );

    /**
     * Removes the mapping for the given @p deviceName
     *
     * @param deviceName the device name
     */
    void removeMapping( const QString& deviceName );

    /**
     * Returns the device name for the given @p signalOrigin
     *
     * @param signalOrigin the object the signal originates
     * @return the device name
     */
    QString mapping( QObject* signalOrigin ) const;

public Q_SLOTS:
    /**
     * This slot should be connected with the signal you want to map
     *
     * @param job a job object
     */
    void map( KMobileTools::JobXP* job );

private Q_SLOTS:
    void signalOriginDestroyed();

Q_SIGNALS:
    /**
     * This signal is emitted when map() is called
     */
    void mapped( const QString& deviceName, KMobileTools::JobXP* job );

private:
    QHash<QObject*,QString> m_signals;
};

#endif
