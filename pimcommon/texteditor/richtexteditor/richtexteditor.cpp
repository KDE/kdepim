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

#include "richtexteditor.h"
#include "config-kdepim.h"

#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>
#include <KCursor>
#include <KConfigGroup>
#include <QIcon>
#include <KIconTheme>
#include <KConfig>

#include <sonnet/backgroundchecker.h>
#include <Sonnet/Dialog>
#include <Sonnet/Highlighter>
#include <sonnet/speller.h>

#include <QMenu>
#include <QContextMenuEvent>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTextCursor>
#include <QTextDocumentFragment>
#include <QScrollBar>
#include <QApplication>
#include <QClipboard>
#if KDEPIM_HAVE_TEXTTOSPEECH
#include <QtTextToSpeech/QTextToSpeech>
#endif

using namespace PimCommon;
class RichTextEditor::RichTextEditorPrivate
{
public:
    RichTextEditorPrivate()
        : highLighter(0),
          speller(0),
          hasSearchSupport(true),
          customPalette(false),
          hasSpellCheckingSupport(true)
    {
        KConfig sonnetKConfig(QLatin1String("sonnetrc"));
        KConfigGroup group(&sonnetKConfig, "Spelling");
        checkSpellingEnabled = group.readEntry("checkerEnabledByDefault", false);
    }
    ~RichTextEditorPrivate()
    {
        delete highLighter;
        delete speller;
    }

    QString spellCheckingConfigFileName;
    QString spellCheckingLanguage;
    QTextDocumentFragment originalDoc;
    Sonnet::Highlighter *highLighter;
    Sonnet::Speller* speller;
    bool hasSearchSupport;
    bool customPalette;
    bool checkSpellingEnabled;
    bool hasSpellCheckingSupport;
};


RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent),
      d(new RichTextEditorPrivate)
{
    setAcceptRichText(true);
    KCursor::setAutoHideCursor(this, true, false);
}

RichTextEditor::~RichTextEditor()
{
    delete d;
}

void RichTextEditor::defaultPopupMenu(const QPoint &pos)
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
            QAction *autoSpellCheckAction = popup->addAction( i18n( "Auto Spell Check" ), this, SLOT(slotToggleAutoSpellCheck()) );
            autoSpellCheckAction->setCheckable( true );
            autoSpellCheckAction->setChecked( checkSpellingEnabled() );
            popup->addAction(autoSpellCheckAction);

            if (checkSpellingEnabled()) {
                QMenu* languagesMenu = new QMenu(i18n("Spell Checking Language"), popup);
                QActionGroup* languagesGroup = new QActionGroup(languagesMenu);
                languagesGroup->setExclusive(true);
                if (!d->speller)
                    d->speller = new Sonnet::Speller();

                QMapIterator<QString, QString> i(d->speller->availableDictionaries());

                while (i.hasNext()) {
                    i.next();

                    QAction* languageAction = languagesMenu->addAction(i.key());
                    languageAction->setCheckable(true);
                    languageAction->setChecked(spellCheckingLanguage() == i.value() || (spellCheckingLanguage().isEmpty()
                                                                                        && d->speller->defaultLanguage() == i.value()));
                    languageAction->setData(i.value());
                    languageAction->setActionGroup(languagesGroup);
                    connect(languageAction, &QAction::triggered, this, &RichTextEditor::slotLanguageSelected);
                }
                popup->addMenu(languagesMenu);
            }
            popup->addSeparator();
        }
#if KDEPIM_HAVE_TEXTTOSPEECH
        QAction *speakAction = popup->addAction(i18n("Speak Text"));
        speakAction->setIcon(QIcon::fromTheme(QLatin1String("preferences-desktop-text-to-speech")));
        speakAction->setEnabled(!emptyDocument );
        connect(speakAction, &QAction::triggered, this, &RichTextEditor::slotSpeakText);
#endif
        addExtraMenuEntry(popup, pos);
        popup->exec( pos );

        delete popup;
    }
}

