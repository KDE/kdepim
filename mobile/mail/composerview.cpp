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
#include "declarativerecipientseditor.h"

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
#include <KIcon>
#include <KAction>
#include <qdeclarativecontext.h>
#include <qdeclarativeengine.h>

QML_DECLARE_TYPE( DeclarativeEditor )
QML_DECLARE_TYPE( DeclarativeIdentityComboBox )

ComposerView::ComposerView(QWidget* parent) :
  KDeclarativeFullScreenView( QLatin1String( "kmail-composer" ), parent ),
  m_identityCombo( 0 ),
  m_editor( 0 )
{
  setSubject( QString() );

  qmlRegisterType<DeclarativeEditor>( "org.kde.messagecomposer", 4, 5, "Editor" );
  qmlRegisterType<DeclarativeIdentityComboBox>( "org.kde.kpimidentities", 4, 5, "IdentityComboBox" );
  qmlRegisterType<DeclarativeRecipientsEditor>( "org.kde.messagecomposer", 4, 5, "RecipientsEditor" );

  // TODO: Really make this application-global;
  mActionCollection = new KActionCollection( this );
  KAction *action = mActionCollection->addAction( "add_attachment" );
  action->setText( i18n( "Add Attachment" ) );
  action->setIcon( KIcon( "list-add" ) );

  engine()->rootContext()->setContextProperty( "application", QVariant::fromValue( static_cast<QObject*>( this ) ) );

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
  composer->infoPart()->setSubject( subject() );
  composer->infoPart()->setTo( QStringList() << "volker@kdab.com" );
  composer->infoPart()->setFrom( "volker@kdab.com" );
  composer->infoPart()->setUserAgent( "KMail Mobile" );
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
  if ( !job->error() )
    deleteLater();
}

QString ComposerView::subject() const
{
  return m_subject;
}

void ComposerView::setSubject ( const QString& subject )
{
  m_subject = subject;
  if ( !subject.isEmpty() )
    setWindowTitle( subject );
  else
    setWindowTitle( i18n( "New mail" ) );
}

KActionCollection* ComposerView::actionCollection() const
{
  return mActionCollection;
}

QObject* ComposerView::getAction( const QString &name ) const
{
  kDebug() << mActionCollection << mActionCollection->action( name );
  return mActionCollection->action( name );
}

#include "composerview.moc"
