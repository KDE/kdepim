/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This library is free software; you can redistribute it and/or modify it
  under the terms of the GNU Library General Public License as published by
  the Free Software Foundation; either version 2 of the License, or (at your
  option) any later version.

  This library is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
  License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to the
  Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
  02110-1301, USA.

*/

#include "composerview.h"
#include "composereditor.h"

#include <KMenu>
#include <KMessageBox>
#include <KToolInvocation>
#include <KLocale>
#include <KAction>

#include <QAction>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QWebFrame>
#include <QWebElement>
#include <QContextMenuEvent>


namespace ComposerEditorNG {

class ComposerViewPrivate
{
public:
    ComposerViewPrivate(ComposerEditor *_editor, ComposerView *qq)
        : q(qq), editor(_editor)
    {
    }
    void _k_slotSpeakText();
    QWebHitTestResult contextMenuResult;
    ComposerView *q;
    ComposerEditor *editor;
};

void ComposerViewPrivate::_k_slotSpeakText()
{
    // If KTTSD not running, start it.
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.kttsd")))
    {
        QString error;
        if (KToolInvocation::startServiceByDesktopName(QLatin1String("kttsd"), QStringList(), &error))
        {
            KMessageBox::error(q, i18n( "Starting Jovie Text-to-Speech Service Failed"), error );
            return;
        }
    }
    QDBusInterface ktts(QLatin1String("org.kde.kttsd"), QLatin1String("/KSpeech"), QLatin1String("org.kde.KSpeech"));
    QString text = q->selectedText();
    if(text.isEmpty())
        text = q->page()->mainFrame()->toPlainText();
    ktts.asyncCall(QLatin1String("say"), text, 0);
}

ComposerView::ComposerView(ComposerEditor *editor, QWidget *parent)
    : KWebView(parent),d(new ComposerViewPrivate(editor, this))
{
}

ComposerView::~ComposerView()
{
    delete d;
}

QWebHitTestResult ComposerView::hitTestResult() const
{
    return d->contextMenuResult;
}

void ComposerView::contextMenuEvent(QContextMenuEvent* event)
{
    d->contextMenuResult = page()->mainFrame()->hitTestContent(event->pos());

    const bool linkSelected = !d->contextMenuResult.linkElement().isNull();
    const bool imageSelected = !d->contextMenuResult.imageUrl().isEmpty();

    KMenu *menu = new KMenu;
    const QString selectedText = page()->mainFrame()->toPlainText().simplified();
    const bool emptyDocument = selectedText.isEmpty();

    menu->addAction(page()->action(QWebPage::Undo));
    menu->addAction(page()->action(QWebPage::Redo));
    menu->addSeparator();
    menu->addAction(page()->action(QWebPage::Cut));
    menu->addAction(page()->action(QWebPage::Copy));
    menu->addAction(page()->action(QWebPage::Paste));
    menu->addSeparator();
    menu->addAction(page()->action(QWebPage::SelectAll));
    menu->addSeparator();
    if(imageSelected) {
        //TODO
    } else if(linkSelected) {
        //TODO
    }
    menu->addSeparator();
    menu->addAction(d->editor->actionSpellCheck());
    menu->addSeparator();

    QAction *speakAction = menu->addAction(i18n("Speak Text"));
    speakAction->setIcon(KIcon(QLatin1String("preferences-desktop-text-to-speech")));
    speakAction->setEnabled(!emptyDocument );
    connect( speakAction, SIGNAL(triggered(bool)), this, SLOT(_k_slotSpeakText()) );
    menu->exec(event->globalPos());
    delete menu;
}
}

#include "composerview.moc"
