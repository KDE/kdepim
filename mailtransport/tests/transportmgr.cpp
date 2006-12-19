/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

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

#include "transportmgr.h"

#include <mailtransport/transportconfigdialog.h>
#include <mailtransport/transportmanager.h>
#include <mailtransport/transportmanagementwidget.h>

#include <KApplication>
#include <KCmdLineArgs>

#include <QPushButton>

using namespace KPIM;

TransportMgr::TransportMgr()
{
  mComboBox = new TransportComboBox( this );
  QPushButton *b = new QPushButton( "&Edit", this );
  connect( b, SIGNAL(clicked(bool)), SLOT(editBtnClicked()) );
  new TransportManagementWidget( this );
}

void TransportMgr::editBtnClicked()
{
  TransportConfigDialog *t = new TransportConfigDialog( TransportManager::self()->transportById( mComboBox->currentTransportId() ), this );
  t->exec();
  delete t;
}

int main( int argc, char** argv )
{
  KCmdLineArgs::init(argc, argv, "transportmgr", "transportmgr", "Mail Transport Manager Demo", "0" );
  KApplication app;
  TransportMgr* t = new TransportMgr();
  t->show();
  app.exec();
}

#include "transportmgr.moc"

