/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "plaintexteditor.h"

#include <KLocalizedString>
#include <KIconTheme>
#include <KStandardGuiItem>
#include <KMessageBox>
#include <KStandardAction>
#include <KCursor>
#include <QIcon>

#include <sonnet/backgroundchecker.h>
#include <Sonnet/Dialog>
#include "pimcommon/texttospeech/texttospeech.h"

#include <QMenu>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTextDocumentFragment>
#include <QShortcut>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>


using namespace PimCommon;

class PlainTextEditor::PlainTextEditorPrivate
{
public:
    PlainTextEditorPrivate()
        : hasSearchSupport(true),
          customPalette(false),
          hasSpellCheckingSupport(true)
    {
    }
    ~PlainTextEditorPrivate()
    {
    }

    QString spellCheckingLanguage;
    QTextDocumentFragment originalDoc;
    bool hasSearchSupport;
    bool customPalette;
    bool hasSpellCheckingSupport;
};

PlainTextEditor::PlainTextEditor(QWidget *parent)
    : QPlainTextEdit(parent),
      d(new PlainTextEditor::PlainTextEditorPrivate)
{
    KCursor::setAutoHideCursor(this, true, false);
}

PlainTextEditor::~PlainTextEditor()
{
    delete d;
}

void PlainTextEditor::contextMenuEvent( QContextMenuEvent *event )
{
    QMenu *popup = createStandardContextMenu();
    if (popup) {
        const bool emptyDocument = document()->isEmpty();
        if (!isReadOnly()) {
            QList<QAction *> actionList = popup->actions();
            enum { UndoAct, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };
            QAction *separatorAction = 0L;
            const int idx = actionList.indexOf( actionList[SelectAllAct] ) + 1;
            if ( idx < actionList.count() )
                separatorAction = actionList.at( idx );
            if ( separatorAction ) {
                QAction *clearAllAction = KStandardAction::clear(this, SLOT(slotUndoableClear()), popup);
                if ( emptyDocument )
                    clearAllAction->setEnabled( false );
                popup->insertAction( separatorAction, clearAllAction );
            }
        }
        KIconTheme::assignIconsToContextMenu( isReadOnly() ? KIconTheme::ReadOnlyText
                                                           : KIconTheme::TextEditor,
                                              popup->actions() );
        if (d->hasSearchSupport) {
            popup->addSeparator();
            QAction *findAct = popup->addAction( KStandardGuiItem::find().icon(), KStandardGuiItem::find().text(),this, SIGNAL(findText()), Qt::Key_F+Qt::CTRL);
            if ( emptyDocument )
                findAct->setEnabled(false);
            popup->addSeparator();
            if (!isReadOnly()) {
                QAction *act = popup->addAction(i18n("Replace..."),this, SIGNAL(replaceText()), Qt::Key_R+Qt::CTRL);
                if ( emptyDocument )
                    act->setEnabled( false );
                popup->addSeparator();
            }
        } else {
            popup->addSeparator();
        }

        if( !isReadOnly() && d->hasSpellCheckingSupport) {
            QAction *spellCheckAction = popup->addAction( QIcon::fromTheme( QLatin1String("tools-check-spelling") ), i18n( "Check Spelling..." ), this, SLOT(slotCheckSpelling()) );
            if (emptyDocument)
                spellCheckAction->setEnabled(false);
            popup->addSeparator();
        }
        if (PimCommon::TextToSpeech::self()->isReady()) {
            QAction *speakAction = popup->addAction(i18n("Speak Text"));
            speakAction->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-text-to-speech")));
            speakAction->setEnabled(!emptyDocument );
            connect(speakAction, &QAction::triggered, this, &PlainTextEditor::slotSpeakText);
        }
        addExtraMenuEntry(popup, event->pos());
        popup->exec( event->globalPos() );

        delete popup;
    }
}

void PlainTextEditor::addExtraMenuEntry(QMenu *menu, const QPoint &pos)
{
    Q_UNUSED(menu);
    Q_UNUSED(pos);
}

void PlainTextEditor::slotSpeakText()
{
    QString text;
    if (textCursor().hasSelection())
        text = textCursor().selectedText();
    else
        text = toPlainText();
    Q_EMIT say(text);
}

void PlainTextEditor::slotUndoableClear()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

void PlainTextEditor::setSearchSupport(bool b)
{
    d->hasSearchSupport = b;
}

bool PlainTextEditor::searchSupport() const
{
    return d->hasSearchSupport;
}

bool PlainTextEditor::spellCheckingSupport() const
{
    return d->hasSpellCheckingSupport;
}

void PlainTextEditor::setSpellCheckingSupport( bool check )
{
    d->hasSpellCheckingSupport = check;
}

