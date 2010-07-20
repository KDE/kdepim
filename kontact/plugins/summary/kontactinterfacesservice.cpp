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

#include "kontactinterfacesservice.h"
#include <urihandler.h>

KontactInterfacesService::KontactInterfacesService(QObject* parent) :
    Plasma::Service(parent)
{
    kDebug() << "creating kontactinterface service";
    setName("org.kontact.interfaces");
}

KontactInterfacesServiceJob* KontactInterfacesService::createJob(const QString &operation, QMap<QString, QVariant> &parameters)
{
    return new KontactInterfacesServiceJob("", operation, parameters, this);
}

KontactInterfacesServiceJob::KontactInterfacesServiceJob(const QString& destination, const QString &operation, QMap<QString, QVariant> &parameters, QObject *parent) :
    ServiceJob(destination, operation, parameters, parent)
{
}

KontactInterfacesServiceJob::~KontactInterfacesServiceJob()
{
}

void KontactInterfacesServiceJob::start()
{
    QString operation = operationName();
    QString uri = parameters()[ "uri" ].toString();

    kDebug() << "dumping operation" << operation;

    if ( operation == "open" ) {
        UriHandler::process( uri );
    }
}

#include "kontactinterfacesservice.moc"
