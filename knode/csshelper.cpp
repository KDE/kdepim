/*
    KNode, the KDE newsreader
    Copyright (c) 2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
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
  mQuoteColor[0] = app->quoteColor1();
  mQuoteColor[1] = app->quoteColor2();
  mQuoteColor[2] = app->quoteColor3();

  mBodyFont = mPrintFont = app->articleFont();
  mFixedFont = mFixedPrintFont = app->articleFixedFont();

  recalculatePGPColors();
}
