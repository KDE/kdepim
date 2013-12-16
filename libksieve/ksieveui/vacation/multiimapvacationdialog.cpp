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


#include "multiimapvacationdialog.h"
#include "vacationpagewidget.h"
#include "ksieveui/util/util.h"

#include <Akonadi/AgentInstance>

#include <KLocalizedString>
#include <KSharedConfig>
#include <kwindowsystem.h>

#include <QTabWidget>
#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>


using namespace KSieveUi;
MultiImapVacationDialog::MultiImapVacationDialog(QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n("Configure \"Out of Office\" Replies") );

    KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)) );

    mStackedWidget = new QStackedWidget;
    setMainWidget(mStackedWidget);
    mTabWidget = new QTabWidget;
    mStackedWidget->addWidget(mTabWidget);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);
    QLabel *lab = new QLabel(i18n("KMail's Out of Office Reply functionality relies on "
                                  "server-side filtering. You have not yet configured an "
                                  "IMAP server for this."
                                  "You can do this on the \"Filtering\" tab of the IMAP "
                                  "account configuration."));
    vbox->addStretch();
    vbox->addWidget(lab);
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

QList<VacationCreateScriptJob *> MultiImapVacationDialog::listCreateJob() const
{
    return mListCreateJob;
}

void MultiImapVacationDialog::init()
{
    bool foundOneImap = false;
    const Akonadi::AgentInstance::List instances = KSieveUi::Util::imapAgentInstances();
    foreach ( const Akonadi::AgentInstance &instance, instances ) {
        if ( instance.status() == Akonadi::AgentInstance::Broken )
            continue;

        const KUrl url = KSieveUi::Util::findSieveUrlForAccount( instance.identifier() );
        if ( !url.isEmpty() ) {
            const QString serverName = instance.name();
            createPage(serverName, url);
            foundOneImap = true;
        }
    }
    if (foundOneImap) {
        setButtons( Ok | Cancel | Default );
        setDefaultButton( Ok );
    } else {
        mStackedWidget->setCurrentIndex(1);
        setButtons( Close );
    }
}

void MultiImapVacationDialog::createPage(const QString &serverName, const KUrl &url)
{
    VacationPageWidget *page = new VacationPageWidget;
    page->setServerUrl(url);
    page->setServerName(serverName);
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
