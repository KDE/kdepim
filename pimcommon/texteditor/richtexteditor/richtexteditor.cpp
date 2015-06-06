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

#include "richtexteditor.h"
#include "texteditor/commonwidget/textmessageindicator.h"
#include "texteditor/richtexteditor/richtextdecorator.h"

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
#include <pimcommon/texttospeech/texttospeech.h>

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

using namespace PimCommon;
class RichTextEditor::RichTextEditorPrivate
{
public:
    RichTextEditorPrivate(RichTextEditor *qq)
        : q(qq),
          textIndicator(new PimCommon::TextMessageIndicator(q)),
          richTextDecorator(Q_NULLPTR),
          speller(Q_NULLPTR),
          customPalette(false),
          activateLanguageMenu(true),
          showAutoCorrectionButton(false)
    {
        KConfig sonnetKConfig(QStringLiteral("sonnetrc"));
        KConfigGroup group(&sonnetKConfig, "Spelling");
        checkSpellingEnabled = group.readEntry("checkerEnabledByDefault", false);
        supportFeatures |= RichTextEditor::Search;
        supportFeatures |= RichTextEditor::SpellChecking;
        supportFeatures |= RichTextEditor::TextToSpeech;
        supportFeatures |= RichTextEditor::AllowTab;
    }
    ~RichTextEditorPrivate()
    {
        delete richTextDecorator;
        delete speller;
    }

    RichTextEditor *q;
    PimCommon::TextMessageIndicator *textIndicator;
    QString spellCheckingConfigFileName;
    QString spellCheckingLanguage;
    QTextDocumentFragment originalDoc;
    Sonnet::SpellCheckDecorator *richTextDecorator;
    Sonnet::Speller *speller;
    RichTextEditor::SupportFeatures supportFeatures;
    bool customPalette;
    bool checkSpellingEnabled;
    bool activateLanguageMenu;
    bool showAutoCorrectionButton;
};

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent),
      d(new RichTextEditorPrivate(this))
{
    setAcceptRichText(true);
    KCursor::setAutoHideCursor(this, true, false);
}

RichTextEditor::~RichTextEditor()
{
    delete d;
}

void RichTextEditor::slotDisplayMessageIndicator(const QString &message)
{
    d->textIndicator->display(message);
}

Sonnet::Highlighter *RichTextEditor::highlighter() const
{
    if (d->richTextDecorator) {
        return d->richTextDecorator->highlighter();
    } else {
        return 0;
    }
}

bool RichTextEditor::activateLanguageMenu() const
{
    return d->activateLanguageMenu;
}

void RichTextEditor::setActivateLanguageMenu(bool activate)
{
    d->activateLanguageMenu = activate;
}

void RichTextEditor::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *popup = mousePopupMenu(event->pos());
    if (popup) {
        popup->exec(event->globalPos());
        delete popup;
    }
}

