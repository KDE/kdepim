/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>
  
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

#ifndef KMCOMPOSERAUTOCORRECTION_H
#define KMCOMPOSERAUTOCORRECTION_H

#include "messagecomposer_export.h"
#include <QTextEdit>
#include <QTextCursor>
#include <QHash>
#include <QSet>
class MESSAGECOMPOSER_EXPORT KMComposerAutoCorrection
{
public:
  struct TypographicQuotes {
    QChar begin;
    QChar end;
  };

  explicit KMComposerAutoCorrection(QTextEdit *editor);
  ~KMComposerAutoCorrection();

  void setUppercaseFirstCharOfSentence(bool b) { mUppercaseFirstCharOfSentence = b; }
  void setFixTwoUppercaseChars(bool b) { mFixTwoUppercaseChars = b; }
  void setSingleSpaces(bool b) { mSingleSpaces = b; }
  void setAutoFractions(bool b) { mAutoFractions = b; }
  void setCapitalizeWeekDays(bool b) { mCapitalizeWeekDays = b; }
  void setReplaceDoubleQuotes(bool b) { mReplaceDoubleQuotes = b; }
  void setReplaceSingleQuotes(bool b) { mReplaceSingleQuotes = b; }
  void setAdvancedAutocorrect(bool b) { mAdvancedAutocorrect = b; }
  void setTypographicSingleQuotes(TypographicQuotes singleQuote) { mTypographicSingleQuotes = singleQuote; }
  void setTypographicDoubleQuotes(TypographicQuotes doubleQuote) { mTypographicDoubleQuotes = doubleQuote; }
  void setUpperCaseExceptions(const QSet<QString>& exceptions);
  void setTwoUpperLetterExceptions(const QSet<QString>& exceptions);
  void setAutocorrectEntries(const QHash<QString, QString>& entries);

  bool isUppercaseFirstCharOfSentence() const { return mUppercaseFirstCharOfSentence; }
  bool isFixTwoUppercaseChars() const { return mFixTwoUppercaseChars; }
  bool isSingleSpaces() const { return mSingleSpaces; }
  bool isAutoFractions() const { return mAutoFractions; }
  bool isCapitalizeWeekDays() const { return mCapitalizeWeekDays; }
  bool isReplaceDoubleQuotes() const { return mReplaceDoubleQuotes; }
  bool isReplaceSingleQuotes() const { return mReplaceSingleQuotes; }
  bool isAdvancedAutocorrect() const { return mAdvancedAutocorrect; }

  TypographicQuotes typographicSingleQuotes() const { return mTypographicSingleQuotes; }
  TypographicQuotes typographicDoubleQuotes() const { return mTypographicDoubleQuotes; }
  TypographicQuotes typographicDefaultSingleQuotes();
  TypographicQuotes typographicDefaultDoubleQuotes();
  QSet<QString> upperCaseExceptions() const;
  QSet<QString> twoUpperLetterExceptions() const;
  QHash<QString, QString> autocorrectEntries() const;

  void autocorrect();
private:
  void readConfig();
  void writeConfig();
  void fixTwoUppercaseChars();
  bool singleSpaces();
  void capitalizeWeekDays();
  bool autoFractions();
  void uppercaseFirstCharOfSentence();
  void advancedAutocorrect();

  void selectWord(QTextCursor &cursor, int cursorPosition);

  QTextEdit *mEditor;

  bool mSingleSpaces; // suppress double spaces.
  bool mUppercaseFirstCharOfSentence; // convert first letter of a sentence automaticall to uppercase
  bool mFixTwoUppercaseChars;  // convert two uppercase characters to one upper and one lowercase.
  bool mAutoFractions; // replace 1/2 with ½
  bool mCapitalizeWeekDays;
  bool mAdvancedAutocorrect; // autocorrection from a list of entries

  bool mReplaceDoubleQuotes;  // replace double quotes with typographical quotes
  bool mReplaceSingleQuotes;  // replace single quotes with typographical quotes

  bool mEnabled;

  QString mWord;
  QTextCursor mCursor;

  QString m_autocorrectLang;
  QStringList mCacheNameOfDays;
  QHash<QString, QString> mSuperScriptEntries;
  QSet<QString> mUpperCaseExceptions;
  QSet<QString> mTwoUpperLetterExceptions;
  QHash<QString, QString> mAutocorrectEntries;
  TypographicQuotes mTypographicSingleQuotes;
  TypographicQuotes mTypographicDoubleQuotes;
};

#endif // KMCOMPOSERAUTOCORRECTION_H
