/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>
    Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

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

#include "nepomukemailfeeder.h"
#include "messageanalyzer.h"
#include "messagesearch.h"
#include "configdialog.h"
#include "settings.h"

#include <kmime/kmime_message.h>

#include <akonadi/changerecorder.h>
#include <akonadi/item.h>
#include <akonadi/itemfetchscope.h>

#include <gpgme++/context.h>

using namespace Akonadi;

Akonadi::NepomukEMailFeeder::NepomukEMailFeeder( const QString &id ) :
  NepomukFeederAgent<NepomukFast::Mailbox>( id )
{
  addSupportedMimeType( "message/rfc822" );
  addSupportedMimeType( "message/news" );

  setNeedsStrigi( true );

  changeRecorder()->itemFetchScope().fetchFullPayload();

  // failsafe in case we don't have / lost G13 support
  if ( Settings::self()->indexEncryptedContent() == Settings::EncryptedIndex && !GpgME::hasFeature( GpgME::G13VFSFeature ) )
    Settings::self()->setIndexEncryptedContent( Settings::NoIndexing );
  Settings::self()->writeConfig();
}

void NepomukEMailFeeder::updateItem(const Akonadi::Item & item, const QUrl &graphUri)
{
  if ( !item.hasPayload<KMime::Message::Ptr>() )
    return;
  new MessageAnalyzer( item, graphUri, this );
}

void NepomukEMailFeeder::configure(WId windowId)
{
  ConfigDialog* dlg = new ConfigDialog( windowId );
  dlg->setAttribute( Qt::WA_DeleteOnClose );
  dlg->show();
}

void NepomukEMailFeeder::addSearch(const QString& query, const QString& queryLanguage, const Akonadi::Collection& resultCollection)
{
  if ( Settings::self()->indexEncryptedContent() != Settings::EncryptedIndex || !GpgME::hasFeature( GpgME::G13VFSFeature ) )
    return;

  if ( queryLanguage != QLatin1String( "SPARQL" ) ) {
    kDebug() << "Unsupported query language: " << queryLanguage;
    return;
  }

  new MessageSearch( query, resultCollection, this );
}

void NepomukEMailFeeder::removeSearch(const Akonadi::Collection& resultCollection)
{
  // ignored since we don't do persistent searches yet
  Q_UNUSED( resultCollection );
}

AKONADI_AGENT_MAIN( NepomukEMailFeeder )

#include "nepomukemailfeeder.moc"
