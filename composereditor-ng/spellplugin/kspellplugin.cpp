/* ============================================================
*
* Based on kspellplugin from rekonq project
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

#include <stdio.h>
#include <QDebug>
#include "kspellplugin.h"
#include <QTextBoundaryFinder>
#include "globalsettings_base.h"

#define methodDebug() kDebug("KWebSpellChecker: %s", __FUNCTION__)

/////////////////////////////
// KWebSpellChecker

KWebSpellChecker::KWebSpellChecker()
{
    m_speller = new Sonnet::Speller();
}

KWebSpellChecker::~KWebSpellChecker()
{
    delete m_speller;
}

bool KWebSpellChecker::isContinousSpellCheckingEnabled() const
{
    return ComposerEditorNG::GlobalSettingsBase::autoSpellChecking();
}

void KWebSpellChecker::toggleContinousSpellChecking()
{
    //TODO
}

void KWebSpellChecker::learnWord(const QString &word)
{
    Q_UNUSED(word);
}

void KWebSpellChecker::ignoreWordInSpellDocument(const QString &word)
{
    Q_UNUSED(word);
}

static bool isValidWord(const QString &str)
{
    if (str.isEmpty() || (str.length() == 1 && !str[0].isLetter())) {
        return false;
    }
    const int length = str.length();
    for (int i = 0; i < length; ++i) {
        if (!str[i].isNumber()) {
            return true;
        }
    }
    // 'str' only contains numbers
    return false;
}

void KWebSpellChecker::checkSpellingOfString(const QString &word, int *misspellingLocation, int *misspellingLength)
{
    // sanity check
    if (misspellingLocation == NULL || misspellingLength == NULL) {
        return;
    }
//QT5
#if 0
    *misspellingLocation = -1;
    *misspellingLength = 0;

    qDebug() << word << endl;

    QTextBoundaryFinder finder =  QTextBoundaryFinder(QTextBoundaryFinder::Word, word);

    QTextBoundaryFinder::BoundaryReasons boundary = finder.boundaryReasons();
    int start = finder.position(), end = finder.position();
    bool inWord = (boundary & QTextBoundaryFinder::StartWord) != 0;
    while (finder.toNextBoundary() > 0) {
        boundary = finder.boundaryReasons();
        if ((boundary & QTextBoundaryFinder::EndWord) && inWord) {
            end = finder.position();
            QString str = finder.string().mid(start, end - start);
            if (isValidWord(str)) {
#if 1
                qDebug() << "Word at " << start << " word = '"
                         <<  str << "', len = " << str.length();
#endif
                if (m_speller->isMisspelled(str)) {
                    *misspellingLocation = start;
                    *misspellingLength = end - start;
                }
                return;
            }
            inWord = false;
        }
        if ((boundary & QTextBoundaryFinder::StartWord)) {
            start = finder.position();
            inWord = true;
        }
    }
#endif
}

QString KWebSpellChecker::autoCorrectSuggestionForMisspelledWord(const QString &word)
{
    /*
    QStringList words = m_speller->suggest(word);
    if (words.size() > 0)
        return words[0];
    else
        return QString("");
    */

    return QString();
}

void KWebSpellChecker::guessesForWord(const QString &word, const QString &context, QStringList &guesses)
{
    Q_UNUSED(context);

    const QStringList words = m_speller->suggest(word);
    guesses = words;
}

bool KWebSpellChecker::isGrammarCheckingEnabled()
{
    return false;
}

void KWebSpellChecker::toggleGrammarChecking()
{
}

void KWebSpellChecker::checkGrammarOfString(const QString &, QList<GrammarDetail> &, int *badGrammarLocation, int *badGrammarLength)
{
    Q_UNUSED(badGrammarLocation);
    Q_UNUSED(badGrammarLength);
}

////////////////////////////////////////////
// KWebKitPlatformPlugin
KWebKitPlatformPlugin::KWebKitPlatformPlugin()
{
}

KWebKitPlatformPlugin::~KWebKitPlatformPlugin()
{
}

bool KWebKitPlatformPlugin::supportsExtension(Extension ext) const
{
    return ext == SpellChecker;
}

QObject *KWebKitPlatformPlugin::createExtension(Extension ext) const
{
    if (ext == SpellChecker) {
        return new KWebSpellChecker();
    }

    return NULL;
}

//Q_EXPORT_PLUGIN2(kwebspellchecker, KWebKitPlatformPlugin)
//QT5
//Q_IMPORT_PLUGIN(kwebspellchecker)

