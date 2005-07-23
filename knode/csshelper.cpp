/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include "csshelper.h"
#include "knconfig.h"
#include "knconfigmanager.h"
#include "knglobals.h"


KNode::CSSHelper::CSSHelper( const QPaintDeviceMetrics &pdm ) :
  KPIM::CSSHelper( pdm )
{
  KNConfig::Appearance *app = knGlobals.configManager()->appearance();

  mForegroundColor = app->textColor();
  mLinkColor = app->linkColor();
  mVisitedLinkColor = app->linkColor();
  mBackgroundColor = app->backgroundColor();
  for ( int i = 0; i < 3; ++i )
    mQuoteColor[i] = app->quoteColor( i );

  cHtmlWarning = app->htmlWarningColor();
  cPgpOk1H  = app->signOkKeyOkColor();
  cPgpOk0H  = app->signOkKeyBadColor();
  cPgpWarnH = app->signWarnColor();
  cPgpErrH  = app->signErrColor();

  mBodyFont = mPrintFont = app->articleFont();
  mFixedFont = mFixedPrintFont = app->articleFixedFont();

  recalculatePGPColors();
}
