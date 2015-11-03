/*
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
namespace KPIMTextEdit
{
class SlideContainer;
class TextToSpeechWidget;
class TextToSpeechInterface;
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
    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;
};

class MailSourceViewTextBrowserWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MailSourceViewTextBrowserWidget(QWidget *parent = Q_NULLPTR);

    void setText(const QString &text);
    void setPlainText(const QString &text);
    void setFixedFont();
    MessageViewer::MailSourceViewTextBrowser *textBrowser() const;
private Q_SLOTS:
    void slotFind();
private:
    MailSourceViewTextBrowser *mTextBrowser;
    FindBarSourceView *mFindBar;
    KPIMTextEdit::SlideContainer *mSliderContainer;
    KPIMTextEdit::TextToSpeechWidget *mTextToSpeechWidget;

};

class MailSourceViewTextBrowser: public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit MailSourceViewTextBrowser(KPIMTextEdit::TextToSpeechInterface *textToSpeechInterface, QWidget *parent = Q_NULLPTR);
protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
private Q_SLOTS:
    void slotSpeakText();
    void slotSaveAs();
Q_SIGNALS:
    void findText();
private:
    KPIMTextEdit::TextToSpeechInterface *mTextToSpeechInterface;
};

}
#endif // MAILSOURCEVIEWTEXTBROWSERWIDGET_H

