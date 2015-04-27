/*
  This file is part of KAddressBook.

  Copyright (c) 2009-2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kaddressbookpart.h"
#include "mainwidget.h"

#include "kaddressbook_debug.h"
#include <KIconLoader>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KParts/StatusBarExtension>
#include <QAction>
#include <KActionCollection>
#include <QIcon>

#include <QVBoxLayout>

K_PLUGIN_FACTORY(KAddressBookFactory, registerPlugin<KAddressBookPart>();)

KAddressBookPart::KAddressBookPart(QWidget *parentWidget, QObject *parent,
                                   const QVariantList &)
    : KParts::ReadOnlyPart(parent)
{
    setComponentName(QStringLiteral("kaddressbook"), QStringLiteral("kaddressbook"));

    KIconLoader::global()->addAppDir(QStringLiteral("kaddressbook"));
    // create a canvas to insert our widget
    QWidget *canvas = new QWidget(parentWidget);
    canvas->setFocusPolicy(Qt::ClickFocus);
    setWidget(canvas);
    QVBoxLayout *topLayout = new QVBoxLayout(canvas);

    mMainWidget = new MainWidget(this, canvas);
    initAction();

    topLayout->addWidget(mMainWidget);
    topLayout->setMargin(0);
    setXMLFile(QStringLiteral("kaddressbookui.rc"));
}

void KAddressBookPart::updateQuickSearchText()
{
    mMainWidget->updateQuickSearchText();
}

void KAddressBookPart::initAction()
{
    QAction *action = new QAction(QIcon::fromTheme(QStringLiteral("configure")), i18n("&Configure KAddressBook..."), this);
    actionCollection()->addAction(QStringLiteral("kaddressbook_configure"), action);
    connect(action, &QAction::triggered, mMainWidget, &MainWidget::configure);
}

void KAddressBookPart::newContact()
{
    mMainWidget->newContact();
}

void KAddressBookPart::newGroup()
{
    mMainWidget->newGroup();
}

KAddressBookPart::~KAddressBookPart()
{
}

bool KAddressBookPart::openFile()
{
    return false;
}

void KAddressBookPart::guiActivateEvent(KParts::GUIActivateEvent *e)
{
    qCDebug(KADDRESSBOOK_LOG);
    KParts::ReadOnlyPart::guiActivateEvent(e);
}

#include "kaddressbookpart.moc"
