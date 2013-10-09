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

#include "plaintexteditor.h"

#include <KLocale>
#include <KGlobalSettings>
#include <KIconTheme>
#include <KStandardGuiItem>
#include <KMessageBox>
#include <KToolInvocation>
#include <KStandardAction>
#include <KAction>

#include <QMenu>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>


using namespace PimCommon;

PlainTextEditor::PlainTextEditor(QWidget *parent)
    : QPlainTextEdit(parent),
      mHasSearchSupport(true)
{
}

PlainTextEditor::~PlainTextEditor()
{
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
                KAction *clearAllAction = KStandardAction::clear(this, SLOT(slotUndoableClear()), popup);
                if ( emptyDocument )
                    clearAllAction->setEnabled( false );
                popup->insertAction( separatorAction, clearAllAction );
            }
        }
        if (mHasSearchSupport) {
            popup->addSeparator();
            QAction *findAct = popup->addAction( KStandardGuiItem::find().icon(), KStandardGuiItem::find().text(),this, SIGNAL(findText()), Qt::Key_F+Qt::CTRL);
            //Code from KTextBrowser
            KIconTheme::assignIconsToContextMenu( isReadOnly() ? KIconTheme::ReadOnlyText
                                                               : KIconTheme::TextEditor,
                                                  popup->actions() );
            if ( emptyDocument )
                findAct->setEnabled(false);
            popup->addSeparator();
            if (!isReadOnly()) {
                QAction *act = popup->addAction(i18n("Replace..."),this, SIGNAL(replaceText()), Qt::Key_R+Qt::CTRL);
                if ( emptyDocument )
                    act->setEnabled( false );
                popup->addSeparator();
            }
        }
        QAction *speakAction = popup->addAction(i18n("Speak Text"));
        speakAction->setIcon(KIcon(QLatin1String("preferences-desktop-text-to-speech")));
        speakAction->setEnabled(!emptyDocument );
        connect( speakAction, SIGNAL(triggered(bool)), this, SLOT(slotSpeakText()) );
        popup->exec( event->globalPos() );

        delete popup;
    }
}

void PlainTextEditor::slotSpeakText()
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

void PlainTextEditor::slotUndoableClear()
{
    QTextCursor cursor = textCursor();
    cursor.beginEditBlock();
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.endEditBlock();
}

void PlainTextEditor::addSearchSupport(bool b)
{
    mHasSearchSupport = b;
}

#include "plaintexteditor.moc"
