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

SearchWidget::SearchWidget( QWidget *parent )
  : QWidget( parent )
{
  mUi.setupUi( this );

  // set defaults
  mUi.inTitles->setChecked( true );
  mUi.inContents->setChecked( true );
  mUi.collectionCombo->setMimeTypeFilter( QStringList() << QLatin1String( "text/x-vnd.akonadi.note" ) );
}

QString SearchWidget::query() const
{
#ifdef AKONADI_USE_STRIGI_SEARCH
  QStringList containsPatternParts;

  if ( mUi.inTitles->isChecked() ) {
    containsPatternParts << QString::fromLatin1( "<contains>"
                                                 "  <field name=\"subject\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                               ).arg( mUi.searchText->text().toLower() );
  }
  if ( mUi.inContents->isChecked() ) {
    containsPatternParts << QString::fromLatin1( "<contains>"
                                                 "  <field name=\"plainTextMessageContent\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                               ).arg( mUi.searchText->text().toLower() );
  }

  const QString containsPattern = containsPatternParts.isEmpty() ? QString() :
                                                                   QLatin1String( "<or>" ) +
                                                                   containsPatternParts.join( QLatin1String( "\n" ) ) +
                                                                   QLatin1String( "</or>" );

  const QString inCollection = QString::fromLatin1( "<equals>"
                                                    "  <field name=\"isPartOf\"/>"
                                                    "  <string>%1</string>"
                                                    "</equals>"
                                                  ).arg( mUi.collectionCombo->currentCollection().id() );

  const QString isNote = QString::fromLatin1(
                                              "<equals>"
                                              "  <field name=\"type\"/>"
                                              "  <string>Note</string>"
                                              "</equals>"
                                            );

  QString query;
  query += QLatin1String( "<request><query>" );

  query += QLatin1String( "  <and>" );
  query += isNote;
  query += containsPattern;
  if ( mUi.locatedInSpecificCollection->isChecked() )
    query += inCollection;
  query += QLatin1String( "  </and>" );

  query += QLatin1String( "</query></request>" );

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

