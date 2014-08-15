/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>
  code based on calligra autocorrection.
  
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

#include "autocorrection.h"
#include "settings/pimcommonsettings.h"
#include "import/importkmailautocorrection.h"
#include <KGlobal>
#include <KColorScheme>
#include <KStandardDirs>
#include <QDebug>
#include <QTextBlock>
#include <QTextDocument>
#include <QDomDocument>
#include <QFile>
#include <KCalendarSystem>
#include <KLocalizedString>
#include <QStandardPaths>

using namespace PimCommon;

AutoCorrection::AutoCorrection()
    : mSingleSpaces(true),
      mUppercaseFirstCharOfSentence(false),
      mFixTwoUppercaseChars(false),
      mAutoFractions(true),
      mCapitalizeWeekDays(false),
      mReplaceDoubleQuotes(false),
      mReplaceSingleQuotes(false),
      mEnabled(false),
      mSuperScriptAppendix(false),
      mAddNonBreakingSpace(false)
{
    // default double quote open 0x201c
    // default double quote close 0x201d
    // default single quote open 0x2018
    // default single quote close 0x2019
    mTypographicSingleQuotes.begin = QChar(0x2018);
    mTypographicSingleQuotes.end = QChar(0x2019);
    mTypographicDoubleQuotes.begin = QChar(0x201c);
    mTypographicDoubleQuotes.end = QChar(0x201d);

    readConfig();

    KLocale *locale = KLocale::global();
    for (int i = 1; i <=7; ++i)
        mCacheNameOfDays.append(locale->calendar()->weekDayName(i).toLower());
}

AutoCorrection::~AutoCorrection()
{
}

void AutoCorrection::selectWord(QTextCursor &cursor, int cursorPosition)
{
    cursor.setPosition(cursorPosition);
    QTextBlock block = cursor.block();
    cursor.setPosition(block.position());
    cursorPosition -= block.position();
    QString string = block.text();
    int pos = 0;
    bool space = false;
    QString::Iterator iter = string.begin();
    while (iter != string.end()) {
        if (iter->isSpace()) {
            if (space)
                ;// double spaces belong to the previous word
            else if (pos < cursorPosition)
                cursor.setPosition(pos + block.position() + 1); // +1 because we don't want to set it on the space itself
            else
                space = true;
        } else if (space) {
            break;
        }
        ++pos;
        ++iter;
    }
    cursor.setPosition(pos + block.position(), QTextCursor::KeepAnchor);
}


void AutoCorrection::autocorrect(bool htmlMode, QTextDocument& document, int &position)
{
    if (!mEnabled)
        return;
    mCursor =  QTextCursor(&document);
    int oldPosition = position;
    selectWord(mCursor,position);
    mWord = mCursor.selectedText();
    if (mWord.isEmpty())
        return;
    mCursor.beginEditBlock();
    bool done = false;
    if (htmlMode) {
        done = autoFormatURLs();
        if (!done) {
            done = autoBoldUnderline();
        }
        if (!done) {
            superscriptAppendix();
        }
    }
    if (!done)
        done = singleSpaces();
    if (!done)
        done = autoFractions();
    if (!done)
        advancedAutocorrect();
    if (!done)
        uppercaseFirstCharOfSentence();
    if (!done)
        fixTwoUppercaseChars();
    if (!done)
        capitalizeWeekDays();
    if (!done)
        replaceTypographicQuotes();

    if (mCursor.selectedText() != mWord)
        mCursor.insertText(mWord);
    position = oldPosition;
    mCursor.endEditBlock();
}

void AutoCorrection::readConfig()
{
    mAutoBoldUnderline = PimCommon::PimCommonSettings::self()->autoBoldUnderline();
    mAutoFormatUrl = PimCommon::PimCommonSettings::self()->autoFormatUrl();
    mUppercaseFirstCharOfSentence = PimCommon::PimCommonSettings::self()->uppercaseFirstCharOfSentence();
    mFixTwoUppercaseChars = PimCommon::PimCommonSettings::self()->fixTwoUppercaseChars();
    mSingleSpaces = PimCommon::PimCommonSettings::self()->singleSpaces();
    mAutoFractions = PimCommon::PimCommonSettings::self()->autoFractions();
    mCapitalizeWeekDays = PimCommon::PimCommonSettings::self()->capitalizeWeekDays();
    mAdvancedAutocorrect = PimCommon::PimCommonSettings::self()->advancedAutocorrect();
    mReplaceDoubleQuotes = PimCommon::PimCommonSettings::self()->replaceDoubleQuotes();
    mReplaceSingleQuotes = PimCommon::PimCommonSettings::self()->replaceSingleQuotes();
    mEnabled = PimCommon::PimCommonSettings::self()->enabled();
    mSuperScriptAppendix = PimCommon::PimCommonSettings::self()->superScript();
    mAddNonBreakingSpace = PimCommon::PimCommonSettings::self()->addNonBreakingSpaceInFrench();
    readAutoCorrectionXmlFile();
}

