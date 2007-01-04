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

#include "transportconfigdialog.h"
#include "transport.h"
#include "transportmanager.h"
#include "mailtransport_defs.h"

#include "ui_smtpsettings.h"
#include "ui_sendmailsettings.h"

#include <libkdepim/servertest.h>

#include <kconfigdialogmanager.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kprotocolinfo.h>

#include <QButtonGroup>

using namespace KPIM;

class KPIM::TransportConfigDialog::Private
{
  public:
    Transport *transport;

    Ui::SMTPSettings smtp;
    Ui::SendmailSettings sendmail;

    KConfigDialogManager* manager;
    KLineEdit* passwordEdit;
    ServerTest* serverTest;
    QButtonGroup* encryptionGroup;
    QButtonGroup* authGroup;

    // detected authentication capabilities
    QList<int> noEncCapa, sslCapa, tlsCapa;

    void resetAuthCapabilities()
    {
      noEncCapa.clear();
      noEncCapa << Transport::EnumAuthenticationType::LOGIN
                << Transport::EnumAuthenticationType::PLAIN
                << Transport::EnumAuthenticationType::CRAM_MD5
                << Transport::EnumAuthenticationType::DIGEST_MD5
                << Transport::EnumAuthenticationType::NTLM
                << Transport::EnumAuthenticationType::GSSAPI;
      sslCapa = tlsCapa = noEncCapa;
      if ( authGroup )
        updateAuthCapbilities();
    }

    void updateAuthCapbilities()
    {
      Q_ASSERT( transport->type() == Transport::EnumType::SMTP );

      QList<int> capa = noEncCapa;
      if ( smtp.ssl->isChecked() )
        capa = sslCapa;
      else if ( smtp.tls->isChecked() )
        capa = tlsCapa;

      for ( int i = 0; i < authGroup->buttons().count(); ++i )
        authGroup->buttons().at( i )->setEnabled( capa.contains( i ) );
    }
};

TransportConfigDialog::TransportConfigDialog( Transport* transport, QWidget * parent) :
    KDialog( parent ),
    d( new Private )
{
  Q_ASSERT( transport );

  d->transport = transport;
  d->passwordEdit = 0;
  d->serverTest = 0;
  d->encryptionGroup = 0;
  d->authGroup = 0;
  d->resetAuthCapabilities();

  setButtons( Ok|Cancel );
  connect( this, SIGNAL(okClicked()), SLOT(save()) );
  connect( TransportManager::self(), SIGNAL(passwordsChanged()), SLOT(passwordsLoaded()) );

  switch ( transport->type() ) {
    case Transport::EnumType::SMTP:
    {
      d->smtp.setupUi( mainWidget() );
      d->passwordEdit = d->smtp.password;

      d->encryptionGroup = new QButtonGroup( this );
      d->encryptionGroup->addButton( d->smtp.none );
      d->encryptionGroup->addButton( d->smtp.ssl );
      d->encryptionGroup->addButton( d->smtp.tls );

      d->authGroup = new QButtonGroup( this );
      d->authGroup->addButton( d->smtp.login );
      d->authGroup->addButton( d->smtp.plain );
      d->authGroup->addButton( d->smtp.crammd5 );
      d->authGroup->addButton( d->smtp.digestmd5 );
      d->authGroup->addButton( d->smtp.ntlm );
      d->authGroup->addButton( d->smtp.gssapi );

      if ( KProtocolInfo::capabilities(SMTP_PROTOCOL).contains("SASL") == 0 ) {
        d->smtp.ntlm->hide();
        d->smtp.gssapi->hide();
      }

      connect( d->smtp.checkCapabilities, SIGNAL(clicked()), SLOT(checkSmtpCapabilities()) );
      connect( d->smtp.kcfg_host, SIGNAL(textChanged(QString)), SLOT(hostNameChanged(QString)) );
      connect( d->smtp.kcfg_encryption, SIGNAL(clicked(int)), SLOT(encryptionChanged(int)) );
      break;
    }
    case Transport::EnumType::Sendmail:
    {
      d->sendmail.setupUi( mainWidget() );

      connect( d->sendmail.chooseButton, SIGNAL(clicked()), SLOT(chooseSendmail()) );
      connect( d->sendmail.kcfg_host, SIGNAL(textChanged(QString)), SLOT(hostNameChanged(QString)) );
    }
  }

  // load the password if necessary
  if ( d->passwordEdit ) {
    if ( d->transport->isComplete() )
      d->passwordEdit->setText( d->transport->password() );
    else
      if ( d->transport->requiresAuthentication() )
        TransportManager::self()->loadPasswordsAsync();
  }

  d->manager = new KConfigDialogManager( this, transport );
  d->manager->updateWidgets();
  hostNameChanged( d->transport->host() );
}

