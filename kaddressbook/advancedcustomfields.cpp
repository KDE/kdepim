/*
    This file is part of KAddressbook.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

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

#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdatetimeedit.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qspinbox.h>
#include <qregexp.h>
#include <qtextedit.h>
#include <qwidgetfactory.h>

#include <kdatepicker.h>
#include <kdatetimewidget.h>
#include <kdialog.h>
#include <klineedit.h>
#include <kstandarddirs.h>

#include <libkdepim/designerfields.h>

#include "customfieldswidget.h"

#include "advancedcustomfields.h"

class KABCStorage : public KPIM::DesignerFields::Storage
{
  public:
    KABCStorage( KABC::Addressee *a, const QString &ns )
      : mAddressee( a ), mNs( ns )
    {
    }

    QStringList keys()
    {
      QStringList keys;
    
      QStringList customs = mAddressee->customs();
      QStringList::ConstIterator it;
      for ( it = customs.begin(); it != customs.end(); ++it ) {
        QString app, name, value;
        splitField( *it, app, name, value );
        if ( app == mNs ) keys.append( name );
      }

      return keys;
    }

    QString read( const QString &key )
    {
      return mAddressee->custom( mNs, key );
    }
    
    void write( const QString &key, const QString &value )
    {
      mAddressee->insertCustom( mNs, key, value );
    }

  private:
    KABC::Addressee *mAddressee;
    QString mNs;
};


AdvancedCustomFields::AdvancedCustomFields( const QString &uiFile, KABC::AddressBook *ab,
                                            QWidget *parent, const char *name )
  : KAB::ContactEditorWidget( ab, parent, name )
{
  initGUI( uiFile );
}

void AdvancedCustomFields::loadContact( KABC::Addressee *addr )
{
  QString ns;
  if ( mFields->identifier().upper() == "KADDRESSBOOK" ||
    QRegExp( "^Form\\d\\d?$" ).search( mFields->identifier() ) >= 0 ) {
    ns = "KADDRESSBOOK";
  } else {
    ns = mFields->identifier();
  }

  KABCStorage storage( addr, ns );
  mFields->load( &storage );
}

void AdvancedCustomFields::storeContact( KABC::Addressee *addr )
{
  QString ns;
  if ( mFields->identifier().upper() == "KADDRESSBOOK" ||
    QRegExp( "^Form\\d\\d?$" ).search( mFields->identifier() ) >= 0 ) {
    ns = "KADDRESSBOOK";
  } else {
    ns = mFields->identifier();
  }

  KABCStorage storage( addr, ns );
  mFields->save( &storage );
}

void AdvancedCustomFields::setReadOnly( bool readOnly )
{
  mFields->setReadOnly( readOnly );
}

void AdvancedCustomFields::initGUI( const QString &uiFile )
{
  QVBoxLayout *layout = new QVBoxLayout( this, KDialog::marginHint(),
                                         KDialog::spacingHint() );

  mFields = new KPIM::DesignerFields( uiFile, this );
  layout->addWidget( mFields );

  connect( mFields, SIGNAL( modified() ), SLOT( setModified() ) );
}

QString AdvancedCustomFields::pageIdentifier() const
{
  return mFields->identifier();
}

QString AdvancedCustomFields::pageTitle() const
{
  return mFields->title();
}

#include "advancedcustomfields.moc"
