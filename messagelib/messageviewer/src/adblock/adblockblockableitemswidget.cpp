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

#include "adblockblockableitemswidget.h"
#include "adblockcreatefilterdialog.h"
#include "settings/messageviewersettings.h"
#include "adblock/adblockmanager.h"
#include "PimCommon/CustomTreeView"
#include "messageviewer_debug.h"
#include <KLocalizedString>
#include <KTreeWidgetSearchLine>
#include <QMenu>
#include <KConfigGroup>
#include "adblock/adblockutil.h"
#include <QUrl>
#include <KRun>

#include <QHeaderView>
#include <QVBoxLayout>
#include <QWebFrame>
#include <QWebElement>

#include <QPointer>
#include <QClipboard>
#include <QApplication>
#include <QFile>
#include <QPainter>

using namespace MessageViewer;

AdBlockBlockableItemsWidget::AdBlockBlockableItemsWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    setLayout(lay);
    mListItems = new PimCommon::CustomTreeView;
    mListItems->setDefaultText(i18n("No blockable element found."));

    mListItems->setContextMenuPolicy(Qt::CustomContextMenu);
    mListItems->setAlternatingRowColors(true);
    mListItems->setRootIsDecorated(false);
    connect(mListItems, &PimCommon::CustomTreeView::customContextMenuRequested, this, &AdBlockBlockableItemsWidget::customContextMenuRequested);

    QStringList lst;
    lst << i18n("Filter") << i18n("Address") << i18n("Type");
    mListItems->setHeaderLabels(lst);

    KTreeWidgetSearchLine *searchLine = new KTreeWidgetSearchLine(this, mListItems);
    searchLine->setPlaceholderText(i18n("Search..."));

    lay->addWidget(searchLine);
    lay->addWidget(mListItems);

    KConfigGroup config(MessageViewer::MessageViewerSettings::self()->config(), "AdBlockHeaders");
    mListItems->header()->restoreState(config.readEntry("HeaderState", QByteArray()));
}

AdBlockBlockableItemsWidget::~AdBlockBlockableItemsWidget()
{
    KConfigGroup groupHeader(MessageViewer::MessageViewerSettings::self()->config(), "AdBlockHeaders");
    groupHeader.writeEntry("HeaderState", mListItems->header()->saveState());
    groupHeader.sync();
}

void AdBlockBlockableItemsWidget::setWebFrame(QWebFrame *frame)
{
    mListItems->clear();
    searchBlockableElement(frame);
}

QString AdBlockBlockableItemsWidget::elementTypeToI18n(AdBlockBlockableItemsWidget::TypeElement type)
{
    QString result;
    switch (type) {
    case AdBlockBlockableItemsWidget::Image:
        result = i18n("Image");
        break;
    case AdBlockBlockableItemsWidget::Script:
        result = i18n("Script");
        break;
    case AdBlockBlockableItemsWidget::StyleSheet:
        result = i18n("Stylesheet");
        break;
    case AdBlockBlockableItemsWidget::Font:
        result = i18n("Font");
        break;
    case AdBlockBlockableItemsWidget::Frame:
        result = i18n("Frame");
        break;
    case AdBlockBlockableItemsWidget::XmlRequest:
        result = i18n("XML Request");
        break;
    case AdBlockBlockableItemsWidget::Object:
        result = i18n("Object");
        break;
    case AdBlockBlockableItemsWidget::Media:
        result = i18n("Audio/Video");
        break;
    case AdBlockBlockableItemsWidget::Popup:
        result = i18n("Popup window");
        break;
    case AdBlockBlockableItemsWidget::None:
    default:
        result = i18n("Unknown");
    }
    return result;
}

QString AdBlockBlockableItemsWidget::elementType(AdBlockBlockableItemsWidget::TypeElement type)
{
    QString result;
    switch (type) {
    case AdBlockBlockableItemsWidget::Image:
        result = QStringLiteral("image");
        break;
    case AdBlockBlockableItemsWidget::Script:
        result = QStringLiteral("script");
        break;
    case AdBlockBlockableItemsWidget::StyleSheet:
        result = QStringLiteral("stylesheet");
        break;
    case AdBlockBlockableItemsWidget::Font:
        result = QStringLiteral("font");
        break;
    case AdBlockBlockableItemsWidget::Frame:
        result = QStringLiteral("frame");
        break;
    case AdBlockBlockableItemsWidget::XmlRequest:
        result = QStringLiteral("xmlhttprequest");
        break;
    case AdBlockBlockableItemsWidget::Object:
        result = QStringLiteral("other");
        break;
    case AdBlockBlockableItemsWidget::Media:
        result = QStringLiteral("media");
        break;
    case AdBlockBlockableItemsWidget::Popup:
        result = QStringLiteral("popup");
        break;
    case AdBlockBlockableItemsWidget::None:
    default:
        qCDebug(MESSAGEVIEWER_LOG) << " unknown type " << type;
    }
    return result;
}

