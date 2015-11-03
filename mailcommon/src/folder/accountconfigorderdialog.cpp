/*
  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#include "accountconfigorderdialog.h"
#include "mailcommonsettings_base.h"
#include "PimCommon/PimUtil"
#include "util/mailutil.h"
#include "mailcommon_debug.h"
#include <KLocalizedString>
#include <QPushButton>
#include <QVBoxLayout>
#include <QIcon>

#include <AkonadiCore/AgentInstance>
#include <AkonadiCore/AgentManager>

#include <KMime/KMimeMessage>

#include <QListWidget>
#include <QHBoxLayout>
#include <QCheckBox>
#include <KConfigGroup>
#include <QDialogButtonBox>

using namespace MailCommon;

struct InstanceStruct {
    QString name;
    QIcon icon;
};

class MailCommon::AccountConfigOrderDialogPrivate
{
public:
    AccountConfigOrderDialogPrivate()
        : mListAccount(Q_NULLPTR),
          mUpButton(Q_NULLPTR),
          mDownButton(Q_NULLPTR),
          mEnableAccountOrder(Q_NULLPTR)
    {

    }
    QListWidget *mListAccount;
    QPushButton *mUpButton;
    QPushButton *mDownButton;
    QCheckBox *mEnableAccountOrder;
};

AccountConfigOrderDialog::AccountConfigOrderDialog(QWidget *parent)
    : QDialog(parent),
      d(new MailCommon::AccountConfigOrderDialogPrivate)
{
    setWindowTitle(i18n("Edit Accounts Order"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AccountConfigOrderDialog::reject);
    okButton->setDefault(true);

    QWidget *page = new QWidget(this);
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);

    QVBoxLayout *vbox = new QVBoxLayout;
    page->setLayout(vbox);

    d->mEnableAccountOrder = new QCheckBox(i18n("Use custom order"));
    connect(d->mEnableAccountOrder, &QCheckBox::clicked, this, &AccountConfigOrderDialog::slotEnableAccountOrder);
    vbox->addWidget(d->mEnableAccountOrder);

    QHBoxLayout *vlay = new QHBoxLayout;
    vbox->addLayout(vlay);

    d->mListAccount = new QListWidget(this);
    d->mListAccount->setDragDropMode(QAbstractItemView::InternalMove);
    vlay->addWidget(d->mListAccount);

    QWidget *upDownBox = new QWidget(page);
    QVBoxLayout *upDownBoxVBoxLayout = new QVBoxLayout(upDownBox);
    upDownBoxVBoxLayout->setMargin(0);
    d->mUpButton = new QPushButton(upDownBox);
    upDownBoxVBoxLayout->addWidget(d->mUpButton);
    d->mUpButton->setIcon(QIcon::fromTheme(QStringLiteral("go-up")));
    d->mUpButton->setToolTip(i18nc("Move selected account up.", "Up"));
    d->mUpButton->setEnabled(false);   // b/c no item is selected yet
    d->mUpButton->setFocusPolicy(Qt::StrongFocus);
    d->mUpButton->setAutoRepeat(true);

    d->mDownButton = new QPushButton(upDownBox);
    upDownBoxVBoxLayout->addWidget(d->mDownButton);
    d->mDownButton->setIcon(QIcon::fromTheme(QStringLiteral("go-down")));
    d->mDownButton->setToolTip(i18nc("Move selected account down.", "Down"));
    d->mDownButton->setEnabled(false);   // b/c no item is selected yet
    d->mDownButton->setFocusPolicy(Qt::StrongFocus);
    d->mDownButton->setAutoRepeat(true);

    QWidget *spacer = new QWidget(upDownBox);
    upDownBoxVBoxLayout->addWidget(spacer);
    upDownBoxVBoxLayout->setStretchFactor(spacer, 100);
    vlay->addWidget(upDownBox);

    connect(d->mUpButton, &QPushButton::clicked, this, &AccountConfigOrderDialog::slotMoveUp);
    connect(d->mDownButton, &QPushButton::clicked, this, &AccountConfigOrderDialog::slotMoveDown);
    connect(d->mListAccount, &QListWidget::itemSelectionChanged, this, &AccountConfigOrderDialog::slotEnableControls);
    connect(d->mListAccount->model(), &QAbstractItemModel::rowsMoved, this, &AccountConfigOrderDialog::slotEnableControls);

    connect(okButton, &QPushButton::clicked, this, &AccountConfigOrderDialog::slotOk);
    readConfig();
    init();
}

AccountConfigOrderDialog::~AccountConfigOrderDialog()
{
    writeConfig();
    delete d;
}

void AccountConfigOrderDialog::slotEnableAccountOrder(bool state)
{
    d->mListAccount->setEnabled(state);
    d->mUpButton->setEnabled(state);
    d->mDownButton->setEnabled(state);
    if (state) {
        slotEnableControls();
    }
}

void AccountConfigOrderDialog::slotMoveUp()
{
    if (!d->mListAccount->currentItem()) {
        return;
    }
    const int pos = d->mListAccount->row(d->mListAccount->currentItem());
    d->mListAccount->blockSignals(true);
    QListWidgetItem *item = d->mListAccount->takeItem(pos);
    // now selected item is at idx(idx-1), so
    // insert the other item at idx, ie. above(below).
    d->mListAccount->insertItem(pos - 1,  item);
    d->mListAccount->blockSignals(false);
    d->mListAccount->setCurrentRow(pos - 1);
}

void AccountConfigOrderDialog::slotMoveDown()
{
    if (!d->mListAccount->currentItem()) {
        return;
    }
    const int pos = d->mListAccount->row(d->mListAccount->currentItem());
    d->mListAccount->blockSignals(true);
    QListWidgetItem *item = d->mListAccount->takeItem(pos);
    // now selected item is at idx(idx-1), so
    // insert the other item at idx, ie. above(below).
    d->mListAccount->insertItem(pos + 1, item);
    d->mListAccount->blockSignals(false);
    d->mListAccount->setCurrentRow(pos + 1);
}

void AccountConfigOrderDialog::slotEnableControls()
{
    QListWidgetItem *item = d->mListAccount->currentItem();

    d->mUpButton->setEnabled(item && d->mListAccount->currentRow() != 0);
    d->mDownButton->setEnabled(item && d->mListAccount->currentRow() != d->mListAccount->count() - 1);
}

void AccountConfigOrderDialog::init()
{
    const QStringList listOrderAccount = MailCommon::MailCommonSettings::self()->order();
    QStringList instanceList;

    QMap<QString, InstanceStruct> mapInstance;
    foreach (const Akonadi::AgentInstance &instance, Akonadi::AgentManager::self()->instances()) {
        const QStringList capabilities(instance.type().capabilities());
        if (instance.type().mimeTypes().contains(KMime::Message::mimeType())) {
            if (capabilities.contains(QStringLiteral("Resource")) &&
                    !capabilities.contains(QStringLiteral("Virtual")) &&
                    !capabilities.contains(QStringLiteral("MailTransport"))) {
                const QString identifier = instance.identifier();
                if (!identifier.contains(POP3_RESOURCE_IDENTIFIER)) {
                    instanceList << instance.identifier();
                    InstanceStruct instanceStruct;
                    instanceStruct.name = instance.name();
                    if (PimCommon::Util::isImapResource(identifier)) {
                        instanceStruct.name += QLatin1String(" (IMAP)");
                    } else if (identifier.startsWith(QStringLiteral("akonadi_maildir_resource"))) {
                        instanceStruct.name += QLatin1String(" (Maildir)");
                    } else if (identifier.startsWith(QStringLiteral("akonadi_mailbox_resource"))) {
                        instanceStruct.name += QLatin1String(" (Mailbox)");
                    } else if (identifier.startsWith(QStringLiteral("akonadi_mixedmaildir_resource"))) {
                        instanceStruct.name += QLatin1String(" (Mixedmaildir)");
                    } else {
                        qCDebug(MAILCOMMON_LOG) << " Unknown resource type " << identifier;
                    }
                    instanceStruct.icon = instance.type().icon();
                    mapInstance.insert(instance.identifier(), instanceStruct);
                }
            }
        }
    }
    const int numberOfList(listOrderAccount.count());
    for (int i = 0; i < numberOfList; ++i) {
        instanceList.removeOne(listOrderAccount.at(i));
    }

    QStringList finalList(listOrderAccount);
    finalList += instanceList;

    const int numberOfElement(finalList.count());
    for (int i = 0; i < numberOfElement; ++i) {
        const QString identifier(finalList.at(i));
        if (mapInstance.contains(identifier)) {
            InstanceStruct tmp = mapInstance.value(identifier);
            QListWidgetItem *item = new QListWidgetItem(tmp.icon, tmp.name, d->mListAccount);
            item->setData(AccountConfigOrderDialog::IdentifierAccount, identifier);
            d->mListAccount->addItem(item);
        }
    }
    d->mEnableAccountOrder->setChecked(MailCommon::MailCommonSettings::self()->enableAccountOrder());
    slotEnableAccountOrder(MailCommon::MailCommonSettings::self()->enableAccountOrder());
}

void AccountConfigOrderDialog::slotOk()
{
    QStringList order;
    const int numberOfItems(d->mListAccount->count());
    for (int i = 0; i < numberOfItems; ++i) {
        order << d->mListAccount->item(i)->data(AccountConfigOrderDialog::IdentifierAccount).toString();
    }

    MailCommon::MailCommonSettings::self()->setOrder(order);
    MailCommon::MailCommonSettings::self()->setEnableAccountOrder(d->mEnableAccountOrder->isChecked());
    MailCommon::MailCommonSettings::self()->save();
    QDialog::accept();
}

void AccountConfigOrderDialog::readConfig()
{
    KConfigGroup accountConfigDialog(MailCommon::MailCommonSettings::self()->config(), "AccountConfigOrderDialog");
    const QSize size = accountConfigDialog.readEntry("Size", QSize(600, 400));
    if (size.isValid()) {
        resize(size);
    }
}

void AccountConfigOrderDialog::writeConfig()
{
    KConfigGroup accountConfigDialog(MailCommon::MailCommonSettings::self()->config(), "AccountConfigOrderDialog");
    accountConfigDialog.writeEntry("Size", size());
    accountConfigDialog.sync();
}