void AutoCorrection::writeConfig()
{
    PimCommon::PimCommonSettings::self()->setAutoBoldUnderline(mAutoBoldUnderline);
    PimCommon::PimCommonSettings::self()->setAutoFormatUrl(mAutoFormatUrl);
    PimCommon::PimCommonSettings::self()->setUppercaseFirstCharOfSentence(mUppercaseFirstCharOfSentence);
    PimCommon::PimCommonSettings::self()->setFixTwoUppercaseChars(mFixTwoUppercaseChars);
    PimCommon::PimCommonSettings::self()->setSingleSpaces(mSingleSpaces);
    PimCommon::PimCommonSettings::self()->setAutoFractions(mAutoFractions);
    PimCommon::PimCommonSettings::self()->setCapitalizeWeekDays(mCapitalizeWeekDays);
    PimCommon::PimCommonSettings::self()->setAdvancedAutocorrect(mAdvancedAutocorrect);
    PimCommon::PimCommonSettings::self()->setReplaceDoubleQuotes(mReplaceDoubleQuotes);
    PimCommon::PimCommonSettings::self()->setReplaceSingleQuotes(mReplaceSingleQuotes);
    PimCommon::PimCommonSettings::self()->setEnabled(mEnabled);
    PimCommon::PimCommonSettings::self()->setSuperScript(mSuperScriptAppendix);
    PimCommon::PimCommonSettings::self()->setAddNonBreakingSpaceInFrench(mAddNonBreakingSpace);
    PimCommon::PimCommonSettings::self()->requestSync();
    writeAutoCorrectionXmlFile();
}

void AutoCorrection::addAutoCorrect(const QString &currentWord, const QString &replaceWord)
{
    mAutocorrectEntries.insert(currentWord, replaceWord);
    writeAutoCorrectionXmlFile();
}


void AutoCorrection::setUpperCaseExceptions(const QSet<QString> &exceptions)
{
    mUpperCaseExceptions = exceptions;
}

void AutoCorrection::setTwoUpperLetterExceptions(const QSet<QString> &exceptions)
{
    mTwoUpperLetterExceptions = exceptions;
}

void AutoCorrection::setAutocorrectEntries(const QHash<QString, QString> &entries)
{
    mAutocorrectEntries = entries;
}

AutoCorrection::TypographicQuotes AutoCorrection::typographicDefaultSingleQuotes() const
{
    AutoCorrection::TypographicQuotes quote;
    quote.begin = QChar(0x2018);
    quote.end = QChar(0x2019);
    return quote;
}

AutoCorrection::TypographicQuotes AutoCorrection::typographicDefaultDoubleQuotes() const
{
    AutoCorrection::TypographicQuotes quote;
    quote.begin = QChar(0x201c);
    quote.end = QChar(0x201d);
    return quote;
}

QSet<QString> AutoCorrection::upperCaseExceptions() const
{
    return mUpperCaseExceptions;
}

QSet<QString> AutoCorrection::twoUpperLetterExceptions() const
{
    return mTwoUpperLetterExceptions;
}

QHash<QString, QString> AutoCorrection::autocorrectEntries() const
{
    return mAutocorrectEntries;
}

