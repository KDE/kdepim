/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>
* based on code from rekonq
* Copyright (C) 2010-2012 by Andrea Diamantini <adjam7 at gmail dot com>
*
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License or (at your option) version 3 or any later version
* accepted by the membership of KDE e.V. (or its successor approved
* by the membership of KDE e.V.), which shall act as a proxy
* defined in Section 14 of version 3 of the license.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
* ============================================================ */

// Self Includes
#include "adblocksettingwidget.h"
#include "settings/globalsettings.h"
#include "adblockaddsubscriptiondialog.h"
#include "adblockmanager.h"
#include "adblockshowlistdialog.h"
#include "adblock/adblockutil.h"
#include "pimcommon/widgets/configureimmutablewidgetutils.h"
using namespace PimCommon::ConfigureImmutableWidgetUtils;

#include "pimcommon/util/pimutil.h"

// KDE Includes
#include <KSharedConfig>

#include <QIcon>
#include <QDebug>
#include <KMessageBox>
#include <KLocalizedString>

// Qt Includes
#include <QWhatsThis>
#include <QListWidgetItem>
#include <QFile>
#include <QPointer>
#include <QTextStream>
#include <QStandardPaths>

using namespace MessageViewer;
AdBlockSettingWidget::AdBlockSettingWidget(QWidget *parent)
    : QWidget(parent)
    , mChanged(false)
{
    setupUi(this);

    hintLabel->setText(i18n("<qt>Filter expression (e.g. <tt>http://www.example.com/ad/*</tt>, <a href=\"filterhelp\">more information</a>):"));
    connect(hintLabel, &QLabel::linkActivated, this, &AdBlockSettingWidget::slotInfoLinkActivated);

    manualFiltersListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    searchLine->setListWidget(manualFiltersListWidget);

    insertButton->setIcon(QIcon::fromTheme(QLatin1String("list-add")));
    connect(insertButton, &QToolButton::clicked, this, &AdBlockSettingWidget::insertRule);

    removeButton->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
    connect(removeButton, &QPushButton::clicked, this, &AdBlockSettingWidget::removeRule);
    connect(removeSubscription, &QPushButton::clicked, this, &AdBlockSettingWidget::slotRemoveSubscription);
    connect(manualFiltersListWidget, &QListWidget::currentItemChanged, this, &AdBlockSettingWidget::slotUpdateManualButtons);

    spinBox->setSuffix(ki18np(" day", " days"));

    removeSubscription->setEnabled(false);
    showList->setEnabled(false);
    // emit changed signal
    connect(checkEnableAdblock, &QCheckBox::stateChanged, this, &AdBlockSettingWidget::hasChanged);
    connect(checkHideAds, &QCheckBox::stateChanged, this, &AdBlockSettingWidget::hasChanged);
    connect(spinBox, static_cast<void (KPluralHandlingSpinBox::*)(int)>(&KPluralHandlingSpinBox::valueChanged), this, &AdBlockSettingWidget::hasChanged);
    connect(addFilters, &QPushButton::clicked, this, &AdBlockSettingWidget::slotAddFilter);
    connect(showList, &QPushButton::clicked, this, &AdBlockSettingWidget::slotShowList);
    connect(editFilter, &QPushButton::clicked, this, &AdBlockSettingWidget::slotEditFilter);

    connect(automaticFiltersListWidget, &MessageViewer::AdBlockListWidget::itemChanged, this, &AdBlockSettingWidget::hasChanged);
    connect(automaticFiltersListWidget, &MessageViewer::AdBlockListWidget::currentItemChanged, this, &AdBlockSettingWidget::slotUpdateButtons);
    connect(automaticFiltersListWidget, &MessageViewer::AdBlockListWidget::itemDoubleClicked, this, &AdBlockSettingWidget::slotAutomaticFilterDouble);

    connect(importFilters, &QPushButton::clicked, this, &AdBlockSettingWidget::slotImportFilters);
    connect(exportFilters, &QPushButton::clicked, this, &AdBlockSettingWidget::slotExportFilters);
    connect(addFilterLineEdit, &QLineEdit::textChanged, this, &AdBlockSettingWidget::slotManualFilterLineEditTextChanged);
    slotUpdateManualButtons();
    insertButton->setEnabled(false);
}

