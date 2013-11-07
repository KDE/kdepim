/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "configureidentity.h"
#include "activitymanager.h"

#include <KPIMIdentities/IdentityManager>
#include <KPIMIdentities/Identity>


#include <QVBoxLayout>
#include <QDebug>
#include <QListWidget>

namespace PimActivity {

ConfigureIdentity::ConfigureIdentity(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mListIdentity = new QListWidget;
    lay->addWidget(mListIdentity);
    init();
    setLayout(lay);
    connect(mListIdentity, SIGNAL(itemChanged(QListWidgetItem*)), SIGNAL(changed()));
}

ConfigureIdentity::~ConfigureIdentity()
{
    delete mManager;
}

void ConfigureIdentity::init()
{
    mManager = new KPIMIdentities::IdentityManager( false, this, "mIdentityManager" );
    KPIMIdentities::IdentityManager::Iterator end( mManager->modifyEnd() );

    for ( KPIMIdentities::IdentityManager::Iterator it = mManager->modifyBegin(); it != end; ++it ) {
        QListWidgetItem *item = new QListWidgetItem(mListIdentity);
        item->setCheckState(Qt::Checked);
        item->setData(IdentityID, (*it).uoid());
        item->setText((*it).identityName());
    }
}

void ConfigureIdentity::readConfig(const QString &id)
{
    KSharedConfigPtr conf = ActivityManager::configFromActivity(id);
    if (conf->hasGroup(QLatin1String("identity"))) {
        KConfigGroup grp = conf->group(QLatin1String("identity"));
        const QStringList list = grp.readEntry(QLatin1String("NoActiveIdentity"), QStringList());
        const int numberOfItems(mListIdentity->count());
        for (int i = 0; i < numberOfItems; ++i) {
            QListWidgetItem *item = mListIdentity->item(i);
            if (list.contains(item->data(IdentityID).toString())) {
                item->setCheckState(Qt::Unchecked);
            } else {
                item->setCheckState(Qt::Checked);
            }
        }
    }
    emit changed(false);
}

void ConfigureIdentity::writeConfig(const QString &id)
{
    KSharedConfigPtr conf = ActivityManager::configFromActivity(id);
    KConfigGroup grp = conf->group(QLatin1String("identity"));
    const int numberOfItems(mListIdentity->count());
    QStringList lst;
    for (int i = 0; i < numberOfItems; ++i) {
        QListWidgetItem *item = mListIdentity->item(i);
        if (item->checkState() == Qt::Unchecked) {
            lst << item->data(IdentityID).toString();
        }
    }
    grp.writeEntry(QLatin1String("NoActiveIdentity"), lst);
}

void ConfigureIdentity::setDefault()
{
    const int numberOfItems(mListIdentity->count());
    for (int i = 0; i < numberOfItems; ++i) {
        QListWidgetItem *item = mListIdentity->item(i);
        item->setCheckState(Qt::Checked);
    }
}

}

