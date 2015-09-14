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

#ifndef SIEVESYNTAXHIGHLIGHTERRULES_H
#define SIEVESYNTAXHIGHLIGHTERRULES_H

#include <QVector>
#include "pimcommon_export.h"

namespace KPIMTextEdit
{
class Rule;
}

namespace PimCommon
{
class SieveSyntaxHighlighterRulesPrivate;
class PIMCOMMON_EXPORT SieveSyntaxHighlighterRules
{
public:
    SieveSyntaxHighlighterRules();
    ~SieveSyntaxHighlighterRules();

    void addCapabilities(const QStringList &capabilities);

    QVector<KPIMTextEdit::Rule> rules() const;
private:
    void init();
    SieveSyntaxHighlighterRulesPrivate *const d;
};
}
#endif // SIEVESYNTAXHIGHLIGHTERRULES_H
