/* -*- C++ -*-
   This file declares printing classes to paint entries on QPainters.

   the KDE addressbook

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2002, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org $
   $ License: GPL with the following explicit clarification:
         This code may be linked against any version of the Qt toolkit
         from Troll Tech, Norway. $

   $Revision$
*/

#ifndef KAB_PRINTMETHODS_H
#define KAB_PRINTMETHODS_H

#include <qfont.h>
#include <qcolor.h>
#include <qvaluelist.h>
#include <qrect.h>
#include <kabc/addressbook.h>

typedef QValueList<QRect> QRectList;

class KABEntryPainter
{
public:
  KABEntryPainter(
      QColor foreColor_=Qt::black,
      QColor headerColor_=Qt::white, bool useHeaderColor=true,
      QColor backColor_=Qt::black,
      QFont header_=QFont("Helvetica", 12, QFont::Normal, true),
      QFont headlines_=QFont("Helvetica", 12, QFont::Normal, true),
      QFont body_=QFont("Helvetica", 12, QFont::Normal, true),
      QFont fixed_=QFont("Courier", 12, QFont::Normal, true),
      QFont comment_=QFont("Helvetica", 10, QFont::Normal, true),
      bool showAddresses_=true,
      bool showEmails_=true,
      bool showTelephones_=true,
      bool showURLs_=true);
  ~KABEntryPainter();
  /** Paint one entry using the given painter. May not only be used on
      printer objects but on any suitable QPaintDevice.
      If fake is true, the method does not actually paint something,
      but just calculates the space needed to draw this entry onto the
      given window.
      top is the starting pixel in vertical direction (coordinate
      system origin in the upper left corner), it is zero by default,
      but may be larger than this to place another entry below an
      already printed one.
      The function does not paint a background, just the contents of the entry.
      A list of QRect is stored in @see emails containing
      the bounding rectangles of the email addresses in the order they
      appear in the entry. Same is with @see telephones and @see URLs.
      @return false if something failed, else true */
  bool printEntry(const KABC::Addressee&, const QRect& window,
		  QPainter *,
		  int top=0,
		  bool fake=false,
		  QRect *rect=0);
  void setShowAddresses(bool);
  void setShowEmails(bool);
  void setShowTelephones(bool);
  void setShowURLs(bool);
  /** Returns the index of the rectangle if the point p is inside of
      one of the email addresses. The index is the position of the
      email address in the emails list. If it does not point at one of
      the email addresses, -1 is returned. */
  int hitsEmail(QPoint p);
  /** Returns the index of the rectangle if the point p is inside of
      one of the telephone numbers. See @see hitsEmail */
  int hitsTelephones(QPoint p);
  /** Returns the index of the rectangle if the point p is inside of
      one of the telephone numbers. See @see hitsEmail */
  int hitsTalkAddresses(QPoint p);
  /** Returns the index of the rectangle if the point p is inside of
      one of the telephone numbers. See @see hitsEmail */
  int hitsURLs(QPoint p);
protected:
  // helper for the public hitsSomething methods
  int hits(const QRectList& rects, QPoint p);
  // the text color:
  QColor foreColor;
  // the text color in the header:
  QColor headerColor;
    // color headlines background
    bool useHeaderColor;
  // the background color (only) for filled areas etc:
  QColor backColor;
  // ----- the fonts:
  // the upper header:
  QFont header;
  // the headlines of the different sections:
  QFont headlines;
  // the body text:
  QFont body;
  // a fixed font to use:
  QFont fixed;
  // the comment font, usually a smaller scale of the body font:
  QFont comment;
  // ----- general options:
  // print the postal addresses?:
  bool showAddresses;
  // print the email addresses?:
  bool showEmails;
  // print the telephone numbers?:
  bool showTelephones;
  // print the URLs?:
  bool showURLs;
  // bounding rectangles of the email addresses printed on the painter
  // in the order they appear in the entry data
  QRectList emails;
  // bounding rectangles of the telephone numbers printed on the painter
  // in the order they appear in the entry data
  QRectList telephones;
  // bounding rectangles of the URLs printed on the painter
  // in the order they appear in the entry data
  QRectList URLs;
  // bounding rectangles of the talk addresses printed on the painter
  // in the order they appear in the entry data
  QRectList talk;
};

#endif // KAB_PRINTMETHODS_H
