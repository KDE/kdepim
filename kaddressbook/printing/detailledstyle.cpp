/*
    This file is part of KAddressBook.
    Copyright (c) 1996-2002 Mirko Boehm <mirko@kde.org>

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

#include <kapplication.h>
#include <QCheckBox>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kfontcombo.h>
#include <kglobalsettings.h>
#include <QLayout>
#include <klocale.h>
#include <knuminput.h>
#include <q3paintdevicemetrics.h>
#include <QPainter>
#include <kprinter.h>
#include <kstandarddirs.h>
#include <kglobal.h>

#include "ui_ds_appearance.h"
#include "printingwizard.h"
#include "printprogress.h"
#include "printstyle.h"

#include "detailledstyle.h"

using namespace KABPrinting;

const char *ConfigSectionName = "DetailedPrintStyle";
const char *UseKDEFonts = "UseKDEFonts";
const char *HeaderFont = "HeaderFont";
const char *HeaderFontSize = "HeaderFontSize";
const char *HeadlinesFont = "HeadlineFont";
const char *HeadlinesFontSize = "HeadlineFontSize";
const char *BodyFont = "BodyFont";
const char *BodyFontSize = "BodyFontSize";
const char *DetailsFont = "DetailsFont";
const char *DetailsFontSize = "DetailsFontSize";
const char *FixedFont = "FixedFont";
const char *FixedFontSize = "FixedFontSize";
const char *ColoredContactHeaders = "UseColoredContactHeaders";
const char *ContactHeaderForeColor = "ContactHeaderForeColor";
const char *ContactHeaderBGColor = "ContactHeaderBGColor";

class KABPrinting::AppearancePage : public QWidget, public Ui::AppearancePage_Base
{
public:
  AppearancePage( QWidget* parent ) : QWidget( parent )
  {
    setupUi( this );
    setObjectName( "AppearancePage" );
  }
};

DetailledPrintStyle::DetailledPrintStyle( PrintingWizard *parent, const char *name )
  : PrintStyle( parent, name ),
    mPageAppearance( new AppearancePage( parent ) ),
    mPainter( 0 ),
    mPrintProgress( 0 )
{
  KConfig *config;
  QFont font;
  bool kdeFonts;
  QFont standard = KGlobalSettings::generalFont();
  QFont fixed = KGlobalSettings::fixedFont();

  setPreview( "detailed-style.png" );

  addPage( mPageAppearance, i18n( "Detailed Print Style - Appearance" ) );

  config = KGlobal::config();
  config->setGroup( ConfigSectionName );

  kdeFonts = config->readEntry( UseKDEFonts, true );
  mPageAppearance->cbStandardFonts->setChecked( kdeFonts );

  font = config->readEntry( HeaderFont, standard );
  mPageAppearance->kfcHeaderFont->setCurrentFont( font.family() );
  mPageAppearance->kisbHeaderFontSize->setValue( font.pointSize() );

  font = config->readEntry( HeadlinesFont, standard );
  mPageAppearance->kfcHeadlineFont->setCurrentFont( font.family() );
  mPageAppearance->kisbHeadlineFontSize->setValue( font.pointSize() );

  font = config->readEntry( BodyFont, standard );
  mPageAppearance->kfcBodyFont->setCurrentFont( font.family() );
  mPageAppearance->kisbBodyFontSize->setValue( font.pointSize() );

  font = config->readEntry( DetailsFont, standard );
  mPageAppearance->kfcDetailsFont->setCurrentFont( font.family() );
  mPageAppearance->kisbDetailsFontSize->setValue( font.pointSize() );

  font = config->readEntry( FixedFont, fixed );
  mPageAppearance->kfcFixedFont->setCurrentFont( font.family() );
  mPageAppearance->kisbFixedFontSize->setValue( font.pointSize() );

  mPageAppearance->cbBackgroundColor->setChecked(
      config->readEntry( ColoredContactHeaders, true ) );
  QColor col(Qt::black);
  mPageAppearance->kcbHeaderBGColor->setColor(
      config->readEntry( ContactHeaderBGColor, col ) );
  col = QColor(Qt::white);
  mPageAppearance->kcbHeaderTextColor->setColor(
      config->readEntry( ContactHeaderForeColor, col ) );

  mPageAppearance->layout()->setMargin( KDialog::marginHint() );
  mPageAppearance->layout()->setSpacing( KDialog::spacingHint() );
}

DetailledPrintStyle::~DetailledPrintStyle()
{
  delete mPainter;
  mPainter = 0;
}

void DetailledPrintStyle::print( const KABC::Addressee::List &contacts, PrintProgress *progress )
{
  mPrintProgress = progress;

  progress->addMessage( i18n( "Setting up fonts and colors" ) );
  progress->setProgress( 0 );

  bool useKDEFonts;
  QFont font;
  QColor foreColor = Qt::black;
  QColor headerColor = Qt::white;
  bool useHeaderColor = true;
  QColor backColor = Qt::black;
  bool useBGColor;

  // save, always available defaults:
  QFont header = QFont("Helvetica", 12, QFont::Normal);
  QFont headlines = QFont("Helvetica", 12, QFont::Normal, true);
  QFont body = QFont("Helvetica", 12, QFont::Normal);
  QFont fixed = QFont("Courier", 12, QFont::Normal);
  QFont comment = QFont("Helvetica", 10, QFont::Normal);

  // store the configuration settings:
  KConfig *config = KGlobal::config();
  config->setGroup( ConfigSectionName );
  useKDEFonts = mPageAppearance->cbStandardFonts->isChecked();
  config->writeEntry( UseKDEFonts, useKDEFonts );

  // read the font and color selections from the wizard pages:
  useBGColor=mPageAppearance->cbBackgroundColor->isChecked();
  config->writeEntry( ColoredContactHeaders, useBGColor );

  // use colored contact headers, otherwise use plain black and white):
  if ( useBGColor ) {
    headerColor = mPageAppearance->kcbHeaderTextColor->color();
    backColor = mPageAppearance->kcbHeaderBGColor->color();
    config->writeEntry( ContactHeaderForeColor, headerColor );
    config->writeEntry( ContactHeaderBGColor, backColor );
  }

  if ( mPageAppearance->cbStandardFonts->isChecked() ) {
    QFont standard = KGlobalSettings::generalFont();
    header = standard;
    headlines = standard;
    body = standard;
    fixed = KGlobalSettings::fixedFont();
    comment = standard;
  } else {
    header.setFamily( mPageAppearance->kfcHeaderFont->currentText() );
    header.setPointSize( mPageAppearance->kisbHeaderFontSize->value() );
    config->writeEntry( HeaderFont, header );

    // headlines:
    headlines.setFamily( mPageAppearance->kfcHeadlineFont->currentText() );
    headlines.setPointSize( mPageAppearance->kisbHeadlineFontSize->value() );
    config->writeEntry( HeadlinesFont, headlines );

    // body:
    body.setFamily( mPageAppearance->kfcBodyFont->currentText() );
    body.setPointSize( mPageAppearance->kisbBodyFontSize->value() );
    config->writeEntry( BodyFont, body );

    // details:
    comment.setFamily( mPageAppearance->kfcDetailsFont->currentText() );
    comment.setPointSize( mPageAppearance->kisbDetailsFontSize->value() );
    config->writeEntry( DetailsFont, comment );

    // fixed:
    fixed.setFamily( mPageAppearance->kfcFixedFont->currentText() );
    fixed.setPointSize( mPageAppearance->kisbFixedFontSize->value() );
    config->writeEntry( FixedFont, fixed );
  }

  mPainter = new KABEntryPainter;
  mPainter->setForegroundColor( foreColor );
  mPainter->setHeaderColor( headerColor );
  mPainter->setBackgroundColor( backColor );
  mPainter->setUseHeaderColor( useHeaderColor );
  mPainter->setHeaderFont( header );
  mPainter->setHeadLineFont( headlines );
  mPainter->setBodyFont( body );
  mPainter->setFixedFont( fixed );
  mPainter->setCommentFont( comment );

  KPrinter *printer = wizard()->printer();

  QPainter painter;
  progress->addMessage( i18n( "Setting up margins and spacing" ) );
  int marginTop = 0,
      marginLeft = 64, // to allow stapling, need refinement with two-side prints
      marginRight = 0,
      marginBottom = 0;

  register int left, top, width, height;

  painter.begin( printer );
  printer->setFullPage( true ); // use whole page


  left = qMax( printer->margins().width(), marginLeft );
  top = qMax( printer->margins().height(), marginTop );
  width = printer->width() - left - qMax( printer->margins().width(), marginRight );
  height = printer->height() - top - qMax( printer->margins().height(), marginBottom );

  painter.setViewport( left, top, width, height );
  progress->addMessage( i18n( "Printing" ) );

  printEntries( contacts, printer, &painter,
                QRect( 0, 0, printer->width(), printer->height() ) );

  progress->addMessage( i18n( "Done" ) );
  painter.end();

  config->sync();
}

bool DetailledPrintStyle::printEntries( const KABC::Addressee::List &contacts,
                                        KPrinter *printer,
                                        QPainter *painter,
                                        const QRect &window)
{
  QRect brect;
  int ypos = 0, count = 0;

  KABC::Addressee::List::ConstIterator it;
  for ( it = contacts.begin(); it != contacts.end(); ++it ) {
    if ( !(*it).isEmpty() ) {
      // do a faked print to get the bounding rect:
      if ( !mPainter->printAddressee( *it, window, painter, ypos, true, &brect) ) {
        // it does not fit on the page beginning at ypos:
        printer->newPage();

        // WORK_TO_DO: this assumes the entry fits on the whole page
        // (dunno how to fix this without being illogical)
        ypos = 0;
      }

      mPainter->printAddressee( *it, window, painter, ypos, false, &brect );
      ypos += brect.height();
    }

    mPrintProgress->setProgress( (count++ * 100) / contacts.count() );
  }

  mPrintProgress->setProgress( 100 );

  return true;
}

DetailledPrintStyleFactory::DetailledPrintStyleFactory( PrintingWizard *parent,
                                                        const char *name )
  : PrintStyleFactory( parent, name )
{
}

PrintStyle *DetailledPrintStyleFactory::create() const
{
  return new DetailledPrintStyle( mParent, mName );
}

QString DetailledPrintStyleFactory::description() const
{
  return i18n( "Detailed Style" );
}

#include "detailledstyle.moc"
