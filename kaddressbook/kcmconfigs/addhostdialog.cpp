/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtooltip.h>

#include <kaccelmanager.h>
#include <kbuttonbox.h>
#include <klineedit.h>
#include <klocale.h>
#include "addhostdialog.h"

AddHostDialog::AddHostDialog( KPIM::LdapServer *server, QWidget* parent,  const char* name )
  : KDialogBase( Plain, i18n( "Add Host" ), Ok | Cancel, Ok, parent, name, true, true )
{
  mServer = server;

  QWidget *page = plainPage();
  QHBoxLayout *layout = new QHBoxLayout( page, marginHint(), spacingHint() );

  mCfg = new KABC::LdapConfigWidget(
       KABC::LdapConfigWidget::W_USER |
       KABC::LdapConfigWidget::W_PASS |
       KABC::LdapConfigWidget::W_BINDDN |
       KABC::LdapConfigWidget::W_REALM |
       KABC::LdapConfigWidget::W_HOST |
       KABC::LdapConfigWidget::W_PORT |
       KABC::LdapConfigWidget::W_VER |
       KABC::LdapConfigWidget::W_TIMELIMIT |
       KABC::LdapConfigWidget::W_SIZELIMIT |
       KABC::LdapConfigWidget::W_DN |
       KABC::LdapConfigWidget::W_SECBOX |
       KABC::LdapConfigWidget::W_AUTHBOX,
        page );

  layout->addWidget( mCfg );
  mCfg->setHost( mServer->host() );
  mCfg->setPort( mServer->port() );
  mCfg->setDn( mServer->baseDN() );
  mCfg->setUser( mServer->user() );
  mCfg->setBindDN( mServer->bindDN() );
  mCfg->setPassword( mServer->pwdBindDN() );
  mCfg->setTimeLimit( mServer->timeLimit() );
  mCfg->setSizeLimit( mServer->sizeLimit() );
  mCfg->setVer( mServer->version() );
 //0-No, 1-TLS, 2-SSL - KDE4: add an enum to LdapConfigWidget
  switch ( mServer->security() ) {
    case 1:
      mCfg->setSecTLS();
      break;
    case 2:
      mCfg->setSecSSL();
      break;
    default:
      mCfg->setSecNO();
  }
 //0-Anonymous, 1-simple, 2-SASL - KDE4: add an enum to LdapConfigWidget
  switch ( mServer->auth() ) {
    case 1:
      mCfg->setAuthSimple();
      break;
    case 2:
      mCfg->setAuthSASL();
      break;
    default:
      mCfg->setAuthAnon();
  }
  mCfg->setMech( mServer->mech() );

  KAcceleratorManager::manage( this );

}

AddHostDialog::~AddHostDialog()
{
}

void AddHostDialog::slotHostEditChanged( const QString &text )
{
  enableButtonOK( !text.isEmpty() );
}

void AddHostDialog::slotOk()
{
  mServer->setHost( mCfg->host() );
  mServer->setPort( mCfg->port() );  
  mServer->setBaseDN( mCfg->dn().stripWhiteSpace() );
  mServer->setUser( mCfg->user() );
  mServer->setBindDN( mCfg->bindDN() );
  mServer->setPwdBindDN( mCfg->password() );
  mServer->setTimeLimit( mCfg->timeLimit() );
  mServer->setSizeLimit( mCfg->sizeLimit() );
  mServer->setVersion( mCfg->ver() );
  mServer->setSecurity( 0 );
  if ( mCfg->isSecTLS() ) mServer->setSecurity( 1 );
  if ( mCfg->isSecSSL() ) mServer->setSecurity( 2 );
  mServer->setAuth( 0 );
  if ( mCfg->isAuthSimple() ) mServer->setAuth( 1 );
  if ( mCfg->isAuthSASL() ) mServer->setAuth( 2 );
  mServer->setMech( mCfg->mech() );
  KDialog::accept();
}

#include "addhostdialog.moc"
