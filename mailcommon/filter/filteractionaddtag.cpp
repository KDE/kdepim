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
#include "filtermanager.h"
#include "filteractionmissingargumentdialog.h"
#include "messageviewer/minimumcombobox.h"

#include <Nepomuk2/Tag>
#include <Nepomuk2/Resource>
#include <Nepomuk2/ResourceManager>

#include <QTextDocument>
#include <QPointer>

using namespace MailCommon;

FilterAction* FilterActionAddTag::newAction()
{
  return new FilterActionAddTag;
}

FilterActionAddTag::FilterActionAddTag( QObject *parent )
  : FilterAction( "add tag", i18n( "Add Tag" ), parent )
{
    mList = FilterManager::instance()->tagList();
    connect(FilterManager::instance(),SIGNAL(tagListingFinished()),SLOT(slotTagListingFinished()));
}

QWidget* FilterActionAddTag::createParamWidget( QWidget *parent ) const
{
  MessageViewer::MinimumComboBox *comboBox = new MessageViewer::MinimumComboBox( parent );
  comboBox->setEditable( false );
  QMapIterator<QUrl, QString> i(mList);
  while (i.hasNext()) {
      i.next();
      comboBox->addItem(i.value(), i.key());
  }

  setParamWidgetValue( comboBox );

  connect( comboBox, SIGNAL(currentIndexChanged(int)),
           this, SIGNAL(filterActionModified()) );

  return comboBox;
}

void FilterActionAddTag::applyParamWidgetValue( QWidget *paramWidget )
{
    MessageViewer::MinimumComboBox* combo = static_cast<MessageViewer::MinimumComboBox*>( paramWidget );
    mParameter = combo->itemData(combo->currentIndex()).toString();
}

void FilterActionAddTag::setParamWidgetValue( QWidget *paramWidget ) const
{
  const int index = static_cast<MessageViewer::MinimumComboBox*>( paramWidget )->findData(mParameter);

  static_cast<MessageViewer::MinimumComboBox*>( paramWidget )->setCurrentIndex( index < 0 ? 0 : index );
}

void FilterActionAddTag::clearParamWidget( QWidget *paramWidget ) const
{
  static_cast<MessageViewer::MinimumComboBox*>( paramWidget )->setCurrentIndex( 0 );
}


bool FilterActionAddTag::isEmpty() const
{
  return false;
}

void FilterActionAddTag::slotTagListingFinished()
{
    mList = FilterManager::instance()->tagList();
}

bool FilterActionAddTag::argsFromStringInteractive( const QString &argsStr, const QString& filterName )
{
  bool needUpdate = false;
  argsFromString( argsStr );
  if( mList.isEmpty() )
    return false;
  const bool index = mList.contains( mParameter );
  if ( Nepomuk2::ResourceManager::instance()->initialized() ) {
    if ( !index ) {
        //TODO adapt it.
      QPointer<FilterActionMissingTagDialog> dlg = new FilterActionMissingTagDialog( mList, filterName, argsStr );
      if ( dlg->exec() ) {
        mParameter = dlg->selectedTag();
        needUpdate = true;
      }
      delete dlg;
    }
  }
  return needUpdate;
}


FilterAction::ReturnCode FilterActionAddTag::process( ItemContext &context ) const
{
  if(!mList.contains(mParameter)) {
    return ErrorButGoOn;
  }
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
  if( mList.isEmpty() ) {
    mParameter = argsStr;
    return;
  }
  if(mList.contains(argsStr)) {
      mParameter = argsStr;
      return;
  }
  if ( !mList.isEmpty() )
      mParameter = mList.values().at(0);
}

QString FilterActionAddTag::argsAsString() const
{
  if(!mList.contains(mParameter)) {
     return QString();
  }

  return mList.value(mParameter);
}

QString FilterActionAddTag::displayString() const
{
  return label() + QLatin1String( " \"" ) + Qt::escape( argsAsString() ) + QLatin1String( "\"" );
}

#include "filteractionaddtag.moc"
