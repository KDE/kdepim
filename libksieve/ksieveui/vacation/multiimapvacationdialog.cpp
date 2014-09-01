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
#include "ksieveui/util/util.h"

#include <AgentInstance>

#include <KLocalizedString>
#include <KSharedConfig>
#include <kwindowsystem.h>
#include <QTabWidget>
#include <KIconLoader>
#include <QUrl>

#include <QTabBar>
#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>

using namespace KSieveUi;
MultiImapVacationDialog::MultiImapVacationDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(i18n("Configure \"Out of Office\" Replies"));

    KWindowSystem::setIcons(winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop), IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small), IconSize(KIconLoader::Small)));

    init();
    readConfig();
}

MultiImapVacationDialog::~MultiImapVacationDialog()
{
    writeConfig();
}

void MultiImapVacationDialog::switchToServerNamePage(const QString &serverName)
{
    for (int i = 0; i < mTabWidget->count(); ++i) {
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
    mStackedWidget = new QStackedWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mStackedWidget);
    mTabWidget = new QTabWidget;
    mStackedWidget->addWidget(mTabWidget);
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    w->setLayout(vbox);
    QLabel *lab = new QLabel(i18n("KMail's Out of Office Reply functionality relies on "
                                  "server-side filtering. You have not yet configured an "
                                  "IMAP server for this. "
                                  "You can do this on the \"Filtering\" tab of the IMAP "
                                  "account configuration."));
    vbox->addWidget(lab);
    vbox->addStretch();
    lab->setWordWrap(true);
    mStackedWidget->addWidget(w);
    mStackedWidget->setCurrentIndex(0);
    bool foundOneImap = false;
    const Akonadi::AgentInstance::List instances = KSieveUi::Util::imapAgentInstances();
    foreach (const Akonadi::AgentInstance &instance, instances) {
        if (instance.status() == Akonadi::AgentInstance::Broken) {
            continue;
        }

        const QUrl url = KSieveUi::Util::findSieveUrlForAccount(instance.identifier());
        if (!url.isEmpty()) {
            const QString serverName = instance.name();
            createPage(serverName, url);
            foundOneImap = true;
        }
    }
    QDialogButtonBox *buttonBox = 0;
    if (foundOneImap) {
        buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::RestoreDefaults);
        QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
        okButton->setDefault(true);
        okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOkClicked()));
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(slotCanceled()));
        connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), this, SLOT(slotDefaultClicked));
        okButton->setDefault(true);
    } else {
        mStackedWidget->setCurrentIndex(1);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
        connect(buttonBox, SIGNAL(accepted()), this, SLOT(slotOkClicked()));
        connect(buttonBox, SIGNAL(rejected()), this, SLOT(slotCanceled()));
    }
    mainLayout->addWidget(buttonBox);
    if (mTabWidget->count() <= 1) {
        mTabWidget->tabBar()->hide();
    }
}

void MultiImapVacationDialog::slotCanceled()
{
    Q_EMIT cancelClicked();
}

void MultiImapVacationDialog::createPage(const QString &serverName, const QUrl &url)
{
    VacationPageWidget *page = new VacationPageWidget;
    page->setServerUrl(url);
    page->setServerName(serverName);
    mTabWidget->addTab(page, serverName + QString::fromLatin1(" (%1)").arg(url.userName()));
}

void MultiImapVacationDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "MultiImapVacationDialog");
    const QSize size = group.readEntry("Size", QSize());
    if (size.isValid()) {
        resize(size);
    } else {
        resize(sizeHint().width(), sizeHint().height());
    }
}

void MultiImapVacationDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "MultiImapVacationDialog");
    group.writeEntry("Size", size());
}

void MultiImapVacationDialog::slotOkClicked()
{
    for (int i = 0; i < mTabWidget->count(); ++i) {
        VacationPageWidget *vacationPage = qobject_cast<VacationPageWidget *>(mTabWidget->widget(i));
        if (vacationPage) {
            VacationCreateScriptJob *job = vacationPage->writeScript();
            if (job) {
                mListCreateJob.append(job);
            }
        }
    }
    Q_EMIT okClicked();
}

void MultiImapVacationDialog::slotDefaultClicked()
{
    for (int i = 0; i < mTabWidget->count(); ++i) {
        VacationPageWidget *vacationPage = qobject_cast<VacationPageWidget *>(mTabWidget->widget(i));
        if (vacationPage) {
            vacationPage->setDefault();
        }
    }
}

