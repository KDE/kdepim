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

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>

SearchWidget::SearchWidget( QWidget *parent )
  : QWidget( parent )
{
  mUi.setupUi( this );

  // set defaults
  mUi.inNames->setChecked( true );
  mUi.inEmailAddresses->setChecked( true );
  mUi.inCategories->setChecked( true );
  mUi.collectionCombo->setMimeTypeFilter( QStringList() << KABC::Addressee::mimeType()
                                                        << KABC::ContactGroup::mimeType() );
}

QString SearchWidget::query() const
{
#ifdef AKONADI_USE_STRIGI_SEARCH
  QStringList containsPatternParts;

  if ( mUi.inNames->isChecked() ) {
    containsPatternParts << QString::fromLatin1(
                                                 "<contains>"
                                                 "  <field name=\"nameGiven\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"nameAdditional\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"nameFamily\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"fullname\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"nickname\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"contactGroupName\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                               ).arg( mUi.searchText->text().toLower() );
  }
  if ( mUi.inEmailAddresses->isChecked() ) {
    containsPatternParts << QString::fromLatin1( "<contains>"
                                                 "  <field name=\"emailAddress\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                               ).arg( mUi.searchText->text().toLower() );
  }
  if ( mUi.inPostalAddresses->isChecked() ) {
    containsPatternParts << QString::fromLatin1(
                                                 "<contains>"
                                                 "  <field name=\"country\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"locality\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"pobox\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"postalcode\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"region\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                                 "<contains>"
                                                 "  <field name=\"streetAddress\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                               ).arg( mUi.searchText->text().toLower() );
  }
  if ( mUi.inCategories->isChecked() ) {
    containsPatternParts << QString::fromLatin1( "<contains>"
                                                 "  <field name=\"categories\"/>"
                                                 "  <string>%1</string>"
                                                 "</contains>"
                                               ).arg( mUi.searchText->text().toLower() );
  }
  if ( mUi.inPhoneNumbers->isChecked() ) {
    containsPatternParts << QString::fromLatin1( "<contains>"
                                                 "  <field name=\"phoneNumber\"/>"
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

  const QString isContactOrGroup = QString::fromLatin1(
                                                        "<or>"
                                                        "  <equals>"
                                                        "    <field name=\"type\"/>"
                                                        "    <string>PersonContact</string>"
                                                        "  </equals>"
                                                        "  <equals>"
                                                        "    <field name=\"type\"/>"
                                                        "    <string>ContactGroup</string>"
                                                        "  </equals>"
                                                        "</or>"
                                                      );

  QString query;
  query += QLatin1String( "<request><query>" );

  query += QLatin1String( "  <and>" );
  query += isContactOrGroup;
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

