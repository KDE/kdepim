/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 2002 Jost Schenck <jost@schenck.de>
                                                                        
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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include "ringbinderstyle.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>
#include <qspinbox.h>
#include <qstringlist.h>

#include <kabc/addresseelist.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kconfig.h>
#include <kdebug.h>
#include <klistbox.h>
#include <klocale.h>
#include <kprinter.h>
#include <kstandarddirs.h>

#include "printingwizard.h"
#include "printprogress.h"
#include "printstyle.h"

#include "rbs_appearance.h"

using namespace KABPrinting;

const char* RingBinderConfigSectionName = "RingBinderPrintStyle";
const char* ShowPhoneNumbers = "ShowPhoneNumbers";
const char* ShowEmailAddresses = "ShowEmailAddresses";
const char* ShowStreetAddresses = "ShowStreetAddresses";
const char* ShowOrganization = "ShowOrganization";
const char* ShowBirthday = "ShowBirthday";
const char* FillWithEmptyFields = "FillWithEmptyFields";
const char* MinNumberOfEmptyFields = "MinNumberOfEmptyFields";
const char* LetterGroups = "LetterGroups";

RingBinderPrintStyle::RingBinderPrintStyle( PrintingWizard* parent, const char* name )
  : PrintStyle( parent, name ),
    mPageAppearance( new RingBinderStyleAppearanceForm( parent, "AppearancePage" ) ),
    mPrintProgress( 0 )
{
  setPreview( "ringbinder-style.png" );

  addPage( mPageAppearance, i18n( "Ring Binder Printing Style - Appearance" ) );

  // applying previous settings
  KConfig * config = kapp->config();
  config->setGroup( RingBinderConfigSectionName );
  mPageAppearance->cbPhoneNumbers->setChecked( config->readBoolEntry( ShowPhoneNumbers, true ) );
  mPageAppearance->cbEmails->setChecked( config->readBoolEntry( ShowEmailAddresses, true ) );
  mPageAppearance->cbStreetAddresses->setChecked( config->readBoolEntry( ShowStreetAddresses, true ) );
  mPageAppearance->cbOrganization->setChecked( config->readBoolEntry( ShowOrganization, true ) );
  mPageAppearance->cbBirthday->setChecked( config->readBoolEntry( ShowBirthday, false ) );
  mPageAppearance->cbFillEmpty->setChecked( config->readBoolEntry( FillWithEmptyFields, true ) );
  mPageAppearance->sbMinNumFill->setValue( config->readUnsignedNumEntry( MinNumberOfEmptyFields, 0 ) );

  QStringList tabNames = config->readListEntry( LetterGroups, ',' );
  if ( tabNames.isEmpty() )
    tabNames = QStringList::split( ',', QString( "AB,CD,EF,GH,IJK,LM,NO,PQR,S,TU,VW,XYZ" ) );

  mPageAppearance->letterListBox->insertStringList( tabNames );
}

RingBinderPrintStyle::~RingBinderPrintStyle()
{
}

void RingBinderPrintStyle::print( const KABC::Addressee::List &contacts, PrintProgress *progress )
{
  mPrintProgress = progress;
  progress->addMessage( i18n( "Setting up fonts and colors" ) );
  progress->setProgress( 0 );

  // first write current config settings
  KConfig *config = kapp->config();
  config->setGroup( RingBinderConfigSectionName );
  config->writeEntry( ShowPhoneNumbers, mPageAppearance->cbPhoneNumbers->isChecked() );
  config->writeEntry( ShowEmailAddresses, mPageAppearance->cbEmails->isChecked() );
  config->writeEntry( ShowStreetAddresses, mPageAppearance->cbStreetAddresses->isChecked() );
  config->writeEntry( ShowOrganization, mPageAppearance->cbOrganization->isChecked() );
  config->writeEntry( ShowBirthday, mPageAppearance->cbBirthday->isChecked() );
  config->writeEntry( FillWithEmptyFields, mPageAppearance->cbFillEmpty->isChecked() );
  config->writeEntry( MinNumberOfEmptyFields, mPageAppearance->sbMinNumFill->value() );
  QStringList tmpstrl;
  for ( uint i = 0; i < mPageAppearance->letterListBox->count(); i++ ) {
    if ( !mPageAppearance->letterListBox->text( i ).isEmpty() ) {
      tmpstrl.append( mPageAppearance->letterListBox->text( i ) );
    }
  }
  config->writeEntry( LetterGroups, tmpstrl );

  KPrinter *printer = wizard() ->printer();
  QPainter painter;

  // margins like in detailledprintstyle. FIXME: See how we can make this configurable.
  progress->addMessage( i18n( "Setting up margins and spacing" ) );
  int marginTop = 0,
      marginLeft = 64,  // to allow stapling, need refinement with two-side prints
      marginRight = 0,
      marginBottom = 0;
  register int left, top, width, height;

  painter.begin( printer );
  painter.setPen( Qt::black );
  printer->setFullPage( true ); // use whole page
  QPaintDeviceMetrics metrics( printer );

  left = QMAX( printer->margins().width(), marginLeft );
  top = QMAX( printer->margins().height(), marginTop );
  width = metrics.width() - left - QMAX( printer->margins().width(), marginRight );
  height = metrics.height() - top - QMAX( printer->margins().height(), marginBottom );

  painter.setViewport( left, top, width, height );
  progress->addMessage( i18n( "Printing" ) );
  printEntries( contacts, printer, &painter,
                QRect( 0, 0, metrics.width(), metrics.height() ) );
  progress->addMessage( i18n( "Done" ) );
  painter.end();
  config->sync();
}

