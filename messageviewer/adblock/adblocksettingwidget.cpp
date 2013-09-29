/* ============================================================
*
* This file is a part of the rekonq project
*
* Copyright (c) 2013 Montel Laurent <montel@kde.org>
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

// KDE Includes
#include <KSharedConfig>
#include <KStandardDirs>
#include <KIcon>
#include <KDebug>
#include <KMessageBox>

// Qt Includes
#include <QWhatsThis>
#include <QListWidgetItem>
#include <QFile>
#include <QPointer>
#include <QTextStream>

using namespace MessageViewer;
AdBlockSettingWidget::AdBlockSettingWidget(QWidget *parent)
    : QWidget(parent)
    , _changed(false)
{
    setupUi(this);

    hintLabel->setText(i18n("<qt>Filter expression (e.g. <tt>http://www.example.com/ad/*</tt>, <a href=\"filterhelp\">more information</a>):"));
    connect(hintLabel, SIGNAL(linkActivated(QString)), this, SLOT(slotInfoLinkActivated(QString)));

    manualFiltersListWidget->setSortingEnabled(true);
    manualFiltersListWidget->setSelectionMode(QAbstractItemView::MultiSelection);

    searchLine->setListWidget(manualFiltersListWidget);

    insertButton->setIcon(KIcon(QLatin1String("list-add")));
    connect(insertButton, SIGNAL(clicked()), this, SLOT(insertRule()));

    removeButton->setIcon(KIcon(QLatin1String("list-remove")));
    connect(removeButton, SIGNAL(clicked()), this, SLOT(removeRule()));
    connect(removeSubscription, SIGNAL(clicked()), SLOT(slotRemoveSubscription()));

    spinBox->setSuffix(ki18np(" day", " days"));

    removeSubscription->setEnabled(false);
    showList->setEnabled(false);
    // emit changed signal
    connect(checkEnableAdblock, SIGNAL(stateChanged(int)),   this, SLOT(hasChanged()));
    connect(checkHideAds,       SIGNAL(stateChanged(int)),   this, SLOT(hasChanged()));
    connect(spinBox,            SIGNAL(valueChanged(int)),   this, SLOT(hasChanged()));
    connect(addFilters, SIGNAL(clicked()), this, SLOT(slotAddFilter()));
    connect(showList, SIGNAL(clicked()), this, SLOT(slotShowList()));

    connect(automaticFiltersListWidget, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(hasChanged()));
    connect(automaticFiltersListWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(slotUpdateButtons()));

}

void AdBlockSettingWidget::slotUpdateButtons()
{
    const bool enabled = automaticFiltersListWidget->currentItem();
    removeSubscription->setEnabled(enabled);
    showList->setEnabled(enabled);
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
    if (rule.isEmpty())
        return;

    manualFiltersListWidget->addItem(rule);
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
    hasChanged();
}


void AdBlockSettingWidget::doLoadFromGlobalSettings()
{
    checkEnableAdblock->setChecked(GlobalSettings::self()->adBlockEnabled());

    // update enabled status
    tabWidget->setEnabled(GlobalSettings::self()->adBlockEnabled());
    checkHideAds->setChecked(GlobalSettings::self()->hideAdsEnabled());
    const int days = GlobalSettings::self()->adBlockUpdateInterval();
    spinBox->setValue(days);

    // ------------------------------------------------------------------------------

    // automatic filters
    KConfig config(QLatin1String("messagevieweradblockrc"));

    const QStringList itemList = config.groupList().filter( QRegExp( QLatin1String("FilterList \\d+") ) );
    Q_FOREACH(const QString &item, itemList) {
        KConfigGroup filtersGroup(&config, item);
        const bool isFilterEnabled = filtersGroup.readEntry(QLatin1String("FilterEnabled"), false);
        const QString url = filtersGroup.readEntry(QLatin1String("url"));
        const QString path = filtersGroup.readEntry(QLatin1String("path"));
        const QString name = filtersGroup.readEntry(QLatin1String("name"));
        const QDateTime lastUpdate = filtersGroup.readEntry(QLatin1String("lastUpdate"), QDateTime());
        if (url.isEmpty() || path.isEmpty() || name.isEmpty())
            continue;

        QListWidgetItem *subItem = new QListWidgetItem(automaticFiltersListWidget);
        subItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable| Qt::ItemIsSelectable);
        if (isFilterEnabled)
            subItem->setCheckState(Qt::Checked);
        else
            subItem->setCheckState(Qt::Unchecked);

        subItem->setData(UrlList, url);
        subItem->setData(PathList, path);
        subItem->setData(LastUpdateList, lastUpdate);
        subItem->setText(name);
    }

    // ------------------------------------------------------------------------------

    // local filters
    QString localRulesFilePath = KStandardDirs::locateLocal("appdata" , QLatin1String("adblockrules_local"));

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::ReadOnly | QFile::Text)) {
        kDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream in(&ruleFile);
    while (!in.atEnd()) {
        QString stringRule = in.readLine();
        QListWidgetItem *subItem = new QListWidgetItem(manualFiltersListWidget);
        subItem->setText(stringRule);
    }
}


void AdBlockSettingWidget::save()
{
    if (!_changed)
        return;

    // General settings    
    GlobalSettings::self()->setHideAdsEnabled(checkHideAds->isChecked());
    GlobalSettings::self()->setAdBlockEnabled(checkEnableAdblock->isChecked());
    GlobalSettings::self()->setAdBlockUpdateInterval(spinBox->value());

    // automatic filters
    KConfig config(QLatin1String("messagevieweradblockrc"));
    const QStringList list = config.groupList().filter( QRegExp( QLatin1String("MessageListTab\\d+") ) );
    foreach ( const QString &group, list ) {
        config.deleteGroup( group );
    }

    const int numberItem(automaticFiltersListWidget->count());
    for (int i = 0; i < numberItem; ++i) {
        QListWidgetItem *subItem = automaticFiltersListWidget->item(i);
        KConfigGroup grp = config.group(QString::fromLatin1("FilterList %1").arg(i));
        grp.writeEntry(QLatin1String("FilterEnabled"), subItem->checkState() == Qt::Checked);
        grp.writeEntry(QLatin1String("url"), subItem->data(UrlList).toString());
        grp.writeEntry(QLatin1String("name"), subItem->text());
        grp.writeEntry(QLatin1String("lastUpdate"), subItem->data(LastUpdateList).toDateTime());
        QString path = subItem->data(PathList).toString();
        if (path.isEmpty()) {
            path = KStandardDirs::locateLocal("appdata", QString::fromLatin1("adblockrules-%1").arg(i));
        }
        grp.writeEntry(QLatin1String("path"), path);
    }

    config.sync();
    // local filters
    const QString localRulesFilePath = KStandardDirs::locateLocal("appdata" , QLatin1String("adblockrules_local"));

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::WriteOnly | QFile::Text)) {
        kDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream out(&ruleFile);
    for (int i = 0; i < manualFiltersListWidget->count(); ++i) {
        QListWidgetItem *subItem = manualFiltersListWidget->item(i);
        const QString stringRule = subItem->text();
        out << stringRule << '\n';
    }

    // -------------------------------------------------------------------------------
    _changed = false;
    emit changed(false);
    AdBlockManager::self()->reloadConfig();
}


void AdBlockSettingWidget::hasChanged()
{
    // update enabled status
    checkHideAds->setEnabled(checkEnableAdblock->isChecked());
    tabWidget->setEnabled(checkEnableAdblock->isChecked());
    _changed = true;
    emit changed(true);
}


bool AdBlockSettingWidget::changed() const
{
    return _changed;
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
        subItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable);
        subItem->setCheckState(Qt::Checked);
        subItem->setText(name);
        subItem->setData(UrlList, url);
        subItem->setData(LastUpdateList, QDateTime());
        subItem->setData(PathList, QString());
    }
    delete dlg;
}

void AdBlockSettingWidget::slotRemoveSubscription()
{
    if (automaticFiltersListWidget->currentItem()) {
        if (KMessageBox::questionYesNo(this, i18n("Do you want to delete current list?"), i18n("Delete current list")) == KMessageBox::Yes) {
            QListWidgetItem *item = automaticFiltersListWidget->takeItem(automaticFiltersListWidget->currentRow());
            const QString path = item->data(PathList).toString();
            if (!path.isEmpty()) {
                if (!QFile(path).remove())
                    qDebug()<<" we can remove file:"<<path;
            }
            delete item;
        }
    }
}

void AdBlockSettingWidget::slotShowList()
{
    QListWidgetItem *item = automaticFiltersListWidget->currentItem();
    if (item) {
        QPointer<AdBlockShowListDialog> dlg = new AdBlockShowListDialog(this);
        dlg->setAdBlockListPath(item->data(PathList).toString(), item->data(UrlList).toString());
        dlg->exec();
        delete dlg;
    }
}

#include "adblocksettingwidget.moc"
