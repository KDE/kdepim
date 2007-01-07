/*
    Copyright (c) 2007 Volker Krause <vkrause@kde.org>

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

#include "configmodule.h"
#include "transportmanagementwidget.h"

#include <kgenericfactory.h>
#include <qboxlayout.h>

using namespace MailTransport;

typedef KGenericFactory<ConfigModule, QWidget> MailTransportConfigFactory;
K_EXPORT_COMPONENT_FACTORY( kcm_mailtransport, MailTransportConfigFactory( "mailtrasnport" ) )

ConfigModule::ConfigModule( QWidget * parent, const QStringList & args ) :
    KCModule( MailTransportConfigFactory::instance(), parent, args )
{
  setButtons( 0 );
  QVBoxLayout *l = new QVBoxLayout( this );
  l->setMargin( 0 );
  TransportManagementWidget *tmw = new TransportManagementWidget( this );
  l->addWidget( tmw );
}