void PlainTextEditor::setReadOnly( bool readOnly )
{
    if ( readOnly == isReadOnly() )
        return;

    if ( readOnly ) {
        d->customPalette = testAttribute( Qt::WA_SetPalette );
        QPalette p = palette();
        QColor color = p.color( QPalette::Disabled, QPalette::Background );
        p.setColor( QPalette::Base, color );
        p.setColor( QPalette::Background, color );
        setPalette( p );
    } else {
        if ( d->customPalette && testAttribute( Qt::WA_SetPalette ) ) {
            QPalette p = palette();
            QColor color = p.color( QPalette::Normal, QPalette::Base );
            p.setColor( QPalette::Base, color );
            p.setColor( QPalette::Background, color );
            setPalette( p );
        } else
            setPalette( QPalette() );
    }

    QPlainTextEdit::setReadOnly( readOnly );
}


void PlainTextEditor::slotCheckSpelling()
{
    if(document()->isEmpty()) {
        KMessageBox::information(this, i18n("Nothing to spell check."));
        return;
    }
    Sonnet::BackgroundChecker *backgroundSpellCheck = new Sonnet::BackgroundChecker;
    if(!d->spellCheckingLanguage.isEmpty())
        backgroundSpellCheck->changeLanguage(d->spellCheckingLanguage);
    Sonnet::Dialog *spellDialog = new Sonnet::Dialog(backgroundSpellCheck, 0);
    backgroundSpellCheck->setParent(spellDialog);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    connect(spellDialog, &Sonnet::Dialog::replace, this, &PlainTextEditor::slotSpellCheckerCorrected);
    connect(spellDialog, &Sonnet::Dialog::misspelling, this, &PlainTextEditor::slotSpellCheckerMisspelling);
    connect(spellDialog, &Sonnet::Dialog::autoCorrect, this, &PlainTextEditor::slotSpellCheckerAutoCorrect);
    connect(spellDialog, SIGNAL(done(QString)),
            this, SLOT(slotSpellCheckerFinished()));

    connect(spellDialog, &Sonnet::Dialog::cancel, this, &PlainTextEditor::slotSpellCheckerCanceled);
    connect(spellDialog, &Sonnet::Dialog::spellCheckStatus, this, &PlainTextEditor::spellCheckStatus);
    connect(spellDialog, &Sonnet::Dialog::languageChanged, this, &PlainTextEditor::languageChanged);
    d->originalDoc = QTextDocumentFragment(document());
    spellDialog->setBuffer(toPlainText());
    spellDialog->show();
}

void PlainTextEditor::slotSpellCheckerCanceled()
{
    QTextDocument *doc = document();
    doc->clear();
    QTextCursor cursor(doc);
    cursor.insertFragment(d->originalDoc);
    slotSpellCheckerFinished();
}

void PlainTextEditor::slotSpellCheckerAutoCorrect(const QString& currentWord,const QString& autoCorrectWord)
{
    emit spellCheckerAutoCorrect(currentWord, autoCorrectWord);
}

void PlainTextEditor::slotSpellCheckerMisspelling( const QString &text, int pos )
{
    highlightWord( text.length(), pos );
}

void PlainTextEditor::slotSpellCheckerCorrected( const QString& oldWord, int pos,const QString &newWord)
{
    if (oldWord != newWord ) {
        QTextCursor cursor(document());
        cursor.setPosition(pos);
        cursor.setPosition(pos+oldWord.length(),QTextCursor::KeepAnchor);
        cursor.insertText(newWord);
    }
}

void PlainTextEditor::slotSpellCheckerFinished()
{
    QTextCursor cursor(document());
    cursor.clearSelection();
    setTextCursor(cursor);
}