void AutoCorrection::superscriptAppendix()
{
    if (!mSuperScriptAppendix)
        return;

    const QString trimmed = mWord.trimmed();
    int startPos = -1;
    int endPos = -1;

    QHash<QString, QString>::const_iterator i = mSuperScriptEntries.constBegin();
    while (i != mSuperScriptEntries.constEnd()) {
        if (i.key() == trimmed) {
            startPos = mCursor.selectionStart() + 1;
            endPos = startPos - 1 + trimmed.length();
            break;
        }
        else if (i.key() == QLatin1String("othernb")) {
            const int pos = trimmed.indexOf(i.value());
            if (pos > 0) {
                QString number = trimmed.left(pos);
                QString::ConstIterator constIter = number.constBegin();
                bool found = true;
                // don't apply superscript to 1th, 2th and 3th
                if (number.length() == 1 &&
                        (*constIter == QLatin1Char('1') || *constIter == QLatin1Char('2') || *constIter == QLatin1Char('3')))
                    found = false;
                if (found) {
                    while (constIter != number.constEnd()) {
                        if (!constIter->isNumber()) {
                            found = false;
                            break;
                        }
                        ++constIter;
                    }
                }
                if (found && number.length() + i.value().length() == trimmed.length()) {
                    startPos = mCursor.selectionStart() + pos;
                    endPos = startPos - pos + trimmed.length();
                    break;
                }
            }
        }
        ++i;
    }

    if (startPos != -1 && endPos != -1) {
        QTextCursor cursor(mCursor);
        cursor.setPosition(startPos);
        cursor.setPosition(endPos, QTextCursor::KeepAnchor);

        QTextCharFormat format;
        format.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
        cursor.mergeCharFormat(format);
    }
}

bool AutoCorrection::autoBoldUnderline()
{
    if (!mAutoBoldUnderline)
        return false;

    const QString trimmed = mWord.trimmed();

    if (trimmed.length() < 3)
        return false;

    const bool underline = (trimmed.at(0) == QLatin1Char('_') && trimmed.at(trimmed.length() - 1) == QLatin1Char('_'));
    const bool bold = (trimmed.at(0) == QLatin1Char('*') && trimmed.at(trimmed.length() - 1) == QLatin1Char('*'));
    const bool strikeOut = (trimmed.at(0) == QLatin1Char('-') && trimmed.at(trimmed.length() - 1) == QLatin1Char('-'));
    if (underline || bold || strikeOut) {
        int startPos = mCursor.selectionStart();
        const QString replacement = trimmed.mid(1, trimmed.length() - 2);
        bool foundLetterNumber = false;

        QString::ConstIterator constIter = replacement.constBegin();
        while (constIter != replacement.constEnd()) {
            if (constIter->isLetterOrNumber()) {
                foundLetterNumber = true;
                break;
            }
            ++constIter;
        }

        // if no letter/number found, don't apply autocorrection like in OOo 2.x
        if (!foundLetterNumber)
            return false;

        mCursor.setPosition(startPos);
        mCursor.setPosition(startPos + trimmed.length(), QTextCursor::KeepAnchor);
        mCursor.insertText(replacement);
        mCursor.setPosition(startPos);
        mCursor.setPosition(startPos + replacement.length(), QTextCursor::KeepAnchor);

        QTextCharFormat format;
        format.setFontUnderline(underline ? true : mCursor.charFormat().fontUnderline());
        format.setFontWeight(bold ? QFont::Bold : mCursor.charFormat().fontWeight());
        format.setFontStrikeOut(strikeOut ? true : mCursor.charFormat().fontStrikeOut());
        mCursor.mergeCharFormat(format);

        // to avoid the selection being replaced by mWord
        mWord = mCursor.selectedText();

        // don't do this again if the text is already underlined and bold
        if (mCursor.charFormat().fontUnderline()
                && mCursor.charFormat().fontWeight() == QFont::Bold
                && mCursor.charFormat().fontStrikeOut()) {
            return true;
        } else {
            return autoBoldUnderline();
        }
    }
    else
        return false;

    return true;
}

