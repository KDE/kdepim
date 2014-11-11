/*
  Copyright (c) 2014 Montel Laurent <montel@kde.org>

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

#include "baloosyntaxhighlighter.h"
#include <QRegExp>

using namespace PimCommon;

BalooSyntaxHighlighter::BalooSyntaxHighlighter(QTextDocument *doc)
    : SyntaxHighlighterBase(doc)
{
    init();
}

BalooSyntaxHighlighter::~BalooSyntaxHighlighter()
{

}

void BalooSyntaxHighlighter::init()
{
    QTextCharFormat testFormat;
    testFormat.setForeground( Qt::black );
    testFormat.setFontWeight( QFont::Bold );
    QStringList testType;
    //Collection
    testType << QLatin1String("C\\d+");

    //Emails:
    //From
    testType << QLatin1String("\\bF");
    //To
    testType << QLatin1String("\\bT");
    //CC
    testType << QLatin1String("\\bCC");
    //BC
    testType << QLatin1String("\\bBC");
    //Organization
    testType << QLatin1String("\\bO");
    //Reply To
    testType << QLatin1String("\\bRT");
    //Resent-from
    testType << QLatin1String("\\bRF");
    //List Id
    testType << QLatin1String("\\bLI");
    //X-Loop
    testType << QLatin1String("\\bXL");
    //X-Mailing-List
    testType << QLatin1String("\\bXML");
    //X-Spam-Flag
    testType << QLatin1String("\\bXSF");
    //BO body element
    testType << QLatin1String("\\bBO");

    //Contacts:
    //Name
    testType << QLatin1String("\\bNA");
    //NickName
    testType << QLatin1String("\\bNI");


    Q_FOREACH ( const QString &s, testType ) {
        const QRegExp regex( s, Qt::CaseSensitive );
        m_rules.append( Rule( regex, testFormat ) );
    }
}

