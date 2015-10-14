/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef RICHTEXTCOMPOSERCONTROLER_H
#define RICHTEXTCOMPOSERCONTROLER_H

#include <QObject>
#include "richtextcomposer.h"
#include "messagecomposer_export.h"
class QTextCursor;
namespace MessageComposer
{
class RichTextComposer;
class NestedListHelper;
class TextPart;
class RichTextComposerImages;
class MESSAGECOMPOSER_EXPORT RichTextComposerControler : public QObject
{
    Q_OBJECT
public:
    explicit RichTextComposerControler(RichTextComposer *richtextComposer, QObject *parent = Q_NULLPTR);
    ~RichTextComposerControler();

    RichTextComposer *richTextComposer() const;

    QString currentLinkUrl() const;

    QString currentLinkText() const;
    void selectLinkText() const;
    QString toCleanHtml() const;

    bool canIndentList() const;
    bool canDedentList() const;

    NestedListHelper *nestedListHelper() const;
    void insertShareLink(const QString &url);
    void insertLink(const QString &url);
    void setCursorPositionFromStart(unsigned int pos);
    void ensureCursorVisible();
    void fillComposerTextPart(MessageComposer::TextPart *textPart);

    RichTextComposerImages *composerImages() const;
    bool painterActive() const;
    void disablePainter();
    bool isFormattingUsed() const;

    void setFontForWholeText(const QFont &font);
    void textModeChanged(MessageComposer::RichTextComposer::Mode mode);
public Q_SLOTS:
    void insertHorizontalRule();
    void alignLeft();
    void alignCenter();
    void alignRight();
    void alignJustify();
    void makeRightToLeft();
    void makeLeftToRight();
    void setTextBold(bool bold);
    void setTextItalic(bool italic);
    void setTextUnderline(bool underline);
    void setTextStrikeOut(bool strikeOut);
    void setTextForegroundColor(const QColor &color);
    void setTextBackgroundColor(const QColor &color);
    void setFontFamily(const QString &fontFamily);
    void setFontSize(int size);
    void setFont(const QFont &font);
    void setTextSuperScript(bool superscript);
    void setTextSubScript(bool subscript);
    void setChangeTextForegroundColor();
    void setChangeTextBackgroundColor();
    void manageLink();
    void indentListMore();
    void indentListLess();
    void setListStyle(int styleIndex);
    void slotAddEmoticon(const QString &text);
    void slotInsertHtml();
    void slotFormatReset();
    void slotDeleteLine();
    void slotPasteWithoutFormatting();
    void slotPasteAsQuotation();
    void slotRemoveQuotes();
    void slotAddQuotes();
    void slotAddImage();
    void slotFormatPainter(bool active);
    void ensureCursorVisibleDelayed();
private:
    class RichTextComposerControlerPrivate;
    RichTextComposerControlerPrivate *const d;
};
}
#endif // RICHTEXTCOMPOSERCONTROLER_H
