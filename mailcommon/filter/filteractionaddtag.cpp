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

#include <nepomuk2/tag.h>
#include <Nepomuk2/Resource>

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
  foreach( const Nepomuk2::Tag &tag, Nepomuk2::Tag::allTags() ) {
    mParameterList.append( tag.label() );
    mLabelList.append( tag.uri().toString() );
  }
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