void AdBlockSettingWidget::slotManualFilterLineEditTextChanged(const QString &text)
{
    insertButton->setEnabled(!text.isEmpty());
}

void AdBlockSettingWidget::slotEditFilter()
{
    QListWidgetItem *item = manualFiltersListWidget->currentItem();
    if (item) {
        manualFiltersListWidget->editItem(item);
    }
}

void AdBlockSettingWidget::slotUpdateButtons()
{
    const bool enabled = automaticFiltersListWidget->currentItem();
    removeSubscription->setEnabled(enabled);
    showList->setEnabled(enabled);
}

void AdBlockSettingWidget::slotUpdateManualButtons()
{
    const bool enabled = manualFiltersListWidget->currentItem();
    removeButton->setEnabled(enabled);
    editFilter->setEnabled(enabled);
    exportFilters->setEnabled(manualFiltersListWidget->count() > 0);
}

void AdBlockSettingWidget::slotInfoLinkActivated(const QString &url)
{
    Q_UNUSED(url)

    const QString hintHelpString = i18n("<qt><p>Enter an expression to filter. Filters can be defined as either:"
                                        "<ul><li>a shell-style wildcard, e.g. <tt>http://www.example.com/ads*</tt>, "
                                        "the wildcards <tt>*?[]</tt> may be used</li>"
                                        "<li>a full regular expression by surrounding the string with '<tt>/</tt>', "
                                        "e.g. <tt>/\\/(ad|banner)\\./</tt></li></ul>"
                                        "<p>Any filter string can be preceded by '<tt>@@</tt>' to whitelist (allow) any matching URL, "
                                        "which takes priority over any blacklist (blocking) filter.");

    QWhatsThis::showText(QCursor::pos(), hintHelpString);
}

void AdBlockSettingWidget::insertRule()
{
    const QString rule = addFilterLineEdit->text();
    if (rule.isEmpty()) {
        return;
    }
    const int numberItem(manualFiltersListWidget->count());
    for (int i = 0; i < numberItem; ++i) {
        if (manualFiltersListWidget->item(i)->text() == rule) {
            addFilterLineEdit->clear();
            return;
        }
    }

    addManualFilter(rule);
    exportFilters->setEnabled(manualFiltersListWidget->count() > 0);
    addFilterLineEdit->clear();
    hasChanged();
}

void AdBlockSettingWidget::removeRule()
{
    QList<QListWidgetItem *> select = manualFiltersListWidget->selectedItems();
    if (select.isEmpty()) {
        return;
    }
    Q_FOREACH (QListWidgetItem *item, select) {
        delete item;
    }
    exportFilters->setEnabled(manualFiltersListWidget->count() > 0);
    hasChanged();
}

void AdBlockSettingWidget::doResetToDefaultsOther()
{
    const bool bUseDefaults = MessageViewer::GlobalSettings::self()->useDefaults(true);
    loadWidget(checkEnableAdblock, GlobalSettings::self()->adBlockEnabledItem());
    tabWidget->setEnabled(GlobalSettings::self()->adBlockEnabled());
    saveCheckBox(checkHideAds, GlobalSettings::self()->hideAdsEnabledItem());
    loadWidget(spinBox, GlobalSettings::self()->adBlockUpdateIntervalItem());
    MessageViewer::GlobalSettings::self()->useDefaults(bUseDefaults);
}

