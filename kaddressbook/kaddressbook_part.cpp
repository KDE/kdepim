/*
    This file is part of KAddressbook.
    Copyright (c) 2000 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

// $Id$

#include <qlayout.h>

#include <kapplication.h>
#include <kinstance.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kiconloader.h>
#include <kaction.h>
#include <kdebug.h>
#include <kparts/genericfactory.h>

#include "kaddressbook.h"
#include "kaddressbooktableview.h"
#include "viewmanager.h"
#include "kaddressbookiface.h"
#include "actionmanager.h"

#include "kaddressbook_part.h"

typedef KParts::GenericFactory< KAddressbookPart > KAddressbookFactory;
K_EXPORT_COMPONENT_FACTORY( libkaddressbookpart, KAddressbookFactory );

KAddressbookPart::KAddressbookPart(QWidget *parentWidget, const char *widgetName,
                               QObject *parent, const char *name, const QStringList &) :
  KParts::ReadOnlyPart(parent, name), DCOPObject("KAddressBookIface")
{
  kdDebug() << "KAddressbookPart()" << endl;
  kdDebug() << "  InstanceName: " << kapp->instanceName() << endl;

  setInstance(KAddressbookFactory::instance());

  kdDebug() << "KAddressbookPart()..." << endl;
  kdDebug() << "  InstanceName: " << kapp->instanceName() << endl;

  // create a canvas to insert our widget
  QWidget *canvas = new QWidget(parentWidget, widgetName);
  canvas->setFocusPolicy(QWidget::ClickFocus);
  setWidget(canvas);

  m_extension = new KAddressbookBrowserExtension(this);

  QVBoxLayout *topLayout = new QVBoxLayout(canvas);
   
  KGlobal::iconLoader()->addAppDir("kaddressbook");

  widget = new KAddressBook(canvas);
  widget->readConfig();
  topLayout->addWidget(widget);
  widget->viewManager()->setQuickEditVisible( false );

  widget->show();

  mActionManager = new ActionManager(this, widget, false, this);
  
  setXMLFile( "kaddressbook_part.rc" );
}

KAddressbookPart::~KAddressbookPart()
{
  closeURL();
}

KAboutData *KAddressbookPart::createAboutData()
{
  KAboutData *about = new KAboutData("kaddressbook", I18N_NOOP("KAddressBook"),
                                     "3.0", I18N_NOOP("The KDE Address Book"),
                                     KAboutData::License_BSD, 
                                     I18N_NOOP("(c) 1997-2002, The KDE PIM Team"));
  about->addAuthor("Don Sanders",I18N_NOOP("Original author and maintainer"));
  about->addAuthor("Cornelius Schumacher",
                  I18N_NOOP("libkabc port, csv import/export"),
                  "schumacher@kde.org");
  about->addAuthor("Greg Stern", I18N_NOOP("DCOP interface"));
  about->addAuthor("Mark Westcott",I18N_NOOP("Contact pinning"));
  about->addAuthor("Mischel Boyer de la Giroday", I18N_NOOP("LDAP Lookup"), 
		   "michel@klaralvdalens-datakonsult.se");
  about->addAuthor("Steffen Hansen", I18N_NOOP("LDAP Lookup"), "hansen@kde.org");

  return about;
}

bool KAddressbookPart::openFile()
{
  kdDebug() << "KAddressbookPart:openFile()" << endl;

  widget->show();
  return true;
}

void KAddressbookPart::guiActivateEvent(KParts::GUIActivateEvent *e)
{
  kdDebug() << "KAddressbookPart::guiActivateEvent" << endl;
  KParts::ReadOnlyPart::guiActivateEvent(e);
  
  mActionManager->initActionViewList();
}

KAddressbookBrowserExtension::KAddressbookBrowserExtension(KAddressbookPart *parent) :
  KParts::BrowserExtension(parent, "KAddressbookBrowserExtension")
{
}

KAddressbookBrowserExtension::~KAddressbookBrowserExtension()
{
}

using namespace KParts;
#include "kaddressbook_part.moc"
