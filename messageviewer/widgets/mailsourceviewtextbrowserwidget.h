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
#ifndef MAILSOURCEVIEWTEXTBROWSERWIDGET_H
#define MAILSOURCEVIEWTEXTBROWSERWIDGET_H

#include <QSyntaxHighlighter>
#include <QPlainTextEdit>
#include <QDialog>
#include <KConfigGroup>

namespace PimCommon
{
class SlideContainer;
}
namespace MessageViewer
{
class FindBarSourceView;

/**
 * A tiny little class to use for displaying raw messages, textual
 * attachments etc.
 *
 * Auto-deletes itself when closed.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class MailSourceViewTextBrowser;
class MailSourceHighlighter : public QSyntaxHighlighter
{
public:
    explicit MailSourceHighlighter(QTextDocument *textdocument)
        : QSyntaxHighlighter(textdocument)
    {}
protected:
    void highlightBlock(const QString &text);
};

class MailSourceViewTextBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MailSourceViewTextBrowserWidget(QWidget *parent = 0);

    void setText(const QString &text);
    void setPlainText(const QString &text);
    void setFixedFont();
    MessageViewer::MailSourceViewTextBrowser *textBrowser() const;
private slots:
    void slotFind();
private:
    MailSourceViewTextBrowser *mTextBrowser;
    FindBarSourceView *mFindBar;
    PimCommon::SlideContainer *mSliderContainer;
};

class MailSourceViewTextBrowser: public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit MailSourceViewTextBrowser(QWidget *parent = 0);
protected:
    void contextMenuEvent(QContextMenuEvent *event);
private slots:
    void slotSpeakText();
    void slotSaveAs();
signals:
    void findText();
};

namespace HTMLPrettyFormatter
{
// Best to be really verbose about this one...
const QRegExp htmlTagRegExp(QLatin1String("<"
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
                                         ));

const QString reformat(const QString &src);
}
}
#endif // MAILSOURCEVIEWTEXTBROWSERWIDGET_H