TransportConfigDialog::~ TransportConfigDialog()
{
  delete d;
}

void TransportConfigDialog::checkSmtpCapabilities()
{
  Q_ASSERT( d->transport->type() == Transport::EnumType::SMTP );

  delete d->serverTest;
  d->serverTest = new ServerTest( SMTP_PROTOCOL, d->smtp.kcfg_host->text(), d->smtp.kcfg_port->value() );
  connect( d->serverTest,
           SIGNAL( capabilities(QStringList,QStringList,QString,QString,QString)),
           SLOT( smtpCapabilities(QStringList,QStringList,QString,QString,QString)) );
  d->smtp.checkCapabilities->setEnabled( false );
}

void TransportConfigDialog::save()
{
  d->manager->updateSettings();
  if ( d->passwordEdit )
    d->transport->setPassword( d->passwordEdit->text() );

  // enforce unique name
  QStringList existingNames;
  foreach ( Transport *t, TransportManager::self()->transports() )
    if ( t->id() != d->transport->id() )
      existingNames << t->name();
  int suffix = 1;
  QString origName = d->transport->name();
  while ( existingNames.contains( d->transport->name() ) ) {
    d->transport->setName( i18nc("%1: name; %2: number appended to it to make it unique among a list of names", "%1 %2",
                           origName, suffix ) );
    ++suffix;
  }

  d->transport->writeConfig();
}

void TransportConfigDialog::chooseSendmail()
{
  Q_ASSERT( d->transport->type() == Transport::EnumType::Sendmail );

  KFileDialog dialog( KUrl("/"), QString(), this );
  dialog.setCaption( i18n("Choose sendmail Location") );

  if ( dialog.exec() == QDialog::Accepted ) {
    KUrl url = dialog.selectedUrl();
    if ( url.isEmpty() == true )
      return;
    if ( !url.isLocalFile() ) {
      KMessageBox::sorry( this, i18n( "Only local files allowed." ) );
      return;
    }
    d->sendmail.kcfg_host->setText( url.path() );
  }
}

void TransportConfigDialog::passwordsLoaded()
{
  Q_ASSERT( d->passwordEdit );

  if ( d->passwordEdit->text().isEmpty() )
    d->passwordEdit->setText( d->transport->password() );
}

static QList<int> authMethodsFromStringList( const QStringList &list )
{
  QList<int> result;
  for ( QStringList::ConstIterator it = list.begin() ; it != list.end() ; ++it ) {
    if (  *it == "LOGIN" )
      result << Transport::EnumAuthenticationType::LOGIN;
   else if ( *it == "PLAIN" )
      result << Transport::EnumAuthenticationType::PLAIN;
    else if ( *it == "CRAM-MD5" )
      result << Transport::EnumAuthenticationType::CRAM_MD5;
    else if ( *it == "DIGEST-MD5" )
      result << Transport::EnumAuthenticationType::DIGEST_MD5;
    else if ( *it == "NTLM" )
      result << Transport::EnumAuthenticationType::NTLM;
    else if ( *it == "GSSAPI" )
      result << Transport::EnumAuthenticationType::GSSAPI;
  }

  // LOGIN doesn't offer anything over PLAIN, requires more server
  // roundtrips and is not an official SASL mechanism, but a MS-ism,
  // so only enable it if PLAIN isn't available:
  if ( result.contains( Transport::EnumAuthenticationType::PLAIN ) )
    result.removeAll( Transport::EnumAuthenticationType::LOGIN );

  return result;
}