void AdBlockBlockableItemsWidget::adaptSrc(QString &src, const QString &hostName)
{
    if (src.startsWith(QStringLiteral("http://")) || src.startsWith(QStringLiteral("https://"))) {
        //Nothing
    } else if (src.startsWith(QStringLiteral("//"))) {
        src = QLatin1String("https:") + src;
    } else if (src.startsWith(QLatin1Char('/'))) {
        src = QLatin1String("https://") + hostName + src;
    } else {
        src = QString();
    }
}

void AdBlockBlockableItemsWidget::searchBlockableElement(QWebFrame *frame)
{
    const QUrl url = frame->requestedUrl();
    const QString host = url.host();
    const QWebElementCollection images = frame->findAllElements(QStringLiteral("img"));
    Q_FOREACH (const QWebElement &img, images) {
        if (img.hasAttribute(QStringLiteral("src"))) {
            QString src = img.attribute(QStringLiteral("src"));
            if (src.isEmpty()) {
                continue;
            }
            adaptSrc(src, host);
            if (src.isEmpty()) {
                continue;
            }
            QTreeWidgetItem *item = new QTreeWidgetItem(mListItems);
            item->setText(Url, src);
            item->setText(Type, elementTypeToI18n(AdBlockBlockableItemsWidget::Image));
            item->setTextColor(FilterValue, Qt::red);
            item->setData(Type, Element, Image);
        }
    }
    const QWebElementCollection scripts = frame->findAllElements(QStringLiteral("script"));
    Q_FOREACH (const QWebElement &script, scripts) {
        QString src = script.attribute(QStringLiteral("src"));
        if (src.isEmpty()) {
            continue;
        }
        adaptSrc(src, host);
        if (src.isEmpty()) {
            continue;
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(mListItems);
        item->setText(Url, src);
        item->setText(Type, elementTypeToI18n(AdBlockBlockableItemsWidget::Script));
        item->setTextColor(FilterValue, Qt::red);
        item->setData(Type, Element, Script);
    }
    foreach (QWebFrame *childFrame, frame->childFrames()) {
        searchBlockableElement(childFrame);
    }
}

void AdBlockBlockableItemsWidget::customContextMenuRequested(const QPoint &)
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item) {
        return;
    }

    QMenu menu;
    menu.addAction(i18n("Copy url"), this, SLOT(slotCopyItem()));
    if (!item->text(FilterValue).isEmpty()) {
        menu.addAction(i18n("Copy filter"), this, SLOT(slotCopyFilterItem()));
    }
    menu.addAction(i18n("Block item..."), this, SLOT(slotBlockItem()));
    menu.addSeparator();
    menu.addAction(i18n("Open"), this, SLOT(slotOpenItem()));
    if (!item->text(FilterValue).isEmpty()) {
        menu.addSeparator();
        menu.addAction(i18n("Remove filter"), this, SLOT(slotRemoveFilter()));
    }
    menu.exec(QCursor::pos());
}

void AdBlockBlockableItemsWidget::slotCopyFilterItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item) {
        return;
    }
    QClipboard *cb = QApplication::clipboard();
    cb->setText(item->text(FilterValue), QClipboard::Clipboard);
}

void AdBlockBlockableItemsWidget::slotOpenItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item) {
        return;
    }
    const QUrl url(item->text(Url));
    KRun *runner = new KRun(url, this);   // will delete itself
    runner->setRunExecutables(false);
}

void AdBlockBlockableItemsWidget::slotBlockItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item) {
        return;
    }

    QPointer<AdBlockCreateFilterDialog> dlg = new AdBlockCreateFilterDialog(this);
    dlg->setPattern(static_cast<TypeElement>(item->data(Type, Element).toInt()), item->text(Url));
    if (dlg->exec()) {
        const QString filter = dlg->filter();
        item->setText(FilterValue, filter);
    }
    delete dlg;
}

void AdBlockBlockableItemsWidget::slotCopyItem()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item) {
        return;
    }
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

    if (filters.isEmpty()) {
        return;
    }

    const QString localRulesFilePath = MessageViewer::AdBlockUtil::localFilterPath();

    QFile ruleFile(localRulesFilePath);
    if (!ruleFile.open(QFile::WriteOnly | QFile::Text)) {
        qCDebug(MESSAGEVIEWER_LOG) << "Unable to open rule file" << localRulesFilePath;
        return;
    }

    QTextStream out(&ruleFile);
    out << filters;

    AdBlockManager::self()->reloadConfig();
}

void AdBlockBlockableItemsWidget::slotRemoveFilter()
{
    QTreeWidgetItem *item = mListItems->currentItem();
    if (!item) {
        return;
    }
    item->setText(FilterValue, QString());
}