bool RingBinderPrintStyle::printEntries( const KABC::Addressee::List &contacts,
                                         KPrinter *printer, QPainter *painter,
                                         const QRect& window )
{
  // FIXME: handle following situations
  // - handle situation in which we sort descending. In this case the
  //   letter groups should first also be sorted descending.

  // FIXME: downcast should not be necessary, change printstyle interface
  KABC::AddresseeList * tmpl = ( KABC::AddresseeList* ) & contacts;
#if KDE_VERSION >= 319
  KABC::Field* sfield = tmpl->sortingField();
#else
  KABC::Field* sfield = *(KABC::Field::defaultFields().begin());
#endif
 
  // we now collect the letter groups. For reverse sorted address books we
  // reverse the sorting of the groups:
  QStringList ltgroups;
  if ( !tmpl->reverseSorting() ) {
    for ( unsigned int i = 0; i < mPageAppearance->letterListBox->count(); i++ )
      ltgroups.append( mPageAppearance->letterListBox->text( i ) );
  } else {
    for ( unsigned int i = mPageAppearance->letterListBox->count() - 1; i > 0; i-- )
      ltgroups.append( mPageAppearance->letterListBox->text( i ) );
  }

  // the yposition of the current entry
  int ypos = 0;

  // counter variable for the progress widget
  int count = 0;

  // counter for the letter group in which we currently are:
  uint grpnum = 0;

  // iterate through the contacts
  printPageHeader( ltgroups[ grpnum ], window, painter );
  ypos = pageHeaderMetrics( window, painter ).height();

  KABC::AddresseeList::ConstIterator it;
  for ( it = contacts.begin(); it != contacts.end(); ++it ) {
    KABC::Addressee addressee = ( *it );
    if ( !addressee.isEmpty() ) {
      // let's see if we have to open the next group:
      while ( ltgroups.count() > grpnum + 1 ) {
        QChar nextchar;
        QChar nowchar;
        if ( !tmpl->reverseSorting() ) {
          nextchar = ltgroups[ grpnum + 1 ].at( 0 ).upper();
        } else {
          QString tmpstr = ltgroups[ grpnum + 1 ];
          nextchar = tmpstr.at( tmpstr.length() - 1 ).upper();
        }

        // determine nowchar depending on sorting criterion
        {
          QString tmpstr = sfield->value( addressee );
          if ( !tmpstr.isEmpty() ) {
            nowchar = tmpstr.at( 0 ).upper();
          }
        }

        if ( ( !tmpl->reverseSorting() && nowchar >= nextchar )
             || ( tmpl->reverseSorting() && nowchar <= nextchar ) ) {
          // we have reached the next letter group:
          //
          // first check if we should fill the rest of the page or even more
          // with empty fields:
          fillEmpty( window, printer, painter, ypos, grpnum );

          // now do change letter group
          grpnum++;
          printer->newPage();
          printPageHeader( ltgroups[ grpnum ], window, painter );
          ypos = pageHeaderMetrics( window, painter ).height();
          // -> next loop as there might be empty letter groups
        } else {
          break;
        }
      }
      // print it:
      kdDebug(5720) << "RingBinderPrintStyle::printEntries: printing addressee "
      << addressee.realName() << endl;

      // get the bounding rect:
      int entryheight = entryMetrics( addressee, window, painter, ypos ).height();

      if ( entryheight > ( window.height() - ypos ) && !( entryheight > window.height() ) ) { 
        // it does not fit on the page beginning at ypos:
        printer->newPage();
        printPageHeader( mPageAppearance->letterListBox->text( grpnum ), window, painter );
        ypos = pageHeaderMetrics( window, painter ).height();
      }

      printEntry( addressee, window, painter, ypos );
      ypos += entryheight;
    } else {
      kdDebug(5720) << "RingBinderPrintStyle::printEntries: strange, addressee "
      << "with UID " << addressee.uid() << " not available." << endl;
    }

    mPrintProgress->setProgress( (count++ * 100) / contacts.count() );
  }

  // check again if we should fill the last page with empty fields
  // (as the above call won't be reached for the last letter group)
  fillEmpty( window, printer, painter, ypos, grpnum );

  mPrintProgress->setProgress( 100 );

  return true;
}