bool AutoCorrection::autoFormatURLs()
{
    if (!mAutoFormatUrl)
        return false;

    const QString link = autoDetectURL(mWord);
    if (link.isNull())
        return false;

    const QString trimmed = mWord.trimmed();
    int startPos = mCursor.selectionStart();
    mCursor.setPosition(startPos);
    mCursor.setPosition(startPos + trimmed.length(), QTextCursor::KeepAnchor);

    QTextCharFormat format;
    format.setAnchorHref(link);
    format.setFontItalic(true);
    format.setAnchor(true);
    format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    format.setUnderlineColor(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
    format.setForeground(KColorScheme(QPalette::Active, KColorScheme::View).foreground(KColorScheme::LinkText).color());
    mCursor.mergeCharFormat(format);

    mWord = mCursor.selectedText();
    return true;
}

QString AutoCorrection::autoDetectURL(const QString &_word) const
{
    QString word = _word;

    /* this method is ported from lib/kotext/KoAutoFormat.cpp KoAutoFormat::doAutoDetectUrl
     * from Calligra 1.x branch */
    // qDebug() <<"link:" << word;

    bool secure = false;
    int link_type = 0;
    int pos = word.indexOf(QLatin1String("http://"));
    int tmp_pos = word.indexOf(QLatin1String("https://"));

    if (tmp_pos != -1 && pos == -1) {
        secure = true;
    }

    if (tmp_pos < pos && tmp_pos != -1) {
        pos = tmp_pos;
    }

    tmp_pos = word.indexOf(QLatin1String("mailto:/"));
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
        pos = tmp_pos;
    tmp_pos = word.indexOf(QLatin1String("ftp://"));
    const int secureftp = word.indexOf(QLatin1String("ftps://"));
    if (secureftp != -1 && tmp_pos == -1) {
        secure = true;
    }

    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
        pos = tmp_pos;
    tmp_pos = word.indexOf(QLatin1String("ftp."));

    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1) {
        pos = tmp_pos;
        link_type = 3;
    }
    tmp_pos = word.indexOf(QLatin1String("file:/"));
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
        pos = tmp_pos;
    tmp_pos = word.indexOf(QLatin1String("news:"));
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
        pos = tmp_pos;
    tmp_pos = word.indexOf(QLatin1String("www."));

    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1 && word.indexOf(QLatin1Char('.'), tmp_pos+4) != -1 ) {
        pos = tmp_pos;
        link_type = 2;
    }
    tmp_pos = word.indexOf(QLatin1Char('@'));
    if (pos == -1 && tmp_pos != -1) {
        pos = tmp_pos-1;
        QChar c;

        while (pos >= 0) {
            c = word.at(pos);
            if (c.isPunct() && c != QLatin1Char('.') && c != QLatin1Char('_')) break;
            else --pos;
        }
        if (pos == tmp_pos - 1) // not a valid address
            pos = -1;
        else
            ++pos;
        link_type = 1;
    }

    if (pos != -1) {
        // A URL inside e.g. quotes (like "http://www.calligra.org" with the quotes) shouldn't include the quote in the URL.
        while (!word.at(word.length()-1).isLetter() &&  !word.at(word.length()-1).isDigit() && word.at(word.length()-1) != QLatin1Char('/'))
            word.truncate(word.length() - 1);
        word.remove(0, pos);
        QString newWord = word;

        switch(link_type) {
        case 1:
            newWord = QLatin1String("mailto:") + word;
            break;
        case 2:
            newWord = (secure ? QLatin1String("https://") : QLatin1String("http://")) + word;
            break;
        case 3:
            newWord = (secure ? QLatin1String("ftps://") : QLatin1String("ftp://")) + word;
            break;
        }
        //qDebug() <<"newWord:" << newWord;
        return newWord;
    }

    return QString();
}

void AutoCorrection::fixTwoUppercaseChars()
{
    if (!mFixTwoUppercaseChars)
        return;
    if (mWord.length() <= 2)
        return;

    if (mTwoUpperLetterExceptions.contains(mWord.trimmed()))
        return;

    const QChar firstChar = mWord.at(0);
    const QChar secondChar = mWord.at(1);

    if (secondChar.isUpper() && firstChar.isUpper()) {
        const QChar thirdChar = mWord.at(2);

        if (thirdChar.isLower())
            mWord.replace(1, 1, secondChar.toLower());
    }
}


bool AutoCorrection::singleSpaces()
{
    if (!mSingleSpaces)
        return false;
    if (!mCursor.atBlockStart() && mWord.length() == 1 && mWord.at(0) == QLatin1Char(' ')) {
        // then when the prev char is also a space, don't insert one.
        const QTextBlock block = mCursor.block();
        const QString text = block.text();
        if (text.at(mCursor.position() -1 - block.position()) == QLatin1Char(' ')) {
            mWord.clear();
            return true;
        }
    }
    return false;
}

void AutoCorrection::capitalizeWeekDays()
{
    if (!mCapitalizeWeekDays)
        return;

    const QString trimmed = mWord.trimmed();
    Q_FOREACH (const QString & name, mCacheNameOfDays) {
        if (trimmed == name) {
            const int pos = mWord.indexOf(name);
            mWord.replace(pos, 1, name.at(0).toUpper());
            return;
        }
    }
}

