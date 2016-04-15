/*
    Copyright (c) 2014 Sandro Knauß <knauss@kolabsys.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef SETUPAUTOCONFIGKOLABLDAPDB_H
#define SETUPAUTOCONFIGKOLABLDAPDB_H

#include "setupobject.h"

class AutoconfigKolabLdap;

class SetupAutoconfigKolabLdap : public SetupObject
{
    Q_OBJECT
public:
    /** Constructor */
    explicit SetupAutoconfigKolabLdap(QObject *parent = Q_NULLPTR);
    ~SetupAutoconfigKolabLdap();

    void create() Q_DECL_OVERRIDE;
    void destroy() Q_DECL_OVERRIDE;

public Q_SLOTS:
    Q_SCRIPTABLE void fillLdapServer(int i, QObject *) const;
    Q_SCRIPTABLE int countLdapServers() const;

    Q_SCRIPTABLE void start();

    Q_SCRIPTABLE void setEmail(const QString &);
    Q_SCRIPTABLE void setPassword(const QString &);

Q_SIGNALS:
    void ispdbFinished(bool);

private Q_SLOTS:
    void onIspdbFinished(bool);

private :
    AutoconfigKolabLdap *mIspdb;

};

#endif
