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

#ifndef KONTACTINTERFACESSERVICE_H
#define KONTACTINTERFACESSERVICE_H

#include <Plasma/Service>
#include <Plasma/ServiceJob>

class KontactInterfacesServiceJob : public Plasma::ServiceJob
{
public:
    KontactInterfacesServiceJob( const QString& destination, const QString& operation, QMap<QString, QVariant>& parameters, QObject* parent = NULL);
    ~KontactInterfacesServiceJob();

protected:
    void start();
};

/**
 * A service to control applications which are running inside of Kontact using
 * the UriHandler class. This Service allows applets to display events, mails,
 * and contacts by calling the @m createjob method with the the "open" 
 * @p operation. The "uri" parameter controls what will be opened:
 * kmail:<uid>          open the specified mail
 * mailto:<address>     open a new mail to the specified address
 * uid:<uid>            open KAddressBook's editor to the specified contact
 * urn:x-ical<uid>      open KOrganizer to the entry specified by the uid
 * news:<url>           open KNode to the specified url
 * 
 * Any other URL will be opened using KRun.
 *
 * @author Ryan Rix <ry@n.rix.si>
 * @since 4.6
 **/
class KontactInterfacesService : public Plasma::Service 
{
Q_OBJECT
public:
    KontactInterfacesService(QObject* parent = 0);

    KontactInterfacesServiceJob* createJob( const QString& operation, QMap<QString, QVariant> &parameters);
};

#endif
