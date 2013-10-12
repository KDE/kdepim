/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

namespace Sonnet {
class Highlighter;
}
namespace PimCommon {
class PIMCOMMON_EXPORT PlainTextEditor : public QPlainTextEdit
{
    Q_OBJECT
    Q_PROPERTY(bool searchSupport READ searchSupport WRITE setSearchSupport)
public:
    explicit PlainTextEditor(QWidget *parent=0);
    ~PlainTextEditor();

    void setSearchSupport(bool b);
    bool searchSupport() const;

    bool checkSpellingEnabled() const;
    void setCheckSpellingEnabled( bool check );

    void setSpellCheckingLanguage(const QString &_language);
    const QString& spellCheckingLanguage() const;

    virtual void setReadOnly( bool readOnly );

private Q_SLOTS:
    void slotUndoableClear();
    void slotSpeakText();
    void slotCheckSpelling();
    void slotSpellCheckerMisspelling( const QString &text, int pos );
    void slotSpellCheckerCorrected( const QString &, int,const QString &);
    void slotSpellCheckerAutoCorrect(const QString&, const QString&);
    void slotSpellCheckerCanceled();
    void slotSpellCheckerFinished();

protected:
    virtual void addExtraMenuEntry(QMenu *menu);

protected:
    void contextMenuEvent( QContextMenuEvent *event );
    void wheelEvent( QWheelEvent *event );


Q_SIGNALS:
    void findText();
    void replaceText();
    void spellCheckerAutoCorrect(const QString& currentWord, const QString& autoCorrectWord);
    void checkSpellingChanged(bool);
    void languageChanged(const QString &);

private:
    void highlightWord( int length, int pos );
    class PlainTextEditorPrivate;
    PlainTextEditorPrivate *const d;
};
}
#endif // PLAINTEXTEDITOR_H
