/*                                                                      
    This file is part of KAddressBook.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/

#include <kabc/phonenumber.h>
#include <kabc/address.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kstringhandler.h>
#include <ktextbrowser.h>

#include "look_html.h"

KABHtmlView::KABHtmlView( QWidget *parent, const char *name )
  : KABBasicLook( parent, name )
{
  mTextBrowser = new KTextBrowser( this );
  mTextBrowser->setWrapPolicy( QTextEdit::AtWordBoundary );
  mTextBrowser->setLinkUnderline( false );
  mTextBrowser->setVScrollBarMode( QScrollView::AlwaysOff );
  mTextBrowser->setHScrollBarMode( QScrollView::AlwaysOff );

  QStyleSheet *sheet = mTextBrowser->styleSheet();
  QStyleSheetItem *link = sheet->item( "a" );
  link->setColor( KGlobalSettings::linkColor() );
}

KABHtmlView::~KABHtmlView()
{
}

void KABHtmlView::setAddressee( const KABC::Addressee &addr )
{
  mTextBrowser->setText( QString::null );

  if ( addr.isEmpty() )
    return;

  QString name = ( addr.formattedName().isEmpty() ? addr.assembledName() :
                   addr.formattedName() );

  QString dynamicPart;

  KABC::PhoneNumber::List phones = addr.phoneNumbers();
  KABC::PhoneNumber::List::ConstIterator phoneIt;
  for ( phoneIt = phones.begin(); phoneIt != phones.end(); ++phoneIt ) {
    dynamicPart += QString( 
      "<tr><td align=\"right\"><b>%1</b></td>"
      "<td align=\"left\">%2</td></tr>" )
      .arg( KABC::PhoneNumber::typeLabel( (*phoneIt).type() ) )
      .arg( (*phoneIt).number() );
  }

  QStringList emails = addr.emails();
  QStringList::ConstIterator emailIt;
  QString type = i18n( "Privat" );
  for ( emailIt = emails.begin(); emailIt != emails.end(); ++emailIt ) {
    dynamicPart += QString( 
      "<tr><td align=\"right\"><b>%1</b></td>"
      "<td align=\"left\"><a href=\"mailto:%2\">%3</a></td></tr>" )
      .arg( type )
      .arg( *emailIt )
      .arg( *emailIt );
    type = i18n( "Other" );
  }

  if ( !addr.url().url().isEmpty() ) {
    dynamicPart += QString(
      "<tr><td align=\"right\"><b>%1</b></td>"
      "<td align=\"left\">%2</td></tr>" )
      .arg( i18n( "Homepage" ) )
      .arg( KStringHandler::tagURLs( addr.url().url() ) );
  }

  KABC::Address::List addresses = addr.addresses();
  KABC::Address::List::ConstIterator addrIt;
  for ( addrIt = addresses.begin(); addrIt != addresses.end(); ++addrIt ) {
    QString formattedAddress = (*addrIt).formattedAddress().stripWhiteSpace();
    formattedAddress = formattedAddress.replace( '\n', "<br>" );

    dynamicPart += QString( 
      "<tr><td align=\"right\"><b>%1</b></td>"
      "<td align=\"left\">%2</td></tr>" )
      .arg( KABC::Address::typeLabel( (*addrIt).type() ) )
      .arg( formattedAddress );
  }

  QString notes;
  if ( !addr.note().isEmpty() ) {
    notes = QString(
      "<tr><td colspan=\"2\"><hr noshade=\"1\"></td></tr>"
      "<tr>"
      "<td align=\"right\" valign=\"top\"><b>%1:</b></td>" // note label
      "<td align=\"left\">%2</td>"          // note
      "</tr>" ).arg( i18n( "Notes" ) ).arg( addr.note() );
  }

  QString strAddr = QString::fromLatin1(
  "<html>"
  "<body bgcolor=\"#ffffff\">"
  "<table>"
  "<tr>"
  "<td rowspan=\"3\" align=\"right\" valign=\"top\">"
  "<img src=\"myimage\" width=\"50\" height=\"70\">"
  "</td>"
  "<td align=\"left\"><font size=\"+2\"><b>%1</b></font></td>"   // name
  "</tr>"
  "<tr>"
  "<td align=\"left\">%2</td>"          // role
  "</tr>"
  "<tr>"
  "<td align=\"left\">%3</td>"          // organization
  "</tr>"
  "<tr><td colspan=\"2\">&nbsp;</td></tr>"
  "%4"                                  // dynamic part
  "%5"
  "</table>"
  "</body>"
  "</html>").arg( name ).arg( addr.role() ).arg( addr.organization() )
  .arg( dynamicPart ).arg( notes );

  KABC::Picture picture = addr.photo();
  if ( picture.isIntern() && !picture.data().isNull() )
    QMimeSourceFactory::defaultFactory()->setImage( "myimage", picture.data() );
  else
    QMimeSourceFactory::defaultFactory()->setPixmap( "myimage",
      KGlobal::iconLoader()->loadIcon( "penguin", KIcon::Desktop, 128 ) );

  mTextBrowser->setText( strAddr );
}

#include "look_html.moc"
