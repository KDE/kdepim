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


#include "selectiontypetreewidget.h"
#include "pimsettingexporter/utils.h"
#include "xml/templateselection.h"

#include "pimcommon/util/pimutil.h"

#include <KDebug>
#include <KFileDialog>

#include <QTreeWidgetItem>
#include <QHeaderView>
#include <QPointer>

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
    mKmailItem->setCheckState(0, Qt::Checked);
    mKmailItem->setText(0, Utils::appTypeToI18n(Utils::KMail));
    createSubItem(mKmailItem, Utils::Identity);
    createSubItem(mKmailItem, Utils::Mails);
    createSubItem(mKmailItem, Utils::MailTransport);
    createSubItem(mKmailItem, Utils::Resources);
    createSubItem(mKmailItem, Utils::Config);
    //createSubItem(mKmailItem, Utils::AkonadiDb);

    mKaddressbookItem = new QTreeWidgetItem(this);
    mKaddressbookItem->setText(0, Utils::appTypeToI18n(Utils::KAddressBook));
    mKaddressbookItem->setCheckState(0, Qt::Checked);
    createSubItem(mKaddressbookItem, Utils::Resources);
    createSubItem(mKaddressbookItem, Utils::Config);

    mKalarmItem = new QTreeWidgetItem(this);
    mKalarmItem->setText(0, Utils::appTypeToI18n(Utils::KAlarm));
    mKalarmItem->setCheckState(0, Qt::Checked);
    createSubItem(mKalarmItem, Utils::Resources);
    createSubItem(mKalarmItem, Utils::Config);

    mKorganizerItem = new QTreeWidgetItem(this);
    mKorganizerItem->setText(0, Utils::appTypeToI18n(Utils::KOrganizer));
    mKorganizerItem->setCheckState(0, Qt::Checked);
    createSubItem(mKorganizerItem, Utils::Resources);
    createSubItem(mKorganizerItem, Utils::Config);

    mKjotsItem = new QTreeWidgetItem(this);
    mKjotsItem->setText(0, Utils::appTypeToI18n(Utils::KJots));
    mKjotsItem->setCheckState(0, Qt::Checked);
    createSubItem(mKjotsItem, Utils::Resources);
    createSubItem(mKjotsItem, Utils::Config);

    mKNotesItem = new QTreeWidgetItem(this);
    mKNotesItem->setText(0, Utils::appTypeToI18n(Utils::KNotes));
    mKNotesItem->setCheckState(0, Qt::Checked);
    createSubItem(mKNotesItem, Utils::Config);
    createSubItem(mKNotesItem, Utils::Data);

    mAkregatorItem = new QTreeWidgetItem(this);
    mAkregatorItem->setText(0, Utils::appTypeToI18n(Utils::Akregator));
    mAkregatorItem->setCheckState(0, Qt::Checked);
    createSubItem(mAkregatorItem, Utils::Config);
    createSubItem(mAkregatorItem, Utils::Data);

    mBlogiloItem = new QTreeWidgetItem(this);
    mBlogiloItem->setText(0, Utils::appTypeToI18n(Utils::Blogilo));
    mBlogiloItem->setCheckState(0, Qt::Checked);
    createSubItem(mBlogiloItem, Utils::Config);
    createSubItem(mBlogiloItem, Utils::Data);

    mKNodeItem = new QTreeWidgetItem(this);
    mKNodeItem->setText(0, Utils::appTypeToI18n(Utils::KNode));
    mKNodeItem->setCheckState(0, Qt::Checked);
    createSubItem(mKNodeItem, Utils::Config);
    createSubItem(mKNodeItem, Utils::Data);


    connect(this, SIGNAL(itemChanged(QTreeWidgetItem*,int)), SLOT(slotItemChanged(QTreeWidgetItem*,int)));
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
    var = typeChecked(mKNodeItem);
    if (!var.isEmpty())
        stored.insert(Utils::KNode, var);
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
        item->setText(0, Utils::storedTypeToI18n(Utils::Identity));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Mails:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Mails));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::MailTransport:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::MailTransport));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Resources:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Resources));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Config:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Config));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::AkonadiDb:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::AkonadiDb));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Data:
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Data));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    default:
        kDebug()<<" Type not supported: "<<type;
        break;
    }
}

void SelectionTypeTreeWidget::selectAllItems()
{
    setSelectItems(true);
}

void SelectionTypeTreeWidget::unSelectAllItems()
{
    setSelectItems(false);
}

void SelectionTypeTreeWidget::setSelectItems(bool b)
{
    changeState(mKmailItem, b);
    changeState(mKalarmItem, b);
    changeState(mKaddressbookItem, b);
    changeState(mKorganizerItem, b);
    changeState(mKjotsItem, b);
    changeState(mKNotesItem, b);
    changeState(mAkregatorItem, b);
    changeState(mBlogiloItem, b);
    changeState(mKNodeItem, b);
}

void SelectionTypeTreeWidget::changeState(QTreeWidgetItem *item, bool b)
{
    blockSignals(true);
    item->setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
    for (int i=0; i < item->childCount(); ++i) {
        item->child(i)->setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
    }
    blockSignals(false);
}

void SelectionTypeTreeWidget::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (column!=0)
        return;
    //Parent
    if (item->childCount()!=0) {
        changeState(item, item->checkState(0) == Qt::Checked);
    } else { //child
        blockSignals(true);
        QTreeWidgetItem *parent = item->parent();
        Qt::CheckState state = Qt::PartiallyChecked;
        for (int i=0; i < parent->childCount(); ++i) {
            if (i == 0) {
                state = parent->child(i)->checkState(0);
            } else {
                if (state != parent->child(i)->checkState(0)) {
                    state = Qt::PartiallyChecked;
                    break;
                }
            }
        }
        parent->setCheckState(0, state);
        blockSignals(false);
    }
}

void SelectionTypeTreeWidget::loadTemplate()
{
    QPointer<KFileDialog> dlg = new KFileDialog(KUrl(), QLatin1String("*.xml"), this);
    dlg->setMode(KFile::File);
    if (dlg->exec()) {
        const QString file = dlg->selectedFile();
        TemplateSelection templateSelection(file);
        const QHash<Utils::AppsType, Utils::importExportParameters> params = templateSelection.loadTemplate();
    }
    delete dlg;

}

void SelectionTypeTreeWidget::saveAsTemplate()
{
    TemplateSelection templateSelection;
    templateSelection.createTemplate(storedType());
    const QString templateStr = templateSelection.document().toString(2);
    const QString filter(QLatin1String("*.xml"));
    PimCommon::Util::saveTextAs(templateStr, filter, this);
}

void SelectionTypeTreeWidget::setParameters(const QHash<Utils::AppsType, Utils::importExportParameters> &params)
{
    QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator i = params.constBegin();
    while (i != params.constEnd())  {
        switch(i.key()) {
        case Utils::KMail: {
            break;
        }
        case Utils::KAddressBook: {
            break;
        }
        case Utils::KAlarm: {
            break;
        }
        case Utils::KOrganizer: {
            break;
        }
        case Utils::KJots: {
            break;
        }
        case Utils::KNotes: {
            break;
        }
        case Utils::Akregator: {
            break;
        }
        case Utils::Blogilo: {
            break;
        }
        case Utils::KNode: {
            break;
        }
        }
        ++i;
    }

    //TODO
}