void PlainTextEditor::highlightWord( int length, int pos )
{
    QTextCursor cursor(document());
    cursor.setPosition(pos);
    cursor.setPosition(pos+length,QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    ensureCursorVisible();
}

static void deleteWord(QTextCursor cursor, QTextCursor::MoveOperation op)
{
    cursor.clearSelection();
    cursor.movePosition( op, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
}

void PlainTextEditor::deleteWordBack()
{
    deleteWord(textCursor(), QTextCursor::PreviousWord);
}

void PlainTextEditor::deleteWordForward()
{
    deleteWord(textCursor(), QTextCursor::WordRight);
}

bool PlainTextEditor::event(QEvent* ev)
{
    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>( ev );
        if (overrideShortcut(e)) {
            e->accept();
            return true;
        }
    }
    return QPlainTextEdit::event(ev);
}

bool PlainTextEditor::overrideShortcut(const QKeyEvent* event)
{
    const int key = event->key() | event->modifiers();
    if ( KStandardShortcut::copy().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::paste().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::cut().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::undo().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::redo().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::deleteWordBack().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::deleteWordForward().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::backwardWord().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::forwardWord().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::next().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::prior().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::begin().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::end().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::beginningOfLine().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::endOfLine().contains( key ) ) {
        return true;
    } else if ( KStandardShortcut::pasteSelection().contains( key ) ) {
        return true;
    } else if (d->hasSearchSupport && KStandardShortcut::find().contains(key)) {
        return true;
    } else if (d->hasSearchSupport && KStandardShortcut::replace().contains(key)) {
        return true;
    } else if (event->matches(QKeySequence::SelectAll)) { // currently missing in QTextEdit
        return true;
    }
    return false;
}

bool PlainTextEditor::handleShortcut(const QKeyEvent* event)
{
    const int key = event->key() | event->modifiers();

    if ( KStandardShortcut::copy().contains( key ) ) {
        copy();
        return true;
    } else if ( KStandardShortcut::paste().contains( key ) ) {
        paste();
        return true;
    } else if ( KStandardShortcut::cut().contains( key ) ) {
        cut();
        return true;
    } else if ( KStandardShortcut::undo().contains( key ) ) {
        if (!isReadOnly())
            undo();
        return true;
    } else if ( KStandardShortcut::redo().contains( key ) ) {
        if (!isReadOnly())
            redo();
        return true;
    } else if ( KStandardShortcut::deleteWordBack().contains( key ) ) {
        if (!isReadOnly())
            deleteWordBack();
        return true;
    } else if ( KStandardShortcut::deleteWordForward().contains( key ) ) {
        if (!isReadOnly())
            deleteWordForward();
        return true;
    } else if ( KStandardShortcut::backwardWord().contains( key ) ) {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::PreviousWord );
        setTextCursor( cursor );
        return true;
    } else if ( KStandardShortcut::forwardWord().contains( key ) ) {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::NextWord );
        setTextCursor( cursor );
        return true;
    } else if ( KStandardShortcut::next().contains( key ) ) {
        QTextCursor cursor = textCursor();
        bool moved = false;
        qreal lastY = cursorRect(cursor).bottom();
        qreal distance = 0;
        do {
            qreal y = cursorRect(cursor).bottom();
            distance += qAbs(y - lastY);
            lastY = y;
            moved = cursor.movePosition(QTextCursor::Down);
        } while (moved && distance < viewport()->height());

        if (moved) {
            cursor.movePosition(QTextCursor::Up);
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepAdd);
        }
        setTextCursor(cursor);
        return true;
    } else if ( KStandardShortcut::prior().contains( key ) ) {
        QTextCursor cursor = textCursor();
        bool moved = false;
        qreal lastY = cursorRect(cursor).bottom();
        qreal distance = 0;
        do {
            qreal y = cursorRect(cursor).bottom();
            distance += qAbs(y - lastY);
            lastY = y;
            moved = cursor.movePosition(QTextCursor::Up);
        } while (moved && distance < viewport()->height());

        if (moved) {
            cursor.movePosition(QTextCursor::Down);
            verticalScrollBar()->triggerAction(QAbstractSlider::SliderPageStepSub);
        }
        setTextCursor(cursor);
        return true;
    } else if ( KStandardShortcut::begin().contains( key ) ) {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::Start );
        setTextCursor( cursor );
        return true;
    } else if ( KStandardShortcut::end().contains( key ) ) {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::End );
        setTextCursor( cursor );
        return true;
    } else if ( KStandardShortcut::beginningOfLine().contains( key ) ) {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::StartOfLine );
        setTextCursor( cursor );
        return true;
    } else if ( KStandardShortcut::endOfLine().contains( key ) ) {
        QTextCursor cursor = textCursor();
        cursor.movePosition( QTextCursor::EndOfLine );
        setTextCursor( cursor );
        return true;
    } else if (d->hasSearchSupport && KStandardShortcut::find().contains(key)) {
        Q_EMIT findText();
        return true;
    } else if (d->hasSearchSupport && KStandardShortcut::replace().contains(key)) {
        if (!isReadOnly())
            Q_EMIT replaceText();
        return true;
    } else if ( KStandardShortcut::pasteSelection().contains( key ) ) {
        QString text = QApplication::clipboard()->text( QClipboard::Selection );
        if ( !text.isEmpty() )
            insertPlainText( text );  // TODO: check if this is html? (MiB)
        return true;
    }
    return false;
}


void PlainTextEditor::keyPressEvent( QKeyEvent *event )
{
    if (handleShortcut(event)) {
        event->accept();
    } else {
        QPlainTextEdit::keyPressEvent(event);
    }
}

