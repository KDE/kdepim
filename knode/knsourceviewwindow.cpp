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

#include <tqaccel.h>

#include <kapplication.h>

#include "knsourceviewwindow.h"
#include "knglobals.h"
#include "knconfigmanager.h"
#include "utilities.h"


KNSourceViewWindow::KNSourceViewWindow( const TQString &text )
  : KTextBrowser(0)
{
  setWFlags(WType_TopLevel | WDestructiveClose);
  TQAccel *accel = new TQAccel( this, "browser close-accel" );
  accel->connectItem( accel->insertItem( Qt::Key_Escape ), this , TQT_SLOT( close() ));
  KNConfig::Appearance *app=knGlobals.configManager()->appearance();

  setTextFormat( PlainText );

  setCaption(kapp->makeStdCaption(i18n("Article Source")));
  setPaper( TQBrush(app->backgroundColor()) );
  setFont( app->articleFixedFont() );
  setColor( app->textColor() );
  setWordWrap( KTextBrowser::NoWrap );

  setText( text );
  KNHelper::restoreWindowSize("sourceWindow", this, TQSize(500,300));
  show();
}


void KNSourceViewWindow::setPalette( const TQPalette &pal )
{
  TQPalette p = pal;
  p.setColor( TQColorGroup::Text, knGlobals.configManager()->appearance()->textColor() );
  p.setColor( TQColorGroup::Background, knGlobals.configManager()->appearance()->backgroundColor() );
  KTextBrowser::setPalette( p );
}


KNSourceViewWindow::~KNSourceViewWindow()
{
  KNHelper::saveWindowSize("sourceWindow",size());
}


#include "knsourceviewwindow.moc"
