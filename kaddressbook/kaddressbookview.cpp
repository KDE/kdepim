/*
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
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


#include "kaddressbookview.h"


#include <qlayout.h>
#include <qapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>
#include <kabc/addressbook.h>

///////////////////////////////
// KAddressBookView

KAddressBookView::KAddressBookView(KABC::AddressBook *doc, QWidget *parent, 
                                   const char *name)
    : QWidget(parent, name), mDocument(doc), mFieldList()
{
    initGUI();
}

KAddressBookView::~KAddressBookView()
{
  kdDebug() << "KAddressBookView::~KAddressBookView: destroying - "
            << name() << endl;
}

void KAddressBookView::readConfig(KConfig *config)
{
  mFieldList = KABC::Field::restoreFields( config, "KABCFields" );

  if ( mFieldList.isEmpty() ) {
    mFieldList = KABC::Field::defaultFields();
  }
  
  mDefaultFilterType = (DefaultFilterType)config->
                                readNumEntry("DefaultFilterType", 1);
  mDefaultFilterName = config->readEntry("DefaultFilterName", QString::null);
}

void KAddressBookView::writeConfig(KConfig *)
{
  // Most of writing the config is handled by the ConfigureViewDialog
}

QString KAddressBookView::selectedEmails()
{
  bool first = true;
  QString emailAddrs;
  QStringList uidList = selectedUids();
  KABC::Addressee a;
  QString email;
  
  // Loop through the list of selected addressees, geting each one from the
  // document and asking it for the email address.
  QStringList::Iterator iter;
  for (iter = uidList.begin(); iter != uidList.end(); ++iter)
  {
      a = mDocument->findByUid(*iter);
      
      if (!a.isEmpty())
      {
        email = a.fullEmail();
        
        if (!first)
          emailAddrs += ", ";
        else
          first = false;
          
        emailAddrs += email;
      }
  }
  
  return emailAddrs;
}

KABC::Addressee::List KAddressBookView::addressees()
{
  KABC::Addressee::List addresseeList;
  
  KABC::AddressBook::Iterator iter;
  for (iter = mDocument->begin(); iter != mDocument->end(); ++iter)
  {
    if (mFilter.filterAddressee(*iter))
      addresseeList.append(*iter);
  }
    
  return addresseeList;
}

void KAddressBookView::initGUI()
{
    // Create the layout
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Add the view widget
    mViewWidget = new QWidget(this, "mViewWidget");
    layout->addWidget(mViewWidget);
    
}

void KAddressBookView::incrementalSearch(const QString &, 
                                         KABC::Field *)
{
    // Does nothing unless overloaded
}

void KAddressBookView::setFilter(const Filter &f)
{
  mFilter = f;
}

#include "kaddressbookview.moc"