void RichTextEditor::slotSpeakText()
{
#if KDEPIM_HAVE_TEXTTOSPEECH
    //Port to QtSpeech
    QString text;
    if (textCursor().hasSelection())
        text = textCursor().selectedText();
    else
        text = toPlainText();
    //PORT ME ktts.asyncCall(QLatin1String("say"), text, 0);
#endif
}

void RichTextEditor::setSearchSupport(bool b)
{
    d->hasSearchSupport = b;
}

bool RichTextEditor::searchSupport() const
{
    return d->hasSearchSupport;
}

bool RichTextEditor::spellCheckingSupport() const
{
    return d->hasSpellCheckingSupport;
}

void RichTextEditor::setSpellCheckingSupport( bool check )
{
    d->hasSpellCheckingSupport = check;
}

void RichTextEditor::addExtraMenuEntry(QMenu *menu, const QPoint &pos)
{
    Q_UNUSED(menu);
    Q_UNUSED(pos);
}

void RichTextEditor::slotUndoableClear()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

void RichTextEditor::setReadOnly( bool readOnly )
{
    if ( !readOnly && hasFocus() && d->checkSpellingEnabled && !d->highLighter )
        createHighlighter();

    if ( readOnly == isReadOnly() )
        return;

    if ( readOnly ) {
        delete d->highLighter;
        d->highLighter = 0;

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

    QTextEdit::setReadOnly( readOnly );
}

void RichTextEditor::slotCheckSpelling()
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
    connect(spellDialog, &Sonnet::Dialog::replace, this, &RichTextEditor::slotSpellCheckerCorrected);
    connect(spellDialog, &Sonnet::Dialog::misspelling, this, &RichTextEditor::slotSpellCheckerMisspelling);
    connect(spellDialog, &Sonnet::Dialog::autoCorrect, this, &RichTextEditor::slotSpellCheckerAutoCorrect);
    connect(spellDialog, SIGNAL(done(QString)), this, SLOT(slotSpellCheckerFinished()));
    connect(spellDialog, &Sonnet::Dialog::cancel, this, &RichTextEditor::slotSpellCheckerCanceled);
    connect(spellDialog, &Sonnet::Dialog::spellCheckStatus, this, &RichTextEditor::spellCheckStatus);
    connect(spellDialog, &Sonnet::Dialog::languageChanged, this, &RichTextEditor::languageChanged);
    d->originalDoc = QTextDocumentFragment(document());
    spellDialog->setBuffer(toPlainText());
    spellDialog->show();
}

void RichTextEditor::slotSpellCheckerCanceled()
{
    QTextDocument *doc = document();
    doc->clear();
    QTextCursor cursor(doc);
    cursor.insertFragment(d->originalDoc);
    slotSpellCheckerFinished();
}

void RichTextEditor::slotSpellCheckerAutoCorrect(const QString& currentWord,const QString& autoCorrectWord)
{
    Q_EMIT spellCheckerAutoCorrect(currentWord, autoCorrectWord);
}

void RichTextEditor::slotSpellCheckerMisspelling( const QString &text, int pos )
{
    highlightWord( text.length(), pos );
}

void RichTextEditor::slotSpellCheckerCorrected( const QString& oldWord, int pos,const QString &newWord)
{
    if (oldWord != newWord ) {
        QTextCursor cursor(document());
        cursor.setPosition(pos);
        cursor.setPosition(pos+oldWord.length(),QTextCursor::KeepAnchor);
        cursor.insertText(newWord);
    }
}

void RichTextEditor::slotSpellCheckerFinished()
{
    QTextCursor cursor(document());
    cursor.clearSelection();
    setTextCursor(cursor);
    if (d->highLighter)
        d->highLighter->rehighlight();
}

