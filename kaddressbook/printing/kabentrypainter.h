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

#ifndef KABENTRYPAINTER_H
#define KABENTRYPAINTER_H

#include <kabc/addressbook.h>

#include <tqcolor.h>
#include <tqfont.h>
#include <tqrect.h>
#include <tqvaluelist.h>

typedef TQValueList<TQRect> QRectList;

class KABEntryPainter
{
  public:
    KABEntryPainter();
    ~KABEntryPainter();

    /**
      Paint one entry using the given painter. May not only be used on
      printer objects but on any suitable TQPaintDevice.
      The function does not paint a background, just the contents of the
      addressee.

      @param addr The addressee that should be printed.
      @param window The rectangle where the addressee should be printed to.
      @param p A painter object to print it.
      @param top The starting pixel in vertical direction (coordinate
                 system origin in the upper left corner), it is zero by
                 default, but may be larger than this to place another
                 addressee below an already printed one.
      @param fake If this is true, the addressee is not really printed,
                  it only calculates the space needed to draw this entry
                  onto the given window.

      @returns false if some error happens, otherwise true.
     */
    bool printAddressee( const KABC::Addressee &addr, const TQRect &window,
                         TQPainter *p, int top = 0, bool fake = false,
                         TQRect *rect = 0 );

    void setForegroundColor( const TQColor &color = Qt::black );
    void setBackgroundColor( const TQColor &color = Qt::black );
    void setHeaderColor( const TQColor &color = Qt::white );

    void setHeaderFont( const TQFont &font = TQFont( "Helvetica", 12, TQFont::Normal, true ) );
    void setHeadLineFont( const TQFont &font = TQFont( "Helvetica", 12, TQFont::Normal, true ) );
    void setBodyFont( const TQFont &font = TQFont( "Helvetica", 12, TQFont::Normal, true ) );
    void setFixedFont( const TQFont &font = TQFont( "Courier", 12, TQFont::Normal, true ) );
    void setCommentFont( const TQFont &font = TQFont( "Helvetica", 10, TQFont::Normal, true ) );

    void setUseHeaderColor( bool value = true );

    void setShowAddresses( bool value = true );
    void setShowEmails( bool value = true );
    void setShowPhones( bool value = true );
    void setShowURLs( bool value = true );

    /**
      Returns the index of the rectangle if the point p is inside of
      one of the email addresses. The index is the position of the
      email address in the emails list. If it does not point at one of
      the email addresses, -1 is returned.
     */
    int hitsEmail( const TQPoint &p );

    /**
      Returns the index of the rectangle if the point p is inside of
      one of the telephone numbers. See hitsEmail
     */
    int hitsPhone( const TQPoint &p );

    /**
      Returns the index of the rectangle if the point p is inside of
      one of the telephone numbers. See hitsEmail
     */
    int hitsTalk( const TQPoint &p );

    /**
      Returns the index of the rectangle if the point p is inside of
      one of the telephone numbers. See hitsEmail
     */
    int hitsURL( const TQPoint &p );

  private:
    int hits( const QRectList& rects, const TQPoint &p );

    TQColor mForegroundColor;
    TQColor mBackgroundColor;
    TQColor mHeaderColor;

    TQFont mHeaderFont;
    TQFont mHeadLineFont;
    TQFont mBodyFont;
    TQFont mFixedFont;
    TQFont mCommentFont;

    bool mUseHeaderColor;
    bool mShowAddresses;
    bool mShowEmails;
    bool mShowPhones;
    bool mShowURLs;

    QRectList mEmailRects;
    QRectList mPhoneRects;
    QRectList mURLRects;
    QRectList mTalkRects;
};

#endif
