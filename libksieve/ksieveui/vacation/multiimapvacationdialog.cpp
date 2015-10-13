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


#include "multiimapvacationdialog.h"
#include "vacationpagewidget.h"
#include "multiimapvacationmanager.h"
#include "ksieveui/util/util.h"

#include <Akonadi/AgentInstance>

#include <KLocalizedString>
#include <KSharedConfig>
#include <kwindowsystem.h>
#include <KTabWidget>

#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>


using namespace KSieveUi;
MultiImapVacationDialog::MultiImapVacationDialog(MultiImapVacationManager *manager, QWidget *parent)
    : KDialog(parent)
    , mVacationManager(manager)
{
    setCaption( i18n("Configure \"Out of Office\" Replies") );

    KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)) );

    mStackedWidget = new QStackedWidget;
    setMainWidget(mStackedWidget);
    mTabWidget = new KTabWidget;
    mStackedWidget->addWidget(mTabWidget);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);
    QLabel *lab = new QLabel(i18n("KMail's Out of Office Reply functionality relies on "
                                  "server-side filtering. You have not yet configured an "
                                  "IMAP server for this. "
                                  "You can do this on the \"Filtering\" tab of the IMAP "
                                  "account configuration."));
    lab->setWordWrap(true);
    vbox->addWidget(lab);
    vbox->addStretch();
    mStackedWidget->addWidget(w);
    mStackedWidget->setCurrentIndex(0);
    init();
    readConfig();
    connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
    connect(this, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));
}

MultiImapVacationDialog::~MultiImapVacationDialog()
{
    writeConfig();
}

void MultiImapVacationDialog::switchToServerNamePage(const QString &serverName)
{
    for (int i=0; i < mTabWidget->count(); ++i) {
        if (mTabWidget->tabText(i) == serverName) {
            mTabWidget->setCurrentIndex(i);
            break;
        }
    }
}

QList<VacationCreateScriptJob *> MultiImapVacationDialog::listCreateJob() const
{
    return mListCreateJob;
}

void MultiImapVacationDialog::init()
{
    bool foundOneImap = false;

    QMap <QString, KUrl> list = mVacationManager->serverList();
    foreach (const QString &serverName, list.keys()) {
        const KUrl url = list.value(serverName);
        createPage(serverName, url);
        foundOneImap = true;
    }
    if (foundOneImap) {
        setButtons( Ok | Cancel | Default );
        setDefaultButton( Ok );
    } else {
        mStackedWidget->setCurrentIndex(1);
        setButtons( Close );
    }
    if (mTabWidget->count() <= 1)
        mTabWidget->setTabBarHidden(true);
}

void MultiImapVacationDialog::createPage(const QString &serverName, const KUrl &url)
{
    VacationPageWidget *page = new VacationPageWidget;
    page->setServerUrl(url);
    page->setServerName(serverName);
    page->setVacationManager(mVacationManager);
    mTabWidget->addTab(page,serverName);
}

void MultiImapVacationDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "MultiImapVacationDialog" );
    const QSize size = group.readEntry( "Size", QSize() );
    if ( size.isValid() ) {
        resize( size );
    } else {
        resize( sizeHint().width(), sizeHint().height() );
    }
}

void MultiImapVacationDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "MultiImapVacationDialog" );
    group.writeEntry( "Size", size() );
}

void MultiImapVacationDialog::slotOkClicked()
{
    for (int i=0; i < mTabWidget->count(); ++i) {
        VacationPageWidget *vacationPage = qobject_cast<VacationPageWidget *>(mTabWidget->widget(i));
        if (vacationPage) {
            VacationCreateScriptJob *job = vacationPage->writeScript();
            if (job)
                mListCreateJob.append(job);
        }
    }
}

void MultiImapVacationDialog::slotDefaultClicked()
{
    for (int i=0; i < mTabWidget->count(); ++i) {
        VacationPageWidget *vacationPage = qobject_cast<VacationPageWidget *>(mTabWidget->widget(i));
        if (vacationPage) {
            vacationPage->setDefault();
        }
    }
}
