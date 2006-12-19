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

#include "ui_smtpsettings.h"
#include "ui_sendmailsettings.h"

#include <libkdepim/servertest.h>

#include <kconfigdialogmanager.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kprotocolinfo.h>

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
};

TransportConfigDialog::TransportConfigDialog( Transport* transport, QWidget * parent) :
    KDialog( parent ),
    d( new Private )
{
  Q_ASSERT( transport );

  d->transport = transport;
  d->passwordEdit = 0;
  d->serverTest = 0;

  setButtons( Ok|Cancel );
  connect( this, SIGNAL(okClicked()), SLOT(save()) );
  connect( TransportManager::self(), SIGNAL(passwordsChanged()), SLOT(passwordsLoaded()) );

  switch ( transport->type() ) {
    case Transport::EnumType::SMTP:
    {
      d->smtp.setupUi( mainWidget() );
      d->passwordEdit = d->smtp.password;

      if ( KProtocolInfo::capabilities("smtp").contains("SASL") == 0 ) {
        d->smtp.ntlm->hide();
        d->smtp.gssapi->hide();
      }

      connect( d->smtp.checkCapabilities, SIGNAL(clicked()), SLOT(checkSmtpCapabilities()) );
      break;
    }
    case Transport::EnumType::Sendmail:
    {
      d->sendmail.setupUi( mainWidget() );

      connect( d->sendmail.chooseButton, SIGNAL(clicked()), SLOT(chooseSendmail()) );
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
}

TransportConfigDialog::~ TransportConfigDialog()
{
  delete d;
}

void TransportConfigDialog::checkSmtpCapabilities()
{
  Q_ASSERT( d->transport->type() == Transport::EnumType::SMTP );

  delete d->serverTest;
  d->serverTest = new ServerTest( "smtp", d->smtp.kcfg_host->text(), d->smtp.kcfg_port->text().toInt() );
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

void TransportConfigDialog::smtpCapabilities( const QStringList &capaNormal, const QStringList &capaSSL,
                                              const QString &authNone, const QString &authSSL, const QString &authTLS )
{
  kDebug() << k_funcinfo << capaNormal << capaSSL << authNone << authSSL << authTLS << endl;
  d->smtp.checkCapabilities->setEnabled( true );
  // TODO: need to fix kio_smtp first
}

#include "transportconfigdialog.moc"
