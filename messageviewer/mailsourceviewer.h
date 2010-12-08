/*  -*- mode: C++; c-file-style: "gnu" -*-
 *
 *  This file is part of KMail, the KDE mail client.
 *
 *  Copyright (c) 2002-2003 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2003      Zack Rusin <zack@kde.org>
 *
 *  KMail is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License, version 2, as
 *  published by the Free Software Foundation.
 *
 *  KMail is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */
#ifndef MAILSOURCEVIEWER_H
#define MAILSOURCEVIEWER_H

#include <QSyntaxHighlighter>
#include <KTabWidget>
#include <KTextBrowser>
class QString;
class QWidget;
class QShowEvent;

namespace MessageViewer {

/**
 * A tiny little class to use for displaying raw messages, textual
 * attachments etc.
 *
 * Auto-deletes itself when closed.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class MailSourceHighlighter : public QSyntaxHighlighter
{
public:
  MailSourceHighlighter( KTextBrowser* browser )
    : QSyntaxHighlighter( browser )
  {}
protected:
  virtual void highlightBlock ( const QString & text );
};

class HTMLSourceHighlighter : public QSyntaxHighlighter
{
public:
  HTMLSourceHighlighter( KTextBrowser* browser )
    : QSyntaxHighlighter( browser )
  {}
protected:
  virtual void highlightBlock ( const QString & text );
};

namespace HTMLPrettyFormatter
{
  // Best to be really verbose about this one...
  const QRegExp htmlTagRegExp( "<"
                          "(/)?"    //Captures the / if this is an end tag.
                          "(\\w+)"    //Captures TagName
                          "(?:"                //Groups tag contents
                          "(?:\\s+"            //Groups attributes
                          "(?:\\w+)"  //Attribute name
                                  "(?:"                //groups =value portion.
                                      "\\s*=\\s*"            // =
                                      "(?:"        //Groups attribute "value" portion.
                                      "\\\"(?:[^\\\"]*)\\\""    // attVal='double quoted'
                                          "|'(?:[^']*)'"        // attVal='single quoted'
                                          "|(?:[^'"">\\s]+)"    // attVal=urlnospaces
                                      ")"
                                  ")?"        //end optional att value portion.
                             ")+\\s*"        //One or more attribute pairs
                              "|\\s*"            //Some white space.
                          ")"
                       "(/)?>" //Captures the "/" if this is a complete tag.
                      );

  const QString reformat( const QString &src );
};

class MailSourceViewer : public QWidget
{
Q_OBJECT
public:
  MailSourceViewer( QWidget *parent = 0 );
  ~MailSourceViewer();

  void setRawSource( const QString &source );
  void setDisplayedSource( const QString &source );

private:
  KTabWidget *mTabWidget;
  bool mShowHTMLBrowser;
  KTextBrowser *mRawBrowser;
#ifndef NDEBUG
  KTextBrowser *mHtmlBrowser;
  HTMLSourceHighlighter *mHtmlSourceHighLighter;
#endif
  MailSourceHighlighter *mRawSourceHighLighter;
};

}
#endif // MAILSOURCEVIEWER_H
