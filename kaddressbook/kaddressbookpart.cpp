/*
  This file is part of KAddressBook.

  Copyright (c) 2009 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kaddressbookpart.h"
#include "aboutdata.h"
#include "mainwidget.h"

#include <QDebug>
#include <KIconLoader>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KParts/StatusBarExtension>
#include <QAction>
#include <KActionCollection>
#include <QIcon>

#include <QVBoxLayout>

K_PLUGIN_FACTORY( KAddressBookFactory, registerPlugin<KAddressBookPart>(); )
K_EXPORT_PLUGIN( KAddressBookFactory( AboutData() ) )

KAddressBookPart::KAddressBookPart( QWidget *parentWidget, QObject *parent,
                                    const QVariantList & )
  : KParts::ReadOnlyPart( parent )
{
  //QT5 setComponentData( KAddressBookFactory::componentData() );

  KIconLoader::global()->addAppDir( QLatin1String("kaddressbook") );
  // create a canvas to insert our widget
  QWidget *canvas = new QWidget( parentWidget );
  canvas->setFocusPolicy( Qt::ClickFocus );
  setWidget( canvas );
  QVBoxLayout *topLayout = new QVBoxLayout( canvas );

  mMainWidget = new MainWidget( this, canvas );
  initAction();

  topLayout->addWidget( mMainWidget );
  topLayout->setMargin(0);
  setXMLFile( QLatin1String("kaddressbookui.rc") );
}

void KAddressBookPart::updateQuickSearchText()
{
  mMainWidget->updateQuickSearchText();
}

void KAddressBookPart::initAction()
{
  QAction *action = new QAction( QIcon::fromTheme( QLatin1String("configure") ), i18n( "&Configure KAddressBook..." ), this );
  actionCollection()->addAction( QLatin1String("kaddressbook_configure"), action );
  connect( action, SIGNAL(triggered(bool)), mMainWidget,
           SLOT(configure()) );
}

void KAddressBookPart::newContact()
{
  mMainWidget->newContact();
}

void KAddressBookPart::newGroup()
{
  mMainWidget->newGroup();
}

KAddressBookPart::~KAddressBookPart()
{
}

bool KAddressBookPart::openFile()
{
  return false;
}

void KAddressBookPart::guiActivateEvent( KParts::GUIActivateEvent *e )
{
   qDebug();
   KParts::ReadOnlyPart::guiActivateEvent( e );
}

#include "kaddressbookpart.moc"
