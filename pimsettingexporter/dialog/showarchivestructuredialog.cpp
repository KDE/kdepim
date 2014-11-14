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

#include "showarchivestructuredialog.h"
#include "pimsettingexporter/utils.h"
#include "pimcommon/util/pimutil.h"
#include <QDialog>
#include <KLocalizedString>
#include <KZip>
#include <KMessageBox>

#include <QTreeWidget>
#include <QHeaderView>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

ShowArchiveStructureDialog::ShowArchiveStructureDialog(const QString &filename, QWidget *parent)
    : QDialog(parent),
      mFileName(filename)
{
    setWindowTitle(i18n("Show Archive Content on file \"%1\"", filename));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *user1Button = new QPushButton;
    buttonBox->addButton(user1Button, QDialogButtonBox::ActionRole);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ShowArchiveStructureDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ShowArchiveStructureDialog::reject);
    setModal(true);
    mTreeWidget = new QTreeWidget;
    mTreeWidget->header()->hide();
    mTreeWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(mTreeWidget);
    mainLayout->addWidget(buttonBox);
    const bool result = fillTree();
    mTreeWidget->expandAll();
    readConfig();
    user1Button->setText(i18n("Save As Text..."));
    user1Button->setEnabled(result);
    connect(user1Button, &QPushButton::clicked, this, &ShowArchiveStructureDialog::slotExportAsLogFile);
}

ShowArchiveStructureDialog::~ShowArchiveStructureDialog()
{
    writeConfig();
}

void ShowArchiveStructureDialog::slotExportAsLogFile()
{
    PimCommon::Util::saveTextAs(mLogFile, QStringLiteral("*.txt"), this, QUrl(), i18n("Export Log File"));
}

bool ShowArchiveStructureDialog::fillTree()
{
    KZip *zip = new KZip(mFileName);
    bool result = zip->open(QIODevice::ReadOnly);
    if (!result) {
        KMessageBox::error(this, i18n("Archive cannot be opened in read mode."), i18n("Cannot open archive"));
        delete zip;
        return result;
    }
    const KArchiveDirectory *topDirectory = zip->directory();
    const bool isAValidArchive = searchArchiveElement(Utils::infoPath(), topDirectory, i18n("Info"));
    if (!isAValidArchive) {
        KMessageBox::error(this, i18n("This is not kdepim archive."), i18n("Show information"));
        result = false;
    } else {
        searchArchiveElement(Utils::mailsPath(), topDirectory, Utils::appTypeToI18n(Utils::KMail));
        searchArchiveElement(Utils::alarmPath(), topDirectory, Utils::appTypeToI18n(Utils::KAlarm));
        searchArchiveElement(Utils::calendarPath(), topDirectory, Utils::appTypeToI18n(Utils::KOrganizer));
        searchArchiveElement(Utils::addressbookPath(), topDirectory, Utils::appTypeToI18n(Utils::KAddressBook));
        searchArchiveElement(Utils::jotPath(), topDirectory, Utils::appTypeToI18n(Utils::KJots));
        searchArchiveElement(Utils::identitiesPath(), topDirectory, Utils::storedTypeToI18n(Utils::Identity));
        searchArchiveElement(Utils::resourcesPath(), topDirectory, Utils::storedTypeToI18n(Utils::Resources));
        searchArchiveElement(Utils::configsPath(), topDirectory, Utils::storedTypeToI18n(Utils::Config));
        searchArchiveElement(Utils::transportsPath(), topDirectory, Utils::storedTypeToI18n(Utils::MailTransport));
        searchArchiveElement(Utils::dataPath(), topDirectory, Utils::storedTypeToI18n(Utils::Data));
        searchArchiveElement(Utils::akonadiPath(), topDirectory, Utils::storedTypeToI18n(Utils::AkonadiDb));
    }
    delete zip;
    return result;
}

bool ShowArchiveStructureDialog::searchArchiveElement(const QString &path, const KArchiveDirectory *topDirectory, const QString &name)
{
    const KArchiveEntry *topEntry = topDirectory->entry(path);
    bool result = true;
    if (topEntry) {
        mLogFile += name + QLatin1Char('\n');
        QTreeWidgetItem *item = addTopItem(name);
        addSubItems(item, topEntry, 0);
    } else {
        result = false;
    }
    return result;
}

void ShowArchiveStructureDialog::addSubItems(QTreeWidgetItem *parent, const KArchiveEntry *entry, int indent)
{
    const KArchiveDirectory *dir = static_cast<const KArchiveDirectory *>(entry);
    ++indent;
    const QString space = QString(indent * 2, QLatin1Char(' '));
    Q_FOREACH (const QString &entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry) {
            if (entry->isDirectory()) {
                const KArchiveDirectory *dirEntry = static_cast<const KArchiveDirectory *>(entry);
                QTreeWidgetItem *newTopItem = addItem(parent, dirEntry->name());
                QFont font(newTopItem->font(0));
                font.setBold(true);
                mLogFile += space + dirEntry->name() + QLatin1Char('\n');
                newTopItem->setFont(0, font);
                addSubItems(newTopItem, entry, indent);
            } else if (entry->isFile()) {
                const KArchiveFile *file = static_cast<const KArchiveFile *>(entry);
                addItem(parent, file->name());
                mLogFile += space + file->name() + QLatin1Char('\n');
            }
        }
    }
}

QTreeWidgetItem *ShowArchiveStructureDialog::addItem(QTreeWidgetItem *parent, const QString &name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0, name);
    return item;
}

QTreeWidgetItem *ShowArchiveStructureDialog::addTopItem(const QString &name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0, font);
    item->setText(0, name);
    mTreeWidget->addTopLevelItem(item);
    return item;
}

void ShowArchiveStructureDialog::writeConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ShowArchiveStructureDialog");
    group.writeEntry("Size", size());
}

void ShowArchiveStructureDialog::readConfig()
{
    KConfigGroup group(KSharedConfig::openConfig(), "ShowArchiveStructureDialog");
    const QSize sizeDialog = group.readEntry("Size", QSize(600, 400));
    if (sizeDialog.isValid()) {
        resize(sizeDialog);
    }
}

