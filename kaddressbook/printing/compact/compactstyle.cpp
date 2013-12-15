/*
  This file is part of KAddressBook.
  Copyright (c) 2011 Mario Scheel <zweistein12@gmx.de>

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

#include "compactstyle.h"
#include "contactfields.h"
#include "printprogress.h"
#include "printstyle.h"
#include "printingwizard.h"
#include "ui_compactstyle.h"

#include <KABC/Addressee>

#include <KLocalizedString>

#include <QCheckBox>
#include <QPrinter>
#include <QTextDocument>

using namespace KABPrinting;

const char *CompactStyleConfigSectionName = "CompactStyle";
const char *WithAlternating = "WithAlternating";
const char *WithHomeAddress = "WithHomeAddress";
const char *WithBusinessAddress = "WithBusinessAddress";
const char *WithBirthday = "WithBirthday";
const char *WithEMail = "WithEMail";
const char *FirstColor = "FirstColor";
const char *SecondColor = "SecondColor";

namespace KABPrinting {

class CompactStyleForm : public QWidget, public Ui::CompactStyleForm_Base
{
  public:
    explicit CompactStyleForm( QWidget *parent )
      : QWidget( parent )
    {
      setObjectName( QLatin1String("AppearancePage") );
      setupUi( this );
    }
};

}

QString CompactStyle::contactsToHtml( const KABC::Addressee::List &contacts ) const
{
  // collect the fields are need to print
  ContactFields::Fields fields;
  fields    << ContactFields::FormattedName;
  if (this->withHomeAddress) {
    fields  << ContactFields::HomeAddressStreet
            << ContactFields::HomeAddressPostalCode
            << ContactFields::HomeAddressLocality
            << ContactFields::HomePhone
            << ContactFields::MobilePhone;
  }
  if (this->withBusinessAddress) {
    fields  << ContactFields::BusinessAddressStreet
            << ContactFields::BusinessAddressPostalCode
            << ContactFields::BusinessAddressLocality
            << ContactFields::BusinessPhone;
  }
  if (this->withEMail) {
    fields  << ContactFields::PreferredEmail
            << ContactFields::Email2;
  }
  if (this->withBirthday) {
    fields  << ContactFields::Birthday;
  }

  QString content;

  content += QLatin1String("<html>\n");
  content += QLatin1String(" <body>\n");
  content += QLatin1String("  <table style=\"font-size:50%; border-width: 0px; \"width=\"100%\">\n");

  bool odd = false;
  foreach ( const KABC::Addressee &contact, contacts ) {
    // get the values
    QStringList values;
    foreach ( const ContactFields::Field &field, fields ) {
      // we need only values with content
      QString value = ContactFields::value( field, contact ).trimmed();
      if ( value.size() > 0 ) {
        values << value;
      }
    }

    content += QLatin1String("   <tr>\n");
    QString style = QLatin1String("background-color:");
    if ( this->withAlternating ) {
      style += ( odd ) ? this->firstColor.name() : this->secondColor.name();
    } else {
      style += QLatin1String("#ffffff");
    }
    content += QLatin1String("    <td style=\"")+ style + QLatin1String(";\">") + values.join(QLatin1String("; ")) + QLatin1String("</td>\n");
    content += QLatin1String("   </tr>\n");
    odd = !odd;
  }

  content += QLatin1String("  </table>\n");
  content += QLatin1String(" </body>\n");
  content += QLatin1String("</html>\n");

  return content;
}

CompactStyle::CompactStyle( PrintingWizard *parent )
  : PrintStyle( parent ),
    mPageSettings( new CompactStyleForm( parent ) )
{
  setPreview( QLatin1String("compact-style.png") );
  setPreferredSortOptions( ContactFields::FormattedName, Qt::AscendingOrder );

  addPage( mPageSettings, i18n( "Compact Style" ) );

  connect( mPageSettings->cbAlternating, SIGNAL(clicked()),
           this, SLOT(setAlternatingColors()) );

  // set the controls, with the values in config
  KConfigGroup config( KGlobal::config(), CompactStyleConfigSectionName );

  withAlternating = config.readEntry( WithAlternating, true );
  withHomeAddress = config.readEntry( WithHomeAddress, true );
  withBusinessAddress = config.readEntry( WithBusinessAddress, false );
  withBirthday = config.readEntry( WithBirthday, true );
  withEMail = config.readEntry( WithEMail, true );

  mPageSettings->cbFirst->setColor( config.readEntry( FirstColor, QColor( 220, 220, 220 ) ) );
  mPageSettings->cbSecond->setColor( config.readEntry( SecondColor, QColor( 255, 255, 255 ) ) );
  mPageSettings->cbAlternating->setChecked( withAlternating );
  mPageSettings->cbWithHomeAddress->setChecked( withHomeAddress );
  mPageSettings->cbWithBusinessAddress->setChecked( withBusinessAddress );
  mPageSettings->cbWithBirthday->setChecked( withBirthday );
  mPageSettings->cbWithEMail->setChecked( withEMail );

  // set up the color boxes
  setAlternatingColors();
}

CompactStyle::~CompactStyle()
{
}

void CompactStyle::print( const KABC::Addressee::List &contacts, PrintProgress *progress )
{
  // from UI to members
  withAlternating = mPageSettings->cbAlternating->isChecked();
  firstColor = mPageSettings->cbFirst->color();
  secondColor = mPageSettings->cbSecond->color();
  withHomeAddress = mPageSettings->cbWithHomeAddress->isChecked();
  withBusinessAddress = mPageSettings->cbWithBusinessAddress->isChecked();
  withBirthday = mPageSettings->cbWithBirthday->isChecked();
  withEMail = mPageSettings->cbWithEMail->isChecked();

  // to config
  KConfigGroup config( KGlobal::config(), CompactStyleConfigSectionName );

  config.writeEntry( WithAlternating, withAlternating );
  config.writeEntry( FirstColor, firstColor );
  config.writeEntry( SecondColor, secondColor );
  config.writeEntry( WithHomeAddress, withHomeAddress );
  config.writeEntry( WithBusinessAddress, withBusinessAddress );
  config.writeEntry( WithBirthday, withBirthday );
  config.writeEntry( WithEMail, withEMail );
  config.sync();

  // print
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

void CompactStyle::setAlternatingColors()
{
  mPageSettings->cbFirst->setEnabled( mPageSettings->cbAlternating->isChecked() );
  mPageSettings->lbCbFirst->setEnabled( mPageSettings->cbAlternating->isChecked() );
  mPageSettings->cbSecond->setEnabled( mPageSettings->cbAlternating->isChecked() );
  mPageSettings->lbCbSecond->setEnabled( mPageSettings->cbAlternating->isChecked() );
}

CompactStyleFactory::CompactStyleFactory( PrintingWizard *parent )
  : PrintStyleFactory( parent )
{
}

PrintStyle *CompactStyleFactory::create() const
{
  return new CompactStyle( mParent );
}

QString CompactStyleFactory::description() const
{
  return i18n( "Compact Printing Style" );
}

