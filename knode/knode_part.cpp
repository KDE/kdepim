/*
    This file is part of KNode.
    Copyright (c) 2003      Laurent Montel  <montel@kde.org>,
    Based on the work of Cornelius Schumacher <schumacher@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "knode_part.h"
#include "knglobals.h"
#include "knmainwidget.h"
#include "aboutdata.h"
#include "kncollectionview.h"
#include "knwidgets.h"

#include "sidebarextension.h"

#include <kapplication.h>
#include <kparts/genericfactory.h>
#include <kparts/statusbarextension.h>
#include <knotifyclient.h>
#include <dcopclient.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kstatusbar.h>
#include <krsqueezedtextlabel.h>

#include <qlayout.h>


typedef KParts::GenericFactory< KNodePart > KNodeFactory;
K_EXPORT_COMPONENT_FACTORY( libknodepart, KNodeFactory )

KNodePart::KNodePart(QWidget *parentWidget, const char *widgetName,
		     QObject *parent, const char *name, const QStringList &)
  : KParts::ReadOnlyPart(parent, name),
  mParentWidget( parentWidget )
{
  kdDebug(5003) << "KNodePart()" << endl;
  kdDebug(5003) << "  InstanceName: " << kapp->instanceName() << endl;

  setInstance( KNodeFactory::instance() );

  kdDebug(5003) << "KNodePart()..." << endl;
  kdDebug(5003) << "  InstanceName: " << kapp->instanceName() << endl;

  KGlobal::locale()->insertCatalogue("libkdenetwork");
  kapp->dcopClient()->suspend(); // Don't handle DCOP requests yet
  KGlobal::iconLoader()->addAppDir("knode");
  knGlobals.instance = KNodeFactory::instance();

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget(parentWidget, widgetName);
  canvas->setFocusPolicy(QWidget::ClickFocus);
  setWidget(canvas);

  mainWidget = new KNMainWidget( this, false, canvas, "knode_widget" );
  QVBoxLayout *topLayout = new QVBoxLayout(canvas);
  topLayout->addWidget(mainWidget);
  mainWidget->setFocusPolicy(QWidget::ClickFocus);

  kapp->dcopClient()->resume(); // Ok. We are ready for DCOP requests.

  new KParts::SideBarExtension( mainWidget->collectionView(),
                                this,
                                "KNodeSidebar" );

  KParts::StatusBarExtension* statusBar = new KParts::StatusBarExtension(this);
  statusBar->addStatusBarItem(mainWidget->statusBarLabelFilter(), 10, false);
  statusBar->addStatusBarItem(mainWidget->statusBarLabelGroup(), 15, false);

  setXMLFile( "knodeui.rc" );
}

KNodePart::~KNodePart()
{
  mainWidget->prepareShutdown();
}

KAboutData *KNodePart::createAboutData()
{
  return new KNode::AboutData();
}

bool KNodePart::openFile()
{
  kdDebug(5003) << "KNodePart:openFile()" << endl;

  mainWidget->show();
  return true;
}

void KNodePart::guiActivateEvent(KParts::GUIActivateEvent *e)
{
  kdDebug(5003) << "KNodePart::guiActivateEvent" << endl;
  KParts::ReadOnlyPart::guiActivateEvent(e);
}


QWidget* KNodePart::parentWidget() const
{
  return mParentWidget;
}



#include "knode_part.moc"

