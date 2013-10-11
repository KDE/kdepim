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

#include "richtexteditor.h"

#include <KLocale>
#include <KMessageBox>
#include <KToolInvocation>
#include <KAction>
#include <KStandardAction>
#include <KGlobalSettings>
#include <KCursor>

#include <sonnet/backgroundchecker.h>
#include <Sonnet/Dialog>
#include <Sonnet/Highlighter>

#include <QMenu>
#include <QContextMenuEvent>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTextCursor>
#include <QTextDocumentFragment>

using namespace PimCommon;
class RichTextEditor::RichTextEditorPrivate
{
public:
    RichTextEditorPrivate()
        : highLighter(0),
          hasSearchSupport(true),
          customPalette(false)
    {
        KConfig sonnetKConfig(QLatin1String("sonnetrc"));
        KConfigGroup group(&sonnetKConfig, "Spelling");
        checkSpellingEnabled = group.readEntry("checkerEnabledByDefault", false);
    }
    ~RichTextEditorPrivate()
    {
        delete highLighter;
    }

    QString spellCheckingConfigFileName;
    QString spellCheckingLanguage;
    QTextDocumentFragment originalDoc;
    Sonnet::Highlighter *highLighter;
    bool hasSearchSupport;
    bool customPalette;
    bool checkSpellingEnabled;
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

void RichTextEditor::contextMenuEvent( QContextMenuEvent *event )
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
                KAction *clearAllAction = KStandardAction::clear(this, SLOT(slotUndoableClear()), popup);
                if ( emptyDocument )
                    clearAllAction->setEnabled( false );
                popup->insertAction( separatorAction, clearAllAction );
            }
        }
        //Code from KTextBrowser
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

        if( !isReadOnly() ) {
            QAction *spellCheckAction = popup->addAction( KIcon( QLatin1String("tools-check-spelling") ), i18n( "Check Spelling..." ), this, SLOT(slotCheckSpelling()) );
            if (emptyDocument)
                spellCheckAction->setEnabled(false);
            popup->addSeparator();
            QAction *autoSpellCheckAction = popup->addAction( i18n( "Auto Spell Check" ), this, SLOT(slotToggleAutoSpellCheck()) );
            autoSpellCheckAction->setCheckable( true );
            autoSpellCheckAction->setChecked( checkSpellingEnabled() );
            popup->addAction(autoSpellCheckAction);
            popup->addSeparator();
        }

        QAction *speakAction = popup->addAction(i18n("Speak Text"));
        speakAction->setIcon(KIcon(QLatin1String("preferences-desktop-text-to-speech")));
        speakAction->setEnabled(!emptyDocument );
        connect( speakAction, SIGNAL(triggered(bool)), this, SLOT(slotSpeakText()) );
        addExtraMenuEntry(popup);
        popup->exec( event->globalPos() );

        delete popup;
    }
}

void RichTextEditor::slotSpeakText()
{
    // If KTTSD not running, start it.
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.kttsd"))) {
        QString error;
        if (KToolInvocation::startServiceByDesktopName(QLatin1String("kttsd"), QStringList(), &error)) {
            KMessageBox::error(this, i18n( "Starting Jovie Text-to-Speech Service Failed"), error );
            return;
        }
    }
    QDBusInterface ktts(QLatin1String("org.kde.kttsd"), QLatin1String("/KSpeech"), QLatin1String("org.kde.KSpeech"));
    QString text;
    if (textCursor().hasSelection())
        text = textCursor().selectedText();
    else
        text = toPlainText();
    ktts.asyncCall(QLatin1String("say"), text, 0);
}

void RichTextEditor::setSearchSupport(bool b)
{
    d->hasSearchSupport = b;
}

bool RichTextEditor::searchSupport() const
{
    return d->hasSearchSupport;
}

void RichTextEditor::addExtraMenuEntry(QMenu *menu)
{
    Q_UNUSED(menu);
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

void RichTextEditor::wheelEvent( QWheelEvent *event )
{
    if ( KGlobalSettings::wheelMouseZooms() )
        QTextEdit::wheelEvent( event );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        QAbstractScrollArea::wheelEvent( event );
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
    connect(spellDialog, SIGNAL(replace(QString,int,QString)),
            this, SLOT(slotSpellCheckerCorrected(QString,int,QString)));
    connect(spellDialog, SIGNAL(misspelling(QString,int)),
            this, SLOT(slotSpellCheckerMisspelling(QString,int)));
    connect(spellDialog, SIGNAL(autoCorrect(QString,QString)),
            this, SLOT(slotSpellCheckerAutoCorrect(QString,QString)));
    connect(spellDialog, SIGNAL(done(QString)),
            this, SLOT(slotSpellCheckerFinished()));
    connect(spellDialog, SIGNAL(cancel()),
            this, SLOT(slotSpellCheckerCanceled()));
    connect(spellDialog, SIGNAL(spellCheckStatus(QString)),
            this, SIGNAL(slotSpellCheckStatus(QString)));
    connect(spellDialog, SIGNAL(languageChanged(QString)),
            this, SIGNAL(languageChanged(QString)));
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
    emit spellCheckerAutoCorrect(currentWord, autoCorrectWord);
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
    if ( d->checkSpellingEnabled && !isReadOnly() && !d->highLighter )
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
    emit checkSpellingChanged( check );
    if ( check == d->checkSpellingEnabled )
        return;

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
        emit languageChanged(_language);
    }
}

void RichTextEditor::slotToggleAutoSpellCheck()
{
    setCheckSpellingEnabled( !checkSpellingEnabled() );
}


#include "richtexteditor.moc"