void RingBinderPrintStyle::fillEmpty( const QRect& window, KPrinter *printer,
                                      QPainter* painter, int top, int grpnum )
{
  if ( mPageAppearance->cbFillEmpty->isChecked() ) {
    // print as many empty fields as fit on the page
    int ypos = top;
    int fieldscounter = 0;
    int entryheight = emptyEntryMetrics( window, painter, ypos ).height();
    do {
      while ( ( window.height() - ypos ) > entryheight ) {
        printEmptyEntry( window, painter, ypos );
        ypos += entryheight;
        fieldscounter++;
      }
      if ( fieldscounter < mPageAppearance->sbMinNumFill->value() ) {
        printer->newPage();
        printPageHeader( mPageAppearance->letterListBox->text( grpnum )
                       , window, painter );
        ypos = pageHeaderMetrics( window, painter ).height();
      }
    } while ( fieldscounter < mPageAppearance->sbMinNumFill->value() );
  }
}

bool RingBinderPrintStyle::printEntry( const KABC::Addressee& contact, const QRect& window,
                                       QPainter* painter, int top, bool fake, QRect* brect )
{
  QFont normfont( "Helvetica", 10, QFont::Normal );
  QFontMetrics fmnorm( normfont );
  QPen thickpen( Qt::black, 0 );
  QPen thinpen ( Qt::black, 0 );

  // store at which line we are and how many lines we have for this entry:
  int linenum = 0;
  int maxlines = 0;
  painter->setFont( normfont );
  linenum++;
  // FIXME: maybe we should not rely on formattedName only but make this
  // configurable -- if somebody sorts by familyName, but most entries have
  // a formatted name of "GivenName FamilyName" it might look strange.
  if ( !fake ) {
    QString namestr = contact.formattedName();
    if ( namestr.isEmpty() ) {
      namestr = contact.familyName() + ", " + contact.givenName();
    }
    if ( mPageAppearance->cbOrganization->isChecked() 
        && !contact.organization().isEmpty() ) {
      namestr += QString( " (" ) + contact.organization() + QString( ")" );
    }
    if ( mPageAppearance->cbBirthday->isChecked() && !contact.birthday().isNull() ) {
      namestr += QString( " *" ) + KGlobal::locale()->formatDate( 
          contact.birthday().date(), true );
    }
    painter->drawText( 5, top + ( linenum * fmnorm.lineSpacing() ) 
                              - fmnorm.leading(), namestr );
  }
  painter->setFont( normfont );

  // print street addresses:
  if ( mPageAppearance->cbStreetAddresses->isChecked() ) {
    const KABC::Address::List addrl = contact.addresses();
    KABC::Address::List::ConstIterator it;
    for ( it = addrl.begin(); it != addrl.end(); ++it ) {
      if ( !( *it ).isEmpty() ) {
        //FIXME:draw type label somehow
        // linenum++;
        // if ( !fake ) 
        //   painter->drawText(5, top + (linenum*fmnorm.lineSpacing()) 
        //                            - fmnorm.leading(), (*it).typeLabel());
        painter->setFont( normfont );
        QString formattedAddress;
#if KDE_VERSION >= 319
        formattedAddress = (*it).formattedAddress();
#else
        formattedAddress = (*it).label();
#endif
        const QStringList laddr = QStringList::split( QChar( '\n' ), 
                                                formattedAddress );
        for ( QStringList::ConstIterator it = laddr.begin(); it != laddr.end(); ++it ) {
          linenum++;
          if ( !fake ) {
            painter->drawText( 20, top + ( linenum * fmnorm.lineSpacing() )
                               - fmnorm.leading(), *it );
          }
        }
      }
    }
  }
  maxlines = linenum;
  linenum = 0;

  // print phone numbers
  if ( mPageAppearance->cbPhoneNumbers->isChecked() ) {
    const KABC::PhoneNumber::List phonel( contact.phoneNumbers() );
    KABC::PhoneNumber::List::ConstIterator nit;
    for ( nit = phonel.begin(); nit != phonel.end(); ++nit ) {
      // don't print empty lines just reading "Home:"
      if ( ( *nit ).number().isEmpty() ) {
        continue;
      }
      linenum++;

      // construct phone string and draw it
      QString numstr = ( *nit ).typeLabel();
      if ( !numstr.isEmpty() ) {
        numstr.append( ": " );
      }
      numstr.append( ( *nit ).number() );
      if ( !fake ) {
        painter->drawText( ( int ) ( window.width() * 0.5 ) + 5,
                         top + ( linenum * fmnorm.lineSpacing() ) 
                             - fmnorm.leading(), numstr );
      }
    }
  }

  // print email addresses
  if ( mPageAppearance->cbEmails->isChecked() ) {
    const QStringList emails( contact.emails() );
    for ( QStringList::ConstIterator it = emails.begin(); it != emails.end(); ++it ) {
      // don't print empty lines
      if ( ( *it ).isEmpty() ) {
        continue;
      }
      linenum++;
      if ( !fake ) {
        painter->drawText( ( int ) ( window.width() * 0.5 ) + 5,
                         top + ( linenum * fmnorm.lineSpacing() ) 
                             - fmnorm.leading(), *it );
      }
    }
  }

  // total number of lines:
  if ( linenum > maxlines ) {
    maxlines = linenum;
  }
  if ( brect ) {
    brect->setRect( 0, top, window.width(), 
        ( maxlines * fmnorm.lineSpacing() ) + fmnorm.leading() );
  }
  if ( fake ) { // nothing to do anymore as we already have dimensions
    return true;
  }
  painter->setPen( thickpen );
  if ( !fake )
    painter->drawRect( 0, top, window.width(), 
        ( maxlines * fmnorm.lineSpacing() ) + fmnorm.leading() );
  if ( !fake )
    painter->drawLine( ( int ) ( window.width() * 0.5 ), top, 
        (int)( window.width() * 0.5 ), 
        top + ( maxlines * fmnorm.lineSpacing() ) + fmnorm.leading() );
  painter->setPen( thinpen );
  return true;
}

