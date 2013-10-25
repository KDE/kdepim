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


#include "selectiontypetreewidget.h"

#include <KLocale>

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <KDebug>

SelectionTypeTreeWidget::SelectionTypeTreeWidget(QWidget *parent)
    : QTreeWidget(parent)
{
    initialize();
    expandAll();
    header()->hide();
}

SelectionTypeTreeWidget::~SelectionTypeTreeWidget()
{
}

void SelectionTypeTreeWidget::initialize()
{
    mKmailItem = new QTreeWidgetItem(this);
    mKmailItem->setText(0, i18n("KMail"));
    createSubItem(mKmailItem, Utils::Identity);
    createSubItem(mKmailItem, Utils::Mails);
    createSubItem(mKmailItem, Utils::MailTransport);
    createSubItem(mKmailItem, Utils::Resources);
    createSubItem(mKmailItem, Utils::Config);
    createSubItem(mKmailItem, Utils::AkonadiDb);

    mKaddressbookItem = new QTreeWidgetItem(this);
    mKaddressbookItem->setText(0, i18n("KAddressBook"));
    createSubItem(mKaddressbookItem, Utils::Resources);
    createSubItem(mKaddressbookItem, Utils::Config);

    mKalarmItem = new QTreeWidgetItem(this);
    mKalarmItem->setText(0, i18n("KAlarm"));
    createSubItem(mKalarmItem, Utils::Resources);
    createSubItem(mKalarmItem, Utils::Config);

    mKorganizerItem = new QTreeWidgetItem(this);
    mKorganizerItem->setText(0, i18n("KOrganizer"));
    createSubItem(mKorganizerItem, Utils::Resources);
    createSubItem(mKorganizerItem, Utils::Config);

    mKjotsItem = new QTreeWidgetItem(this);
    mKjotsItem->setText(0, i18n("KJots"));
    createSubItem(mKjotsItem, Utils::Resources);
    createSubItem(mKjotsItem, Utils::Config);

    mKNotesItem = new QTreeWidgetItem(this);
    mKNotesItem->setText(0, i18n("KNotes"));
    createSubItem(mKNotesItem, Utils::Config);
    createSubItem(mKNotesItem, Utils::Data);

    mAkregatorItem = new QTreeWidgetItem(this);
    mAkregatorItem->setText(0, i18n("Akregator"));
    createSubItem(mAkregatorItem, Utils::Config);
    createSubItem(mAkregatorItem, Utils::Data);

    mBlogiloItem = new QTreeWidgetItem(this);
    mBlogiloItem->setText(0, i18n("Blogilo"));
    createSubItem(mBlogiloItem, Utils::Config);
    createSubItem(mBlogiloItem, Utils::Data);
}

QHash<Utils::AppsType, Utils::importExportParameters> SelectionTypeTreeWidget::storedType() const
{
    QHash<Utils::AppsType, Utils::importExportParameters> stored;
    Utils::importExportParameters var = typeChecked(mKmailItem);
    if (!var.isEmpty())
        stored.insert(Utils::KMail, var);
    var = typeChecked(mKalarmItem);
    if (!var.isEmpty())
        stored.insert(Utils::KAlarm, var);
    var = typeChecked(mKaddressbookItem);
    if (!var.isEmpty())
        stored.insert(Utils::KAddressBook, var);
    var = typeChecked(mKorganizerItem);
    if (!var.isEmpty())
        stored.insert(Utils::KOrganizer, var);
    var = typeChecked(mKjotsItem);
    if (!var.isEmpty())
        stored.insert(Utils::KJots, var);
    var = typeChecked(mKNotesItem);
    if (!var.isEmpty())
        stored.insert(Utils::KNotes, var);
    var = typeChecked(mAkregatorItem);
    if (!var.isEmpty())
        stored.insert(Utils::Akregator, var);
    var = typeChecked(mBlogiloItem);
    if (!var.isEmpty())
        stored.insert(Utils::Blogilo, var);
    return stored;
}

Utils::importExportParameters SelectionTypeTreeWidget::typeChecked(QTreeWidgetItem *parent) const
{
    Utils::importExportParameters parameters;
    int numberOfStep = 0;
    Utils::StoredTypes types = Utils::None;
    for (int i = 0; i<parent->childCount(); ++i) {
        QTreeWidgetItem *item = parent->child(i);
        if (item->checkState(0) == Qt::Checked) {
            types |= static_cast<Utils::StoredType>(item->data(0, action).toInt());
            ++numberOfStep;
        }
    }
    parameters.types = types;
    parameters.numberSteps = numberOfStep;
    return parameters;
}


void SelectionTypeTreeWidget::createSubItem(QTreeWidgetItem *parent, Utils::StoredType type)
{
    switch (type) {
    case Utils::None:
        break;
    case Utils::Identity:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Identity"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Mails:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Mails"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    case Utils::MailTransport:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Mail Transport"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Resources:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Resources"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Config:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Config"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    case Utils::AkonadiDb:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Akonadi Database"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Nepomuk:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Nepomuk Database"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Data:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, i18n("Data"));
        item->setCheckState(0, Qt::Unchecked);
        item->setData(0, action, type);
        break;
    }
    default:
        kDebug()<<" Type not supported: "<<type;
        break;
    }
}

#include "selectiontypetreewidget.moc"
