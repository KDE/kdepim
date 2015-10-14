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

#include "akonadisearchsyntaxhighlighter.h"
#include <QRegExp>

using namespace PimCommon;

AkonadiSearchSyntaxHighlighter::AkonadiSearchSyntaxHighlighter(QTextDocument *doc)
    : SyntaxHighlighterBase(doc)
{
    init();
}

AkonadiSearchSyntaxHighlighter::~AkonadiSearchSyntaxHighlighter()
{

}

void AkonadiSearchSyntaxHighlighter::init()
{
    QTextCharFormat testFormat;
    testFormat.setForeground(Qt::black);
    testFormat.setFontWeight(QFont::Bold);
    QStringList testType;
    //Collection
    testType << QStringLiteral("C\\d+");

    //Emails:
    //From
    testType << QStringLiteral("\\bF");
    //To
    testType << QStringLiteral("\\bT");
    //CC
    testType << QStringLiteral("\\bCC");
    //BC
    testType << QStringLiteral("\\bBC");
    //Organization
    testType << QStringLiteral("\\bO");
    //Reply To
    testType << QStringLiteral("\\bRT");
    //Resent-from
    testType << QStringLiteral("\\bRF");
    //List Id
    testType << QStringLiteral("\\bLI");
    //X-Loop
    testType << QStringLiteral("\\bXL");
    //X-Mailing-List
    testType << QStringLiteral("\\bXML");
    //X-Spam-Flag
    testType << QStringLiteral("\\bXSF");
    //BO body element
    testType << QStringLiteral("\\bBO");

    //Contacts:
    //Name
    testType << QStringLiteral("\\bNA");
    //NickName
    testType << QStringLiteral("\\bNI");

    //Calendar
    testType << QStringLiteral("\\bO");
    testType << QStringLiteral("\\bPS");
    testType << QStringLiteral("\\bS");
    testType << QStringLiteral("\\bL");
    Q_FOREACH (const QString &s, testType) {
        const QRegExp regex(s, Qt::CaseSensitive);
        m_rules.append(KPIMTextEdit::Rule(regex, testFormat));
    }
}

