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

#include "richtextcomposer.h"
#include "richtextcomposercontroler.h"
#include "klinkdialog_p.h"
#include "nestedlisthelper_p.h"

#include <KColorScheme>
#include <QColorDialog>
#include <QTextBlock>

using namespace MessageComposer;

class RichTextComposerControler::RichTextComposerControlerPrivate
{
public:
    RichTextComposerControlerPrivate(RichTextComposer *composer)
        : richtextComposer(composer)
    {
        nestedListHelper = new NestedListHelper(composer);
    }
    ~RichTextComposerControlerPrivate()
    {
        delete nestedListHelper;
    }

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    NestedListHelper *nestedListHelper;
    RichTextComposer *richtextComposer;
};

void RichTextComposerControler::RichTextComposerControlerPrivate::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = richtextComposer->textCursor();
    QTextCursor wordStart(cursor);
    QTextCursor wordEnd(cursor);

    wordStart.movePosition(QTextCursor::StartOfWord);
    wordEnd.movePosition(QTextCursor::EndOfWord);

    cursor.beginEditBlock();
    if (!cursor.hasSelection() && cursor.position() != wordStart.position() && cursor.position() != wordEnd.position()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }
    cursor.mergeCharFormat(format);
    richtextComposer->mergeCurrentCharFormat(format);
    cursor.endEditBlock();
}


RichTextComposerControler::RichTextComposerControler(RichTextComposer *richtextComposer, QObject *parent)
    : QObject(parent), d(new RichTextComposerControlerPrivate(richtextComposer))
{

}

RichTextComposerControler::~RichTextComposerControler()
{
    delete d;
}

NestedListHelper *RichTextComposerControler::nestedListHelper() const
{
    return d->nestedListHelper;
}

RichTextComposer *RichTextComposerControler::richTextComposer() const
{
    return d->richtextComposer;
}

void RichTextComposerControler::insertHorizontalRule()
{
    QTextCursor cursor = d->richtextComposer->textCursor();
    QTextBlockFormat bf = cursor.blockFormat();
    QTextCharFormat cf = cursor.charFormat();

    cursor.beginEditBlock();
    cursor.insertHtml(QStringLiteral("<hr>"));
    cursor.insertBlock(bf, cf);
    d->richtextComposer->setTextCursor(cursor);
    d->richtextComposer->activateRichText();
    cursor.endEditBlock();
}