void AdBlockSettingWidget::doLoadFromGlobalSettings()
{
    manualFiltersListWidget->clear();
    automaticFiltersListWidget->clear();
    loadWidget(checkEnableAdblock, GlobalSettings::self()->adBlockEnabledItem());

    // update enabled status
    tabWidget->setEnabled(GlobalSettings::self()->adBlockEnabled());
    loadWidget(checkHideAds, GlobalSettings::self()->hideAdsEnabledItem());
    loadWidget(spinBox, GlobalSettings::self()->adBlockUpdateIntervalItem());

    // ------------------------------------------------------------------------------

    // automatic filters
    KConfig config(QLatin1String("messagevieweradblockrc"));

    const QStringList itemList = config.groupList().filter(QRegExp(QLatin1String("FilterList \\d+")));
    Q_FOREACH (const QString &item, itemList) {
        KConfigGroup filtersGroup(&config, item);
        const bool isFilterEnabled = filtersGroup.readEntry(QLatin1String("FilterEnabled"), false);
        const QString url = filtersGroup.readEntry(QLatin1String("url"));
        const QString path = filtersGroup.readEntry(QLatin1String("path"));
        const QString name = filtersGroup.readEntry(QLatin1String("name"));
        const QDateTime lastUpdate = filtersGroup.readEntry(QLatin1String("lastUpdate"), QDateTime());
        if (url.isEmpty() || path.isEmpty() || name.isEmpty()) {
            continue;
        }

        QListWidgetItem *subItem = new QListWidgetItem(automaticFiltersListWidget);
        subItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        if (isFilterEnabled) {
            subItem->setCheckState(Qt::Checked);
        } else {
            subItem->setCheckState(Qt::Unchecked);
        }

        subItem->setData(UrlList, url);
        subItem->setData(PathList, path);
        subItem->setData(LastUpdateList, lastUpdate);
        subItem->setText(name);
    }

    // ------------------------------------------------------------------------------

    // local filters
    const QString localRulesFilePath = MessageViewer::AdBlockUtil::localFilterPath();

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::ReadOnly | QFile::Text)) {
        qDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream in(&ruleFile);
    while (!in.atEnd()) {
        QString stringRule = in.readLine();
        addManualFilter(stringRule);
    }
}

void AdBlockSettingWidget::save()
{
    if (!mChanged) {
        return;
    }

    // General settings
    saveCheckBox(checkEnableAdblock, GlobalSettings::self()->adBlockEnabledItem());
    saveCheckBox(checkHideAds, GlobalSettings::self()->hideAdsEnabledItem());
    saveSpinBox(spinBox, GlobalSettings::self()->adBlockUpdateIntervalItem());

    // automatic filters
    KConfig config(QLatin1String("messagevieweradblockrc"));
    const QStringList list = config.groupList().filter(QRegExp(QLatin1String("FilterList \\d+")));
    foreach (const QString &group, list) {
        config.deleteGroup(group);
    }

    const int numberItem(automaticFiltersListWidget->count());
    for (int i = 0; i < numberItem; ++i) {
        QListWidgetItem *subItem = automaticFiltersListWidget->item(i);
        KConfigGroup grp = config.group(QString::fromLatin1("FilterList %1").arg(i));
        grp.writeEntry(QLatin1String("FilterEnabled"), subItem->checkState() == Qt::Checked);
        grp.writeEntry(QLatin1String("url"), subItem->data(UrlList).toString());
        grp.writeEntry(QLatin1String("name"), subItem->text());
        if (subItem->data(LastUpdateList).toDateTime().isValid()) {
            grp.writeEntry(QLatin1String("lastUpdate"), subItem->data(LastUpdateList).toDateTime());
        }
        QString path = subItem->data(PathList).toString();
        if (path.isEmpty()) {
            path = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QLatin1Char('/') + QString::fromLatin1("kmail2/adblockrules-%1").arg(i);
        }
        grp.writeEntry(QLatin1String("path"), path);
    }

    config.sync();
    // local filters
    const QString localRulesFilePath = MessageViewer::AdBlockUtil::localFilterPath();

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::WriteOnly | QFile::Text)) {
        qDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream out(&ruleFile);
    for (int i = 0; i < manualFiltersListWidget->count(); ++i) {
        QListWidgetItem *subItem = manualFiltersListWidget->item(i);
        const QString stringRule = subItem->text();
        if (!stringRule.trimmed().isEmpty()) {
            out << stringRule << '\n';
        }
    }

    // -------------------------------------------------------------------------------
    mChanged = false;
    emit changed(false);
    AdBlockManager::self()->reloadConfig();
}

void AdBlockSettingWidget::hasChanged()
{
    // update enabled status
    checkHideAds->setEnabled(checkEnableAdblock->isChecked());
    tabWidget->setEnabled(checkEnableAdblock->isChecked());
    mChanged = true;
    emit changed(true);
}

bool AdBlockSettingWidget::changed() const
{
    return mChanged;
}

