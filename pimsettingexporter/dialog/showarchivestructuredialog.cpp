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

#include "showarchivestructuredialog.h"
#include "pimsettingexporter/utils.h"

#include <KDialog>
#include <KLocale>
#include <KZip>
#include <KMessageBox>

#include <QTreeWidget>
#include <QHeaderView>

ShowArchiveStructureDialog::ShowArchiveStructureDialog(const KUrl &archiveUrl, QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Show Archive Content" ) );
    setButtons( Close );
    setModal( true );
    mTreeWidget = new QTreeWidget;
    mTreeWidget->header()->hide();
    setMainWidget(mTreeWidget);
    fillTree(archiveUrl);
    mTreeWidget->expandAll();
}

ShowArchiveStructureDialog::~ShowArchiveStructureDialog()
{
}

void ShowArchiveStructureDialog::fillTree(const KUrl &archiveUrl)
{
    KZip *zip = new KZip(archiveUrl.path());
    bool result = zip->open(QIODevice::ReadOnly);
    if (!result) {
        KMessageBox::error(this, i18n("Archive cannot be opened in read mode."), i18n("Can not open archive"));
        delete zip;
        return;
    }
    const KArchiveDirectory *topDirectory = zip->directory();
    searchArchiveElement(Utils::infoPath(), topDirectory, i18n("Infos"));
    searchArchiveElement(Utils::mailsPath(), topDirectory, i18n("KMail"));
    searchArchiveElement(Utils::alarmPath(), topDirectory, i18n("KAlarm"));
    searchArchiveElement(Utils::calendarPath(), topDirectory, i18n("KOrganizer"));
    searchArchiveElement(Utils::addressbookPath(), topDirectory, i18n("KAddressBook"));
    searchArchiveElement(Utils::jotPath(), topDirectory, i18n("KJots"));
    searchArchiveElement(Utils::identitiesPath(), topDirectory, i18n("Identity"));
    searchArchiveElement(Utils::resourcesPath(), topDirectory, i18n("Resources"));
    searchArchiveElement(Utils::configsPath(), topDirectory, i18n("Configs"));
    searchArchiveElement(Utils::transportsPath(), topDirectory, i18n("Transports Config"));
    searchArchiveElement(Utils::dataPath(), topDirectory, i18n("Datas"));
    searchArchiveElement(Utils::akonadiPath(), topDirectory, i18n("Akonadi"));
    delete zip;
}

void ShowArchiveStructureDialog::searchArchiveElement(const QString &path, const KArchiveDirectory *topDirectory, const QString &name)
{
    const KArchiveEntry *topEntry = topDirectory->entry(path);
    if (topEntry) {
        QTreeWidgetItem *item = addTopItem(name);
        addSubItems(item, topEntry);
    }
}

void ShowArchiveStructureDialog::addSubItems(QTreeWidgetItem *parent, const KArchiveEntry *entry)
{
    const KArchiveDirectory *dir = static_cast<const KArchiveDirectory *>(entry);
    Q_FOREACH(const QString& entryName, dir->entries()) {
        const KArchiveEntry *entry = dir->entry(entryName);
        if (entry) {
            if (entry->isDirectory()) {
                const KArchiveDirectory *dirEntry = static_cast<const KArchiveDirectory *>(entry);
                QTreeWidgetItem *newTopItem = addItem(parent, dirEntry->name());
                addSubItems(newTopItem, entry);
            } else if (entry->isFile()) {
                const KArchiveFile *file = static_cast<const KArchiveFile *>(entry);
                addItem(parent,file->name());
            }
        }
    }
}

QTreeWidgetItem *ShowArchiveStructureDialog::addItem(QTreeWidgetItem *parent,const QString &name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(parent);
    item->setText(0,name);
    return item;
}


QTreeWidgetItem *ShowArchiveStructureDialog::addTopItem(const QString &name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    QFont font = item->font(0);
    font.setBold(true);
    item->setFont(0,font);
    item->setText(0,name);
    mTreeWidget->addTopLevelItem(item);
    return item;
}

#include "moc_showarchivestructuredialog.cpp"
