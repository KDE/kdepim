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

#ifndef RICHTEXTEDITOR_H
#define RICHTEXTEDITOR_H

#include "pimcommon_export.h"
#include <QTextEdit>

class QContextMenuEvent;
class QMenu;
namespace Sonnet
{
class Highlighter;
class SpellCheckDecorator;
}
namespace PimCommon
{
class PIMCOMMON_EXPORT RichTextEditor : public QTextEdit
{
    Q_OBJECT
    Q_PROPERTY(bool searchSupport READ searchSupport WRITE setSearchSupport)
    Q_PROPERTY(bool spellCheckingSupport READ spellCheckingSupport WRITE setSpellCheckingSupport)
    Q_PROPERTY(bool textToSpeechSupport READ textToSpeechSupport WRITE setTextToSpeechSupport)
    Q_PROPERTY(bool activateLanguageMenu READ activateLanguageMenu WRITE setActivateLanguageMenu)
public:
    explicit RichTextEditor(QWidget *parent = Q_NULLPTR);
    ~RichTextEditor();
    enum SupportFeature {
        None = 0,
        Search = 1,
        SpellChecking = 2,
        TextToSpeech = 4,
        AllowTab = 8
    };
    Q_DECLARE_FLAGS(SupportFeatures, SupportFeature)

    void setSearchSupport(bool b);
    bool searchSupport() const;

    bool spellCheckingSupport() const;
    void setSpellCheckingSupport(bool check);

    void setSpellCheckingConfigFileName(const QString &_fileName);

    bool checkSpellingEnabled() const;
    void setCheckSpellingEnabled(bool check);

    void setSpellCheckingLanguage(const QString &_language);
    const QString &spellCheckingLanguage() const;

    virtual void setReadOnly(bool readOnly);
    virtual void createHighlighter();

    bool textToSpeechSupport() const;
    void setTextToSpeechSupport(bool b);
    Sonnet::Highlighter *highlighter() const;

    bool activateLanguageMenu() const;
    void setActivateLanguageMenu(bool activate);

    void setAllowTabSupport(bool b);
    bool allowTabSupport() const;

    void setShowAutoCorrectButton(bool b);
    bool showAutoCorrectButton() const;

    void forceSpellChecking();
Q_SIGNALS:
    void say(const QString &text);

public Q_SLOTS:
    void slotDisplayMessageIndicator(const QString &message);
    void slotSpeakText();
    void slotCheckSpelling();

private Q_SLOTS:
    void slotUndoableClear();
    void slotSpellCheckerMisspelling(const QString &text, int pos);
    void slotSpellCheckerCorrected(const QString &, int, const QString &);
    void slotSpellCheckerAutoCorrect(const QString &, const QString &);
    void slotSpellCheckerCanceled();
    void slotSpellCheckerFinished();
    void slotToggleAutoSpellCheck();
    void slotLanguageSelected();
    void slotAllowTab();

protected:
    virtual void addExtraMenuEntry(QMenu *menu, const QPoint &pos);
    void contextMenuEvent(QContextMenuEvent *event) Q_DECL_OVERRIDE;
    void focusInEvent(QFocusEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *ev) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

    QMenu *mousePopupMenu(const QPoint &pos);
    virtual Sonnet::SpellCheckDecorator *createSpellCheckDecorator();
Q_SIGNALS:
    void findText();
    void replaceText();
    void spellCheckerAutoCorrect(const QString &currentWord, const QString &autoCorrectWord);
    void checkSpellingChanged(bool);
    void languageChanged(const QString &);
    void spellCheckStatus(const QString &);
    void spellCheckingFinished();
    void spellCheckingCanceled();

private:
    bool handleShortcut(const QKeyEvent *event);
    bool overrideShortcut(const QKeyEvent *event);
    void deleteWordBack();
    void deleteWordForward();
    void defaultPopupMenu(const QPoint &pos);
    void setHighlighter(Sonnet::Highlighter *_highLighter);
    void highlightWord(int length, int pos);
    void checkSpelling(bool force);
    class RichTextEditorPrivate;
    RichTextEditorPrivate *const d;
};
}

#endif // RICHTEXTEDITOR_H
