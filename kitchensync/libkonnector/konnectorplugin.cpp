/* This file is part of the KDE libraries
    Copyright (C) 2002 Holger Freyther <freyher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#include "konnectorplugin.h"

using namespace KSync;

KonnectorPlugin::KonnectorPlugin( QObject *obj, const char *name, const QStringList &/*args*/)
  : QObject(obj, name )
{

}
void KonnectorPlugin::setUDI( const QString& udi ) {
    m_udi = udi;
}
QString KonnectorPlugin::udi() const {
    return m_udi;
}
KonnectorPlugin::~KonnectorPlugin()
{

}
#include "konnectorplugin.moc"