QRect RingBinderPrintStyle::entryMetrics( const KABC::Addressee& contact,
                                          const QRect& window, QPainter* painter,
                                          int top )
{
  QRect ret;
  printEntry( contact, window, painter, top, true, &ret );
  return ret;
}

bool RingBinderPrintStyle::printEmptyEntry( const QRect& window, QPainter* painter,
                                            int top )
{
  QFont normfont( "Helvetica", 10, QFont::Normal );
  QFontMetrics fmnorm( normfont );
  QPen thickpen( Qt::black, 0 );
  QPen thinpen ( Qt::black, 0 );
  painter->setFont( normfont );
  painter->setPen( thickpen );
  painter->drawRect( 0, top, window.width(), ( 3 * fmnorm.lineSpacing() ) );
  painter->setPen( thinpen );
  for ( int i = 1; i < 3; i++ ) {
    painter->drawLine( 0, top + i * fmnorm.lineSpacing(), window.width(), 
        top + i * fmnorm.lineSpacing() );
  }
  painter->drawLine( (int)( window.width() * 0.5 ), top, 
        (int)( window.width() * 0.5 ), top + ( 3 * fmnorm.lineSpacing() ) );

  // this line not as deep as we need room for the email field
  painter->drawLine( (int)( window.width() * 0.75 ), top, 
        (int)( window.width() * 0.75 ), top + ( 2 * fmnorm.lineSpacing() ) );

  return true;
}

QRect RingBinderPrintStyle::emptyEntryMetrics( const QRect& window, QPainter*, int top )
{
  QFont normfont( "Helvetica", 10, QFont::Normal );
  QFontMetrics fmnorm( normfont );
  return QRect( 0, top, window.width(), ( 3 * fmnorm.lineSpacing() ) );
}


bool RingBinderPrintStyle::printPageHeader( const QString section, const QRect& window,
                                            QPainter* painter )
{
  QFont sectfont( "Helvetica", 16, QFont::Normal );
  QFontMetrics fmsect( sectfont );
  painter->setFont( sectfont );
  painter->drawText( QRect( 0, 0, window.width(), fmsect.height() ), 
                     Qt::AlignRight, section );
  return true;
}

QRect RingBinderPrintStyle::pageHeaderMetrics( const QRect& window, QPainter* )
{
  QFont sectfont( "Helvetica", 16, QFont::Normal );
  QFont normfont( "Helvetica", 12, QFont::Normal );
  QFontMetrics fmsect( sectfont );
  QFontMetrics fmnorm( normfont );

  return QRect( 0, 0, window.width(), fmsect.height() + 10 );
}


RingBinderPrintStyleFactory::RingBinderPrintStyleFactory( PrintingWizard *parent,
                                                          const char *name )
  : PrintStyleFactory( parent, name )
{
}

PrintStyle *RingBinderPrintStyleFactory::create() const
{
  return new RingBinderPrintStyle( mParent, mName );
}

QString RingBinderPrintStyleFactory::description() const
{
  return i18n( "Printout for Ring Binders" );
}

#include "ringbinderstyle.moc"
// vim:tw=78 cin et sw=2 comments=sr\:/*,mb\:\ ,ex\:*/,\://