bool AutoCorrection::excludeToUppercase(const QString &word) const
{
    if (word.startsWith(QLatin1String("http://")) ||
            word.startsWith(QLatin1String("www.")) ||
            word.startsWith(QLatin1String("mailto:")) ||
            word.startsWith(QLatin1String("ftp://")) ||
            word.startsWith(QLatin1String("https://")) ||
            word.startsWith(QLatin1String("ftps://")))
        return true;
    return false;
}

void AutoCorrection::uppercaseFirstCharOfSentence()
{
    if (!mUppercaseFirstCharOfSentence)
        return;

    int startPos = mCursor.selectionStart();
    QTextBlock block = mCursor.block();

    mCursor.setPosition(block.position());
    mCursor.setPosition(startPos, QTextCursor::KeepAnchor);

    int position = mCursor.selectionEnd();

    const QString text = mCursor.selectedText();

    if (text.isEmpty()) {// start of a paragraph
        if (!excludeToUppercase(mWord))
            mWord.replace(0, 1, mWord.at(0).toUpper());
    } else {
        QString::ConstIterator constIter = text.constEnd();
        --constIter;

        while (constIter != text.constBegin()) {
            while (constIter != text.begin() && constIter->isSpace()) {
                constIter--;
                position--;
            }

            if (constIter != text.constBegin() && (*constIter == QLatin1Char('.') || *constIter == QLatin1Char('!') || *constIter == QLatin1Char('?'))) {
                constIter--;
                while (constIter != text.constBegin() && !(constIter->isLetter())) {
                    position--;
                    constIter--;
                }
                selectWord(mCursor, --position);
                const QString prevWord = mCursor.selectedText();

                // search for exception
                if (mUpperCaseExceptions.contains(prevWord.trimmed()))
                    break;
                if (excludeToUppercase(mWord))
                    break;

                mWord.replace(0, 1, mWord.at(0).toUpper());
                break;
            } else {
                break;
            }
        }
    }

    mCursor.setPosition(startPos);
    mCursor.setPosition(startPos + mWord.length(), QTextCursor::KeepAnchor);
}

bool AutoCorrection::autoFractions()
{
    if (!mAutoFractions)
        return false;

    const QString trimmed = mWord.trimmed();
    if (trimmed.length() > 3) {
        const QChar x = trimmed.at(3);
        if (!(x.unicode() == '.' || x.unicode() == ',' || x.unicode() == '?' || x.unicode() == '!'
              || x.unicode() == ':' || x.unicode() == ';'))
            return false;
    } else if (trimmed.length() < 3) {
        return false;
    }

    if (trimmed.startsWith(QLatin1String("1/2")))
        mWord.replace(0, 3, QString::fromUtf8("½"));
    else if (trimmed.startsWith(QLatin1String("1/4")))
        mWord.replace(0, 3, QString::fromUtf8("¼"));
    else if (trimmed.startsWith(QLatin1String("3/4")))
        mWord.replace(0, 3, QString::fromUtf8("¾"));
    else
        return false;

    return true;
}

void AutoCorrection::advancedAutocorrect()
{
    if (!mAdvancedAutocorrect)
        return;

    const int startPos = mCursor.selectionStart();
    const int length = mWord.length();

    const QString trimmedWord = mWord.trimmed();
    QString actualWord = trimmedWord;

    if (actualWord.isEmpty())
        return;

    // If the last char is punctuation, drop it for now
    bool hasPunctuation = false;
    QChar lastChar = actualWord.at(actualWord.length() - 1);
    if (lastChar.unicode() == '.' || lastChar.unicode() == ',' || lastChar.unicode() == '?' ||
            lastChar.unicode() == '!' || lastChar.unicode() == ':' || lastChar.unicode() == ';') {
        hasPunctuation = true;
        actualWord.chop(1);
    }

    QString actualWordWithFirstUpperCase = actualWord;
    actualWordWithFirstUpperCase[0] = actualWordWithFirstUpperCase[0].toUpper();
    if (mAutocorrectEntries.contains(actualWord) ||
            mAutocorrectEntries.contains(actualWord.toLower() ) ||
            mAutocorrectEntries.contains(actualWordWithFirstUpperCase)) {
        int pos = mWord.indexOf(trimmedWord);
        QString replacement = mAutocorrectEntries.value(actualWord, QString());
        if (replacement.isEmpty()) {
            replacement = mAutocorrectEntries.value(actualWord.toLower());
            if (replacement.isEmpty()) {
                replacement = mAutocorrectEntries.value(actualWordWithFirstUpperCase);
            }
        }

        // Keep capitalized words capitalized.
        // (Necessary to make sure the first letters match???)
        if (actualWord.at(0) == replacement.at(0).toLower()) {
            if (mWord.at(0).isUpper()) {
                replacement[0] = replacement[0].toUpper();
            } else {
                //Don't replace toUpper letter
                if (replacement.at(0).isLower()) {
                    replacement[0] = replacement[0].toLower();
                }
            }
        }

        // If a punctuation mark was on the end originally, add it back on
        if (hasPunctuation) {
            replacement.append(lastChar);
        }

        mWord.replace(pos, pos + trimmedWord.length(), replacement);

        // We do replacement here, since the length of new word might be different from length of
        // the old world. Length difference might affect other type of autocorrection
        mCursor.setPosition(startPos);
        mCursor.setPosition(startPos + length, QTextCursor::KeepAnchor);
        mCursor.insertText(mWord);
        mCursor.setPosition(startPos); // also restore the selection
        mCursor.setPosition(startPos + mWord.length(), QTextCursor::KeepAnchor);
    }
}