static QList<int> authMethodsFromString( const QString &s )
{
  QStringList list;
  foreach ( QString tmp, s.toUpper().split( '\n', QString::SkipEmptyParts ) )
    list << tmp.remove( "SASL/" );
  return authMethodsFromStringList( list );
}

static void checkHighestEnabledButton( QButtonGroup *group )
{
  Q_ASSERT( group );

  for ( int i = group->buttons().count() - 1; i >= 0 ; --i ) {
    QAbstractButton *b = group->buttons().at( i );
    if ( b && b->isEnabled() ) {
      b->animateClick();
      return;
    }
  }
}

void TransportConfigDialog::smtpCapabilities( const QStringList &capaNormal, const QStringList &capaSSL,
                                              const QString &authNone, const QString &authSSL, const QString &authTLS )
{
  d->smtp.checkCapabilities->setEnabled( true );

  // encryption method
  d->smtp.none->setEnabled( !capaNormal.isEmpty() );
  d->smtp.ssl->setEnabled( !capaSSL.isEmpty() );
  d->smtp.tls->setEnabled( capaNormal.indexOf("STARTTLS") != -1 );
  checkHighestEnabledButton( d->encryptionGroup );

  // authentication methods
  if ( authNone.isEmpty() && authSSL.isEmpty() && authTLS.isEmpty() ) {
    // slave doesn't seem to support "* AUTH METHODS" metadata (or server can't do AUTH)
    d->noEncCapa = authMethodsFromStringList( capaNormal );
    if ( d->smtp.tls->isEnabled() )
      d->tlsCapa = d->noEncCapa;
    else
      d->tlsCapa.clear();
    d->sslCapa = authMethodsFromStringList( capaSSL );
  } else {
    d->noEncCapa = authMethodsFromString( authNone );
    d->sslCapa = authMethodsFromString( authSSL );
    d->tlsCapa = authMethodsFromString( authTLS );
  }
  d->updateAuthCapbilities();
  checkHighestEnabledButton( d->authGroup );

  delete d->serverTest;
  d->serverTest = 0;
}

void TransportConfigDialog::hostNameChanged( const QString &text )
{
  d->resetAuthCapabilities();
  enableButton( Ok, !text.isEmpty() );
  for ( int i = 0; d->encryptionGroup && i < d->encryptionGroup->buttons().count(); i++ )
    d->encryptionGroup->buttons().at( i )->setEnabled( true );
}

void TransportConfigDialog::encryptionChanged(int enc)
{
  Q_ASSERT( d->transport->type() == Transport::EnumType::SMTP );
  kDebug() << k_funcinfo << enc << endl;

  // adjust port
  if ( enc == Transport::EnumEncryption::SSL ) {
    if ( d->smtp.kcfg_port->value() == SMTP_PORT )
      d->smtp.kcfg_port->setValue( SMTPS_PORT );
  } else {
    if ( d->smtp.kcfg_port->value() == SMTPS_PORT )
      d->smtp.kcfg_port->setValue( SMTP_PORT );
  }

  // adjust available authentication methods
  d->updateAuthCapbilities();
  foreach ( QAbstractButton* b, d->authGroup->buttons() ) {
    if ( b->isChecked() && !b->isEnabled() ) {
      checkHighestEnabledButton( d->authGroup );
      break;
    }
  }
}

#include "transportconfigdialog.moc"