void AdBlockSettingWidget::slotAddFilter()
{
    QStringList excludeList;
    const int numberItem(automaticFiltersListWidget->count());
    for (int i = 0; i < numberItem; ++i) {
        excludeList << automaticFiltersListWidget->item(i)->text();
    }
    QPointer<MessageViewer::AdBlockAddSubscriptionDialog> dlg = new MessageViewer::AdBlockAddSubscriptionDialog(excludeList, this);
    if (dlg->exec()) {
        QString name;
        QString url;
        dlg->selectedList(name, url);
        QListWidgetItem *subItem = new QListWidgetItem(automaticFiltersListWidget);
        subItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        subItem->setCheckState(Qt::Checked);
        subItem->setText(name);
        subItem->setData(UrlList, url);
        subItem->setData(LastUpdateList, QDateTime());
        subItem->setData(PathList, QString());
        hasChanged();
    }
    delete dlg;
}

void AdBlockSettingWidget::slotRemoveSubscription()
{
    QListWidgetItem *item = automaticFiltersListWidget->currentItem();
    if (item) {
        if (KMessageBox::questionYesNo(this, i18n("Do you want to delete list \"%1\"?", item->text()), i18n("Delete current list")) == KMessageBox::Yes) {
            const QString path = item->data(PathList).toString();
            if (!path.isEmpty()) {
                if (!QFile(path).remove()) {
                    qDebug() << " we can not remove file:" << path;
                }
            }
            delete item;
        }
        hasChanged();
    }
}

void AdBlockSettingWidget::slotShowList()
{
    showAutomaticFilterList(automaticFiltersListWidget->currentItem());
}

void AdBlockSettingWidget::showAutomaticFilterList(QListWidgetItem *item)
{
    if (item) {
        QPointer<AdBlockShowListDialog> dlg = new AdBlockShowListDialog(this);
        dlg->setListName(item->text());
        dlg->setAdBlockListPath(item->data(PathList).toString(), item->data(UrlList).toString());
        connect(dlg.data(), &AdBlockShowListDialog::deleteList, this, &AdBlockSettingWidget::slotDeleteList);
        dlg->exec();
        delete dlg;
    }
}

void AdBlockSettingWidget::slotDeleteList(const QString &listName)
{
    QListWidgetItem *item = automaticFiltersListWidget->currentItem();
    if (item && item->text() == listName) {
        const QString path = item->data(PathList).toString();
        if (!path.isEmpty()) {
            if (!QFile(path).remove()) {
                qDebug() << " we can not remove file:" << path;
            }
        }
        delete item;
        hasChanged();
    }
}

void AdBlockSettingWidget::slotImportFilters()
{
    const QString filter = i18n("*|all files (*)");
    const QString result = PimCommon::Util::loadToFile(filter, this, i18n("Import Filters"));
    if (result.isEmpty()) {
        return;
    }
    const QStringList listFilter = result.split(QLatin1Char('\n'));
    QStringList excludeFilter;
    const int numberOfElement(manualFiltersListWidget->count());
    for (int i = 0; i < numberOfElement; ++i) {
        QListWidgetItem *subItem = manualFiltersListWidget->item(i);
        excludeFilter.append(subItem->text());
    }

    Q_FOREACH (const QString &element, listFilter) {
        if (element == QLatin1String("\n")) {
            continue;
        }
        if (excludeFilter.contains(element)) {
            continue;
        }
        addManualFilter(element);
    }
}

void AdBlockSettingWidget::addManualFilter(const QString &text)
{
    QListWidgetItem *subItem = new QListWidgetItem(manualFiltersListWidget);
    subItem->setFlags(subItem->flags() | Qt::ItemIsEditable);
    subItem->setText(text);
}

void AdBlockSettingWidget::slotExportFilters()
{
    const QString filter = i18n("*|all files (*)");
    QString exportFilters;
    const int numberOfElement(manualFiltersListWidget->count());
    for (int i = 0; i < numberOfElement; ++i) {
        QListWidgetItem *subItem = manualFiltersListWidget->item(i);
        const QString stringRule = subItem->text();
        if (!stringRule.isEmpty()) {
            exportFilters += stringRule + QLatin1Char('\n');
        }
    }
    PimCommon::Util::saveTextAs(exportFilters, filter, this, QUrl(), i18n("Export Filters"));
}

void AdBlockSettingWidget::slotAutomaticFilterDouble(QListWidgetItem *item)
{
    showAutomaticFilterList(item);
}

