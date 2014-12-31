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

#ifndef SYNTAXHIGHLIGHTERBASE_H
#define SYNTAXHIGHLIGHTERBASE_H

#include <QSyntaxHighlighter>
#include "pimcommon_export.h"
namespace PimCommon {
class PIMCOMMON_EXPORT Rule {
public:
    Rule() {}
    Rule( const QRegExp &r, const QTextCharFormat &f )
        : pattern( r ), format( f ) {}

    QRegExp pattern;
    QTextCharFormat format;
};

class PIMCOMMON_EXPORT SyntaxHighlighterBase : public QSyntaxHighlighter
{
    Q_OBJECT
public:
    explicit SyntaxHighlighterBase(QTextDocument *doc);
    ~SyntaxHighlighterBase();

    void highlightBlock( const QString &text );

protected:
    virtual void init() = 0;
    QVector<Rule> m_rules;
};
}
Q_DECLARE_TYPEINFO( PimCommon::Rule, Q_MOVABLE_TYPE );

#endif // SYNTAXHIGHLIGHTERBASE_H
