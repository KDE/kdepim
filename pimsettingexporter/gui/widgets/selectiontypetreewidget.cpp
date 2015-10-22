/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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
#include "pimsettingexporter/core/utils.h"
#include "pimsettingexporter/core/xml/templateselection.h"

#include "PimCommon/PimUtil"

#include <QFileDialog>

#include <KLocalizedString>

#include <QTreeWidgetItem>
#include <QHeaderView>
#include "gui/pimsettingexportgui_debug.h"
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

    connect(this, &SelectionTypeTreeWidget::itemChanged, this, &SelectionTypeTreeWidget::slotItemChanged);
}

QHash<Utils::AppsType, Utils::importExportParameters> SelectionTypeTreeWidget::storedType() const
{
    QHash<Utils::AppsType, Utils::importExportParameters> stored;
    Utils::importExportParameters var = typeChecked(mKmailItem);
    if (!var.isEmpty()) {
        stored.insert(Utils::KMail, var);
    }
    var = typeChecked(mKalarmItem);
    if (!var.isEmpty()) {
        stored.insert(Utils::KAlarm, var);
    }
    var = typeChecked(mKaddressbookItem);
    if (!var.isEmpty()) {
        stored.insert(Utils::KAddressBook, var);
    }
    var = typeChecked(mKorganizerItem);
    if (!var.isEmpty()) {
        stored.insert(Utils::KOrganizer, var);
    }
    var = typeChecked(mKNotesItem);
    if (!var.isEmpty()) {
        stored.insert(Utils::KNotes, var);
    }
    var = typeChecked(mAkregatorItem);
    if (!var.isEmpty()) {
        stored.insert(Utils::Akregator, var);
    }
    var = typeChecked(mBlogiloItem);
    if (!var.isEmpty()) {
        stored.insert(Utils::Blogilo, var);
    }
    return stored;
}

Utils::importExportParameters SelectionTypeTreeWidget::typeChecked(QTreeWidgetItem *parent) const
{
    Utils::importExportParameters parameters;
    int numberOfStep = 0;
    Utils::StoredTypes types = Utils::None;
    for (int i = 0; i < parent->childCount(); ++i) {
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
    case Utils::Identity: {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Identity));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Mails: {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Mails));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::MailTransport: {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::MailTransport));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Resources: {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Resources));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Config: {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Config));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::AkonadiDb: {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::AkonadiDb));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    case Utils::Data: {
        QTreeWidgetItem *item = new QTreeWidgetItem(parent);
        item->setText(0, Utils::storedTypeToI18n(Utils::Data));
        item->setCheckState(0, Qt::Checked);
        item->setData(0, action, type);
        break;
    }
    default:
        qCDebug(PIMSETTINGEXPORTERGUI_LOG) << " Type not supported: " << type;
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
    changeState(mKNotesItem, b);
    changeState(mAkregatorItem, b);
    changeState(mBlogiloItem, b);
}

void SelectionTypeTreeWidget::changeState(QTreeWidgetItem *item, bool b)
{
    blockSignals(true);
    item->setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
    for (int i = 0; i < item->childCount(); ++i) {
        item->child(i)->setCheckState(0, b ? Qt::Checked : Qt::Unchecked);
    }
    blockSignals(false);
}

void SelectionTypeTreeWidget::slotItemChanged(QTreeWidgetItem *item, int column)
{
    if (column != 0) {
        return;
    }
    //Parent
    if (item->childCount() != 0) {
        changeState(item, item->checkState(0) == Qt::Checked);
    } else { //child
        blockSignals(true);
        QTreeWidgetItem *parent = item->parent();
        Qt::CheckState state = Qt::PartiallyChecked;
        for (int i = 0; i < parent->childCount(); ++i) {
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

void SelectionTypeTreeWidget::loadFileName(const QString &fileName)
{
    unSelectAllItems();
    TemplateSelection templateSelection(fileName);
    const QHash<Utils::AppsType, Utils::importExportParameters> params = templateSelection.loadTemplate();
    setParameters(params);
}

void SelectionTypeTreeWidget::loadDefaultTemplate()
{
    QString ret = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QStringLiteral("/pimsettingexporter/defaulttemplate.xml"));
    if (!ret.isEmpty()) {
        loadFileName(ret);
    }
}

void SelectionTypeTreeWidget::loadTemplate(const QString &fileName)
{
    if (fileName.isEmpty()) {
        QPointer<QFileDialog> dlg = new QFileDialog(this, QString(), QString(), QStringLiteral("*.xml"));
        dlg->setFileMode(QFileDialog::ExistingFile);
        if (dlg->exec()) {
            const QStringList file = dlg->selectedFiles();
            loadFileName(file.at(0));
        }
        delete dlg;
    } else {
        loadFileName(fileName);
    }
}

void SelectionTypeTreeWidget::saveAsDefaultTemplate()
{
    TemplateSelection templateSelection;
    templateSelection.createTemplate(storedType());
    const QString templateStr = templateSelection.document().toString(2);
    QString ret = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QStringLiteral("pimsettingexporter/");
    QFileInfo fileInfo(ret);
    QDir().mkpath(fileInfo.absolutePath());
    qDebug()<<" ret :"<<ret;

    PimCommon::Util::saveToFile(ret + QStringLiteral("defaulttemplate.xml"),templateStr);
}

void SelectionTypeTreeWidget::saveAsTemplate()
{
    TemplateSelection templateSelection;
    templateSelection.createTemplate(storedType());
    const QString templateStr = templateSelection.document().toString(2);
    const QString filter(i18n("Template Files (*.xml)"));
    PimCommon::Util::saveTextAs(templateStr, filter, this);
}

void SelectionTypeTreeWidget::initializeSubItem(QTreeWidgetItem *item, Utils::StoredTypes types)
{
    for (int i = 0; i < item->childCount(); ++i) {
        QTreeWidgetItem *child = item->child(i);
        if (types & static_cast<Utils::StoredType>(child->data(0, action).toInt())) {
            child->setCheckState(0, Qt::Checked);
        } else {
            child->setCheckState(0, Qt::Unchecked);
        }
    }
}

void SelectionTypeTreeWidget::setParameters(const QHash<Utils::AppsType, Utils::importExportParameters> &params)
{
    QHash<Utils::AppsType, Utils::importExportParameters>::const_iterator i = params.constBegin();
    while (i != params.constEnd())  {
        switch (i.key()) {
        case Utils::KMail: {
            initializeSubItem(mKmailItem, i.value().types);
            break;
        }
        case Utils::KAddressBook: {
            initializeSubItem(mKaddressbookItem, i.value().types);
            break;
        }
        case Utils::KAlarm: {
            initializeSubItem(mKalarmItem, i.value().types);
            break;
        }
        case Utils::KOrganizer: {
            initializeSubItem(mKorganizerItem, i.value().types);
            break;
        }
        case Utils::KNotes: {
            initializeSubItem(mKNotesItem, i.value().types);
            break;
        }
        case Utils::Akregator: {
            initializeSubItem(mAkregatorItem, i.value().types);
            break;
        }
        case Utils::Blogilo: {
            initializeSubItem(mBlogiloItem, i.value().types);
            break;
        }
        case Utils::Unknown: {
            break;
        }
        }
        ++i;
    }
}
