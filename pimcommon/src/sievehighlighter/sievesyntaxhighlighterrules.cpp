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

#include "sievesyntaxhighlighterrules.h"

#include <QTextCharFormat>

#include <KPIMTextEdit/SyntaxHighlighterBase>

using namespace PimCommon;

class PimCommon::SieveSyntaxHighlighterRulesPrivate
{
public:
    SieveSyntaxHighlighterRulesPrivate()
    {

    }
    void init();
    QVector<KPIMTextEdit::Rule> m_rules;
};

void SieveSyntaxHighlighterRulesPrivate::init()
{
    // Comments
    QTextCharFormat commentFormat;
    commentFormat.setForeground(Qt::darkYellow);
    QRegularExpression commentRegex(QStringLiteral("#.*$"));
    m_rules.append(KPIMTextEdit::Rule(commentRegex, commentFormat));

    commentRegex = QRegularExpression(QStringLiteral("/*.*/$"));
    m_rules.append(KPIMTextEdit::Rule(commentRegex, commentFormat));

    // Keywords
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::darkMagenta);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywords;
    keywords << QStringLiteral("\\brequire\\b");
    keywords << QStringLiteral("\\binclude\\b");
    keywords << QStringLiteral("\\bglobal\\b");
    keywords << QStringLiteral("\\bforeverypart\\b");
    Q_FOREACH (const QString &s, keywords) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        m_rules.append(KPIMTextEdit::Rule(regex, keywordFormat));
    }

    // Text keyword
    QTextCharFormat textKeywordFormat;
    textKeywordFormat.setForeground(Qt::green);
    QStringList textKeywords;
    textKeywords << QStringLiteral("\\btext:");
    Q_FOREACH (const QString &s, textKeywords) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        m_rules.append(KPIMTextEdit::Rule(regex, textKeywordFormat));
    }

    // Special debug info
    QTextCharFormat debugFormat;
    debugFormat.setForeground(Qt::red);
    debugFormat.setFontWeight(QFont::Bold);
    debugFormat.setFontItalic(true);
    textKeywords << QStringLiteral("\\bdebug_log");
    Q_FOREACH (const QString &s, textKeywords) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        m_rules.append(KPIMTextEdit::Rule(regex, debugFormat));
    }

    // Match Type
    QTextCharFormat matchFormat;
    matchFormat.setForeground(Qt::red);
    QStringList matchType;
    matchType << QStringLiteral("\\s:quoteregex\\b")
              << QStringLiteral("\\s:content\\b")
              << QStringLiteral("\\s:raw\\b")
              << QStringLiteral("\\s:type\\b")
              << QStringLiteral("\\s:subtype\\b")
              << QStringLiteral("\\s:anychild\\b")
              << QStringLiteral("\\s:param\\b")
              << QStringLiteral("\\s:value\\b")
              << QStringLiteral("\\s:count\\b")
              << QStringLiteral("\\s:last\\b")
              << QStringLiteral("\\s:text\\b")
              << QStringLiteral("\\s:lower\\b")
              << QStringLiteral("\\s:upper\\b")
              << QStringLiteral("\\s:lowerfirst\\b")
              << QStringLiteral("\\s:upperfirst\\b")
              << QStringLiteral("\\s:quotewilcard\\b")
              << QStringLiteral("\\s:length\\b")
              << QStringLiteral("\\s:contains\\b")
              << QStringLiteral("\\s:matches\\b")
              << QStringLiteral("\\s:global\\b")
              << QStringLiteral("\\s:once\\b")
              << QStringLiteral("\\s:optional\\b")
              << QStringLiteral("\\s:personal\\b")
              << QStringLiteral("\\s:is\\b")
              << QStringLiteral("\\s:over\\b")
              << QStringLiteral("\\s:under\\b")
              << QStringLiteral("\\s:localpart\\b")
              << QStringLiteral("\\s:domain\\b")
              << QStringLiteral("\\s:user\\b")
              << QStringLiteral("\\s:detail\\b")
              << QStringLiteral("\\s:all\\b")
              << QStringLiteral("\\s:copy\\b")
              << QStringLiteral("\\s:message\\b")
              << QStringLiteral("\\s:importance\\b")
              << QStringLiteral("\\s:seconds\\b")
              << QStringLiteral("\\stext:\\b")
              << QStringLiteral("\\s:days\\b")
              << QStringLiteral("\\s:addresses\\b")
              << QStringLiteral("\\s:regex\\b")
              << QStringLiteral("\\s:flags\\b")
              << QStringLiteral("\\s:subject\\b")
              << QStringLiteral("\\s:create\\b")
              << QStringLiteral("\\s:name\\b")
              << QStringLiteral("\\s:headers\\b")
              << QStringLiteral("\\s:list\\b")
              << QStringLiteral("\\s:from\\b")
              << QStringLiteral("\\s:first\\b")
              << QStringLiteral("\\s:comparator\\b");
    Q_FOREACH (const QString &s, matchType) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        m_rules.append(KPIMTextEdit::Rule(regex, matchFormat));
    }

    // Control structure
    QTextCharFormat controlFormat;
    controlFormat.setForeground(Qt::green);
    controlFormat.setFontWeight(QFont::Bold);
    QStringList controlType;
    controlType << QStringLiteral("\\bif\\b") << QStringLiteral("\\belsif\\b") << QStringLiteral("\\belse\\b");
    Q_FOREACH (const QString &s, controlType) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        m_rules.append(KPIMTextEdit::Rule(regex, controlFormat));
    }

    //Action commands:
    QTextCharFormat actionFormat;
    actionFormat.setForeground(Qt::blue);
    actionFormat.setFontWeight(QFont::Bold);
    QStringList actionType;
    actionType << QStringLiteral("\\bstop\\b")
               << QStringLiteral("\\bkeep\\b")
               << QStringLiteral("\\breject\\b")
               << QStringLiteral("\\bdiscard\\b")
               << QStringLiteral("\\bredirect\\b")
               << QStringLiteral("\\bfileinto\\b")
               << QStringLiteral("\\bsetflag\\b")
               << QStringLiteral("\\baddflag\\b")
               << QStringLiteral("\\bremoveflag\\b")
               << QStringLiteral("\\hasflag\\b")
               << QStringLiteral("\\bdeleteheader\\b")
               << QStringLiteral("\\baddheader\\b")
               << QStringLiteral("\\bnotify\\b")
               << QStringLiteral("\\bset\\b")
               << QStringLiteral("\\breturn\\b")
               << QStringLiteral("\\bvacation\\b")
               << QStringLiteral("\\benclose\\b")
               << QStringLiteral("\\breplace\\b")
               << QStringLiteral("\\bextracttext\\b");
    Q_FOREACH (const QString &s, actionType) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        m_rules.append(KPIMTextEdit::Rule(regex, actionFormat));
    }

    //Test commands:
    QTextCharFormat testFormat;
    testFormat.setForeground(Qt::gray);
    testFormat.setFontWeight(QFont::Bold);
    QStringList testType;
    testType << QStringLiteral("\\benvelope\\b")
             << QStringLiteral("\\baddress\\b")
             << QStringLiteral("\\ballof\\b")
             << QStringLiteral("\\banyof\\b")
             << QStringLiteral("\\bexists\\b")
             << QStringLiteral("\\bfalse\\b")
             << QStringLiteral("\\bheader\\b")
             << QStringLiteral("\\bnot\\b")
             << QStringLiteral("\\bsize\\b")
             << QStringLiteral("\\bdate\\b")
             << QStringLiteral("\\bbody\\b")
             << QStringLiteral("\\bcurrentdate\\b")
             << QStringLiteral("\\bmailboxexists\\b")
             << QStringLiteral("\\btrue\\b")
             << QStringLiteral("\\bmetadata\\b")
             << QStringLiteral("\\benvironment\\b")
             << QStringLiteral("\\bspamtest\\b")
             << QStringLiteral("\\bvirustest\\b")
             << QStringLiteral("\\bihave\\b")
             << QStringLiteral("\\bmetadataexists\\b")
             << QStringLiteral("\\bservermetadata\\b")
             << QStringLiteral("\\bservermetadataexists\\b");
    Q_FOREACH (const QString &s, testType) {
        const QRegularExpression regex(s, QRegularExpression::CaseInsensitiveOption);
        m_rules.append(KPIMTextEdit::Rule(regex, testFormat));
    }

    // Literals
    QTextCharFormat literalFormat;
    literalFormat.setForeground(Qt::darkRed);
    QRegularExpression literalRegex(QStringLiteral("(\"[^\"]*\")"));
    m_rules.append(KPIMTextEdit::Rule(literalRegex, literalFormat));
}

SieveSyntaxHighlighterRules::SieveSyntaxHighlighterRules()
    : d(new PimCommon::SieveSyntaxHighlighterRulesPrivate)
{
    d->init();
}

SieveSyntaxHighlighterRules::~SieveSyntaxHighlighterRules()
{
    delete d;
}

void SieveSyntaxHighlighterRules::addCapabilities(const QStringList &capabilities)
{
    // capabilities
    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(Qt::darkGreen);
    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywords;
    keywords << capabilities;
    Q_FOREACH (const QString &s, keywords) {
        const QRegularExpression regex(QStringLiteral("\"%1\"").arg(s), QRegularExpression::CaseInsensitiveOption);
        d->m_rules.append(KPIMTextEdit::Rule(regex, keywordFormat));
    }
}

QVector<KPIMTextEdit::Rule> SieveSyntaxHighlighterRules::rules() const
{
    return d->m_rules;
}
