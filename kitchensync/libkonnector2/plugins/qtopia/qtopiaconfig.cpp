/*
    This file is part of KitchenSync.

    Copyright (c) 2002,2003 Holger Freyther <freyther@kde.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "qtopiaconfig.h"

#include "qtopiakonnector.h"

#include <kapplication.h>
#include <kdialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qcombobox.h>
#include <qlabel.h>

using namespace OpieHelper;

namespace {

void setCurrent( const QString &str, QComboBox *box, bool insert = true )
{
  if ( str.isEmpty() ) return;
  uint b = box->count();
  for ( uint i = 0; i < b; i++ ) {
      if ( box->text(i) == str ) {
        box->setCurrentItem(i );
        return;
      }
  }
  if ( !insert ) return;

  box->insertItem( str );
  box->setCurrentItem( b );
}

}


QtopiaConfig::QtopiaConfig( QWidget *parent, const char *name )
  : KRES::ConfigWidget( parent, name )
{
  initUI();
}

QtopiaConfig::~QtopiaConfig()
{
}

void QtopiaConfig::loadSettings( KRES::Resource *resource )
{
  KSync::QtopiaKonnector *k =
      dynamic_cast<KSync::QtopiaKonnector *>( resource );

  if ( !k )
    return;

  setCurrent( k->userName(), m_cmbUser );
  m_cmbPass->insertItem( k->password() );
  m_cmbPass->setCurrentText( k->password() );
  setCurrent( k->destinationIP(), m_cmbIP );
  setCurrent( k->model(), m_cmbDev, false );
  if ( m_cmbDev->currentText() == QString::fromLatin1("Sharp Zaurus ROM") )
      m_name->setText( k->modelName() );

  slotTextChanged( m_cmbDev->currentText() );
}

void QtopiaConfig::saveSettings( KRES::Resource *resource )
{
  KSync::QtopiaKonnector *k =
      dynamic_cast<KSync::QtopiaKonnector *>( resource );
  if ( !k )
    return;

  k->setDestinationIP( m_cmbIP->currentText() );
  k->setUserName( m_cmbUser->currentText() );
  if ( m_cmbPass->currentText().isEmpty() )
    KMessageBox::information( this, i18n( "You have entered an empty password, this won't work with Qtopia1.7/OPIE" ) );
  k->setPassword( m_cmbPass->currentText() );
  k->setModel( m_cmbDev->currentText() );
  k->setModelName( name() );
}

QString QtopiaConfig::name() const
{
  return m_name->text().isEmpty() ? "Zaurus" + kapp->randomString( 5 ) :
                                    m_name->text();
}

void QtopiaConfig::initUI()
{
  m_layout = new QGridLayout( this, 4, 5 );
  m_layout->setSpacing( KDialog::spacingHint() );

  m_lblUser = new QLabel( i18n("User:"), this );

  m_cmbUser = new QComboBox(this);
  m_cmbUser->setEditable( true );
  m_cmbUser->insertItem( "root");

  m_lblPass = new QLabel( i18n("Password:"), this );

  m_cmbPass = new QComboBox(this);
  m_cmbPass->setEditable( true );
  m_cmbPass->insertItem("Qtopia");

  m_lblName = new QLabel( i18n("Name:"), this );

  m_name = new QLineEdit(this);
  m_name->setEnabled( false );

  m_lblIP = new QLabel( i18n("Destination address:"), this );

  m_cmbIP = new QComboBox(this);
  m_cmbIP->setEditable( true );
  m_cmbIP->insertItem("1.1.1.1", 0);
  m_cmbIP->insertItem("192.168.129.201", 1);

  m_lblDev = new QLabel( i18n("Distribution:"), this );

  m_cmbDev = new QComboBox(this);
  m_cmbDev->insertItem("Sharp Zaurus ROM");
  m_cmbDev->insertItem("Opie and Qtopia1.6", 0 );
  connect( m_cmbDev, SIGNAL( activated( const QString & ) ),
           SLOT( slotTextChanged( const QString &  ) ) );

  m_layout->addWidget( m_lblDev, 0, 0 );
  m_layout->addWidget( m_cmbDev, 0, 1 );

  m_layout->addWidget( m_lblUser, 1, 0 );
  m_layout->addWidget( m_cmbUser, 1, 1 );

  m_layout->addWidget( m_lblPass, 1, 2 );
  m_layout->addWidget( m_cmbPass, 1, 3 );

  m_layout->addWidget( m_lblIP, 2, 0 );
  m_layout->addWidget( m_cmbIP, 2, 1 );

  m_layout->addWidget( m_lblName, 2, 2 );
  m_layout->addWidget( m_name, 2, 3 );
}

void QtopiaConfig::slotTextChanged( const QString &str )
{
  bool b = ( str == QString::fromLatin1("Sharp Zaurus ROM") );

  m_name->setEnabled( b );
  m_lblName->setEnabled( b );

  m_cmbUser->setEnabled( !b );
  m_lblUser->setEnabled( !b );

  m_cmbPass->setEnabled( !b );
  m_lblPass->setEnabled( !b );
}

#include "qtopiaconfig.moc"
