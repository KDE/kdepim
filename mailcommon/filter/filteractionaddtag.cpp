/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
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

#include "filteractionaddtag.h"

#include "filteractionmissingargumentdialog.h"

#include <Nepomuk2/Tag>
#include <Nepomuk2/Resource>
#include <Nepomuk2/Query/ResourceTypeTerm>
#include <Nepomuk2/Query/QueryServiceClient>
#include <Nepomuk2/Query/Result>
#include <Soprano/Vocabulary/NAO>

#include <QTextDocument>
#include <QPointer>

using namespace MailCommon;

FilterAction* FilterActionAddTag::newAction()
{
  return new FilterActionAddTag;
}

FilterActionAddTag::FilterActionAddTag( QObject *parent )
  : FilterActionWithStringList( "add tag", i18n( "Add Tag" ), parent )
{
  initializeTagList();
}

bool FilterActionAddTag::isEmpty() const
{
  return false;
}

void FilterActionAddTag::initializeTagList()
{
  mTagQueryClient = new Nepomuk2::Query::QueryServiceClient(this);
  connect( mTagQueryClient, SIGNAL(newEntries(QList<Nepomuk2::Query::Result>)),
           this, SLOT(newTagEntries(QList<Nepomuk2::Query::Result>)) );
  connect( mTagQueryClient, SIGNAL(finishedListing()),
           this, SLOT(finishedTagListing()) );

  Nepomuk2::Query::Query query( Nepomuk2::Query::ResourceTypeTerm( Soprano::Vocabulary::NAO::Tag() ) );
  mTagQueryClient->query(query);
 }

void FilterActionAddTag::newTagEntries( const QList<Nepomuk2::Query::Result> &results )
{
  foreach(const Nepomuk2::Query::Result &result, results ) {
    Nepomuk2::Resource resource = result.resource();
    mParameterList.append( resource.label() );
    mLabelList.append( resource.uri().toString( ));
  }
}

void FilterActionAddTag::finishedTagListing()
{
  mTagQueryClient->deleteLater();
  mTagQueryClient = 0;
}


bool FilterActionAddTag::argsFromStringInteractive( const QString &argsStr, const QString& filterName )
{
  bool needUpdate = false;
  argsFromString( argsStr );
  if( mParameterList.isEmpty() )
    return false;
  const int index = mParameterList.indexOf( mParameter );
  if ( index == -1 ) {
    QPointer<FilterActionMissingTagDialog> dlg = new FilterActionMissingTagDialog( mParameterList, filterName, argsStr );
    if ( dlg->exec() ) {
      mParameter = dlg->selectedTag();
      needUpdate = true;
    }
    delete dlg;
  }
  return needUpdate;
}


FilterAction::ReturnCode FilterActionAddTag::process( ItemContext &context ) const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index == -1 )
    return ErrorButGoOn;

  Nepomuk2::Resource resource( context.item().url() );
  resource.addTag( mParameter );

  return GoOn;
}

SearchRule::RequiredPart FilterActionAddTag::requiredPart() const
{
  return SearchRule::Envelope;
}

void FilterActionAddTag::argsFromString( const QString &argsStr )
{
  if( mParameterList.isEmpty() ) {
    mParameter = argsStr;
    return;
  }

  foreach ( const QString& tag, mParameterList ) {
    if ( tag == argsStr ) {
      mParameter = tag;
      return;
    }
  }

  if ( !mParameterList.isEmpty() )
    mParameter = mParameterList.at( 0 );
}

QString FilterActionAddTag::argsAsString() const
{
  const int index = mParameterList.indexOf( mParameter );
  if ( index == -1 )
    return QString();

  return mParameterList.at( index );
}

QString FilterActionAddTag::displayString() const
{
  return label() + QLatin1String( " \"" ) + Qt::escape( argsAsString() ) + QLatin1String( "\"" );
}
