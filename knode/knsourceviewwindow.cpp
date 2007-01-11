/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <q3accel.h>

#include <kdialog.h>
#include <klocale.h>

#include "knsourceviewwindow.h"
#include "knglobals.h"
#include "settings.h"
#include "utilities.h"


KNSourceViewWindow::KNSourceViewWindow( const QString &text )
  : KTextBrowser(0)
{
  setWindowFlags( Qt::Window );
  setAttribute( Qt::WA_DeleteOnClose );
  Q3Accel *accel = new Q3Accel( this, "browser close-accel" );
  accel->connectItem( accel->insertItem( Qt::Key_Escape ), this , SLOT( close() ));

  setAcceptRichText( false );

  setWindowTitle(KDialog::makeStandardCaption(i18n("Article Source"), this));
#ifdef __GNUC__
#warning "kde4: porting\n";  
#endif
  //setPaper( QBrush( knGlobals.settings()->backgroundColor()) );
  setFont( knGlobals.settings()->articleFixedFont() );
  setTextColor( knGlobals.settings()->textColor() );
#ifdef __GNUC__
#warning "kde4: porting\n";  
#endif
  //setWordWrap( KTextBrowser::NoWrap );

  setPlainText( text );
  KNHelper::restoreWindowSize("sourceWindow", this, QSize(500,300));
  show();
}


void KNSourceViewWindow::setPalette( const QPalette &pal )
{
  QPalette p = pal;
  p.setColor( QPalette::Text, knGlobals.settings()->textColor() );
  p.setColor( QPalette::Background, knGlobals.settings()->backgroundColor() );
  KTextBrowser::setPalette( p );
}


KNSourceViewWindow::~KNSourceViewWindow()
{
  KNHelper::saveWindowSize("sourceWindow",size());
}


#include "knsourceviewwindow.moc"
