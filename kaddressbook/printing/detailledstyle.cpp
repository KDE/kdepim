/*
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>
                       2009 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "detailledstyle.h"

#include <QtGui/QCheckBox>
#include <QtGui/QPrinter>
#include <QtGui/QTextDocument>

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include "printingwizard.h"
#include "printprogress.h"
#include "printstyle.h"
#include "ui_ds_appearance.h"

using namespace KABPrinting;

const char *ConfigSectionName = "DetailedPrintStyle";
const char *ContactHeaderForeColor = "ContactHeaderForeColor";
const char *ContactHeaderBGColor = "ContactHeaderBGColor";

struct ContactBlock
{
    typedef QList<ContactBlock> List;

    QString header;
    QStringList entries;
};

struct ColorSettings
{
  QString headerTextColor;
  QString headerBackgroundColor;
};

QString contactsToHtml( const KABC::Addressee::List &contacts, const ColorSettings &settings )
{
  QString content;

  content += "<html>\n";
  content += " <head>\n";
  content += "  <style type=\"text/css\">\n";
  content += "    td.indented {\n";
  content += "      padding-left: 20px;\n";
  content += "      font-family: Fixed, monospace;\n";
  content += "    }\n";
  content += "  </style>\n";
  content += " </head>\n";
  content += " <body>\n";
  content += "  <table style=\"border-width: 0px; border-spacing: 0px;\" cellspacing=\"0\" cellpadding=\"0\" width=\"100%\">\n";
  foreach ( const KABC::Addressee &contact, contacts ) {
    const QString name = contact.givenName() + ' ' + contact.familyName();
    const QString birthday = KGlobal::locale()->formatDate( contact.birthday().date(), KLocale::ShortDate );

    ContactBlock::List blocks;

    if ( !contact.emails().isEmpty() ) {
      ContactBlock block;
      block.header = (contact.emails().count() == 1 ? i18n( "Email address:" ) : i18n( "Email addresses:" ));
      block.entries = contact.emails();

      blocks.append( block );
    }

    if ( !contact.phoneNumbers().isEmpty() ) {
      const KABC::PhoneNumber::List numbers = contact.phoneNumbers();

      ContactBlock block;
      block.header = (numbers.count() == 1 ? i18n( "Telephone:" ) : i18n( "Telephones:" ));

      foreach ( const KABC::PhoneNumber &number, numbers ) {
        const QString line = number.typeLabel() + ": " + number.number();
        block.entries.append( line );
      }

      blocks.append( block );
    }

    if ( contact.url().isValid() ) {
      ContactBlock block;
      block.header = i18n( "Web page:" );
      block.entries.append( contact.url().prettyUrl() );

      blocks.append( block );
    }

    if ( !contact.addresses().isEmpty() ) {
      const KABC::Address::List addresses = contact.addresses();

      foreach ( const KABC::Address &address, addresses ) {
        ContactBlock block;

        switch ( address.type() ) {
          case KABC::Address::Dom:
            block.header = i18n( "Domestic Address" );
            break;
          case KABC::Address::Intl:
            block.header = i18n( "International Address" );
            break;
          case KABC::Address::Postal:
            block.header = i18n( "Postal Address" );
            break;
          case KABC::Address::Parcel:
            block.header = i18n( "Parcel Address" );
            break;
          case KABC::Address::Home:
            block.header = i18n( "Home Address" );
            break;
          case KABC::Address::Work:
            block.header = i18n( "Work Address" );
            break;
          case KABC::Address::Pref:
          default:
            block.header = i18n( "Preferred Address" );
        }
        block.header += ':';

        block.entries = address.formattedAddress().split( '\n', QString::KeepEmptyParts );
        blocks.append( block );
      }
    }

    if ( !contact.note().isEmpty() ) {
      ContactBlock block;
      block.header = i18n( "Notes:" );
      block.entries = contact.note().split( '\n', QString::KeepEmptyParts );

      blocks.append( block );
    }

    // add header
    content += "   <tr>\n";
    content += "    <td style=\"color: " + settings.headerTextColor + ";\" bgcolor=\"" + settings.headerBackgroundColor + "\" style=\"padding-left: 20px\">" + name + "</td>\n";
    content += "    <td style=\"color: " + settings.headerTextColor + ";\" align=\"right\" bgcolor=\"" + settings.headerBackgroundColor + "\" style=\"padding-right: 20px\">" + birthday + "</td>\n";
    content += "   </tr>\n";

    for ( int i = 0; i < blocks.count(); i += 2 ) {
      // add empty line for spacing
      content += "   <tr>\n";
      content += "    <td>&nbsp;</td>\n";
      content += "    <td>&nbsp;</td>\n";
      content += "   </tr>\n";

      // add real block data
      const ContactBlock leftBlock = blocks.at( i );
      const ContactBlock rightBlock = ((i + 1 < blocks.count()) ? blocks.at( i + 1 ) : ContactBlock());

      content += "   <tr>\n";
      content += "    <td>" + leftBlock.header + "</td>\n";
      content += "    <td>" + rightBlock.header + "</td>\n";
      content += "   </tr>\n";

      const int maxLines = qMax( leftBlock.entries.count(), rightBlock.entries.count() );
      for ( int j = 0; j < maxLines; ++j ) {
        QString leftLine, rightLine;

        if ( j < leftBlock.entries.count() )
          leftLine = leftBlock.entries.at( j );

        if ( j < rightBlock.entries.count() )
          rightLine = rightBlock.entries.at( j );

        content += "   <tr>\n";
        content += "    <td class=\"indented\">" + leftLine + "</td>\n";
        content += "    <td class=\"indented\">" + rightLine + "</td>\n";
        content += "   </tr>\n";
      }
    }

    // add empty line for spacing
    content += "   <tr>\n";
    content += "    <td>&nbsp;</td>\n";
    content += "    <td>&nbsp;</td>\n";
    content += "   </tr>\n";
  }
  content += "  </table>\n";
  content += " </body>\n";
  content += "</html>\n";

  return content;
}

class KABPrinting::AppearancePage : public QWidget, public Ui::AppearancePage_Base
{
  public:
    AppearancePage( QWidget* parent )
      : QWidget( parent )
  {
    setupUi( this );
    setObjectName( "AppearancePage" );
  }
};

DetailledPrintStyle::DetailledPrintStyle( PrintingWizard *parent )
  : PrintStyle( parent ),
    mPageAppearance( new AppearancePage( parent ) )
{
  setPreview( "detailed-style.png" );

  addPage( mPageAppearance, i18n( "Detailed Print Style - Appearance" ) );

  KConfigGroup config( KGlobal::config(), ConfigSectionName );

  mPageAppearance->kcbHeaderBGColor->setColor( config.readEntry( ContactHeaderBGColor, QColor( Qt::black ) ) );
  mPageAppearance->kcbHeaderBGColor->setToolTip( i18n( "Click on the color button to change the header's background color." ) );
  mPageAppearance->kcbHeaderTextColor->setColor( config.readEntry( ContactHeaderForeColor, QColor( Qt::white ) ) );
  mPageAppearance->kcbHeaderTextColor->setToolTip( i18n( "Click on the color button to change the header's text color." ) );

  mPageAppearance->layout()->setMargin( KDialog::marginHint() );
  mPageAppearance->layout()->setSpacing( KDialog::spacingHint() );
}

DetailledPrintStyle::~DetailledPrintStyle()
{
}

void DetailledPrintStyle::print( const KABC::Addressee::List &contacts, PrintProgress *progress )
{
  progress->addMessage( i18n( "Setting up colors" ) );
  progress->setProgress( 0 );

  const QColor headerBackgroundColor = mPageAppearance->kcbHeaderBGColor->color();
  const QColor headerForegroundColor = mPageAppearance->kcbHeaderTextColor->color();

  KConfigGroup config( KGlobal::config(), ConfigSectionName );
  config.writeEntry( ContactHeaderForeColor, headerForegroundColor );
  config.writeEntry( ContactHeaderBGColor, headerBackgroundColor );
  config.sync();

  ColorSettings settings;
  settings.headerBackgroundColor = headerBackgroundColor.name();
  settings.headerTextColor = headerForegroundColor.name();

  QPrinter *printer = wizard()->printer();

  progress->addMessage( i18n( "Setting up document" ) );

  const QString html = contactsToHtml( contacts, settings );

  QTextDocument document;
  document.setHtml( html );

  progress->addMessage( i18n( "Printing" ) );

  document.print( printer );

  progress->addMessage( i18n( "Done" ) );
}

DetailledPrintStyleFactory::DetailledPrintStyleFactory( PrintingWizard *parent )
  : PrintStyleFactory( parent )
{
}

PrintStyle *DetailledPrintStyleFactory::create() const
{
  return new DetailledPrintStyle( mParent );
}

QString DetailledPrintStyleFactory::description() const
{
  return i18n( "Detailed Style" );
}

#include "detailledstyle.moc"
