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

#include "kmcomposerautocorrection.h"
#include "messagecomposersettings.h"
#include <KLocale>
#include <KGlobal>
#include <KCalendarSystem>
#include <KStandardDirs>
#include <QTextBlock>
#include <QDomDocument>
#include <QFile>

KMComposerAutoCorrection::KMComposerAutoCorrection()
  : mSingleSpaces(true),
    mUppercaseFirstCharOfSentence(false),
    mFixTwoUppercaseChars(false),
    mAutoFractions(true),
    mCapitalizeWeekDays(false),
    mReplaceDoubleQuotes(false),
    mReplaceSingleQuotes(false),
    mEnabled(false)
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

KMComposerAutoCorrection::~KMComposerAutoCorrection()
{
  writeConfig();
}


void KMComposerAutoCorrection::selectWord(QTextCursor &cursor, int cursorPosition)
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


void KMComposerAutoCorrection::autocorrect(QTextDocument& document, int position)
{
  if (!mEnabled)
    return;
  mCursor =  QTextCursor(&document);
  selectWord(mCursor,position);
  mWord = mCursor.selectedText();
  if (mWord.isEmpty())
    return;

  bool done = false;
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

void KMComposerAutoCorrection::readConfig()
{
  mUppercaseFirstCharOfSentence = MessageComposer::MessageComposerSettings::self()->uppercaseFirstCharOfSentence();
  mFixTwoUppercaseChars = MessageComposer::MessageComposerSettings::self()->fixTwoUppercaseChars();
  mSingleSpaces = MessageComposer::MessageComposerSettings::self()->singleSpaces();
  mAutoFractions = MessageComposer::MessageComposerSettings::self()->autoFractions();
  mCapitalizeWeekDays = MessageComposer::MessageComposerSettings::self()->capitalizeWeekDays();
  mAdvancedAutocorrect = MessageComposer::MessageComposerSettings::self()->advancedAutocorrect();
  mReplaceDoubleQuotes = MessageComposer::MessageComposerSettings::self()->replaceDoubleQuotes();
  mReplaceSingleQuotes = MessageComposer::MessageComposerSettings::self()->replaceSingleQuotes();
  mEnabled = MessageComposer::MessageComposerSettings::self()->enabled();
  readAutoCorrectionXmlFile();
}

void KMComposerAutoCorrection::writeConfig()
{
  MessageComposer::MessageComposerSettings::self()->setUppercaseFirstCharOfSentence(mUppercaseFirstCharOfSentence);
  MessageComposer::MessageComposerSettings::self()->setFixTwoUppercaseChars(mFixTwoUppercaseChars);
  MessageComposer::MessageComposerSettings::self()->setSingleSpaces(mSingleSpaces);
  MessageComposer::MessageComposerSettings::self()->setAutoFractions(mAutoFractions);
  MessageComposer::MessageComposerSettings::self()->setCapitalizeWeekDays(mCapitalizeWeekDays);
  MessageComposer::MessageComposerSettings::self()->setAdvancedAutocorrect(mAdvancedAutocorrect);
  MessageComposer::MessageComposerSettings::self()->setReplaceDoubleQuotes(mReplaceDoubleQuotes);
  MessageComposer::MessageComposerSettings::self()->setReplaceSingleQuotes(mReplaceSingleQuotes);
  MessageComposer::MessageComposerSettings::self()->setEnabled(mEnabled);
  MessageComposer::MessageComposerSettings::self()->requestSync();
  writeAutoCorrectionXmlFile();
}


void KMComposerAutoCorrection::setUpperCaseExceptions(const QSet<QString>& exceptions)
{
  mUpperCaseExceptions = exceptions;
}

void KMComposerAutoCorrection::setTwoUpperLetterExceptions(const QSet<QString>& exceptions)
{
  mTwoUpperLetterExceptions = exceptions;
}

void KMComposerAutoCorrection::setAutocorrectEntries(const QHash<QString, QString>& entries)
{
  mAutocorrectEntries = entries;
}


KMComposerAutoCorrection::TypographicQuotes KMComposerAutoCorrection::typographicDefaultSingleQuotes()
{
  KMComposerAutoCorrection::TypographicQuotes quote;
  quote.begin = QChar(0x2018);
  quote.end = QChar(0x2019);
  return quote;
}

KMComposerAutoCorrection::TypographicQuotes KMComposerAutoCorrection::typographicDefaultDoubleQuotes()
{
  KMComposerAutoCorrection::TypographicQuotes quote;
  quote.begin = QChar(0x201c);
  quote.end = QChar(0x201d);
  return quote;
}

QSet<QString> KMComposerAutoCorrection::upperCaseExceptions() const
{
  return mUpperCaseExceptions;
}

QSet<QString> KMComposerAutoCorrection::twoUpperLetterExceptions() const
{
  return mTwoUpperLetterExceptions;
}

QHash<QString, QString> KMComposerAutoCorrection::autocorrectEntries() const
{
  return mAutocorrectEntries;
}


void KMComposerAutoCorrection::fixTwoUppercaseChars()
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


bool KMComposerAutoCorrection::singleSpaces()
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

void KMComposerAutoCorrection::capitalizeWeekDays()
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

void KMComposerAutoCorrection::uppercaseFirstCharOfSentence()
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

bool KMComposerAutoCorrection::autoFractions()
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

void KMComposerAutoCorrection::advancedAutocorrect()
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

void KMComposerAutoCorrection::replaceTypographicQuotes()
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


void KMComposerAutoCorrection::readAutoCorrectionXmlFile()
{

    KLocale *locale = KGlobal::locale();
    QString kdelang = locale->languageList().first();
    kdelang.remove(QRegExp(QLatin1String("@.*")));

    QString fname;
    if (!mAutoCorectLang.isEmpty())
        fname = KGlobal::dirs()->findResource("data", QLatin1String("kmail2/autocorrect/") + mAutoCorectLang + QLatin1String(".xml"));
    if (mAutoCorectLang != QLatin1String("all_languages")) {
        if (fname.isEmpty() && !kdelang.isEmpty())
            fname = KGlobal::dirs()->findResource("data", QLatin1String("kmail2/autocorrect/") + kdelang + QLatin1String(".xml"));
        if (fname.isEmpty() && kdelang.contains(QLatin1String("_"))) {
            kdelang.remove( QRegExp( QLatin1String("_.*") ) );
            fname = KGlobal::dirs()->findResource("data", QLatin1String("kmail2/autocorrect/") + kdelang + QLatin1String(".xml"));
        }
        if (fname.isEmpty())
            fname = KGlobal::dirs()->findResource("data", QLatin1String("kmail2/autocorrect/autocorrect.xml"));
    }
    if (mAutoCorectLang.isEmpty())
        mAutoCorectLang = kdelang;


    qDebug()<<" fname :"<<fname;
    if (fname.isEmpty())
        return;

    QFile xmlFile(fname);
    if (!xmlFile.open(QIODevice::ReadOnly))
        return;

    QDomDocument doc;
    if (!doc.setContent(&xmlFile))
        return;
    if (doc.doctype().name() != QLatin1String("autocorrection"))
        return;

    QDomElement de = doc.documentElement();

    QDomElement upper = de.namedItem(QLatin1String("UpperCaseExceptions")).toElement();
    if (!upper.isNull()) {
        QDomNodeList nl = upper.childNodes();
        for (int i = 0; i < nl.count(); i++)
            mUpperCaseExceptions += nl.item(i).toElement().attribute(QLatin1String("exception"));
    }

    QDomElement twoUpper = de.namedItem(QLatin1String("TwoUpperLetterExceptions")).toElement();
    if (!twoUpper.isNull()) {
        QDomNodeList nl = twoUpper.childNodes();
        for(int i = 0; i < nl.count(); i++)
            mTwoUpperLetterExceptions += nl.item(i).toElement().attribute(QLatin1String("exception"));
    }

    /* Load advanced autocorrect entry, including the format */
    QDomElement item = de.namedItem(QLatin1String("items")).toElement();
    if (!item.isNull())
    {
        QDomNodeList nl = item.childNodes();
        for (int i = 0; i < nl.count(); i++) {
            QDomElement element = nl.item(i).toElement();
            const QString find = element.attribute(QLatin1String("find"));
            const QString replace = element.attribute(QLatin1String("replace"));
            qDebug()<<" find :"<<find<<" replace "<<replace;
            mAutocorrectEntries.insert(find, replace);
        }
    }

    QDomElement doubleQuote = de.namedItem(QLatin1String("DoubleQuote")).toElement();
    if(doubleQuote.isNull()) {
      QDomNodeList nl = doubleQuote.childNodes();
      if(nl.count()==1) {
        QDomElement element = nl.item(0).toElement();
        mTypographicDoubleQuotes.begin = element.attribute(QLatin1String("begin")).at(0);
        mTypographicDoubleQuotes.end = element.attribute(QLatin1String("end")).at(0);
      }
    }

    QDomElement singleQuote = de.namedItem(QLatin1String("SimpleQuote")).toElement();
    if(singleQuote.isNull()) {
      QDomNodeList nl = singleQuote.childNodes();
      if(nl.count()==1) {
        QDomElement element = nl.item(0).toElement();
        mTypographicSingleQuotes.begin = element.attribute(QLatin1String("begin")).at(0);
        mTypographicSingleQuotes.end = element.attribute(QLatin1String("end")).at(0);
      }
    }

}

void KMComposerAutoCorrection::writeAutoCorrectionXmlFile()
{
    const QString fname = KGlobal::dirs()->locateLocal("data", QLatin1String("kmail2/autocorrect.xml"));
    QFile file(fname);
    if( !file.open( QIODevice::WriteOnly | QIODevice::Text ) ) {
        qDebug()<<"We can't save in file :"<<fname;
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
        upperCaseExceptions.appendChild(item);
        ++twoUpper;
    }
    word.appendChild(twoUpperLetterExceptions);


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


