/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <vkrause@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "csshelper.h"
#include "knglobals.h"
#include "settings.h"


KNode::CSSHelper::CSSHelper( const QPaintDevice *pd ) :
  KPIM::CSSHelper( pd )
{
  mForegroundColor = knGlobals.settings()->textColor();
  mLinkColor = knGlobals.settings()->linkColor();
  mVisitedLinkColor = knGlobals.settings()->linkColor();
  mBackgroundColor = knGlobals.settings()->backgroundColor();
  for ( int i = 0; i < 3; ++i )
    mQuoteColor[i] = knGlobals.settings()->quoteColor( i );

  cHtmlWarning = knGlobals.settings()->htmlWarningColor();
  cPgpOk1H  = knGlobals.settings()->signOkKeyOkColor();
  cPgpOk0H  = knGlobals.settings()->signOkKeyBadColor();
  cPgpWarnH = knGlobals.settings()->signWarnColor();
  cPgpErrH  = knGlobals.settings()->signErrColor();

  mBodyFont = mPrintFont = knGlobals.settings()->articleFont();
  mFixedFont = mFixedPrintFont = knGlobals.settings()->articleFixedFont();

  recalculatePGPColors();
}
