/*
    Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Copyright (c) 2010 Tobias Koenig <tobias.koenig@kdab.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "searchwidget.h"

#include "stylesheetloader.h"

#include <QtCore/QDate>
#include <QtCore/QDebug>

SearchWidget::SearchWidget( QWidget *parent )
  : QWidget( parent )
{
  mUi.setupUi( this );

  // set defaults
  mUi.inSummaries->setChecked( true );
  mUi.inDescriptions->setChecked( true );
  mUi.startDate->setDate( QDate::currentDate() );
  mUi.endDate->setDate( QDate::currentDate().addDays( 1 ) );

  // UI workarounds for Maemo5
#ifdef Q_WS_MAEMO_5
  mUi.startDate->setEditable( false );
  mUi.endDate->setEditable( false );
#endif
}

QString SearchWidget::query() const
{
#ifdef AKONADI_USE_STRIGI_SEARCH
  QString query;

  query += "<or>";
  if ( mUi.inSummaries->isChecked() ) {
    query += QString::fromLatin1( "<contains>"
                                  "  <field name=\"summary\"/>"
                                  "  <string>%1</string>"
                                  "</contains>"
                                ).arg( mUi.searchText->text() );
  }
  if ( mUi.inDescriptions->isChecked() ) {
    query += QString::fromLatin1( "<contains>"
                                  "  <field name=\"description\"/>"
                                  "  <string>%1</string>"
                                  "</contains>"
                                ).arg( mUi.searchText->text() );
  }
  if ( mUi.inCategories->isChecked() ) {
    query += QString::fromLatin1( "<contains>"
                                  "  <field name=\"categories\"/>"
                                  "  <string>%1</string>"
                                  "</contains>"
                                ).arg( mUi.searchText->text() );
  }
  if ( mUi.inLocations->isChecked() ) {
    query += QString::fromLatin1( "<contains>"
                                  "  <field name=\"location\"/>"
                                  "  <string>%1</string>"
                                  "</contains>"
                                ).arg( mUi.searchText->text() );
  }
  query += "</or>";

  if ( mUi.includeDateRange->isChecked() ) {
    query.prepend( "<and>" );

    const QString startDate = mUi.startDate->date().toString( "yyyyMMdd" );
    const QString endDate = mUi.endDate->date().toString( "yyyyMMdd" );

    query += QString::fromLatin1( "<greaterThanEquals>"
                                  "  <field name=\"dtstart\"/>"
                                  "  <string>%1</string>"
                                  "</greaterThanEquals>"
                                  "<lessThanEquals>"
                                  "  <field name=\"dtstart\"/>"
                                  "  <string>%2</string>"
                                  "</lessThanEquals>" ).arg( startDate ).arg( endDate );
    query.append( "</and>" );
  }

  query.prepend( "<request><query>" );
  query += "</query></request>";

  return query;
#else

  return QString();
#endif
}

DeclarativeSearchWidget::DeclarativeSearchWidget( QGraphicsItem *parent )
  : QGraphicsProxyWidget( parent ), mSearchWidget( new SearchWidget )
{
  QPalette palette = mSearchWidget->palette();
  palette.setColor( QPalette::Window, QColor( 0, 0, 0, 0 ) );
  mSearchWidget->setPalette( palette );
  StyleSheetLoader::applyStyle( mSearchWidget );

  setWidget( mSearchWidget );
  setFocusPolicy( Qt::StrongFocus );
}

DeclarativeSearchWidget::~DeclarativeSearchWidget()
{
}

QString DeclarativeSearchWidget::query() const
{
  return mSearchWidget->query();
}

#include "searchwidget.moc"
