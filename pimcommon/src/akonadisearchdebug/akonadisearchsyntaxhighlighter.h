/*
  Copyright (c) 2014-2015 Montel Laurent <montel@kde.org>

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

#ifndef AKONADISEARCHSYNTAXHIGHLIGHTER_H
#define AKONADISEARCHSYNTAXHIGHLIGHTER_H

#include <QSyntaxHighlighter>

namespace PimCommon
{
class Rule
{
public:
    Rule() {}
    Rule(const QRegExp &r, const QTextCharFormat &f)
        : pattern(r), format(f) {}

    QRegExp pattern;
    QTextCharFormat format;
};

class AkonadiSearchSyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit AkonadiSearchSyntaxHighlighter(QTextDocument *doc);
    ~AkonadiSearchSyntaxHighlighter();

    void highlightBlock(const QString &text) Q_DECL_OVERRIDE;

protected:
    void init();
    QVector<Rule> m_rules;
};
}
#endif // AKONADISEARCHSYNTAXHIGHLIGHTER_H
