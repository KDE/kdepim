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

#include "filteractionremoveheader.h"

#include "../minimumcombobox.h"

#include <KDE/KLocale>

using namespace MailCommon;

FilterAction* FilterActionRemoveHeader::newAction()
{
  return new FilterActionRemoveHeader;
}

FilterActionRemoveHeader::FilterActionRemoveHeader( QObject *parent )
  : FilterActionWithStringList( "remove header", i18n( "Remove Header" ), parent )
{
  mParameterList << ""
                 << "Reply-To"
                 << "Delivered-To"
                 << "X-KDE-PR-Message"
                 << "X-KDE-PR-Package"
                 << "X-KDE-PR-Keywords";

  mParameter = mParameterList.at( 0 );
}

QWidget* FilterActionRemoveHeader::createParamWidget( QWidget *parent ) const
{
  MinimumComboBox *comboBox = new MinimumComboBox( parent );
  comboBox->setEditable( true );
  comboBox->setInsertPolicy( QComboBox::InsertAtBottom );
  setParamWidgetValue( comboBox );

  connect( comboBox, SIGNAL(currentIndexChanged(int)),
           this, SIGNAL(filterActionModified()) );

  return comboBox;
}

FilterAction::ReturnCode FilterActionRemoveHeader::process( ItemContext &context ) const
{
  if ( mParameter.isEmpty() )
    return ErrorButGoOn;

  KMime::Message::Ptr msg = context.item().payload<KMime::Message::Ptr>();
  while ( msg->headerByType( mParameter.toLatin1() ) )
    msg->removeHeader( mParameter.toLatin1() );

  msg->assemble();

  context.setNeedsPayloadStore();

  return GoOn;
}

void FilterActionRemoveHeader::setParamWidgetValue( QWidget *paramWidget ) const
{
  MinimumComboBox *comboBox = dynamic_cast<MinimumComboBox*>(paramWidget );
  Q_ASSERT( comboBox );

  const int index = mParameterList.indexOf( mParameter );
  comboBox->clear();
  comboBox->addItems( mParameterList );
  if ( index < 0 ) {
    comboBox->addItem( mParameter );
    comboBox->setCurrentIndex( comboBox->count() - 1 );
  } else {
    comboBox->setCurrentIndex( index );
  }
}

