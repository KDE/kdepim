/*
  This file is part of KAddressBook.
  Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                     2002 Mike Pilone <mpilone@slac.com>
                     2009 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "mikesstyle.h"
#include "contactfields.h"
#include "printingwizard.h"
#include "printprogress.h"
#include "printstyle.h"

#include <KABC/Addressee>

#include <KDebug>
#include <KLocale>

#include <QtGui/QPrinter>
#include <QtGui/QTextDocument>

using namespace KABPrinting;

static QString contactsToHtml( const KABC::Addressee::List &contacts )
{
  QString content;

  ContactFields::Fields leftFields, rightFields;
  ContactFields::Fields allFields = ContactFields::allFields();
  allFields.remove( 0 ); // drop 'Undefined' field

  const int middle = allFields.count() / 2;

  for ( int i = 0; i < middle; ++i ) {
    leftFields.append( allFields.at( i ) );
  }

  for ( int i = middle; i < allFields.count(); ++i ) {
    rightFields.append( allFields.at( i ) );
  }

  int counter = 0;
  content += "<html>\n";
  content += " <body>\n";
  foreach ( const KABC::Addressee &contact, contacts ) {
    const int max = qMax( leftFields.count(), rightFields.count() );

    const QString name = contact.realName();

    if ( counter % 2 ) {
      content += "  <br/><br/>\n";
    }

    // start a new page after every second table
    const QString pageBreak = ( counter % 2 ? "page-break-after: always;" : QString() );

    content += "  <table style=\"border-width: 0px; " + pageBreak + "\" width=\"100%\">\n";
    content += "   <tr>\n";
    content += "    <th align=\"left\" style=\"color: black;\" bgcolor=\"gray\" style=\"padding-left: 20px\" colspan=\"4\">" + name + "</th>\n";
    content += "   </tr>\n";

    for ( int i = 0; i < max; i ++ ) {
      QString leftTitle, leftValue, rightTitle, rightValue;

      if ( i < leftFields.count() ) {
        leftTitle = ContactFields::label( leftFields.at( i ) ) + ':';
        leftTitle = leftTitle.replace( ' ', "&nbsp;" );
        leftValue = ContactFields::value( leftFields.at( i ), contact );
      }

      if ( i < rightFields.count() ) {
        rightTitle = ContactFields::label( rightFields.at( i ) ) + ':';
        rightTitle = rightTitle.replace( ' ', "&nbsp;" );
        rightValue = ContactFields::value( rightFields.at( i ), contact );
      }

      content += "   <tr>\n";
      content += "    <td>" + leftTitle + "</td>\n";
      content += "    <td>" + leftValue + "</td>\n";
      content += "    <td>" + rightTitle + "</td>\n";
      content += "    <td>" + rightValue + "</td>\n";
      content += "   </tr>\n";
    }
    content += "  </table>\n";

    counter++;
  }
  content += " </body>\n";
  content += "</html>\n";

  return content;
}

MikesStyle::MikesStyle( PrintingWizard *parent )
  : PrintStyle( parent )
{
  setPreview( "mike-style.png" );
  setPreferredSortOptions( ContactFields::FormattedName, Qt::AscendingOrder );
}

MikesStyle::~MikesStyle()
{
}

void MikesStyle::print( const KABC::Addressee::List &contacts, PrintProgress *progress )
{
  QPrinter *printer = wizard()->printer();
  printer->setPageMargins( 20, 20, 20, 20, QPrinter::DevicePixel );

  progress->addMessage( i18n( "Setting up document" ) );

  const QString html = contactsToHtml( contacts );

  QTextDocument document;
  document.setHtml( html );

  progress->addMessage( i18n( "Printing" ) );

  document.print( printer );

  progress->addMessage( i18nc( "Finished printing", "Done" ) );
}

MikesStyleFactory::MikesStyleFactory( PrintingWizard *parent )
  : PrintStyleFactory( parent )
{
}

PrintStyle *MikesStyleFactory::create() const
{
  return new MikesStyle( mParent );
}

QString MikesStyleFactory::description() const
{
  return i18n( "Mike's Printing Style" );
}

#include "mikesstyle.moc"
