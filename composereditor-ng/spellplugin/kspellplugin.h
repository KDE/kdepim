/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (C) 2012 by Lindsay Mathieson <lindsay dot mathieson at gmail dot com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */


#ifndef TESTQWEBSPELLCHECKER_H
#define TESTQWEBSPELLCHECKER_H


#include <QtGlobal>
#include <QtPlugin>
#include <sonnet/speller.h>
#include "qwebkitplatformplugin.h"




class KWebSpellChecker : public QWebSpellChecker
{
    Q_OBJECT
public:
    Sonnet::Speller *m_speller;

    KWebSpellChecker();
    ~KWebSpellChecker();

    virtual bool isContinousSpellCheckingEnabled() const;
    virtual void toggleContinousSpellChecking();
    virtual void learnWord(const QString& word);
    virtual void ignoreWordInSpellDocument(const QString& word);
    virtual void checkSpellingOfString(const QString& word, int* misspellingLocation, int* misspellingLength);
    virtual QString autoCorrectSuggestionForMisspelledWord(const QString& word);
    virtual void guessesForWord(const QString& word, const QString& context, QStringList& guesses);

    virtual bool isGrammarCheckingEnabled();
    virtual void toggleGrammarChecking();
    virtual void checkGrammarOfString(const QString&, QList<GrammarDetail>&, int* badGrammarLocation, int* badGrammarLength);
};


class KWebKitPlatformPlugin : public QObject, public QWebKitPlatformPlugin
{
    Q_OBJECT
    Q_INTERFACES(QWebKitPlatformPlugin)

public:
    KWebKitPlatformPlugin();
    ~KWebKitPlatformPlugin();

    virtual bool supportsExtension(Extension) const;
    virtual QObject* createExtension(Extension) const;

};

#endif