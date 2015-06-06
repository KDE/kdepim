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

using namespace MessageComposer;

class RichTextComposerControler::RichTextComposerControlerPrivate
{
public:
    RichTextComposerControlerPrivate(RichTextComposer *composer)
        : richtextComposer(composer)
    {

    }
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
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
