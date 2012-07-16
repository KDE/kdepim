/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 * Copyright (C) 2012 Andras Mantia <amantia@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "filteraction.h"

#include "../mailkernel.h"
#include "../mdnadvicedialog.h"

#include <messagecomposer/messagefactory.h>
#include <messagecomposer/messagesender.h>
#include <messagecore/messagehelpers.h>
#include <messageviewer/globalsettings.h>

#include <KDE/KLocale>

using namespace MailCommon;

FilterAction::FilterAction( const char *name, const QString &label, QObject *parent )
  : QObject( parent ), mName( name ), mLabel( label )
{
}

FilterAction::~FilterAction()
{
}

QString FilterAction::label() const
{
  return mLabel;
}

QString FilterAction::name() const
{
  return mName;
}

bool FilterAction::isEmpty() const
{
  return false;
}

FilterAction* FilterAction::newAction()
{
  return 0;
}

QWidget* FilterAction::createParamWidget( QWidget *parent ) const
{
  return new QWidget( parent );
}

void FilterAction::applyParamWidgetValue( QWidget * )
{
}

void FilterAction::setParamWidgetValue( QWidget * ) const
{
}

void FilterAction::clearParamWidget( QWidget * ) const
{
}

bool FilterAction::argsFromStringInteractive( const QString &argsStr, const QString & filterName )
{
  Q_UNUSED( filterName );
  argsFromString(argsStr);
  return false;
}

QString FilterAction::argsAsStringReal() const
{
  return argsAsString();
}

bool FilterAction::folderRemoved( const Akonadi::Collection&, const Akonadi::Collection& )
{
  return false;
}

void FilterAction::sendMDN( const Akonadi::Item &item, KMime::MDN::DispositionType type,
                            const QList<KMime::MDN::DispositionModifier> &modifiers )
{
  const KMime::Message::Ptr msg = MessageCore::Util::message( item );
  if ( !msg )
    return;

  const QPair<bool, KMime::MDN::SendingMode> mdnSend = MDNAdviceHelper::instance()->checkAndSetMDNInfo( item, type, true );
  if ( mdnSend.first ) {
    const int quote =  MessageViewer::GlobalSettings::self()->quoteMessage();
    MessageComposer::MessageFactory factory( msg, Akonadi::Item().id() );
    factory.setIdentityManager( KernelIf->identityManager() );

    const KMime::Message::Ptr mdn = factory.createMDN( KMime::MDN::AutomaticAction, type, mdnSend.second, quote, modifiers );
    if ( mdn ) {
      if ( !KernelIf->msgSender()->send( mdn, MessageSender::SendLater ) ) {
        kDebug() << "Sending failed.";
      }
    }
  }
}

#include "filteraction.moc"
