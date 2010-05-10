/*
    Copyright (c) 2010 Volker Krause <vkrause@kde.org>

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

#include "composerview.h"

#include "declarativeeditor.h"
#include "declarativeidentitycombobox.h"

#include <kpimidentities/identity.h>
#include <kpimidentities/identitycombo.h>
#include <kpimidentities/identitymanager.h>
#include <mailtransport/messagequeuejob.h>
#include <mailtransport/transportmanager.h>
#include <messagecomposer/kmeditor.h>
#include <messagecomposer/signaturecontroller.h>
#include <messagecomposer/composer.h>
#include <messagecomposer/globalpart.h>
#include <messagecomposer/infopart.h>
#include <messagecomposer/textpart.h>

#include <klocalizedstring.h>
#include <KDebug>
#include <qdeclarativecontext.h>
#include <qdeclarativeengine.h>

QML_DECLARE_TYPE( DeclarativeEditor )
QML_DECLARE_TYPE( DeclarativeIdentityComboBox )

ComposerView::ComposerView(QWidget* parent) :
  KDeclarativeFullScreenView( QLatin1String( "kmail-composer" ), parent ),
  m_identityCombo( 0 ),
  m_editor( 0 )
{
  setWindowTitle( i18n( "New mail" ) );

  qmlRegisterType<DeclarativeEditor>( "org.kde.messagecomposer", 4, 5, "Editor" );
  qmlRegisterType<DeclarativeIdentityComboBox>( "org.kde.kpimidentities", 4, 5, "IdentityComboBox" );

  connect( this, SIGNAL(statusChanged(QDeclarativeView::Status)), SLOT(qmlLoaded(QDeclarativeView::Status)) );
}

void ComposerView::qmlLoaded ( QDeclarativeView::Status status )
{
  if ( status != QDeclarativeView::Ready )
    return;

  Q_ASSERT( m_identityCombo );
  Q_ASSERT( m_editor );

  kDebug() << m_identityCombo;
  kDebug() << m_editor;

  Message::SignatureController *signatureController = new Message::SignatureController( this );
  signatureController->setEditor( m_editor );
  signatureController->setIdentityCombo( m_identityCombo );
  signatureController->applyCurrentSignature();
}

void ComposerView::send()
{
  kDebug();
  // ### temporary, more code can be shared with kmail here
  Message::Composer* composer = new Message::Composer( this );
  composer->globalPart()->setCharsets( QList<QByteArray>() << "utf-8" );
  composer->globalPart()->setParentWidgetForGui( this );
  composer->infoPart()->setTo( QStringList() << "volker@kdab.com" );
  composer->infoPart()->setFrom( "volker@kdab.com" );
  m_editor->fillComposerTextPart( composer->textPart() );
  connect( composer, SIGNAL(result(KJob*)), SLOT(composerResult(KJob*)) );
  composer->start();
}

void ComposerView::composerResult ( KJob* job )
{
  kDebug() << job->error() << job->errorText();
  if ( !job->error() ) {
    Message::Composer *composer = qobject_cast<Message::Composer*>( job );
    Q_ASSERT( composer );
    const Message::InfoPart *infoPart = composer->infoPart();
    MailTransport::MessageQueueJob *qjob = new MailTransport::MessageQueueJob( this );
    qjob->transportAttribute().setTransportId( MailTransport::TransportManager::self()->defaultTransportId() );
    qjob->setMessage( composer->resultMessages().first() );
    qjob->addressAttribute().setTo( infoPart->to() );
    qjob->addressAttribute().setCc( infoPart->cc() );
    qjob->addressAttribute().setBcc( infoPart->bcc() );
    connect( qjob, SIGNAL(result(KJob*)), SLOT(sendResult(KJob*)) );
    qjob->start();
  }
}

void ComposerView::sendResult ( KJob* job )
{
  kDebug() << job->error() << job->errorText();
}

#include "composerview.moc"