QMenu *RichTextEditor::mousePopupMenu(const QPoint &pos)
{
    QMenu *popup = createStandardContextMenu();
    if (popup) {
        const bool emptyDocument = document()->isEmpty();
        if (!isReadOnly()) {
            QList<QAction *> actionList = popup->actions();
            enum { UndoAct, RedoAct, CutAct, CopyAct, PasteAct, ClearAct, SelectAllAct, NCountActs };
            QAction *separatorAction = Q_NULLPTR;
            const int idx = actionList.indexOf(actionList[SelectAllAct]) + 1;
            if (idx < actionList.count()) {
                separatorAction = actionList.at(idx);
            }
            if (separatorAction) {
                QAction *clearAllAction = KStandardAction::clear(this, SLOT(slotUndoableClear()), popup);
                if (emptyDocument) {
                    clearAllAction->setEnabled(false);
                }
                popup->insertAction(separatorAction, clearAllAction);
            }
        }
        KIconTheme::assignIconsToContextMenu(isReadOnly() ? KIconTheme::ReadOnlyText
                                             : KIconTheme::TextEditor,
                                             popup->actions());
        if (searchSupport()) {
            popup->addSeparator();
            QAction *findAct = popup->addAction(KStandardGuiItem::find().icon(), KStandardGuiItem::find().text(), this, SIGNAL(findText()), Qt::Key_F + Qt::CTRL);
            if (emptyDocument) {
                findAct->setEnabled(false);
            }
            popup->addSeparator();
            if (!isReadOnly()) {
                QAction *act = popup->addAction(i18n("Replace..."), this, SIGNAL(replaceText()), Qt::Key_R + Qt::CTRL);
                if (emptyDocument) {
                    act->setEnabled(false);
                }
                popup->addSeparator();
            }
        } else {
            popup->addSeparator();
        }

        if (!isReadOnly() && spellCheckingSupport()) {
            QAction *spellCheckAction = popup->addAction(QIcon::fromTheme(QStringLiteral("tools-check-spelling")), i18n("Check Spelling..."), this, SLOT(slotCheckSpelling()));
            if (emptyDocument) {
                spellCheckAction->setEnabled(false);
            }
            popup->addSeparator();
            QAction *autoSpellCheckAction = popup->addAction(i18n("Auto Spell Check"), this, SLOT(slotToggleAutoSpellCheck()));
            autoSpellCheckAction->setCheckable(true);
            autoSpellCheckAction->setChecked(checkSpellingEnabled());
            popup->addAction(autoSpellCheckAction);

            if (checkSpellingEnabled() &&  d->activateLanguageMenu) {
                QMenu *languagesMenu = new QMenu(i18n("Spell Checking Language"), popup);
                QActionGroup *languagesGroup = new QActionGroup(languagesMenu);
                languagesGroup->setExclusive(true);
                if (!d->speller) {
                    d->speller = new Sonnet::Speller();
                }

                QMapIterator<QString, QString> i(d->speller->availableDictionaries());

                while (i.hasNext()) {
                    i.next();

                    QAction *languageAction = languagesMenu->addAction(i.key());
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

        if (allowTabSupport()) {
            QAction *allowTabAction = popup->addAction(i18n("Allow Tabulations"));
            allowTabAction->setCheckable(true);
            allowTabAction->setChecked(!tabChangesFocus());
            connect(allowTabAction, &QAction::triggered, this, &RichTextEditor::slotAllowTab);
        }

        if (PimCommon::TextToSpeech::self()->isReady()) {
            QAction *speakAction = popup->addAction(i18n("Speak Text"));
            speakAction->setIcon(QIcon::fromTheme(QStringLiteral("preferences-desktop-text-to-speech")));
            speakAction->setEnabled(!emptyDocument);
            connect(speakAction, &QAction::triggered, this, &RichTextEditor::slotSpeakText);
        }
        addExtraMenuEntry(popup, pos);
        return popup;
    }
    return Q_NULLPTR;
}

void RichTextEditor::slotSpeakText()
{
    QString text;
    if (textCursor().hasSelection()) {
        text = textCursor().selectedText();
    } else {
        text = toPlainText();
    }
    Q_EMIT say(text);
}

void RichTextEditor::setSearchSupport(bool b)
{
    if (b) {
        d->supportFeatures |= Search;
    } else {
        d->supportFeatures = (d->supportFeatures & ~ Search);
    }
}

bool RichTextEditor::searchSupport() const
{
    return (d->supportFeatures & Search);
}

void RichTextEditor::setAllowTabSupport(bool b)
{
    if (b) {
        d->supportFeatures |= AllowTab;
    } else {
        d->supportFeatures = (d->supportFeatures & ~ AllowTab);
    }
}

bool RichTextEditor::allowTabSupport() const
{
    return (d->supportFeatures & AllowTab);
}

void RichTextEditor::setShowAutoCorrectButton(bool b)
{
    d->showAutoCorrectionButton = b;
}

bool RichTextEditor::showAutoCorrectButton() const
{
    return d->showAutoCorrectionButton;
}


bool RichTextEditor::spellCheckingSupport() const
{
    return (d->supportFeatures & SpellChecking);
}

void RichTextEditor::setSpellCheckingSupport(bool check)
{
    if (check) {
        d->supportFeatures |= SpellChecking;
    } else {
        d->supportFeatures = (d->supportFeatures & ~ SpellChecking);
    }
}

void RichTextEditor::setTextToSpeechSupport(bool b)
{
    if (b) {
        d->supportFeatures |= TextToSpeech;
    } else {
        d->supportFeatures = (d->supportFeatures & ~ TextToSpeech);
    }
}

bool RichTextEditor::textToSpeechSupport() const
{
    return (d->supportFeatures & TextToSpeech);
}

void RichTextEditor::slotAllowTab()
{
    setTabChangesFocus(!tabChangesFocus());
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

void RichTextEditor::setReadOnly(bool readOnly)
{
    if (!readOnly && hasFocus() && d->checkSpellingEnabled && !d->richTextDecorator) {
        createHighlighter();
    }

    if (readOnly == isReadOnly()) {
        return;
    }

    if (readOnly) {
        delete d->richTextDecorator;
        d->richTextDecorator = Q_NULLPTR;

        d->customPalette = testAttribute(Qt::WA_SetPalette);
        QPalette p = palette();
        QColor color = p.color(QPalette::Disabled, QPalette::Background);
        p.setColor(QPalette::Base, color);
        p.setColor(QPalette::Background, color);
        setPalette(p);
    } else {
        if (d->customPalette && testAttribute(Qt::WA_SetPalette)) {
            QPalette p = palette();
            QColor color = p.color(QPalette::Normal, QPalette::Base);
            p.setColor(QPalette::Base, color);
            p.setColor(QPalette::Background, color);
            setPalette(p);
        } else {
            setPalette(QPalette());
        }
    }

    QTextEdit::setReadOnly(readOnly);
}

void RichTextEditor::slotCheckSpelling()
{
    if (document()->isEmpty()) {
        KMessageBox::information(this, i18n("Nothing to spell check."));
        return;
    }
    Sonnet::BackgroundChecker *backgroundSpellCheck = new Sonnet::BackgroundChecker;
    if (!d->spellCheckingLanguage.isEmpty()) {
        backgroundSpellCheck->changeLanguage(d->spellCheckingLanguage);
    }
    Sonnet::Dialog *spellDialog = new Sonnet::Dialog(backgroundSpellCheck, Q_NULLPTR);
    backgroundSpellCheck->setParent(spellDialog);
    spellDialog->setAttribute(Qt::WA_DeleteOnClose, true);
    spellDialog->activeAutoCorrect(d->showAutoCorrectionButton);
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

void RichTextEditor::slotSpellCheckerAutoCorrect(const QString &currentWord, const QString &autoCorrectWord)
{
    Q_EMIT spellCheckerAutoCorrect(currentWord, autoCorrectWord);
}

void RichTextEditor::slotSpellCheckerMisspelling(const QString &text, int pos)
{
    highlightWord(text.length(), pos);
}

void RichTextEditor::slotSpellCheckerCorrected(const QString &oldWord, int pos, const QString &newWord)
{
    if (oldWord != newWord) {
        QTextCursor cursor(document());
        cursor.setPosition(pos);
        cursor.setPosition(pos + oldWord.length(), QTextCursor::KeepAnchor);
        cursor.insertText(newWord);
    }
}

void RichTextEditor::slotSpellCheckerFinished()
{
    QTextCursor cursor(document());
    cursor.clearSelection();
    setTextCursor(cursor);
    if (highlighter()) {
        highlighter()->rehighlight();
    }
}

void RichTextEditor::highlightWord(int length, int pos)
{
    QTextCursor cursor(document());
    cursor.setPosition(pos);
    cursor.setPosition(pos + length, QTextCursor::KeepAnchor);
    setTextCursor(cursor);
    ensureCursorVisible();
}

void RichTextEditor::createHighlighter()
{
    setHighlighter(new Sonnet::Highlighter(this, d->spellCheckingConfigFileName));
}

Sonnet::SpellCheckDecorator *RichTextEditor::createSpellCheckDecorator()
{
    return new RichTextDecorator(this);
}

void RichTextEditor::setHighlighter(Sonnet::Highlighter *_highLighter)
{
    Sonnet::SpellCheckDecorator *decorator = createSpellCheckDecorator();
    decorator->setHighlighter(_highLighter);

    //KTextEdit used to take ownership of the highlighter, Sonnet::SpellCheckDecorator does not.
    //so we reparent the highlighter so it will be deleted when the decorator is destroyed
    _highLighter->setParent(decorator);
    d->richTextDecorator = decorator;
}

void RichTextEditor::focusInEvent(QFocusEvent *event)
{
    if (d->checkSpellingEnabled && !isReadOnly() && !d->richTextDecorator && spellCheckingSupport()) {
        createHighlighter();
    }

    QTextEdit::focusInEvent(event);
}

void RichTextEditor::setSpellCheckingConfigFileName(const QString &_fileName)
{
    d->spellCheckingConfigFileName = _fileName;
}

bool RichTextEditor::checkSpellingEnabled() const
{
    return d->checkSpellingEnabled;
}

void RichTextEditor::setCheckSpellingEnabled(bool check)
{
    if (check == d->checkSpellingEnabled) {
        return;
    }
    Q_EMIT checkSpellingChanged(check);
    // From the above statment we know know that if we're turning checking
    // on that we need to create a new highlighter and if we're turning it
    // off we should remove the old one.

    d->checkSpellingEnabled = check;
    if (check) {
        if (hasFocus()) {
            createHighlighter();
            if (!d->spellCheckingLanguage.isEmpty()) {
                setSpellCheckingLanguage(spellCheckingLanguage());
            }
        }
    } else {
        delete d->richTextDecorator;
        d->richTextDecorator = Q_NULLPTR;
    }
}

const QString &RichTextEditor::spellCheckingLanguage() const
{
    return d->spellCheckingLanguage;
}

void RichTextEditor::setSpellCheckingLanguage(const QString &_language)
{
    if (highlighter()) {
        highlighter()->setCurrentLanguage(_language);
        highlighter()->rehighlight();
    }

    if (_language != d->spellCheckingLanguage) {
        d->spellCheckingLanguage = _language;
        Q_EMIT languageChanged(_language);
    }
}

void RichTextEditor::slotToggleAutoSpellCheck()
{
    setCheckSpellingEnabled(!checkSpellingEnabled());
}

void RichTextEditor::slotLanguageSelected()
{
    QAction *languageAction = static_cast<QAction *>(QObject::sender());
    setSpellCheckingLanguage(languageAction->data().toString());
}
static void deleteWord(QTextCursor cursor, QTextCursor::MoveOperation op)
{
    cursor.clearSelection();
    cursor.movePosition(op, QTextCursor::KeepAnchor);
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

bool RichTextEditor::event(QEvent *ev)
{
    if (ev->type() == QEvent::ShortcutOverride) {
        QKeyEvent *e = static_cast<QKeyEvent *>(ev);
        if (overrideShortcut(e)) {
            e->accept();
            return true;
        }
    }
    return QTextEdit::event(ev);
}

bool RichTextEditor::handleShortcut(const QKeyEvent *event)
{
    const int key = event->key() | event->modifiers();

    if (KStandardShortcut::copy().contains(key)) {
        copy();
        return true;
    } else if (KStandardShortcut::paste().contains(key)) {
        paste();
        return true;
    } else if (KStandardShortcut::cut().contains(key)) {
        cut();
        return true;
    } else if (KStandardShortcut::undo().contains(key)) {
        if (!isReadOnly()) {
            undo();
        }
        return true;
    } else if (KStandardShortcut::redo().contains(key)) {
        if (!isReadOnly()) {
            redo();
        }
        return true;
    } else if (KStandardShortcut::deleteWordBack().contains(key)) {
        if (!isReadOnly()) {
            deleteWordBack();
        }
        return true;
    } else if (KStandardShortcut::deleteWordForward().contains(key)) {
        if (!isReadOnly()) {
            deleteWordForward();
        }
        return true;
    } else if (KStandardShortcut::backwardWord().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::PreviousWord);
        setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::forwardWord().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::NextWord);
        setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::next().contains(key)) {
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
    } else if (KStandardShortcut::prior().contains(key)) {
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
    } else if (KStandardShortcut::begin().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::Start);
        setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::end().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::beginningOfLine().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::StartOfLine);
        setTextCursor(cursor);
        return true;
    } else if (KStandardShortcut::endOfLine().contains(key)) {
        QTextCursor cursor = textCursor();
        cursor.movePosition(QTextCursor::EndOfLine);
        setTextCursor(cursor);
        return true;
    } else if (searchSupport() && KStandardShortcut::find().contains(key)) {
        Q_EMIT findText();
        return true;
    } else if (searchSupport() && KStandardShortcut::replace().contains(key)) {
        if (!isReadOnly()) {
            Q_EMIT replaceText();
        }
        return true;
    } else if (KStandardShortcut::pasteSelection().contains(key)) {
        QString text = QApplication::clipboard()->text(QClipboard::Selection);
        if (!text.isEmpty()) {
            insertPlainText(text);    // TODO: check if this is html? (MiB)
        }
        return true;
    }
    return false;
}

bool RichTextEditor::overrideShortcut(const QKeyEvent *event)
{
    const int key = event->key() | event->modifiers();

    if (KStandardShortcut::copy().contains(key)) {
        return true;
    } else if (KStandardShortcut::paste().contains(key)) {
        return true;
    } else if (KStandardShortcut::cut().contains(key)) {
        return true;
    } else if (KStandardShortcut::undo().contains(key)) {
        return true;
    } else if (KStandardShortcut::redo().contains(key)) {
        return true;
    } else if (KStandardShortcut::deleteWordBack().contains(key)) {
        return true;
    } else if (KStandardShortcut::deleteWordForward().contains(key)) {
        return true;
    } else if (KStandardShortcut::backwardWord().contains(key)) {
        return true;
    } else if (KStandardShortcut::forwardWord().contains(key)) {
        return true;
    } else if (KStandardShortcut::next().contains(key)) {
        return true;
    } else if (KStandardShortcut::prior().contains(key)) {
        return true;
    } else if (KStandardShortcut::begin().contains(key)) {
        return true;
    } else if (KStandardShortcut::end().contains(key)) {
        return true;
    } else if (KStandardShortcut::beginningOfLine().contains(key)) {
        return true;
    } else if (KStandardShortcut::endOfLine().contains(key)) {
        return true;
    } else if (KStandardShortcut::pasteSelection().contains(key)) {
        return true;
    } else if (searchSupport() && KStandardShortcut::find().contains(key)) {
        return true;
    } else if (searchSupport() && KStandardShortcut::findNext().contains(key)) {
        return true;
    } else if (searchSupport() && KStandardShortcut::replace().contains(key)) {
        return true;
    } else if (event->matches(QKeySequence::SelectAll)) { // currently missing in QTextEdit
        return true;
    }
    return false;
}

void RichTextEditor::keyPressEvent(QKeyEvent *event)
{
    if (handleShortcut(event)) {
        event->accept();
    } else {
        QTextEdit::keyPressEvent(event);
    }
}

