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
#include "configuremailtransport.h"

#include "mailtransport/transportmanager.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

namespace PimActivity {
ConfigureMailtransport::ConfigureMailtransport(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mListTransport = new QListWidget;
    lay->addWidget(mListTransport);
    init();
    setLayout(lay);
    connect(mListTransport, SIGNAL(itemChanged(QListWidgetItem*)), SIGNAL(changed()));
}

ConfigureMailtransport::~ConfigureMailtransport()
{
}

void ConfigureMailtransport::init()
{
    QStringList listNames = MailTransport::TransportManager::self()->transportNames();
    QList<int> listIds = MailTransport::TransportManager::self()->transportIds();
    int i = 0;
    Q_FOREACH (const QString &name, listNames) {
        QListWidgetItem *item = new QListWidgetItem(mListTransport);
        item->setCheckState(Qt::Checked);
        //TODO check it or not.
        item->setText(name);
        item->setData(TransportID, listIds.at(i));
        ++i;
    }

}

void ConfigureMailtransport::readConfig(const QString &id)
{
    KSharedConfigPtr conf = configFromActivity(id);
    if (conf->hasGroup(QLatin1String("mailtransport"))) {
        KConfigGroup grp = conf->group(QLatin1String("mailtransport"));
        const QStringList list = grp.readEntry(QLatin1String("ActiveMailTransport"), QStringList());
        const int numberOfItems(mListTransport->count());
        for (int i = 0; i < numberOfItems; ++i) {
            QListWidgetItem *item = mListTransport->item(i);
            if (list.contains(item->data(TransportID).toString())) {
                item->setCheckState(Qt::Checked);
            } else {
                item->setCheckState(Qt::Unchecked);
            }
        }
    }
}

void ConfigureMailtransport::writeConfig(const QString &id)
{
    KSharedConfigPtr conf = configFromActivity(id);
    KConfigGroup grp = conf->group(QLatin1String("mailtransport"));
    const int numberOfItems(mListTransport->count());
    QStringList lst;
    for (int i = 0; i < numberOfItems; ++i) {
        QListWidgetItem *item = mListTransport->item(i);
        if (item->checkState() == Qt::Checked) {
            lst << item->data(TransportID).toString();
        }
    }
    grp.writeEntry(QLatin1String("ActiveMailTransport"), lst);
}

}

#include "configuremailtransport.moc"
