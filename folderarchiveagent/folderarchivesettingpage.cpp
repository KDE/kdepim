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

#include "folderarchivesettingpage.h"
#include "folderarchiveaccountinfo.h"
#include "mailcommon/folder/folderrequester.h"

#include <KLocale>
#include <KGlobal>
#include <KSharedConfig>

#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

FolderArchiveSettingPage::FolderArchiveSettingPage(const QString &instanceName, QWidget *parent)
    : QWidget(parent),
      mInstanceName(instanceName),
      mInfo(0)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mEnabled = new QCheckBox(i18n("Enable"));
    connect(mEnabled, SIGNAL(toggled(bool)), this, SLOT(slotEnableChanged(bool)));
    lay->addWidget(mEnabled);

    QHBoxLayout *hbox = new QHBoxLayout;
    QLabel *lab = new QLabel(i18n("Archive folder:"));
    hbox->addWidget(lab);
    mArchiveFolder = new MailCommon::FolderRequester;
    hbox->addWidget(mArchiveFolder);
    lay->addLayout(hbox);
    lay->addStretch();

    setLayout(lay);
}

FolderArchiveSettingPage::~FolderArchiveSettingPage()
{
    delete mInfo;
}

void FolderArchiveSettingPage::slotEnableChanged(bool enabled)
{
    mArchiveFolder->setEnabled(enabled);
}

void FolderArchiveSettingPage::loadSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    const QString groupName = QLatin1String("FolderArchiveAccount ") + mInstanceName;
    if (config->hasGroup(groupName)) {
        KConfigGroup grp = config->group(groupName);
        mInfo = new FolderArchiveAccountInfo(grp);
        mEnabled->setChecked(mInfo->enabled());
        mArchiveFolder->setCollection(Akonadi::Collection(mInfo->archiveTopLevel()));
    } else {
        mInfo = new FolderArchiveAccountInfo();
        mEnabled->setChecked(false);
    }
    slotEnableChanged(mEnabled->isChecked());
}

void FolderArchiveSettingPage::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup grp = config->group(QLatin1String("FolderArchiveAccount ") + mInstanceName);
    mInfo->setInstanceName(mInstanceName);
    mInfo->setEnabled(mEnabled->isChecked());
    mInfo->setArchiveTopLevel(mArchiveFolder->collection().id());
    mInfo->writeConfig(grp);
}

#include "folderarchivesettingpage.moc"