void RichTextEditor::highlightWord( int length, int pos )
{
    QTextCursor cursor(document());
    cursor.setPosition(pos);
    cursor.setPosition(pos+length,QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    ensureCursorVisible();
}

void RichTextEditor::createHighlighter()
{
    setHighlighter(new Sonnet::Highlighter(this, d->spellCheckingConfigFileName));
}

void RichTextEditor::setHighlighter(Sonnet::Highlighter *_highLighter)
{
    delete d->highLighter;
    d->highLighter = _highLighter;
}

void RichTextEditor::focusInEvent( QFocusEvent *event )
{
    if ( d->checkSpellingEnabled && !isReadOnly() && !d->highLighter && d->hasSpellCheckingSupport)
        createHighlighter();

    QTextEdit::focusInEvent( event );
}

void RichTextEditor::setSpellCheckingConfigFileName(const QString &_fileName)
{
    d->spellCheckingConfigFileName = _fileName;
}

bool RichTextEditor::checkSpellingEnabled() const
{
    return d->checkSpellingEnabled;
}

void RichTextEditor::setCheckSpellingEnabled( bool check )
{
    if ( check == d->checkSpellingEnabled )
        return;
    Q_EMIT checkSpellingChanged( check );
    // From the above statment we know know that if we're turning checking
    // on that we need to create a new highlighter and if we're turning it
    // off we should remove the old one.

    d->checkSpellingEnabled = check;
    if ( check ) {
        if ( hasFocus() ) {
            createHighlighter();
            if (!d->spellCheckingLanguage.isEmpty())
                setSpellCheckingLanguage(spellCheckingLanguage());
        }
    } else {
        delete d->highLighter;
        d->highLighter = 0;
    }
}

const QString& RichTextEditor::spellCheckingLanguage() const
{
    return d->spellCheckingLanguage;
}

void RichTextEditor::setSpellCheckingLanguage(const QString &_language)
{
    if (d->highLighter) {
        d->highLighter->setCurrentLanguage(_language);
        d->highLighter->rehighlight();
    }

    if (_language != d->spellCheckingLanguage) {
        d->spellCheckingLanguage = _language;
        Q_EMIT languageChanged(_language);
    }
}

void RichTextEditor::slotToggleAutoSpellCheck()
{
    setCheckSpellingEnabled( !checkSpellingEnabled() );
}

void RichTextEditor::slotLanguageSelected()
{
    QAction* languageAction = static_cast<QAction*>(QObject::sender());
    setSpellCheckingLanguage(languageAction->data().toString());
}

void RichTextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    // Obtain the cursor at the mouse position and the current cursor
    QTextCursor cursorAtMouse = cursorForPosition(event->pos());
    const int mousePos = cursorAtMouse.position();
    QTextCursor cursor = textCursor();

    // Check if the user clicked a selected word
    const bool selectedWordClicked = cursor.hasSelection() &&
            mousePos >= cursor.selectionStart() &&
            mousePos <= cursor.selectionEnd();

    // Get the word under the (mouse-)cursor and see if it is misspelled.
    // Don't include apostrophes at the start/end of the word in the selection.
    QTextCursor wordSelectCursor(cursorAtMouse);
    wordSelectCursor.clearSelection();
    wordSelectCursor.select(QTextCursor::WordUnderCursor);
    QString selectedWord = wordSelectCursor.selectedText();

    bool isMouseCursorInsideWord = true;
    if ((mousePos < wordSelectCursor.selectionStart() ||
         mousePos >= wordSelectCursor.selectionEnd())
            && (selectedWord.length() > 1)) {
        isMouseCursorInsideWord = false;
    }

    // Clear the selection again, we re-select it below (without the apostrophes).
    wordSelectCursor.setPosition(wordSelectCursor.position()-selectedWord.size());
    if (selectedWord.startsWith(QLatin1Char('\'')) || selectedWord.startsWith(QLatin1Char('\"'))) {
        selectedWord = selectedWord.right(selectedWord.size() - 1);
        wordSelectCursor.movePosition(QTextCursor::NextCharacter, QTextCursor::MoveAnchor);
    }
    if (selectedWord.endsWith(QLatin1Char('\'')) || selectedWord.endsWith(QLatin1Char('\"')))
        selectedWord.chop(1);

    wordSelectCursor.movePosition(QTextCursor::NextCharacter,
                                  QTextCursor::KeepAnchor, selectedWord.size());

    const bool wordIsMisspelled = isMouseCursorInsideWord &&
            checkSpellingEnabled() &&
            !selectedWord.isEmpty() &&
            d->highLighter &&
            d->highLighter->isWordMisspelled(selectedWord);

    // If the user clicked a selected word, do nothing.
    // If the user clicked somewhere else, move the cursor there.
    // If the user clicked on a misspelled word, select that word.
    // Same behavior as in OpenOffice Writer.
    if (!selectedWordClicked) {
        if (wordIsMisspelled)
            setTextCursor(wordSelectCursor);
        else
            setTextCursor(cursorAtMouse);
        cursor = textCursor();
    }

    // Use standard context menu for already selected words, correctly spelled
    // words and words inside quotes.
    if (!wordIsMisspelled || selectedWordClicked) {
        defaultPopupMenu(event->globalPos());
    } else {
        QMenu menu; 

        //Add the suggestions to the menu
        const QStringList reps = d->highLighter->suggestionsForWord(selectedWord);
        if (reps.isEmpty()) {
            QAction *suggestionsAction = menu.addAction(i18n("No suggestions for %1", selectedWord));
            suggestionsAction->setEnabled(false);
        } else {
            QStringList::const_iterator end(reps.constEnd());
            for (QStringList::const_iterator it = reps.constBegin(); it != end; ++it) {
                menu.addAction(*it);
            }
        }

        menu.addSeparator();

        QAction *ignoreAction = menu.addAction(i18n("Ignore"));
        QAction *addToDictAction = menu.addAction(i18n("Add to Dictionary"));
        //Execute the popup inline
        const QAction *selectedAction = menu.exec(event->globalPos());

        if (selectedAction) {
            Q_ASSERT(cursor.selectedText() == selectedWord);

            if (selectedAction == ignoreAction) {
                d->highLighter->ignoreWord(selectedWord);
                d->highLighter->rehighlight();
            } else if (selectedAction == addToDictAction) {
                d->highLighter->addWordToDictionary(selectedWord);
                d->highLighter->rehighlight();
            }

            // Other actions can only be one of the suggested words
            else {
                const QString replacement = selectedAction->text();
                Q_ASSERT(reps.contains(replacement));
                cursor.insertText(replacement);
                setTextCursor(cursor);
            }
        }
    }
}

