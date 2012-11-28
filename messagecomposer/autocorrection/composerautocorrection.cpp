/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
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

#include "composerautocorrection.h"
#include "messagecomposersettings.h"
#include "import/importkmailautocorrection.h"
#include <KLocale>
#include <KGlobal>
#include <KColorScheme>
#include <KCalendarSystem>
#include <KStandardDirs>
#include <QTextBlock>
#include <QDomDocument>
#include <QFile>

using namespace MessageComposer;

ComposerAutoCorrection::ComposerAutoCorrection()
  : mSingleSpaces(true),
    mUppercaseFirstCharOfSentence(false),
    mFixTwoUppercaseChars(false),
    mAutoFractions(true),
    mCapitalizeWeekDays(false),
    mReplaceDoubleQuotes(false),
    mReplaceSingleQuotes(false),
    mEnabled(false),
    mSuperScriptAppendix(false)
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

  KLocale *locale = KGlobal::locale();
  for (int i = 1; i <=7; i++)
    mCacheNameOfDays.append(locale->calendar()->weekDayName(i).toLower());

}

ComposerAutoCorrection::~ComposerAutoCorrection()
{
}


void ComposerAutoCorrection::selectWord(QTextCursor &cursor, int cursorPosition)
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
    pos++;
    iter++;
  }
  cursor.setPosition(pos + block.position(), QTextCursor::KeepAnchor);
}