void AutoCorrection::replaceTypographicQuotes()
{
    /* this method is ported from lib/kotext/KoAutoFormat.cpp KoAutoFormat::doTypographicQuotes
     * from Calligra 1.x branch */

    if (!(mReplaceDoubleQuotes && mWord.contains(QLatin1Char('"'))) &&
            !(mReplaceSingleQuotes && mWord.contains(QLatin1Char('\'')))) return;


    const bool addNonBreakingSpace = (isFrenchLanguage() && isAddNonBreakingSpace());

    // Need to determine if we want a starting or ending quote.
    // we use a starting quote in three cases:
    //  1. if the previous character is a space
    //  2. if the previous character is some kind of opening punctuation (e.g., "(", "[", or "{")
    //     a. and the character before that is not an opening quote (so that we get quotations of single characters
    //        right)
    //  3. if the previous character is an opening quote (so that we get nested quotations right)
    //     a. and the character before that is not an opening quote (so that we get quotations of single characters
    //         right)
    //     b. and the previous quote of a different kind (so that we get empty quotations right)

    bool ending = true;
    const QChar nbsp = QChar(QChar::Nbsp);
    for (int i = mWord.length(); i>1; --i) {
        const QChar c = mWord.at(i-1);
        if (c == QLatin1Char('"') || c == QLatin1Char('\'')) {
            const bool doubleQuotes = (c == QLatin1Char('"'));
            if (i > 2) {
                QChar::Category c1 = mWord.at(i-1).category();

                // case 1 and 2
                if (c1 == QChar::Separator_Space || c1 == QChar::Separator_Line || c1 == QChar::Separator_Paragraph ||
                        c1 == QChar::Punctuation_Open || c1 == QChar::Other_Control)
                    ending = false;

                // case 3
                if (c1 == QChar::Punctuation_InitialQuote) {
                    QChar openingQuote;

                    if (doubleQuotes)
                        openingQuote = mTypographicDoubleQuotes.begin;
                    else
                        openingQuote = mTypographicSingleQuotes.begin;

                    // case 3b
                    if (mWord.at(i-1) != openingQuote)
                        ending = false;
                }
            }
            // case 2a and 3a
            if ( i > 3 && !ending) {
                const QChar::Category c2 = (mWord.at(i-2)).category();
                ending = (c2 == QChar::Punctuation_InitialQuote);
            }

            if (doubleQuotes && mReplaceDoubleQuotes) {
                if (!ending) {
                    if (addNonBreakingSpace)
                        mWord.replace(i-1, 2, QString(nbsp + mTypographicDoubleQuotes.begin));
                    else
                        mWord[i-1] = mTypographicDoubleQuotes.begin;
                } else {
                    if (addNonBreakingSpace)
                        mWord.replace(i-1, 2,QString(nbsp + mTypographicDoubleQuotes.end));
                    else
                        mWord[i-1] = mTypographicDoubleQuotes.end;
                }
            } else if (mReplaceSingleQuotes) {
                if (!ending) {
                    if (addNonBreakingSpace)
                        mWord.replace(i-1, 2,QString(nbsp + mTypographicSingleQuotes.begin));
                    else
                        mWord[i-1] = mTypographicSingleQuotes.begin;
                } else {
                    if (addNonBreakingSpace)
                        mWord.replace(i-1, 2,QString(nbsp + mTypographicSingleQuotes.end));
                    else
                        mWord[i-1] = mTypographicSingleQuotes.end;
                }
            }
        }
    }

    // first character
    if (mWord.at(0) == QLatin1Char('"') && mReplaceDoubleQuotes) {
        if (addNonBreakingSpace)
            mWord.replace(0, 2, QString(mTypographicDoubleQuotes.begin + nbsp));
        else
            mWord[0] = mTypographicDoubleQuotes.begin;
    } else if (mWord.at(0) == QLatin1Char('\'') && mReplaceSingleQuotes) {
        if (addNonBreakingSpace)
            mWord.replace(0, 2,QString(mTypographicSingleQuotes.begin + nbsp));
        else
            mWord[0] = mTypographicSingleQuotes.begin;
    }
}


