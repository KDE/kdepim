/*
  Copyright (c) 2012-2013-2014 Montel Laurent <montel@kde.org>
  
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

#ifndef AutoCorrection_H
#define AutoCorrection_H

#include "pimcommon_export.h"
#include <QTextCursor>
#include <QHash>
#include <QSet>

namespace PimCommon {

class PIMCOMMON_EXPORT AutoCorrection
{
public:
    struct TypographicQuotes {
        QChar begin;
        QChar end;
    };

    explicit AutoCorrection();
    ~AutoCorrection();

    void setLanguage(const QString& lang, bool forceGlobal = false);
    void setEnabledAutoCorrection(bool b);
    void setUppercaseFirstCharOfSentence(bool b);
    void setFixTwoUppercaseChars(bool b);
    void setSingleSpaces(bool b);
    void setAutoFractions(bool b);
    void setCapitalizeWeekDays(bool b);
    void setReplaceDoubleQuotes(bool b);
    void setReplaceSingleQuotes(bool b);
    void setAdvancedAutocorrect(bool b);
    void setTypographicSingleQuotes(TypographicQuotes singleQuote);
    void setTypographicDoubleQuotes(TypographicQuotes doubleQuote);
    void setUpperCaseExceptions(const QSet<QString>& exceptions);
    void setTwoUpperLetterExceptions(const QSet<QString>& exceptions);
    void setAutocorrectEntries(const QHash<QString, QString>& entries);
    void setAutoFormatUrl(bool b);
    void setAutoBoldUnderline(bool b);
    void setSuperScript(bool b);
    void setAddNonBreakingSpace(bool b);

    bool isEnabledAutoCorrection() const;
    bool isUppercaseFirstCharOfSentence() const;
    bool isFixTwoUppercaseChars() const;
    bool isSingleSpaces() const;
    bool isAutoFractions() const;
    bool isCapitalizeWeekDays() const;
    bool isReplaceDoubleQuotes() const;
    bool isReplaceSingleQuotes() const;
    bool isAdvancedAutocorrect() const;
    bool isAutoFormatUrl() const;
    bool isAutoBoldUnderline() const;
    bool isSuperScript() const;

    bool isAddNonBreakingSpace() const;

    QString language() const;
    TypographicQuotes typographicSingleQuotes() const;
    TypographicQuotes typographicDoubleQuotes() const;
    TypographicQuotes typographicDefaultSingleQuotes() const;
    TypographicQuotes typographicDefaultDoubleQuotes() const;
    QSet<QString> upperCaseExceptions() const;
    QSet<QString> twoUpperLetterExceptions() const;
    QHash<QString, QString> autocorrectEntries() const;

    bool autocorrect(bool htmlMode, QTextDocument &document, int &position);
    void writeConfig();

    bool addAutoCorrect(const QString& currentWord, const QString& replaceWord);

    void writeAutoCorrectionXmlFile(const QString &filename = QString());

private:
    bool isFrenchLanguage() const;
    void readConfig();

    void fixTwoUppercaseChars();
    bool singleSpaces();
    void capitalizeWeekDays();
    bool autoFractions();
    void uppercaseFirstCharOfSentence();
    int advancedAutocorrect();
    void replaceTypographicQuotes();
    void superscriptAppendix();

    void selectPreviousWord(QTextCursor &cursor, int cursorPosition);

    bool autoFormatURLs();
    bool autoBoldUnderline();

    QString autoDetectURL(const QString &_word) const;
    void readAutoCorrectionXmlFile(bool forceGlobal = false);
    bool excludeToUppercase(const QString &word) const;




    bool mSingleSpaces; // suppress double spaces.
    bool mUppercaseFirstCharOfSentence; // convert first letter of a sentence automaticall to uppercase
    bool mFixTwoUppercaseChars;  // convert two uppercase characters to one upper and one lowercase.
    bool mAutoFractions; // replace 1/2 with ½
    bool mCapitalizeWeekDays;
    bool mAdvancedAutocorrect; // autocorrection from a list of entries

    bool mReplaceDoubleQuotes;  // replace double quotes with typographical quotes
    bool mReplaceSingleQuotes;  // replace single quotes with typographical quotes

    bool mAutoFormatUrl;
    bool mAutoBoldUnderline;
    bool mEnabled;
    bool mSuperScriptAppendix;

    bool mAddNonBreakingSpace;

    QString mWord;
    QTextCursor mCursor;

    QString mAutoCorrectLang;
    QStringList mCacheNameOfDays;
    QSet<QString> mUpperCaseExceptions;
    QSet<QString> mTwoUpperLetterExceptions;
    QHash<QString, QString> mAutocorrectEntries;
    QHash<QString, QString> mSuperScriptEntries;
    TypographicQuotes mTypographicSingleQuotes;
    TypographicQuotes mTypographicDoubleQuotes;
};
}

#endif // AutoCorrection_H