static void deleteWord(QTextCursor cursor, QTextCursor::MoveOperation op)
{
    cursor.clearSelection();
    cursor.movePosition( op, QTextCursor::KeepAnchor );
    cursor.removeSelectedText();
}

void RichTextEditor::deleteWordBack()
{
    deleteWord(textCursor(), QTextCursor::PreviousWord);
}

void RichTextEditor::deleteWordForward()
{
    deleteWord(textCursor(), QTextCursor::WordRight);
}

bool RichTextEditor::event(QEvent* ev)
{
    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>( ev );
        if (overrideShortcut(e)) {
            e->accept();
            return true;
        }
    }
    return QTextEdit::event(ev);
}

bool RichTextEditor::handleShortcut(const QKeyEvent* event)
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


bool RichTextEditor::overrideShortcut(const QKeyEvent* event)
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
    } else if (d->hasSearchSupport && KStandardShortcut::findNext().contains(key)) {
        return true;
    } else if (d->hasSearchSupport && KStandardShortcut::replace().contains(key)) {
        return true;
    } else if (event->matches(QKeySequence::SelectAll)) { // currently missing in QTextEdit
        return true;
    }
    return false;
}

void RichTextEditor::keyPressEvent( QKeyEvent *event )
{
    if (handleShortcut(event)) {
        event->accept();
    } else {
        QTextEdit::keyPressEvent(event);
    }
}