void AutoCorrection::readAutoCorrectionXmlFile( bool forceGlobal )
{
    KLocale *locale = KLocale::global();
    QString kdelang = locale->languageList().first();
    kdelang.remove(QRegExp(QLatin1String("@.*")));

    mUpperCaseExceptions.clear();
    mAutocorrectEntries.clear();
    mTwoUpperLetterExceptions.clear();
    mSuperScriptEntries.clear();


    QString LocalFile;
    //Look at local file:
    if (!forceGlobal) {
        if (!mAutoCorrectLang.isEmpty()) {
            LocalFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/custom-") + mAutoCorrectLang + QLatin1String(".xml"));
        } else {
            if (!kdelang.isEmpty())
                LocalFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/custom-") + kdelang + QLatin1String(".xml"));
            if (LocalFile.isEmpty() && kdelang.contains(QLatin1String("_"))) {
                kdelang.remove( QRegExp( QLatin1String("_.*") ) );
                LocalFile = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/custom-") + kdelang + QLatin1String(".xml"));
            }
        }
    }
    QString fname;
    //Load Global directly
    if (!mAutoCorrectLang.isEmpty()) {
        if (mAutoCorrectLang == QLatin1String("en_US")) {
            fname = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/autocorrect.xml"));
        } else {
            fname = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/") + mAutoCorrectLang + QLatin1String(".xml"));
        }
    } else {
        if (fname.isEmpty() && !kdelang.isEmpty())
            fname = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/") + kdelang + QLatin1String(".xml"));
        if (fname.isEmpty() && kdelang.contains(QLatin1String("_"))) {
            kdelang.remove( QRegExp( QLatin1String("_.*") ) );
            fname = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/") + kdelang + QLatin1String(".xml"));
        }
    }
    if (fname.isEmpty())
        fname = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("autocorrect/autocorrect.xml"));


    if (mAutoCorrectLang.isEmpty())
        mAutoCorrectLang = kdelang;
    //qDebug()<<" fname :"<<fname;
    //qDebug()<<" LocalFile:"<<LocalFile;

    if (LocalFile.isEmpty()) {
        if (fname.isEmpty()) {
            mTypographicSingleQuotes = typographicDefaultSingleQuotes();
            mTypographicDoubleQuotes = typographicDefaultDoubleQuotes();
        } else {
            ImportKMailAutocorrection import;
            if (import.import(fname,ImportAbstractAutocorrection::All)) {
                mUpperCaseExceptions = import.upperCaseExceptions();
                mTwoUpperLetterExceptions = import.twoUpperLetterExceptions();
                mAutocorrectEntries = import.autocorrectEntries();
                mTypographicSingleQuotes = import.typographicSingleQuotes();
                mTypographicDoubleQuotes = import.typographicDoubleQuotes();
                mSuperScriptEntries = import.superScriptEntries();
                if ( forceGlobal ) {
                    mTypographicSingleQuotes = typographicDefaultSingleQuotes();
                    mTypographicDoubleQuotes = typographicDefaultDoubleQuotes();
                }
            }
        }
    } else {
        ImportKMailAutocorrection import;
        if (import.import(LocalFile,ImportAbstractAutocorrection::All)) {
            mUpperCaseExceptions = import.upperCaseExceptions();
            mTwoUpperLetterExceptions = import.twoUpperLetterExceptions();
            mAutocorrectEntries = import.autocorrectEntries();
            mTypographicSingleQuotes = import.typographicSingleQuotes();
            mTypographicDoubleQuotes = import.typographicDoubleQuotes();
            //Don't import it in local
            //mSuperScriptEntries = import.superScriptEntries();
        }
        if (!fname.isEmpty() && import.import(fname,ImportAbstractAutocorrection::SuperScript)) {
            mSuperScriptEntries = import.superScriptEntries();
        }
    }
}

