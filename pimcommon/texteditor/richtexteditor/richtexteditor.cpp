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

#include <QMenu>
#include <QContextMenuEvent>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QTextCursor>

using namespace PimCommon;
class RichTextEditor::RichTextEditorPrivate
{
public:
    RichTextEditorPrivate()
        : hasSearchSupport(true),
          customPalette(false)

    {
    }
    bool hasSearchSupport;
    bool customPalette;
};


RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent),
      d(new RichTextEditorPrivate)
{
    setAcceptRichText(true);
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

    QTextEdit::setReadOnly( readOnly );
}

#include "richtexteditor.moc"
