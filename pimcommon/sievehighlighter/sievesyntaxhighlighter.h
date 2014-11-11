/* Copyright (C) 2011, 2012, 2013 Laurent Montel <montel@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KSIEVE_KSIEVEUI_SIEVESYNTAXHIGHLIGHTER_H
#define KSIEVE_KSIEVEUI_SIEVESYNTAXHIGHLIGHTER_H

#include "pimcommon_export.h"
#include "pimcommon/syntaxhighlighter/syntaxhighlighterbase.h"

#include <QList>
#include <QRegExp>

class QTextDocument;

namespace PimCommon {

class PIMCOMMON_EXPORT SieveSyntaxHighlighter : public SyntaxHighlighterBase
{
    Q_OBJECT

public:
    explicit SieveSyntaxHighlighter( QTextDocument *doc );
    ~SieveSyntaxHighlighter();
    void addCapabilities(const QStringList &capabilities);

private:
    void init();

};

}

#endif

