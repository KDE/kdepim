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

#ifndef GRAMMARHIGHLIGHTER_H
#define GRAMMARHIGHLIGHTER_H

#include <QSyntaxHighlighter>

#include "grammar_export.h"

namespace Grammar
{
class GrammarPrivate;
class GRAMMAR_EXPORT GrammarHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit GrammarHighlighter(QTextEdit *textEdit);
    ~GrammarHighlighter();

    QString currentLanguage() const;
    void setCurrentLanguage(const QString &lang);

    void setActive(bool active);
    bool isActive() const;

    void setGrammarColor(const QColor &color);
    QColor grammarColor() const;

protected:
    void highlightBlock(const QString &text);

private:
    friend class GrammarPrivate;
    GrammarPrivate * const d;
};
}

#endif // GRAMMARHIGHLIGHTER_H
