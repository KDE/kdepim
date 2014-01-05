/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "grammarhighlighter.h"

namespace Grammar {

class GrammarPrivate {
public:
    GrammarPrivate(GrammarHighlighter *qq)
        : active(true),
          q(qq)
    {

    }
    QString currentLanguage;
    QColor grammarErrorColor;
    bool active;
    GrammarHighlighter *q;
};

GrammarHighlighter::GrammarHighlighter(QTextEdit *textEdit)
    : QSyntaxHighlighter(textEdit), d(new GrammarPrivate(this))
{
}

GrammarHighlighter::~GrammarHighlighter()
{
    delete d;
}

QString GrammarHighlighter::currentLanguage() const
{
    return d->currentLanguage;
}

void GrammarHighlighter::setCurrentLanguage(const QString &lang)
{
    d->currentLanguage = lang;
}

void GrammarHighlighter::highlightBlock(const QString &text)
{
    if (text.isEmpty() || !d->active)
        return;

    //TODO
    setCurrentBlockState(0);
}

void GrammarHighlighter::setActive(bool active)
{
    d->active = active;
}

bool GrammarHighlighter::isActive() const
{
    return d->active;
}

void GrammarHighlighter::setGrammarColor(const QColor &color)
{
    d->grammarErrorColor = color;
}

QColor GrammarHighlighter::grammarColor() const
{
    return d->grammarErrorColor;
}


}