void ComposerAutoCorrection::autocorrect(bool htmlMode, QTextDocument& document, int position)
{
  if (!mEnabled)
    return;
  mCursor =  QTextCursor(&document);
  selectWord(mCursor,position);
  mWord = mCursor.selectedText();
  if (mWord.isEmpty())
    return;

  bool done = false;
  if (htmlMode) {
     done = autoFormatURLs();
     if(!done) {
         done = autoBoldUnderline();
     }
     if(!done) {
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
}

void ComposerAutoCorrection::readConfig()
{
  mAutoBoldUnderline = MessageComposer::MessageComposerSettings::self()->autoBoldUnderline();
  mAutoFormatUrl = MessageComposer::MessageComposerSettings::self()->autoFormatUrl();
  mUppercaseFirstCharOfSentence = MessageComposer::MessageComposerSettings::self()->uppercaseFirstCharOfSentence();
  mFixTwoUppercaseChars = MessageComposer::MessageComposerSettings::self()->fixTwoUppercaseChars();
  mSingleSpaces = MessageComposer::MessageComposerSettings::self()->singleSpaces();
  mAutoFractions = MessageComposer::MessageComposerSettings::self()->autoFractions();
  mCapitalizeWeekDays = MessageComposer::MessageComposerSettings::self()->capitalizeWeekDays();
  mAdvancedAutocorrect = MessageComposer::MessageComposerSettings::self()->advancedAutocorrect();
  mReplaceDoubleQuotes = MessageComposer::MessageComposerSettings::self()->replaceDoubleQuotes();
  mReplaceSingleQuotes = MessageComposer::MessageComposerSettings::self()->replaceSingleQuotes();
  mEnabled = MessageComposer::MessageComposerSettings::self()->enabled();
  mSuperScriptAppendix = MessageComposer::MessageComposerSettings::self()->superScript();
  readAutoCorrectionXmlFile();
}

void ComposerAutoCorrection::writeConfig()
{
  MessageComposer::MessageComposerSettings::self()->setAutoBoldUnderline(mAutoBoldUnderline);
  MessageComposer::MessageComposerSettings::self()->setAutoFormatUrl(mAutoFormatUrl);
  MessageComposer::MessageComposerSettings::self()->setUppercaseFirstCharOfSentence(mUppercaseFirstCharOfSentence);
  MessageComposer::MessageComposerSettings::self()->setFixTwoUppercaseChars(mFixTwoUppercaseChars);
  MessageComposer::MessageComposerSettings::self()->setSingleSpaces(mSingleSpaces);
  MessageComposer::MessageComposerSettings::self()->setAutoFractions(mAutoFractions);
  MessageComposer::MessageComposerSettings::self()->setCapitalizeWeekDays(mCapitalizeWeekDays);
  MessageComposer::MessageComposerSettings::self()->setAdvancedAutocorrect(mAdvancedAutocorrect);
  MessageComposer::MessageComposerSettings::self()->setReplaceDoubleQuotes(mReplaceDoubleQuotes);
  MessageComposer::MessageComposerSettings::self()->setReplaceSingleQuotes(mReplaceSingleQuotes);
  MessageComposer::MessageComposerSettings::self()->setEnabled(mEnabled);
  MessageComposer::MessageComposerSettings::self()->setSuperScript(mSuperScriptAppendix);
  MessageComposer::MessageComposerSettings::self()->requestSync();
  writeAutoCorrectionXmlFile();
}

void ComposerAutoCorrection::addAutoCorrect(const QString &currentWord, const QString &replaceWord)
{
  mAutocorrectEntries.insert(currentWord, replaceWord);
  writeAutoCorrectionXmlFile();
}


void ComposerAutoCorrection::setUpperCaseExceptions(const QSet<QString>& exceptions)
{
  mUpperCaseExceptions = exceptions;
}

void ComposerAutoCorrection::setTwoUpperLetterExceptions(const QSet<QString>& exceptions)
{
  mTwoUpperLetterExceptions = exceptions;
}

void ComposerAutoCorrection::setAutocorrectEntries(const QHash<QString, QString>& entries)
{
  mAutocorrectEntries = entries;
}


ComposerAutoCorrection::TypographicQuotes ComposerAutoCorrection::typographicDefaultSingleQuotes()
{
  ComposerAutoCorrection::TypographicQuotes quote;
  quote.begin = QChar(0x2018);
  quote.end = QChar(0x2019);
  return quote;
}

ComposerAutoCorrection::TypographicQuotes ComposerAutoCorrection::typographicDefaultDoubleQuotes()
{
  ComposerAutoCorrection::TypographicQuotes quote;
  quote.begin = QChar(0x201c);
  quote.end = QChar(0x201d);
  return quote;
}

QSet<QString> ComposerAutoCorrection::upperCaseExceptions() const
{
  return mUpperCaseExceptions;
}

QSet<QString> ComposerAutoCorrection::twoUpperLetterExceptions() const
{
  return mTwoUpperLetterExceptions;
}

QHash<QString, QString> ComposerAutoCorrection::autocorrectEntries() const
{
  return mAutocorrectEntries;
}

void ComposerAutoCorrection::superscriptAppendix()
{
    if (!mSuperScriptAppendix) return;

    QString trimmed = mWord.trimmed();
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
            int pos = trimmed.indexOf(i.value());
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

bool ComposerAutoCorrection::autoBoldUnderline()
{
    if (!mAutoBoldUnderline)
        return false;

    QString trimmed = mWord.trimmed();

    if (trimmed.length() < 3)
        return false;

    bool underline = (trimmed.at(0) == QLatin1Char('_') && trimmed.at(trimmed.length() - 1) == QLatin1Char('_'));
    bool bold = (trimmed.at(0) == QLatin1Char('*') && trimmed.at(trimmed.length() - 1) == QLatin1Char('*'));

    if (underline || bold) {
        int startPos = mCursor.selectionStart();
        QString replacement = trimmed.mid(1, trimmed.length() - 2);
        bool foundLetterNumber = false;

        QString::ConstIterator constIter = replacement.constBegin();
        while (constIter != replacement.constEnd()) {
            if (constIter->isLetterOrNumber()) {
                foundLetterNumber = true;
                break;
            }
            constIter++;
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
        mCursor.mergeCharFormat(format);

        // to avoid the selection being replaced by mWord
        mWord = mCursor.selectedText();

        // don't do this again if the text is already underlined and bold
        if(mCursor.charFormat().fontUnderline()
            && mCursor.charFormat().fontWeight() == QFont::Bold) {
            return true;
        } else {
            return autoBoldUnderline();
        }
    }
    else
        return false;

    return true;
}

bool ComposerAutoCorrection::autoFormatURLs()
{
    if (!mAutoFormatUrl)
        return false;

    QString link = autoDetectURL(mWord);
    if (link.isNull())
        return false;

    QString trimmed = mWord.trimmed();
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

QString ComposerAutoCorrection::autoDetectURL(const QString &_word) const
{
    QString word = _word;

    /* this method is ported from lib/kotext/KoAutoFormat.cpp KoAutoFormat::doAutoDetectUrl
     * from Calligra 1.x branch */
    // kDebug() <<"link:" << word;

    char link_type = 0;
    int pos = word.indexOf(QLatin1String("http://"));
    int tmp_pos = word.indexOf(QLatin1String("https://"));

    if (tmp_pos < pos && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf(QLatin1String("mailto:/"));
    if ((tmp_pos < pos || pos == -1) && tmp_pos != -1)
          pos = tmp_pos;
    tmp_pos = word.indexOf(QLatin1String("ftp://"));
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

        if (link_type == 1)
            newWord = QLatin1String("mailto:") + word;
        else if (link_type == 2)
            newWord = QLatin1String("http://") + word;
        else if (link_type == 3)
            newWord = QLatin1String("ftp://") + word;

        kDebug() <<"newWord:" << newWord;
        return newWord;
    }

    return QString();
}

void ComposerAutoCorrection::fixTwoUppercaseChars()
{
  if (!mFixTwoUppercaseChars)
    return;
  if (mWord.length() <= 2)
    return;

  if (mTwoUpperLetterExceptions.contains(mWord.trimmed()))
    return;

  QChar firstChar = mWord.at(0);
  QChar secondChar = mWord.at(1);

  if (secondChar.isUpper()) {
    QChar thirdChar = mWord.at(2);

    if (firstChar.isUpper() && thirdChar.isLower())
      mWord.replace(1, 1, secondChar.toLower());
  }
}


bool ComposerAutoCorrection::singleSpaces()
{
  if (!mSingleSpaces)
      return false;
  if (!mCursor.atBlockStart() && mWord.length() == 1 && mWord.at(0) == QLatin1Char(' ')) {
    // then when the prev char is also a space, don't insert one.
    QTextBlock block = mCursor.block();
    QString text = block.text();
    if (text.at(mCursor.position() -1 - block.position()) == QLatin1Char(' ')) {
      mWord.clear();
      return true;
    }
  }
  return false;
}

void ComposerAutoCorrection::capitalizeWeekDays()
{
  if (!mCapitalizeWeekDays)
    return;

  const QString trimmed = mWord.trimmed();
  Q_FOREACH (const QString & name, mCacheNameOfDays) {
    if (trimmed == name) {
       int pos = mWord.indexOf(name);
       mWord.replace(pos, 1, name.at(0).toUpper());
       return;
    }
  }
}

void ComposerAutoCorrection::uppercaseFirstCharOfSentence()
{
    if (!mUppercaseFirstCharOfSentence)
        return;

    int startPos = mCursor.selectionStart();
    QTextBlock block = mCursor.block();

    mCursor.setPosition(block.position());
    mCursor.setPosition(startPos, QTextCursor::KeepAnchor);

    int position = mCursor.selectionEnd();

    QString text = mCursor.selectedText();

    if (text.isEmpty()) // start of a paragraph
        mWord.replace(0, 1, mWord.at(0).toUpper());
    else {
        QString::ConstIterator constIter = text.constEnd();
        constIter--;

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
                bool replace = true;
                selectWord(mCursor, --position);
                QString prevWord = mCursor.selectedText();

                // search for exception
                if (mUpperCaseExceptions.contains(prevWord.trimmed()))
                    replace = false;

                if (replace)
                    mWord.replace(0, 1, mWord.at(0).toUpper());
                break;
            }
            else
                break;
        }
    }

    mCursor.setPosition(startPos);
    mCursor.setPosition(startPos + mWord.length(), QTextCursor::KeepAnchor);
}

bool ComposerAutoCorrection::autoFractions()
{
    if (!mAutoFractions)
        return false;

    QString trimmed = mWord.trimmed();
    if (trimmed.length() > 3) {
        QChar x = trimmed.at(3);
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

void ComposerAutoCorrection::advancedAutocorrect()
{
  if (!mAdvancedAutocorrect)
    return;

  int startPos = mCursor.selectionStart();
  int length = mWord.length();

  QString trimmedWord = mWord.toLower().trimmed();
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

  if (mAutocorrectEntries.contains(actualWord)) {
    int pos = mWord.toLower().indexOf(trimmedWord);
    QString replacement = mAutocorrectEntries.value(actualWord);
    // Keep capitalized words capitalized.
    // (Necessary to make sure the first letters match???)
    if (actualWord.at(0) == replacement.at(0).toLower()) {
      if (mWord.at(0).isUpper()) {
        replacement[0] = replacement[0].toUpper();
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

void ComposerAutoCorrection::replaceTypographicQuotes()
{
    /* this method is ported from lib/kotext/KoAutoFormat.cpp KoAutoFormat::doTypographicQuotes
     * from Calligra 1.x branch */

    if (!(mReplaceDoubleQuotes && mWord.contains(QLatin1Char('"'))) &&
            !(mReplaceSingleQuotes && mWord.contains(QLatin1Char('\'')))) return;

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
    QString::Iterator iter = mWord.end();
    iter--;

    while (iter != mWord.begin()) {
        if (*iter == QLatin1Char('"') || *iter == QLatin1Char('\'')) {
            bool doubleQuotes = *iter == QLatin1Char('"');

            if ((iter - 1) != mWord.begin()) {
                QChar::Category c1 = (*(iter - 1)).category();

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
                    if (*(iter - 1) != openingQuote)
                        ending = false;
                }
            }

            // case 2a and 3a
            if ((iter - 2) != mWord.constBegin() && !ending)
            {
                 QChar::Category c2 = (*(iter - 2)).category();
                 ending = (c2 == QChar::Punctuation_InitialQuote);
            }

            if (doubleQuotes && mReplaceDoubleQuotes) {
                if (!ending)
                    *iter = mTypographicDoubleQuotes.begin;
                else
                    *iter = mTypographicDoubleQuotes.end;
            }
            else if (mReplaceSingleQuotes) {
                if (!ending)
                    *iter = mTypographicSingleQuotes.begin;
                else
                    *iter = mTypographicSingleQuotes.end;
            }
        }
        iter--;
    }

    // first character
    if (*iter == QLatin1Char('"') && mReplaceDoubleQuotes)
        *iter = mTypographicDoubleQuotes.begin;
    else if (*iter == QLatin1Char('\'') && mReplaceSingleQuotes)
        *iter = mTypographicSingleQuotes.begin;
}


void ComposerAutoCorrection::readAutoCorrectionXmlFile( bool forceGlobal )
{
    KLocale *locale = KGlobal::locale();
    QString kdelang = locale->languageList().first();
    kdelang.remove(QRegExp(QLatin1String("@.*")));

    //qDebug()<<"void ComposerAutoCorrection::readAutoCorrectionXmlFile() "<<mAutoCorrectLang;

    mUpperCaseExceptions.clear();
    mAutocorrectEntries.clear();
    mTwoUpperLetterExceptions.clear();
    mSuperScriptEntries.clear();


    QString LocalFile;
    //Look at local file:
    if(!forceGlobal) {
        if (!mAutoCorrectLang.isEmpty()) {
            LocalFile = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/custom-") + mAutoCorrectLang + QLatin1String(".xml"));
        } else {
            if(!kdelang.isEmpty())
                LocalFile = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/custom-") + kdelang + QLatin1String(".xml"));
            if (LocalFile.isEmpty() && kdelang.contains(QLatin1String("_"))) {
                kdelang.remove( QRegExp( QLatin1String("_.*") ) );
                LocalFile = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/custom-") + kdelang + QLatin1String(".xml"));
            }
        }
    }
    QString fname;
    //Load Global directly
    if (!mAutoCorrectLang.isEmpty()) {
        if(mAutoCorrectLang == QLatin1String("en_US")) {
          fname = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/autocorrect.xml"));
        } else {
          fname = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/") + mAutoCorrectLang + QLatin1String(".xml"));
        }
    } else {
        if (fname.isEmpty() && !kdelang.isEmpty())
            fname = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/") + kdelang + QLatin1String(".xml"));
        if (fname.isEmpty() && kdelang.contains(QLatin1String("_"))) {
            kdelang.remove( QRegExp( QLatin1String("_.*") ) );
            fname = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/") + kdelang + QLatin1String(".xml"));
        }
    }
    if (fname.isEmpty())
        fname = KGlobal::dirs()->findResource("data", QLatin1String("autocorrect/autocorrect.xml"));


    if (mAutoCorrectLang.isEmpty())
        mAutoCorrectLang = kdelang;
    //qDebug()<<" fname :"<<fname;
    //qDebug()<<" LocalFile:"<<LocalFile;

    if(LocalFile.isEmpty()) {
        if(fname.isEmpty()) {
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

void ComposerAutoCorrection::writeAutoCorrectionXmlFile()
{
    const QString fname = KGlobal::dirs()->locateLocal("data", QLatin1String("autocorrect/custom-") + (mAutoCorrectLang == QLatin1String("en_US") ? QLatin1String("autocorrect") : mAutoCorrectLang) + QLatin1String(".xml"));
    QFile file(fname);
    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        kDebug()<<"We can't save in file :"<<fname;
        return;
    }
    QDomDocument root(QLatin1String("autocorrection"));

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
    ts << root.toString();
    file.close();
}


QString ComposerAutoCorrection::language() const
{
  return mAutoCorrectLang;
}

void ComposerAutoCorrection::setLanguage(const QString &lang, bool forceGlobal)
{
  if(mAutoCorrectLang != lang || forceGlobal) {
    mAutoCorrectLang = lang;
    //Re-read xml file
    readAutoCorrectionXmlFile(forceGlobal);
  }
}

