/*
    This file is part of libkdepim.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KPIM_HTMLDIFFALGODISPLAY_H
#define KPIM_HTMLDIFFALGODISPLAY_H

#include "diffalgo.h"

#include <ktextbrowser.h>
#include <kdepimmacros.h>

namespace KPIM {

class KDE_EXPORT HTMLDiffAlgoDisplay : virtual public DiffAlgoDisplay, public KTextBrowser
{
  public:
    HTMLDiffAlgoDisplay( TQWidget *parent );

    void begin();
    void end();
    void setLeftSourceTitle( const TQString &title );
    void setRightSourceTitle( const TQString &title );
    void additionalLeftField( const TQString &id, const TQString &value );
    void additionalRightField( const TQString &id, const TQString &value );
    void conflictField( const TQString &id, const TQString &leftValue,
                        const TQString &rightValue );

  private:
    TQString mLeftTitle;
    TQString mRightTitle;
    TQString mText;
};

}

#endif
