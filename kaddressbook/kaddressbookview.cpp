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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlayout.h>
#include <qpopupmenu.h>

#include <kabc/addressbook.h>
#include <kabc/distributionlistdialog.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klocale.h>
#include <kxmlguifactory.h>
#include <kxmlguiclient.h>

#include "core.h"
#include "searchmanager.h"

#include "kaddressbookview.h"

KAddressBookView::KAddressBookView( KAB::Core *core, QWidget *parent,
                                    const char *name )
    : QWidget( parent, name ), mCore( core ), mFieldList()
{
  initGUI();

  connect( mCore->searchManager(), SIGNAL( contactsUpdated() ),
           SLOT( updateView() ) );
}

KAddressBookView::~KAddressBookView()
{
  kdDebug(5720) << "KAddressBookView::~KAddressBookView: destroying - "
                << name() << endl;
}

void KAddressBookView::readConfig( KConfig *config )
{
  mFieldList = KABC::Field::restoreFields( config, "KABCFields" );

  if ( mFieldList.isEmpty() )
    mFieldList = KABC::Field::defaultFields();

  mDefaultFilterType = (DefaultFilterType)config->readNumEntry( "DefaultFilterType", 1 );
  mDefaultFilterName = config->readEntry( "DefaultFilterName" );
}

void KAddressBookView::writeConfig( KConfig* )
{
  // Most of writing the config is handled by the ConfigureViewDialog
}

QString KAddressBookView::selectedEmails()
{
  bool first = true;
  QString emailAddrs;
  const QStringList uidList = selectedUids();
  KABC::Addressee addr;
  QString email;

  QStringList::ConstIterator it;
  for ( it = uidList.begin(); it != uidList.end(); ++it ) {
    addr = mCore->addressBook()->findByUid( *it );

    if ( !addr.isEmpty() ) {
      QString m = QString::null;

      if ( addr.emails().count() > 1 )
        m = KABC::EmailSelector::getEmail( addr.emails(), addr.preferredEmail(), this );

      email = addr.fullEmail( m );

      if ( !first )
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
  if ( mFilter.isEmpty() )
    return mCore->searchManager()->contacts();

  KABC::Addressee::List addresseeList;
  const KABC::Addressee::List contacts = mCore->searchManager()->contacts();

  KABC::Addressee::List::ConstIterator it;
  KABC::Addressee::List::ConstIterator contactsEnd( contacts.end() );
  for ( it = contacts.begin(); it != contactsEnd; ++it ) {
    if ( mFilter.filterAddressee( *it ) )
      addresseeList.append( *it );
  }

  return addresseeList;
}

void KAddressBookView::initGUI()
{
  // Create the layout
  QVBoxLayout *layout = new QVBoxLayout( this );

  // Add the view widget
  mViewWidget = new QWidget( this );
  layout->addWidget( mViewWidget );
}

KABC::Field::List KAddressBookView::fields() const
{
  return mFieldList;
}

void KAddressBookView::setFilter( const Filter &filter )
{
  mFilter = filter;
}

KAddressBookView::DefaultFilterType KAddressBookView::defaultFilterType() const
{
  return mDefaultFilterType;
}

const QString &KAddressBookView::defaultFilterName() const
{
  return mDefaultFilterName;
}

KAB::Core *KAddressBookView::core() const
{
  return mCore;
}

void KAddressBookView::popup( const QPoint &point )
{
  if ( !mCore->guiClient() ) {
    kdWarning() << "No GUI client set!" << endl;
    return;
  }

  QPopupMenu *menu = static_cast<QPopupMenu*>( mCore->guiClient()->factory()->container( "RMBPopup",
                                               mCore->guiClient() ) );
  if ( menu )
    menu->popup( point );
}

QWidget *KAddressBookView::viewWidget()
{
  return mViewWidget;
}

void KAddressBookView::updateView()
{
  const QStringList uidList = selectedUids();

  refresh(); // This relists and deselects everything, in all views

  if ( !uidList.isEmpty() ) {
    // Keep previous selection
    QStringList::ConstIterator it, uidListEnd( uidList.end() );
    for ( it = uidList.begin(); it != uidListEnd; ++it )
      setSelected( *it, true );

  } else {
    const KABC::Addressee::List contacts = mCore->searchManager()->contacts();
    if ( !contacts.isEmpty() )
      setSelected( contacts.first().uid(), true );
    else
      emit selected( QString::null );
  }
}

ViewConfigureWidget *ViewFactory::configureWidget( KABC::AddressBook *ab,
                                                   QWidget *parent,
                                                   const char *name )
{
  return new ViewConfigureWidget( ab, parent, name );
}

#include "kaddressbookview.moc"
