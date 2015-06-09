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
#include "richtextcomposerimages.h"
#include "klinkdialog_p.h"
#include "nestedlisthelper_p.h"
#include "settings/messagecomposersettings.h"

#include <KColorScheme>
#include <KLocalizedString>
#include <QColorDialog>
#include <QTextBlock>
#include <QCoreApplication>
#include <QTimer>
#include <QPointer>
#include <QClipboard>
#include <kpimtextedit/inserthtmldialog.h>
#include <kpimtextedit/textutils.h>
#include <kpimtextedit/insertimagedialog.h>
#include <grantlee/plaintextmarkupbuilder.h>

#include <part/textpart.h>

using namespace MessageComposer;

class RichTextComposerControler::RichTextComposerControlerPrivate
{
public:
    RichTextComposerControlerPrivate(RichTextComposer *composer, RichTextComposerControler *qq)
        : richtextComposer(composer),
          q(qq)
    {
        q->connect(composer, SIGNAL(textModeChanged(MessageComposer::RichTextComposer::Mode)), q, SLOT(slotTextModeChanged(MessageComposer::RichTextComposer::Mode)));
        nestedListHelper = new NestedListHelper(composer);
        richTextImages = new RichTextComposerImages(q);
    }
    ~RichTextComposerControlerPrivate()
    {
        delete nestedListHelper;
    }
    void fixupTextEditString(QString &text) const;
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    QString addQuotesToText(const QString &inputText);
    QFont saveFont;
    NestedListHelper *nestedListHelper;
    RichTextComposer *richtextComposer;
    RichTextComposerImages *richTextImages;
    RichTextComposerControler *q;
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
    : QObject(parent), d(new RichTextComposerControlerPrivate(richtextComposer, this))
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

void RichTextComposerControler::insertLink(const QString &url)
{
    if (url.isEmpty()) {
        return;
    }
    if (d->richtextComposer->textMode() == RichTextComposer::Rich) {
        QTextCursor cursor = d->richtextComposer->textCursor();
        cursor.beginEditBlock();

        QTextCharFormat format = cursor.charFormat();
        // Save original format to create an extra space with the existing char
        // format for the block
        const QTextCharFormat originalFormat = format;
        // Add link details
        format.setAnchor(true);
        format.setAnchorHref(url);
        // Workaround for QTBUG-1814:
        // Link formatting does not get applied immediately when setAnchor(true)
        // is called.  So the formatting needs to be applied manually.
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        format.setForeground(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        // Insert link text specified in dialog, otherwise write out url.
        cursor.insertText(url, format);

        cursor.setPosition(cursor.selectionEnd());
        cursor.setCharFormat(originalFormat);
        cursor.insertText(QLatin1String(" \n"));
        cursor.endEditBlock();
    } else {
        d->richtextComposer->textCursor().insertText(url + QLatin1Char('\n'));
    }
}

void RichTextComposerControler::insertShareLink(const QString &url)
{
    if (url.isEmpty()) {
        return;
    }
    const QString msg = i18n("I've linked 1 file to this email:");
    if (d->richtextComposer->textMode() == RichTextComposer::Rich) {
        QTextCursor cursor = d->richtextComposer->textCursor();

        cursor.beginEditBlock();
        cursor.insertText(QLatin1Char('\n') + msg + QLatin1Char('\n'));

        QTextCharFormat format = cursor.charFormat();
        // Save original format to create an extra space with the existing char
        // format for the block
        const QTextCharFormat originalFormat = format;
        // Add link details
        format.setAnchor(true);
        format.setAnchorHref(url);
        // Workaround for QTBUG-1814:
        // Link formatting does not get applied immediately when setAnchor(true)
        // is called.  So the formatting needs to be applied manually.
        format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        format.setUnderlineColor(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        format.setForeground(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
        // Insert link text specified in dialog, otherwise write out url.
        cursor.insertText(url, format);

        cursor.setPosition(cursor.selectionEnd());
        cursor.setCharFormat(originalFormat);
        cursor.insertText(QLatin1String(" \n"));
        cursor.endEditBlock();
    } else {
        d->richtextComposer->textCursor().insertText(QLatin1Char('\n') + msg + QLatin1Char('\n') + url + QLatin1Char('\n'));
    }
}

void RichTextComposerControler::setCursorPositionFromStart(unsigned int pos)
{
    if (pos > 0) {
        QTextCursor cursor = d->richtextComposer->textCursor();
        //Fix html pos cursor
        cursor.setPosition(qMin(pos, (unsigned int)cursor.document()->characterCount() - 1));
        d->richtextComposer->setTextCursor(cursor);
        ensureCursorVisible();
    }
}

void RichTextComposerControler::ensureCursorVisible()
{
    QCoreApplication::processEvents();

    // Hack: In KMail, the layout of the composer changes again after
    //       creating the editor (the toolbar/menubar creation is delayed), so
    //       the size of the editor changes as well, possibly hiding the cursor
    //       even though we called ensureCursorVisible() before the layout phase.
    //
    //       Delay the actual call to ensureCursorVisible() a bit to work around
    //       the problem.
    QTimer::singleShot(500, d->richtextComposer, SLOT(ensureCursorVisibleDelayed()));
}


void RichTextComposerControler::RichTextComposerControlerPrivate::fixupTextEditString(QString &text) const
{
    // Remove line separators. Normal \n chars are still there, so no linebreaks get lost here
    text.remove(QChar::LineSeparator);

    // Get rid of embedded images, see QTextImageFormat documentation:
    // "Inline images are represented by an object replacement character (0xFFFC in Unicode) "
    text.remove(0xFFFC);

    // In plaintext mode, each space is non-breaking.
    text.replace(QChar::Nbsp, QChar::fromLatin1(' '));
}

QString RichTextComposerControler::toCleanPlainText(const QString &plainText) const
{
    QString temp = plainText.isEmpty() ? d->richtextComposer->toPlainText() : plainText;
    d->fixupTextEditString(temp);
    return temp;
}

void RichTextComposerControler::fillComposerTextPart(MessageComposer::TextPart *textPart)
{
    if (isFormattingUsed() && MessageComposer::MessageComposerSettings::self()->improvePlainTextOfHtmlMessage()) {
        Grantlee::PlainTextMarkupBuilder *pb = new Grantlee::PlainTextMarkupBuilder();

        Grantlee::MarkupDirector *pmd = new Grantlee::MarkupDirector(pb);
        pmd->processDocument(d->richtextComposer->document());
        const QString plainText = pb->getResult();
        textPart->setCleanPlainText(toCleanPlainText(plainText));
        QTextDocument *doc = new QTextDocument(plainText);
        doc->adjustSize();

        textPart->setWrappedPlainText(toWrappedPlainText(doc));
        delete doc;
        delete pmd;
        delete pb;
    } else {
        textPart->setCleanPlainText(toCleanPlainText());
        textPart->setWrappedPlainText(toWrappedPlainText());
    }
    textPart->setWordWrappingEnabled(d->richtextComposer->lineWrapMode() == QTextEdit::FixedColumnWidth);
    if (isFormattingUsed()) {
        QString cleanHtml = toCleanHtml();
        fixHtmlFontSize(cleanHtml);
        textPart->setCleanHtml(cleanHtml);
        //FIXME textPart->setEmbeddedImages(embeddedImages());
    }
}

void RichTextComposerControler::fixHtmlFontSize(QString &cleanHtml)
{
    static const QString FONTSTYLEREGEX = QStringLiteral("<span style=\".*font-size:(.*)pt;.*</span>");
    QRegExp styleRegex(FONTSTYLEREGEX);
    styleRegex.setMinimal(true);

    int offset = styleRegex.indexIn(cleanHtml, 0);
    while (offset != -1) {
        // replace all the matching text with the new line text
        bool ok = false;
        const QString fontSizeStr = styleRegex.cap(1);
        const int ptValue = fontSizeStr.toInt(&ok);
        if (ok) {
            double emValue = (double)ptValue / 12;
            const QString emValueStr = QString::number(emValue, 'g', 2);
            cleanHtml.replace(styleRegex.pos(1), QString(fontSizeStr + QLatin1String("px")).length(), emValueStr + QLatin1String("em"));
        }
        // advance the search offset to just beyond the last replace
        offset += styleRegex.matchedLength();
        // find the next occurrence
        offset = styleRegex.indexIn(cleanHtml, offset);
    }
}

QString RichTextComposerControler::toWrappedPlainText() const
{
    QTextDocument *doc = d->richtextComposer->document();
    return toWrappedPlainText(doc);
}

QString RichTextComposerControler::toWrappedPlainText(QTextDocument *doc) const
{
    QString temp;
    QRegExp rx(QStringLiteral("(http|ftp|ldap)s?\\S+-$"));
    QTextBlock block = doc->begin();
    while (block.isValid()) {
        QTextLayout *layout = block.layout();
        const int numberOfLine(layout->lineCount());
        bool urlStart = false;
        for (int i = 0; i < numberOfLine; ++i) {
            QTextLine line = layout->lineAt(i);
            QString lineText = block.text().mid(line.textStart(), line.textLength());

            if (lineText.contains(rx) ||
                    (urlStart && !lineText.contains(QLatin1Char(' ')) &&
                     lineText.endsWith(QLatin1Char('-')))) {
                // don't insert line break in URL
                temp += lineText;
                urlStart = true;
            } else {
                temp += lineText + QLatin1Char('\n');
            }
        }
        block = block.next();
    }

    // Remove the last superfluous newline added above
    if (temp.endsWith(QLatin1Char('\n'))) {
        temp.chop(1);
    }

    d->fixupTextEditString(temp);
    return temp;
}

bool RichTextComposerControler::isFormattingUsed() const
{
    if (d->richtextComposer->textMode() == RichTextComposer::Plain) {
        return false;
    }

    return KPIMTextEdit::TextUtils::containsFormatting(d->richtextComposer->document());
}

void RichTextComposerControler::slotAddEmoticon(const QString &text)
{
    QTextCursor cursor = d->richtextComposer->textCursor();
    cursor.insertText(text);
}

void RichTextComposerControler::slotInsertHtml()
{
    if (d->richtextComposer->textMode() == RichTextComposer::Rich) {
        QPointer<KPIMTextEdit::InsertHtmlDialog> dialog = new KPIMTextEdit::InsertHtmlDialog(d->richtextComposer);
        if (dialog->exec()) {
            const QString str = dialog->html();
            if (!str.isEmpty()) {
                QTextCursor cursor = d->richtextComposer->textCursor();
                cursor.insertHtml(str);
            }
        }
        delete dialog;
    }
}

void RichTextComposerControler::slotAddImage()
{
    QPointer<KPIMTextEdit::InsertImageDialog> dlg = new KPIMTextEdit::InsertImageDialog(d->richtextComposer);
    if (dlg->exec() == QDialog::Accepted && dlg) {
        const QUrl url = dlg->imageUrl();
        int imageWidth = -1;
        int imageHeight = -1;
        if (!dlg->keepOriginalSize()) {
            imageWidth = dlg->imageWidth();
            imageHeight = dlg->imageHeight();
        }
        //FIXME q->addImage(url, imageWidth, imageHeight);
    }
    delete dlg;
}


void RichTextComposerControler::slotFormatReset()
{
    setTextBackgroundColor(d->richtextComposer->palette().highlightedText().color());
    setTextForegroundColor(d->richtextComposer->palette().text().color());
    d->richtextComposer->setFont(d->saveFont);
}

void RichTextComposerControler::slotDeleteLine()
{
    if (d->richtextComposer->hasFocus()) {
        QTextCursor cursor = d->richtextComposer->textCursor();
        QTextBlock block = cursor.block();
        const QTextLayout *layout = block.layout();

        // The current text block can have several lines due to word wrapping.
        // Search the line the cursor is in, and then delete it.
        for (int lineNumber = 0; lineNumber < layout->lineCount(); ++lineNumber) {
            QTextLine line = layout->lineAt(lineNumber);
            const bool lastLineInBlock = (line.textStart() + line.textLength() == block.length() - 1);
            const bool oneLineBlock = (layout->lineCount() == 1);
            const int startOfLine = block.position() + line.textStart();
            int endOfLine = block.position() + line.textStart() + line.textLength();
            if (!lastLineInBlock) {
                endOfLine -= 1;
            }

            // Found the line where the cursor is in
            if (cursor.position() >= startOfLine && cursor.position() <= endOfLine) {
                int deleteStart = startOfLine;
                int deleteLength = line.textLength();
                if (oneLineBlock) {
                    deleteLength++; // The trailing newline
                }

                // When deleting the last line in the document,
                // remove the newline of the line before the last line instead
                if (deleteStart + deleteLength >= d->richtextComposer->document()->characterCount() &&
                        deleteStart > 0) {
                    deleteStart--;
                }

                cursor.beginEditBlock();
                cursor.setPosition(deleteStart);
                cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, deleteLength);
                cursor.removeSelectedText();
                cursor.endEditBlock();
                return;
            }
        }
    }
}

void RichTextComposerControler::slotTextModeChanged(MessageComposer::RichTextComposer::Mode mode)
{
    if (mode == MessageComposer::RichTextComposer::Rich) {
        d->saveFont = d->richtextComposer->currentFont();
    }
}

void RichTextComposerControler::slotPasteAsQuotation()
{
#ifndef QT_NO_CLIPBOARD
    if (d->richtextComposer->hasFocus()) {
        const QString s = QApplication::clipboard()->text();
        if (!s.isEmpty()) {
            d->richtextComposer->insertPlainText(d->addQuotesToText(s));
        }
    }
#endif
}

void RichTextComposerControler::slotPasteWithoutFormatting()
{
#ifndef QT_NO_CLIPBOARD
    if (d->richtextComposer->hasFocus()) {
        const QString s = QApplication::clipboard()->text();
        if (!s.isEmpty()) {
            d->richtextComposer->insertPlainText(s);
        }
    }
#endif
}

void RichTextComposerControler::slotRemoveQuotes()
{
    QTextCursor cursor = d->richtextComposer->textCursor();
    cursor.beginEditBlock();
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::Document);
    }

    QTextBlock block = d->richtextComposer->document()->findBlock(cursor.selectionStart());
    int selectionEnd = cursor.selectionEnd();
    while (block.isValid() && block.position() <= selectionEnd) {
        cursor.setPosition(block.position());
        if (d->richtextComposer->isLineQuoted(block.text())) {
            int length = d->richtextComposer->quoteLength(block.text());
            cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, length);
            cursor.removeSelectedText();
            selectionEnd -= length;
        }
        block = block.next();
    }
    cursor.clearSelection();
    cursor.endEditBlock();
}

void RichTextComposerControler::slotAddQuotes()
{
    QTextCursor cursor = d->richtextComposer->textCursor();
    cursor.beginEditBlock();
    QString selectedText;
    if (!cursor.hasSelection()) {
        cursor.select(QTextCursor::Document);
        selectedText = cursor.selectedText();
        cursor.removeSelectedText();
    } else {
        selectedText = cursor.selectedText();
    }
    d->richtextComposer->insertPlainText(d->addQuotesToText(selectedText));
    cursor.endEditBlock();
}

QString RichTextComposerControler::RichTextComposerControlerPrivate::addQuotesToText(const QString &inputText)
{
    QString answer = inputText;
    const QString indentStr = richtextComposer->defaultQuoteSign();
    answer.replace(QLatin1Char('\n'), QLatin1Char('\n') + indentStr);
    //cursor.selectText() as QChar::ParagraphSeparator as paragraph separator.
    answer.replace(QChar::ParagraphSeparator, QLatin1Char('\n') + indentStr);
    answer.prepend(indentStr);
    answer += QLatin1Char('\n');
    return richtextComposer->smartQuote(answer);
}
