/*
    Copyright (c) 2009 Volker Krause <vkrause@kde.org>
    Copyright (c) 2010 Tom Albers <toma@kde.org>

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

#include "dialog.h"
#include "personaldatapage.h"
#ifndef ACCOUNTWIZARD_NO_GHNS
#include "providerpage.h"
#endif
#include "typepage.h"
#include "loadpage.h"
#include "global.h"
#include "dynamicpage.h"
#include "setupmanager.h"
#include "servertest.h"
#include "setuppage.h"

#include <KMime/Message>

#include <klocalizedstring.h>
#include <kross/core/action.h>
#include "accountwizard_debug.h"
#include <kmessagebox.h>
#include <qplatformdefs.h>
#include <KAboutData>

#include <KHelpMenu>

Dialog::Dialog(QWidget *parent)
    : KAssistantDialog(parent)
{
    mSetupManager = new SetupManager(this);
    const bool showPersonalDataPage = Global::typeFilter().size() == 1 && Global::typeFilter().at(0) == KMime::Message::mimeType();

    if (showPersonalDataPage) {
        // todo: don't ask these details based on a setting of the desktop file.
        PersonalDataPage *pdpage = new PersonalDataPage(this);
        addPage(pdpage, i18n("Provide personal data"));
        connect(pdpage, &PersonalDataPage::manualWanted, this, &Dialog::slotManualConfigWanted);
        if (!Global::assistant().isEmpty()) {
            pdpage->setHideOptionInternetSearch(true);
        }
    }

    if (Global::assistant().isEmpty()) {
        TypePage *typePage = new TypePage(this);
        connect(typePage->treeview(), &QAbstractItemView::doubleClicked, this, &Dialog::slotNextPage);
#ifndef ACCOUNTWIZARD_NO_GHNS
        connect(typePage, &TypePage::ghnsWanted, this, &Dialog::slotGhnsWanted);
#endif
        mTypePage = addPage(typePage, i18n("Select Account Type"));
        setAppropriate(mTypePage, false);

#ifndef ACCOUNTWIZARD_NO_GHNS
        ProviderPage *ppage = new ProviderPage(this);
        connect(typePage, &TypePage::ghnsWanted, ppage, &ProviderPage::startFetchingData);
        connect(ppage->treeview(), &QAbstractItemView::doubleClicked, this, &Dialog::slotNextPage);
        connect(ppage, &ProviderPage::ghnsNotWanted, this, &Dialog::slotGhnsNotWanted);
        mProviderPage = addPage(ppage, i18n("Select Provider"));
        setAppropriate(mProviderPage, false);
#endif
    }

    LoadPage *loadPage = new LoadPage(this);
    mLoadPage = addPage(loadPage, i18n("Loading Assistant"));
    setAppropriate(mLoadPage, false);
    loadPage->exportObject(this, QStringLiteral("Dialog"));
    loadPage->exportObject(mSetupManager, QStringLiteral("SetupManager"));
    loadPage->exportObject(new ServerTest(this), QStringLiteral("ServerTest"));
    connect(loadPage, &LoadPage::aboutToStart, this, &Dialog::clearDynamicPages);

    SetupPage *setupPage = new SetupPage(this);
    mLastPage = addPage(setupPage, i18n("Setting up Account"));
    mSetupManager->setSetupPage(setupPage);

    slotManualConfigWanted(!showPersonalDataPage);

    Page *page = qobject_cast<Page *>(currentPage()->widget());
    page->enterPageNext();
    Q_EMIT page->pageEnteredNext();
    connect(button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &Dialog::accept);
    KHelpMenu *helpMenu = new KHelpMenu(this, KAboutData::applicationData(), true);
    //Initialize menu
    QMenu *menu = helpMenu->menu();
    helpMenu->action(KHelpMenu::menuAboutApp)->setIcon(QIcon::fromTheme(QStringLiteral("akonadi")));
    button(QDialogButtonBox::Help)->setMenu(menu);

}

KPageWidgetItem *Dialog::addPage(Page *page, const QString &title)
{
    KPageWidgetItem *item = KAssistantDialog::addPage(page, title);
    connect(page, &Page::leavePageNextOk, this, &Dialog::slotNextOk);
    connect(page, &Page::leavePageBackOk, this, &Dialog::slotBackOk);
    page->setPageWidgetItem(item);
    return item;
}

void Dialog::slotNextPage()
{
    next();
}

void Dialog::next()
{
    Page *page = qobject_cast<Page *>(currentPage()->widget());
    page->leavePageNext();
    page->leavePageNextRequested();
}

void Dialog::slotNextOk()
{
    Page *page = qobject_cast<Page *>(currentPage()->widget());
    Q_EMIT page->pageLeftNext();
    KAssistantDialog::next();
    page = qobject_cast<Page *>(currentPage()->widget());
    page->enterPageNext();
    Q_EMIT page->pageEnteredNext();
}

void Dialog::back()
{
    Page *page = qobject_cast<Page *>(currentPage()->widget());
    page->leavePageBack();
    page->leavePageBackRequested();
}

void Dialog::slotBackOk()
{
    Page *page = qobject_cast<Page *>(currentPage()->widget());
    Q_EMIT page->pageLeftBack();
    KAssistantDialog::back();
    page = qobject_cast<Page *>(currentPage()->widget());
    page->enterPageBack();
    Q_EMIT page->pageEnteredBack();
}

QObject *Dialog::addPage(const QString &uiFile, const QString &title)
{
    qCDebug(ACCOUNTWIZARD_LOG) << uiFile;
    DynamicPage *page = new DynamicPage(Global::assistantBasePath() + uiFile, this);
    connect(page, &DynamicPage::leavePageNextOk, this, &Dialog::slotNextOk);
    connect(page, &DynamicPage::leavePageBackOk, this, &Dialog::slotBackOk);
    KPageWidgetItem *item = insertPage(mLastPage, page, title);
    page->setPageWidgetItem(item);
    mDynamicPages.push_back(item);
    return page;
}

void Dialog::slotManualConfigWanted(bool show)
{
    Q_ASSERT(mTypePage);
    setAppropriate(mTypePage, show);
    setAppropriate(mLoadPage, show);
}

#ifndef ACCOUNTWIZARD_NO_GHNS
void Dialog::slotGhnsWanted()
{
    Q_ASSERT(mProviderPage);
    setAppropriate(mProviderPage, true);
    setCurrentPage(mProviderPage);
}

void Dialog::slotGhnsNotWanted()
{
    Q_ASSERT(mProviderPage);
    setAppropriate(mProviderPage, false);
}
#endif

SetupManager *Dialog::setupManager()
{
    return mSetupManager;
}

void Dialog::clearDynamicPages()
{
    foreach (KPageWidgetItem *item, mDynamicPages) {
        removePage(item);
    }
    mDynamicPages.clear();
}

void Dialog::reject()
{
    connect(mSetupManager, &SetupManager::rollbackComplete, this, &Dialog::close);
    mSetupManager->requestRollback();
}

