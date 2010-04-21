/*
    Copyright (c) 2010 Stephen Kelly <steveire@gmail.com>
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "mainview.h"

#include <KDE/KDebug>
#include <KDE/KProcess>
#include <kselectionproxymodel.h>

#include <KMime/Message>
#include <akonadi/kmime/messageparts.h>

#include "messagelistproxy.h"

MainView::MainView(QWidget* parent) :
  KDeclarativeMainView( QLatin1String( "kmail-mobile" ), new MessageListProxy, parent )
{
  addMimeType( KMime::Message::mimeType() );
  setListPayloadPart( Akonadi::MessagePart::Header );
}

void MainView::launchAccountWizard()
{
  int pid = KProcess::startDetached( QLatin1String( "accountwizard" ), QStringList()
                                                                    << QLatin1String( "--type" )
                                                                    << QLatin1String( "message/rfc822" ) );
  if ( !pid )
  {
    // Handle error
    kDebug() << "error creating accountwizard";
  }
}

#include "mainview.moc"

