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

#include "adblockblockableitemswidget.h"
#include "adblockcreatefilterdialog.h"
#include "settings/globalsettings.h"
#include "adblock/adblockmanager.h"

#include <KLocale>
#include <KTreeWidgetSearchLine>
#include <KMenu>
#include <KConfigGroup>
#include <KStandardDirs>
#include <KRun>

#include <QHeaderView>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebElement>
#include <QDebug>
#include <QPointer>
#include <QClipboard>
#include <QApplication>
#include <QFile>

using namespace MessageViewer;

AdBlockBlockableItemsWidget::AdBlockBlockableItemsWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    mListItems = new QTreeWidget;

    mListItems->setContextMenuPolicy(Qt::CustomContextMenu);
    mListItems->setAlternatingRowColors(true);
    mListItems->setRootIsDecorated(false);
    connect(mListItems, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(customContextMenuRequested(QPoint)));

    QStringList lst;
    lst << i18n("Filter") << i18n("Address") << i18n("Type");
    mListItems->setHeaderLabels(lst);

    KTreeWidgetSearchLine *searchLine = new KTreeWidgetSearchLine(this, mListItems);
    searchLine->setPlaceholderText(i18n("Search..."));

    lay->addWidget(searchLine);
    lay->addWidget(mListItems);

    KConfigGroup config( MessageViewer::GlobalSettings::self()->config(),"AdBlockHeaders");
    mListItems->header()->restoreState(config.readEntry("HeaderState",QByteArray()));
}

AdBlockBlockableItemsWidget::~AdBlockBlockableItemsWidget()
{
    KConfigGroup groupHeader( MessageViewer::GlobalSettings::self()->config(),"AdBlockHeaders" );
    groupHeader.writeEntry( "HeaderState", mListItems->header()->saveState());
    groupHeader.sync();
}

void AdBlockBlockableItemsWidget::setWebFrame(QWebFrame *frame)
{
    mListItems->clear();
    searchBlockableElement(frame);
}

void AdBlockBlockableItemsWidget::searchBlockableElement(QWebFrame *frame)
{
    const QWebElementCollection images = frame->findAllElements(QLatin1String("img"));
    Q_FOREACH (const QWebElement &img, images) {
        if (img.hasAttribute(QLatin1String("src"))) {
            const QString src = img.attribute(QLatin1String("src"));
            if (src.startsWith(QLatin1String("http://")) || src.startsWith(QLatin1String("https://")) ) {
                QTreeWidgetItem *item = new QTreeWidgetItem(mListItems);
                item->setText(Url, src);
                item->setText(Type, i18n("Image"));
                item->setTextColor(FilterValue, Qt::red);
                item->setData(Type, Element, Image);
            }
        }
    }
    foreach(QWebFrame *childFrame, frame->childFrames()) {
        searchBlockableElement(childFrame);
    }
}

void AdBlockBlockableItemsWidget::customContextMenuRequested(const QPoint &)
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item)
        return;

    KMenu menu;
    menu.addAction(i18n("Copy url"),this,SLOT(slotCopyItem()));
    menu.addAction(i18n("Block item..."),this,SLOT(slotBlockItem()));
    menu.addSeparator();
    menu.addAction(i18n("Open"), this, SLOT(slotOpenItem()));
    if (!item->text(FilterValue).isEmpty()) {
        menu.addSeparator();
        menu.addAction(i18n("Remove filter"),this,SLOT(slotRemoveFilter()));
    }
    menu.exec(QCursor::pos());
}

void AdBlockBlockableItemsWidget::slotOpenItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item)
        return;
    const KUrl url(item->text(Url));
    KRun *runner = new KRun( url, this ); // will delete itself
    runner->setRunExecutables( false );
}

void AdBlockBlockableItemsWidget::slotBlockItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item)
        return;

    QPointer<AdBlockCreateFilterDialog> dlg = new AdBlockCreateFilterDialog(static_cast<TypeElement>(item->data(Type, Element).toInt()), this);
    dlg->setPattern(item->text(Url));
    if (dlg->exec()) {
        const QString filter = dlg->filter();
        item->setText(FilterValue, filter);
    }
    delete dlg;
}

void AdBlockBlockableItemsWidget::slotCopyItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item)
        return;
    QClipboard *cb = QApplication::clipboard();
    cb->setText(item->text(Url), QClipboard::Clipboard);
}

void AdBlockBlockableItemsWidget::saveFilters()
{
    const int numberOfElement(mListItems->topLevelItemCount());
    QString filters;
    for (int i = 0; i < numberOfElement; ++i) {
        QTreeWidgetItem *item = mListItems->topLevelItem(i);
        if (!item->text(FilterValue).isEmpty()) {
            if (filters.isEmpty()) {
                filters = item->text(FilterValue);
            } else {
                filters += QLatin1Char('\n') + item->text(FilterValue);
            }
        }
    }

    if (filters.isEmpty())
        return;

    const QString localRulesFilePath = KStandardDirs::locateLocal("appdata" , QLatin1String("adblockrules_local"));

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::WriteOnly | QFile::Text)) {
        kDebug() << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream out(&ruleFile);
    out << filters;

    AdBlockManager::self()->reloadConfig();
}

void AdBlockBlockableItemsWidget::slotRemoveFilter()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item)
        return;
    item->setText(FilterValue, QString());
}

#include "adblockblockableitemswidget.moc"
