/*
    This file is part of kdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#ifndef EGROUPWAREWIZARD_H
#define EGROUPWAREWIZARD_H

#include <klocale.h>

#include <wizards/servertype.h>

class EGroupwareWizard : public ServerType
{
  public:
    EGroupwareWizard();
    ~EGroupwareWizard();

    ServerType::ConnectionInfoList connectionInfo() const;
    void addConnection();
    void editConnection( const QString& uid );
    void deleteConnection( const QString& uid );
    void activateConnection( const QString& uid, bool active );

  private:
    void collectConnections();

    ServerType::ConnectionInfoList mInfoList;
};

class EGroupwareWizardFactory : public ServerTypeFactory
{
  public:
    virtual ServerType *serverType( QObject *parent, const char *name = 0 );

    virtual QString identifier() const { return "egroupware"; }
    virtual QString title() const { return i18n( "eGroupware" ); }
};

#endif
