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

ShowArchiveStructureDialog::ShowArchiveStructureDialog(const KUrl &archiveUrl, QWidget *parent)
    : KDialog(parent)
{
    setCaption( i18n( "Show Archive Content" ) );
    setButtons( Close );
    setModal( true );
    mTreeWidget = new QTreeWidget;
    setMainWidget(mTreeWidget);

    fillTree(archiveUrl);
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
    QString topLevel = Utils::mailsPath();
    topDirectory->entry(topLevel);
    if (topDirectory) {
        addTopItem(QLatin1String("kmail"));
    }

    delete zip;
}

QTreeWidgetItem* ShowArchiveStructureDialog::addTopItem(const QString &name)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0,name);
    mTreeWidget->addTopLevelItem(item);
    return item;
}

#include "moc_showarchivestructuredialog.cpp"