void AutoCorrection::writeAutoCorrectionXmlFile(const QString &filename)
{
    const QString fname = filename.isEmpty() ? QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1String("/autocorrect/custom-") + (mAutoCorrectLang == QLatin1String("en_US") ? QLatin1String("autocorrect") : mAutoCorrectLang) + QLatin1String(".xml") : filename;
    QFile file(fname);
    if ( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        qDebug()<<"We can't save in file :"<<fname;
        return;
    }
    QDomDocument root(QLatin1String("autocorrection"));
    root.appendChild(root.createProcessingInstruction(QLatin1String("xml"), QLatin1String("version=\"1.0\" encoding=\"UTF-8\"")));

    QDomElement word = root.createElement(QLatin1String( "Word" ));
    root.appendChild(word);
    QDomElement items = root.createElement(QLatin1String( "items" ));

    QHashIterator<QString, QString> i(mAutocorrectEntries);
    while (i.hasNext()) {
        i.next();
        QDomElement item = root.createElement(QLatin1String( "item" ));
        item.setAttribute(QLatin1String("find"),i.key());
        item.setAttribute(QLatin1String("replace"),i.value());
        items.appendChild(item);
    }
    word.appendChild(items);


    QDomElement upperCaseExceptions = root.createElement(QLatin1String( "UpperCaseExceptions" ));
    QSet<QString>::const_iterator upper = mUpperCaseExceptions.constBegin();
    while (upper != mUpperCaseExceptions.constEnd()) {
        QDomElement item = root.createElement(QLatin1String( "word" ));
        item.setAttribute(QLatin1String("exception"),*upper);
        upperCaseExceptions.appendChild(item);
        ++upper;
    }
    word.appendChild(upperCaseExceptions);

    QDomElement twoUpperLetterExceptions = root.createElement(QLatin1String( "TwoUpperLetterExceptions" ));
    QSet<QString>::const_iterator twoUpper = mTwoUpperLetterExceptions.constBegin();
    while (twoUpper != mTwoUpperLetterExceptions.constEnd()) {
        QDomElement item = root.createElement(QLatin1String( "word" ));
        item.setAttribute(QLatin1String("exception"),*twoUpper);
        twoUpperLetterExceptions.appendChild(item);
        ++twoUpper;
    }
    word.appendChild(twoUpperLetterExceptions);

    //Don't save it as  discussed with Calligra dev
    /*
    QDomElement supperscript = root.createElement(QLatin1String( "SuperScript" ));
    QHashIterator<QString, QString> j(mSuperScriptEntries);
    while (j.hasNext()) {
        j.next();
        QDomElement item = root.createElement(QLatin1String( "superscript" ));
        item.setAttribute(QLatin1String("find"), j.key());
        item.setAttribute(QLatin1String("super"), j.value());
        supperscript.appendChild(item);
    }
    word.appendChild(supperscript);
*/

    QDomElement doubleQuote = root.createElement(QLatin1String( "DoubleQuote" ));
    QDomElement item = root.createElement(QLatin1String( "doublequote" ));
    item.setAttribute(QLatin1String("begin"),mTypographicDoubleQuotes.begin);
    item.setAttribute(QLatin1String("end"),mTypographicDoubleQuotes.end);
    doubleQuote.appendChild(item);
    word.appendChild(doubleQuote);

    QDomElement singleQuote = root.createElement(QLatin1String( "SimpleQuote" ));
    item = root.createElement(QLatin1String( "simplequote" ));
    item.setAttribute(QLatin1String("begin"),mTypographicSingleQuotes.begin);
    item.setAttribute(QLatin1String("end"),mTypographicSingleQuotes.end);
    singleQuote.appendChild(item);
    word.appendChild(singleQuote);


    QTextStream ts( &file );
    ts.setCodec("UTF-8");
    ts << root.toString();
    file.close();
}


QString AutoCorrection::language() const
{
    return mAutoCorrectLang;
}

void AutoCorrection::setLanguage(const QString &lang, bool forceGlobal)
{
    if (mAutoCorrectLang != lang || forceGlobal) {
        mAutoCorrectLang = lang;
        //Re-read xml file
        readAutoCorrectionXmlFile(forceGlobal);
    }
}

bool AutoCorrection::isFrenchLanguage() const
{
    return (mAutoCorrectLang == QLatin1String("FR_fr") || mAutoCorrectLang == QLatin1String("fr"));
}