void RichTextComposerControler::alignLeft()
{
    d->richtextComposer->setAlignment(Qt::AlignLeft);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::alignCenter()
{
    d->richtextComposer->setAlignment(Qt::AlignHCenter);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::alignRight()
{
    d->richtextComposer->setAlignment(Qt::AlignRight);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::alignJustify()
{
    d->richtextComposer->setAlignment(Qt::AlignJustify);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::makeRightToLeft()
{
    QTextBlockFormat format;
    format.setLayoutDirection(Qt::RightToLeft);
    QTextCursor cursor = d->richtextComposer->textCursor();
    cursor.mergeBlockFormat(format);
    d->richtextComposer->setTextCursor(cursor);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::makeLeftToRight()
{
    QTextBlockFormat format;
    format.setLayoutDirection(Qt::LeftToRight);
    QTextCursor cursor = d->richtextComposer->textCursor();
    cursor.mergeBlockFormat(format);
    d->richtextComposer->setTextCursor(cursor);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextBold(bool bold)
{
    QTextCharFormat fmt;
    fmt.setFontWeight(bold ? QFont::Bold : QFont::Normal);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextItalic(bool italic)
{
    QTextCharFormat fmt;
    fmt.setFontItalic(italic);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextUnderline(bool underline)
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(underline);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextStrikeOut(bool strikeOut)
{
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(strikeOut);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextForegroundColor(const QColor &color)
{
    QTextCharFormat fmt;
    fmt.setForeground(color);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextBackgroundColor(const QColor &color)
{
    QTextCharFormat fmt;
    fmt.setBackground(color);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setFontFamily(const QString &fontFamily)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(fontFamily);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setFontSize(int size)
{
    QTextCharFormat fmt;
    fmt.setFontPointSize(size);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setFont(const QFont &font)
{
    QTextCharFormat fmt;
    fmt.setFont(font);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextSuperScript(bool superscript)
{
    QTextCharFormat fmt;
    fmt.setVerticalAlignment(superscript ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setTextSubScript(bool subscript)
{
    QTextCharFormat fmt;
    fmt.setVerticalAlignment(subscript ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
    d->mergeFormatOnWordOrSelection(fmt);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::setChangeTextForegroundColor()
{
    const QColor currentColor = d->richtextComposer->textColor();
    const QColor defaultColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();

    const QColor selectedColor = QColorDialog::getColor(currentColor.isValid() ? currentColor : defaultColor, d->richtextComposer);

    if (!selectedColor.isValid() && !currentColor.isValid()) {
        setTextForegroundColor(defaultColor);
    } else if (selectedColor.isValid()) {
        setTextForegroundColor(selectedColor);
    }
}

void RichTextComposerControler::setChangeTextBackgroundColor()
{
    QTextCharFormat fmt = d->richtextComposer->textCursor().charFormat();
    const QColor currentColor = fmt.background().color();
    const QColor defaultColor = KColorScheme(QPalette::Active, KColorScheme::View).foreground().color();

    const QColor selectedColor = QColorDialog::getColor(currentColor.isValid() ? currentColor : defaultColor, d->richtextComposer);

    if (!selectedColor.isValid() && !currentColor.isValid()) {
        setTextBackgroundColor(defaultColor);
    } else if (selectedColor.isValid()) {
        setTextBackgroundColor(selectedColor);
    }
}

QString RichTextComposerControler::currentLinkUrl() const
{
    return d->richtextComposer->textCursor().charFormat().anchorHref();
}

QString RichTextComposerControler::currentLinkText() const
{
    QTextCursor cursor = d->richtextComposer->textCursor();
    selectLinkText(&cursor);
    return cursor.selectedText();
}

void RichTextComposerControler::selectLinkText() const
{
    QTextCursor cursor = d->richtextComposer->textCursor();
    selectLinkText(&cursor);
    d->richtextComposer->setTextCursor(cursor);
}

void RichTextComposerControler::selectLinkText(QTextCursor *cursor) const
{
    // If the cursor is on a link, select the text of the link.
    if (cursor->charFormat().isAnchor()) {
        QString aHref = cursor->charFormat().anchorHref();

        // Move cursor to start of link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atStart()) {
                break;
            }
            cursor->setPosition(cursor->position() - 1);
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);
        }

        // Move selection to the end of the link
        while (cursor->charFormat().anchorHref() == aHref) {
            if (cursor->atEnd()) {
                break;
            }
            cursor->setPosition(cursor->position() + 1, QTextCursor::KeepAnchor);
        }
        if (cursor->charFormat().anchorHref() != aHref) {
            cursor->setPosition(cursor->position() - 1, QTextCursor::KeepAnchor);
        }
    } else if (cursor->hasSelection()) {
        // Nothing to to. Using the currently selected text as the link text.
    } else {

        // Select current word
        cursor->movePosition(QTextCursor::StartOfWord);
        cursor->movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    }
}

void RichTextComposerControler::manageLink()
{
    selectLinkText();
    KLinkDialog *linkDialog = new KLinkDialog(d->richtextComposer);
    linkDialog->setLinkText(currentLinkText());
    linkDialog->setLinkUrl(currentLinkUrl());

    if (linkDialog->exec()) {
        updateLink(linkDialog->linkUrl(), linkDialog->linkText());
    }

    delete linkDialog;

}

void RichTextComposerControler::updateLink(const QString &linkUrl, const QString &linkText)
{
    selectLinkText();

    QTextCursor cursor = d->richtextComposer->textCursor();
    cursor.beginEditBlock();

    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::WordUnderCursor);
    }

    QTextCharFormat format = cursor.charFormat();
    // Save original format to create an extra space with the existing char
    // format for the block
    const QTextCharFormat originalFormat = format;
    if (!linkUrl.isEmpty()) {
        // Add link details
        format.setAnchor(true);
        format.setAnchorHref(linkUrl);
        // Workaround for QTBUG-1814:
        // Link formatting does not get applied immediately when setAnchor(true)
        // is called.  So the formatting needs to be applied manually.
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        format.setForeground(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        d->richtextComposer->activateRichText();
    } else {
        // Remove link details
        format.setAnchor(false);
        format.setAnchorHref(QString());
        // Workaround for QTBUG-1814:
        // Link formatting does not get removed immediately when setAnchor(false)
        // is called. So the formatting needs to be applied manually.
        QTextDocument defaultTextDocument;
        QTextCharFormat defaultCharFormat = defaultTextDocument.begin().charFormat();

        format.setUnderlineStyle(defaultCharFormat.underlineStyle());
        format.setUnderlineColor(defaultCharFormat.underlineColor());
        format.setForeground(defaultCharFormat.foreground());
    }

    // Insert link text specified in dialog, otherwise write out url.
    QString _linkText;
    if (!linkText.isEmpty()) {
        _linkText = linkText;
    } else {
        _linkText = linkUrl;
    }
    cursor.insertText(_linkText, format);

    // Insert a space after the link if at the end of the block so that
    // typing some text after the link does not carry link formatting
    if (!linkUrl.isEmpty() && cursor.atBlockEnd()) {
        cursor.setPosition(cursor.selectionEnd());
        cursor.setCharFormat(originalFormat);
        cursor.insertText(QStringLiteral(" "));
    }

    cursor.endEditBlock();
}

QString RichTextComposerControler::toCleanHtml() const
{
    QString result = d->richtextComposer->toHtml();

    static const QString EMPTYLINEHTML = QLatin1String(
            "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; "
            "margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; \">&nbsp;</p>");

    // Qt inserts various style properties based on the current mode of the editor (underline,
    // bold, etc), but only empty paragraphs *also* have qt-paragraph-type set to 'empty'.
    static const QString EMPTYLINEREGEX = QLatin1String(
            "<p style=\"-qt-paragraph-type:empty;(.*)</p>");

    static const QString OLLISTPATTERNQT = QLatin1String(
            "<ol style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ULLISTPATTERNQT = QLatin1String(
            "<ul style=\"margin-top: 0px; margin-bottom: 0px; margin-left: 0px;");

    static const QString ORDEREDLISTHTML = QLatin1String(
            "<ol style=\"margin-top: 0px; margin-bottom: 0px;");

    static const QString UNORDEREDLISTHTML = QLatin1String(
                "<ul style=\"margin-top: 0px; margin-bottom: 0px;");

    // fix 1 - empty lines should show as empty lines - MS Outlook treats margin-top:0px; as
    // a non-existing line.
    // Although we can simply remove the margin-top style property, we still get unwanted results
    // if you have three or more empty lines. It's best to replace empty <p> elements with <p>&nbsp;</p>.

    QRegExp emptyLineFinder(EMPTYLINEREGEX);
    emptyLineFinder.setMinimal(true);

    // find the first occurrence
    int offset = emptyLineFinder.indexIn(result, 0);
    while (offset != -1) {
        // replace all the matching text with the new line text
        result.replace(offset, emptyLineFinder.matchedLength(), EMPTYLINEHTML);
        // advance the search offset to just beyond the last replace
        offset += EMPTYLINEHTML.length();
        // find the next occurrence
        offset = emptyLineFinder.indexIn(result, offset);
    }

    // fix 2a - ordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing number; e.g: "1. First item" turns into "First Item"
    result.replace(OLLISTPATTERNQT, ORDEREDLISTHTML);

    // fix 2b - unordered lists - MS Outlook treats margin-left:0px; as
    // a non-existing bullet; e.g: "* First bullet" turns into "First Bullet"
    result.replace(ULLISTPATTERNQT, UNORDEREDLISTHTML);

    return result;
}

bool RichTextComposerControler::canIndentList() const
{
    return d->nestedListHelper->canIndent();
}

bool RichTextComposerControler::canDedentList() const
{
    return d->nestedListHelper->canDedent();
}

void RichTextComposerControler::indentListMore()
{
    d->nestedListHelper->handleOnIndentMore();
    d->richtextComposer->activateRichText();
}

void RichTextComposerControler::indentListLess()
{
    d->nestedListHelper->handleOnIndentLess();
}

void RichTextComposerControler::setListStyle(int _styleIndex)
{
    d->nestedListHelper->handleOnBulletType(-_styleIndex);
    d->richtextComposer->setFocus();
    d->richtextComposer->activateRichText();
}
