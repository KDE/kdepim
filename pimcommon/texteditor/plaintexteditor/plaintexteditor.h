/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef PLAINTEXTEDITOR_H
#define PLAINTEXTEDITOR_H

#include "pimcommon_export.h"

#include <QPlainTextEdit>

namespace PimCommon
{
class PIMCOMMON_EXPORT PlainTextEditor : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(bool searchSupport READ searchSupport WRITE setSearchSupport)
    Q_PROPERTY(bool spellCheckingSupport READ spellCheckingSupport WRITE setSpellCheckingSupport)
    Q_PROPERTY(bool textToSpeechSupport READ textToSpeechSupport WRITE setTextToSpeechSupport)
    Q_PROPERTY(bool webShortcutSupport READ webShortcutSupport WRITE setWebShortcutSupport)
public:
    explicit PlainTextEditor(QWidget *parent = Q_NULLPTR);
    ~PlainTextEditor();

    enum SupportFeature {
        None = 0,
        Search = 1,
        SpellChecking = 2,
        TextToSpeech = 4,
        AllowWebShortcut = 8
    };
    Q_DECLARE_FLAGS(SupportFeatures, SupportFeature)

    void setSearchSupport(bool b);
    bool searchSupport() const;

    bool spellCheckingSupport() const;
    void setSpellCheckingSupport(bool check);

    virtual void setReadOnly(bool readOnly);

    void setTextToSpeechSupport(bool b);
    bool textToSpeechSupport() const;

    void setWebShortcutSupport(bool b);
    bool webShortcutSupport() const;


public Q_SLOTS:
    void slotDisplayMessageIndicator(const QString &message);
    void slotCheckSpelling();
    void slotSpeakText();

private Q_SLOTS:
    void slotUndoableClear();
    void slotSpellCheckerMisspelling(const QString &text, int pos);
    void slotSpellCheckerCorrected(const QString &, int, const QString &);
    void slotSpellCheckerAutoCorrect(const QString &, const QString &);
    void slotSpellCheckerCanceled();
    void slotSpellCheckerFinished();

protected:
    virtual void addExtraMenuEntry(QMenu *menu, const QPoint &pos);

protected:
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *ev) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    bool overrideShortcut(const QKeyEvent *event);
    bool handleShortcut(const QKeyEvent *event);

Q_SIGNALS:
    void findText();
    void replaceText();
    void spellCheckerAutoCorrect(const QString &currentWord, const QString &autoCorrectWord);
    void checkSpellingChanged(bool);
    void languageChanged(const QString &);
    void spellCheckStatus(const QString &);
    void say(const QString &text);

private:
    void deleteWordBack();
    void deleteWordForward();
    void highlightWord(int length, int pos);
    class PlainTextEditorPrivate;
    PlainTextEditorPrivate *const d;
};
}
#endif // PLAINTEXTEDITOR_H
