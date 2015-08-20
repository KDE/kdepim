/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>
    Copyright (C) 2013-2015 Laurent Montel <montel@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "toolbox.h"
#include "dbman.h"
#include "entriescountdialog.h"
#include "addeditblog.h"
#include "backend.h"
#include "bilbopost.h"
#include "bilboblog.h"
#include "catcheckbox.h"
#include "settings.h"

#include <QMenu>
#include <QIcon>
#include <QAction>
#include <QClipboard>
#include <QTimer>
#include <QPointer>
#include <QTimeZone>
#include <qstatusbar.h>
#include <QDesktopServices>
#include "blogilo_debug.h"
#include <kxmlguiwindow.h>
#include <kmessagebox.h>

class Toolbox::Private
{
public:
    QList<CatCheckBox *> listCategoryCheckBoxes;
    int mCurrentBlogId;
    QStatusBar *statusbar;
};
Toolbox::Toolbox(QWidget *parent)
    : QWidget(parent), d(new Private)
{
    d->mCurrentBlogId = -1;
    if (parent) {
        d->statusbar = qobject_cast<KXmlGuiWindow *>(parent)->statusBar();
    } else {
        d->statusbar = new QStatusBar(this);
    }
    setupUi(this);
    setButtonsIcon();
    frameCat->layout()->setAlignment(Qt::AlignTop);
    optionsDate->setDate(QDateTime::currentDateTime().date());
    optionsTime->setTime(QDateTime::currentDateTime().time());

    connect(btnCatReload, &QPushButton::clicked, this, &Toolbox::slotReloadCategoryList);
    connect(btnEntriesUpdate, &QPushButton::clicked, this, &Toolbox::slotUpdateEntries);
    connect(btnEntriesClear, &QPushButton::clicked, this, &Toolbox::clearEntries);

    connect(lstEntriesList, &QListWidget::itemDoubleClicked, this, &Toolbox::slotEntrySelected);
    connect(btnEntriesRemove, &QPushButton::clicked, this, &Toolbox::slotRemoveSelectedEntryFromServer);

    connect(btnOptionsNow, &QPushButton::clicked, this, &Toolbox::setDateTimeNow);
    connect(localEntries, &QTreeWidget::itemDoubleClicked, this, &Toolbox::slotLocalEntrySelected);
    connect(btnLocalRemove, &QPushButton::clicked, this, &Toolbox::slotRemoveLocalEntry);

    lblOptionsTrackBack->setVisible(false);
    txtOptionsTrackback->setVisible(false);
    btnCatAdd->setVisible(false);

    lstEntriesList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(lstEntriesList, &QListWidget::customContextMenuRequested, this, &Toolbox::requestEntriesListContextMenu);

    QTimer::singleShot(1000, this, SLOT(reloadLocalPosts()));
}

Toolbox::~Toolbox()
{
    delete d;
}

void Toolbox::setCurrentBlogId(int blog_id)
{
    if (d->mCurrentBlogId == blog_id) {
        return;
    }
    d->mCurrentBlogId = blog_id;
    if (blog_id <= 0) {
        return;
    }
    slotLoadCategoryListFromDB(blog_id);
    slotLoadEntriesFromDB(blog_id);
    Qt::LayoutDirection ll = DBMan::self()->blogList().value(blog_id)->direction();
    frameCat->setLayoutDirection(ll);
    lstEntriesList->setLayoutDirection(ll);
}

void Toolbox::slotReloadCategoryList()
{
    if (d->mCurrentBlogId == -1) {
        KMessageBox::sorry(this, i18n("No blog has been selected: you have to select a blog from the Blogs page before asking for the list of categories."));
        return;
    }

    Backend *b = new Backend(d->mCurrentBlogId);
    connect(b, &Backend::sigCategoryListFetched, this, &Toolbox::slotLoadCategoryListFromDB);
    connect(b, &Backend::sigError, this, &Toolbox::sigError);
    Q_EMIT sigBusy(true);
    d->statusbar->showMessage(i18n("Requesting list of categories..."));
    b->getCategoryListFromServer();
}

void Toolbox::slotUpdateEntries(int count)
{
    if (d->mCurrentBlogId == -1) {
        KMessageBox::sorry(this, i18n("No blog has been selected: you have to select a blog from the Blogs page before asking for the list of entries."));
        return;
    }
    if (count == 0) {
        count = Settings::updateEntriesCount();
        if (Settings::showUpdateEntriesDialog()) {
            QPointer<EntriesCountDialog> dia = new EntriesCountDialog(this);
            if (!dia->exec()) {
                delete dia;
                return;
            }
            count = dia->count();
            dia->deleteLater();
        }
    }
    Backend *entryB = new Backend(d->mCurrentBlogId, this);
    entryB->getEntriesListFromServer(count);
    connect(entryB, &Backend::sigEntriesListFetched, this, &Toolbox::slotLoadEntriesFromDB);
    connect(entryB, &Backend::sigError, this, &Toolbox::sigError);
    d->statusbar->showMessage(i18n("Requesting list of entries..."));
    setCursor(Qt::BusyCursor);
    Q_EMIT sigBusy(true);
}

void Toolbox::slotLoadEntriesFromDB(int blog_id)
{
    if (blog_id == -1) {
        qCDebug(BLOGILO_LOG) << "Blog Id doesn't set correctly";
        return;
    }
    lstEntriesList->clear();
    const QVector<QVariantMap> listEntries = DBMan::self()->listPostsInfo(blog_id);
    const int count = listEntries.count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *lstItem = new QListWidgetItem(listEntries[i].value(QStringLiteral("title")).toString());
        lstItem->setToolTip(listEntries.at(i).value(QStringLiteral("c_time")).toDateTime().toString());
        if (listEntries.at(i).value(QStringLiteral("is_private")).toBool()) {
            lstItem->setForeground(QBrush(Qt::blue));
            lstItem->setToolTip(i18n("%1 (Draft)", lstItem->toolTip()));
        }
        lstItem->setData(BlogEntryID, listEntries.at(i).value(QStringLiteral("id")).toInt());
        lstEntriesList->addItem(lstItem);
    }
    d->statusbar->showMessage(i18n("List of entries received."), STATUSTIMEOUT);
    unsetCursor();
    Q_EMIT sigBusy(false);
}

void Toolbox::slotLoadCategoryListFromDB(int blog_id)
{
    if (blog_id == -1) {
        qCDebug(BLOGILO_LOG) << "Blog Id do not sets correctly";
        return;
    }
    clearCatList();
    QVector<Category> listCategories;
    listCategories = DBMan::self()->listCategories(blog_id);

    QVector<Category>::const_iterator i;
    QVector<Category>::const_iterator endIt = listCategories.constEnd();
    for (i = listCategories.constBegin(); i != endIt; ++i) {
        CatCheckBox *cb = new CatCheckBox(i->name, this);
        cb->setCategory(*i);
        d->listCategoryCheckBoxes.append(cb);
        frameCat->layout()->addWidget(cb);
    }
    d->statusbar->showMessage(i18n("List of categories received."), STATUSTIMEOUT);
    unsetCursor();
    Q_EMIT sigBusy(false);
}

void Toolbox::slotRemoveSelectedEntryFromServer()
{
    if (lstEntriesList->selectedItems().count() < 1) {
        return;
    }
    if (KMessageBox::warningYesNoCancel(this, i18n("Removing a post from your blog cannot be undone.\nAre you sure you want to remove the post with title \"%1\" from your blog?", lstEntriesList->currentItem()->text()))
            == KMessageBox::Yes) {
        BilboPost *post = new BilboPost(DBMan::self()->getPostInfo(lstEntriesList->currentItem()->
                                        data(BlogEntryID).toInt()));
        Backend *b = new Backend(d->mCurrentBlogId, this);
        connect(b, &Backend::sigPostRemoved, this, &Toolbox::slotPostRemoved);
        connect(b, &Backend::sigError, this, &Toolbox::slotError);
        b->removePost(post);
        d->statusbar->showMessage(i18n("Removing post..."));
    }
}

void Toolbox::slotPostRemoved(int blog_id, const BilboPost &post)
{
    KMessageBox::information(this, i18nc("Post removed from Blog", "Post with title \"%1\" removed from \"%2\".",
                                         post.title(), DBMan::self()->blogList().value(blog_id)->title()));
    slotLoadEntriesFromDB(blog_id);
    d->statusbar->showMessage(i18n("Post removed"), STATUSTIMEOUT);
    sender()->deleteLater();
}

void Toolbox::slotError(const QString &errorMessage)
{
    KMessageBox::detailedError(this, i18n("An error occurred in the latest transaction."), errorMessage);
    d->statusbar->showMessage(i18nc("Operation failed", "Failed"), STATUSTIMEOUT);
    sender()->deleteLater();
}

void Toolbox::clearFields()
{
    clearCatList();
    lstEntriesList->clear();
    txtCatTags->clear();
    chkOptionsTime->setChecked(false);
    optionsDate->setDate(QDateTime::currentDateTime().date());
    optionsTime->setTime(QDateTime::currentDateTime().time());
    txtOptionsTrackback->clear();
    txtSlug->clear();
    txtSummary->clear();
    chkOptionsComments->setChecked(true);
    chkOptionsTrackback->setChecked(true);
    comboOptionsStatus->setCurrentIndex(0);
}

void Toolbox::resetFields()
{
    unCheckCatList();
    txtCatTags->clear();
    chkOptionsTime->setChecked(false);
    optionsDate->setDate(QDateTime::currentDateTime().date());
    optionsTime->setTime(QDateTime::currentDateTime().time());
    txtOptionsTrackback->clear();
    txtSlug->clear();
    txtSummary->clear();
    chkOptionsComments->setChecked(true);
    chkOptionsTrackback->setChecked(true);
    comboOptionsStatus->setCurrentIndex(0);
}

void Toolbox::clearCatList()
{
    foreach (CatCheckBox *cat, d->listCategoryCheckBoxes) {
        cat->deleteLater();
    }
    d->listCategoryCheckBoxes.clear();
}

void Toolbox::getFieldsValue(BilboPost *currentPost)
{
    currentPost->setCategoryList(selectedCategories());
    currentPost->setTags(currentTags());
    currentPost->setModifyTimeStamp(chkOptionsTime->isChecked());
    if (currentPost->status() == KBlog::BlogPost::New) {
        if (chkOptionsTime->isChecked()) {
            currentPost->setModificationDateTime(QDateTime(optionsDate->date(), optionsTime->time()));
            currentPost->setCreationDateTime(QDateTime(optionsDate->date(), optionsTime->time()));
        } else {
            currentPost->setCreationDateTime(QDateTime::currentDateTime());
            currentPost->setModificationDateTime(QDateTime::currentDateTime());
        }
    } else {
        currentPost->setCreationDateTime(QDateTime(optionsDate->date(), optionsTime->time()));
        currentPost->setModificationDateTime(QDateTime(optionsDate->date(), optionsTime->time()));
    }
    if (currentPost->creationDateTime().timeZone() == QTimeZone("UTC") || currentPost->modificationDateTime().timeZone() == QTimeZone("UTC")) {
        qCDebug(BLOGILO_LOG) << "creationDateTime was UTC!";
        currentPost->setCreationDateTime(currentPost->creationDateTime().toLocalTime());
        currentPost->setModificationDateTime(currentPost->modificationDateTime().toLocalTime());
    }
    currentPost->setSlug(txtSlug->text());
    currentPost->setPrivate((comboOptionsStatus->currentIndex() == 1) ? true : false);
    currentPost->setCommentAllowed(chkOptionsComments->isChecked());
    currentPost->setTrackBackAllowed(chkOptionsTrackback->isChecked());
    currentPost->setSummary(txtSummary->toPlainText());
}

void Toolbox::setFieldsValue(BilboPost *post)
{
    if (post == Q_NULLPTR) {
        resetFields();
        qCDebug(BLOGILO_LOG) << "post is NULL";
        return;
    }

    setSelectedCategories(post->categories());
    txtCatTags->setText(post->tags().join(QStringLiteral(", ")));
//     qCDebug(BLOGILO_LOG) << "Post status is: " << post->status();
    if (post->status() == KBlog::BlogPost::New) {
        comboOptionsStatus->setCurrentIndex(2);
    } else if (post->isPrivate()) {
        comboOptionsStatus->setCurrentIndex(1);
    } else {
        comboOptionsStatus->setCurrentIndex(0);
    }
    chkOptionsComments->setChecked(post->isCommentAllowed());
    chkOptionsTrackback->setChecked(post->isTrackBackAllowed());
    chkOptionsTime->setChecked(post->isModifyTimeStamp());
    if (post->creationDateTime().timeZone() == QTimeZone("UTC") || post->modificationDateTime().timeZone() == QTimeZone("UTC")) {
        qCDebug(BLOGILO_LOG) << "creationDateTime was UTC!";
        post->setCreationDateTime(post->creationDateTime().toLocalTime());
        post->setModificationDateTime(post->modificationDateTime().toLocalTime());
    }
    optionsTime->setTime(post->creationDateTime().time());
    optionsDate->setDate(post->creationDateTime().date());
    txtSlug->setText(QUrl::fromPercentEncoding(post->slug().toLatin1()));
    txtSummary->setPlainText(post->summary());
}

QVector<Category> Toolbox::selectedCategories() const
{
    QVector<Category> list;
    const int count = d->listCategoryCheckBoxes.count();
    for (int i = 0; i < count; ++i) {
        if (d->listCategoryCheckBoxes.at(i)->isChecked()) {
            list.append(d->listCategoryCheckBoxes.at(i)->category());
        }
    }
    return list;
}

QStringList Toolbox::selectedCategoriesTitle() const
{
    QStringList list;
    const int count = d->listCategoryCheckBoxes.count();
    for (int i = 0; i < count; ++i) {
        if (d->listCategoryCheckBoxes.at(i)->isChecked()) {
            list.append(d->listCategoryCheckBoxes.at(i)->category().name);
        }
    }
    return list;
}

void Toolbox::setSelectedCategories(const QStringList &list)
{
    unCheckCatList();
    int count = d->listCategoryCheckBoxes.count();
    for (int i = 0; i < count; ++i) {
        if (list.contains(d->listCategoryCheckBoxes.at(i)->category().name, Qt::CaseInsensitive)) {
            d->listCategoryCheckBoxes.at(i)->setChecked(true);
        }
    }
}

QStringList Toolbox::currentTags()
{
    QStringList t = txtCatTags->text().split(QRegExp(QStringLiteral(",|،")), QString::SkipEmptyParts);
    for (int i = 0; i < t.count() ; ++i) {
        t[i] = t[i].trimmed();
    }
    return t;
}

void Toolbox::slotEntrySelected(QListWidgetItem *item)
{
    BilboPost post = DBMan::self()->getPostInfo(item->data(BlogEntryID).toInt());
    qCDebug(BLOGILO_LOG) << "Emiting sigEntrySelected...";
    Q_EMIT sigEntrySelected(post, d->mCurrentBlogId);
}

void Toolbox::setCurrentPage(int index)
{
    box->setCurrentIndex(index);
}

void Toolbox::slotEntriesCopyUrl()
{
    if (lstEntriesList->currentItem() == Q_NULLPTR) {
        return;
    }
    BilboPost post = DBMan::self()->getPostInfo(lstEntriesList->currentItem()->data(BlogEntryID).toInt());
    if (!post.permaLink().isEmpty()) {
        QApplication::clipboard()->setText(post.permaLink().toDisplayString());
    } else if (!post.link().isEmpty()) {
        QApplication::clipboard()->setText(post.link().toDisplayString());
    } else {
        KMessageBox::sorry(this, i18n("No link field is available in the database for this entry."));
    }
}

void Toolbox::unCheckCatList()
{
    const int count = d->listCategoryCheckBoxes.count();
    for (int j = 0; j < count; ++j) {
        d->listCategoryCheckBoxes.at(j)->setChecked(false);
    }
}

void Toolbox::setButtonsIcon()
{
    btnEntriesUpdate->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    btnEntriesRemove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    btnEntriesClear->setIcon(QIcon::fromTheme(QStringLiteral("edit-clear")));
    btnCatReload->setIcon(QIcon::fromTheme(QStringLiteral("view-refresh")));
    btnCatAdd->setIcon(QIcon::fromTheme(QStringLiteral("list-add")));
    btnLocalRemove->setIcon(QIcon::fromTheme(QStringLiteral("list-remove")));
    ///TODO Add option for selecting only text or only Icon for Toolbox buttons!
}

void Toolbox::reloadLocalPosts()
{
    qCDebug(BLOGILO_LOG);

    localEntries->clear();

    QList<QVariantMap> localList = DBMan::self()->listLocalPosts();
    const int count = localList.count();

    for (int i = 0; i < count; ++i) {
        const QString postTitle = localList.at(i).value(QStringLiteral("post_title")).toString();
        const QString blogTitle = localList.at(i).value(QStringLiteral("blog_title")).toString();

        QTreeWidgetItem *item = new QTreeWidgetItem(localEntries, QStringList() << postTitle << blogTitle);
        item->setData(0, LocalEntryID, localList.at(i).value(QStringLiteral("local_id")).toInt());
        item->setData(1, LocalEntryID, localList.at(i).value(QStringLiteral("blog_id")).toInt());
    }
}

void Toolbox::slotLocalEntrySelected(QTreeWidgetItem *item, int column)
{
    qCDebug(BLOGILO_LOG) << "Emitting sigEntrySelected...";
    Q_UNUSED(column);
    BilboPost post = DBMan::self()->localPost(item->data(0, LocalEntryID).toInt());
    Q_EMIT sigEntrySelected(post, item->data(1, LocalEntryID).toInt());
}

void Toolbox::slotRemoveLocalEntry()
{
    if (localEntries->currentItem()) {
        if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to remove the selected local entry?"))
                == KMessageBox::No) {
            return;
        }
        const int local_id = localEntries->currentItem()->data(0, LocalEntryID).toInt();
        if (DBMan::self()->removeLocalEntry(local_id)) {
            delete localEntries->currentItem();
        } else {
            KMessageBox::detailedError(this, i18n("Cannot remove selected local entry."),
                                       DBMan::self()->lastErrorText());
        }
    } else {
        KMessageBox::sorry(this, i18n("You have to select at least one entry from list."));
    }
}

void Toolbox::clearEntries()
{
    if (d->mCurrentBlogId == -1) {
        return;
    }
    if (KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to clear the list of entries?")) ==
            KMessageBox::Cancel) {
        return;
    }
    if (DBMan::self()->clearPosts(d->mCurrentBlogId)) {
        lstEntriesList->clear();
    } else {
        KMessageBox::detailedSorry(this, i18n("Cannot clear the list of entries.") , DBMan::self()->lastErrorText());
    }
}

void Toolbox::setDateTimeNow()
{
    optionsDate->setDate(QDate::currentDate());
    optionsTime->setTime(QTime::currentTime());
}

void Toolbox::requestEntriesListContextMenu(const QPoint &pos)
{
    if (lstEntriesList->selectedItems().isEmpty()) {
        return;
    }
    Q_UNUSED(pos);
    QMenu *entriesContextMenu = new QMenu;
    QAction *actEntriesOpenInBrowser = new QAction(QIcon::fromTheme(QStringLiteral("applications-internet")),
            i18n("Open in browser"), entriesContextMenu);
    connect(actEntriesOpenInBrowser, &QAction::triggered, this, &Toolbox::openPostInBrowser);
    QAction *actEntriesCopyUrl = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")),
            i18n("Copy URL"), entriesContextMenu);
    connect(actEntriesCopyUrl, &QAction::triggered, this, &Toolbox::slotEntriesCopyUrl);
    QAction *actEntriesCopyTitle = new QAction(QIcon::fromTheme(QStringLiteral("edit-copy")),
            i18n("Copy title"), entriesContextMenu);
    connect(actEntriesCopyTitle, &QAction::triggered, this, &Toolbox::copyPostTitle);
    entriesContextMenu->addAction(actEntriesOpenInBrowser);
    entriesContextMenu->addAction(actEntriesCopyUrl);
    entriesContextMenu->addAction(actEntriesCopyTitle);
    entriesContextMenu->exec(QCursor::pos());
    delete entriesContextMenu;
}

void Toolbox::openPostInBrowser()
{
    if (lstEntriesList->selectedItems().isEmpty()) {
        return;
    }
    BilboPost post = DBMan::self()->getPostInfo(lstEntriesList->currentItem()->data(BlogEntryID).toInt());
    QUrl url;
    if (!post.permaLink().isEmpty()) {
        url = post.permaLink().url();
    } else if (!post.link().isEmpty()) {
        url = post.link().url();
    } else {
        url = QUrl(DBMan::self()->blogList().value(d->mCurrentBlogId)->blogUrl());
    }
    QDesktopServices::openUrl(url);
}

void Toolbox::copyPostTitle()
{
    if (!lstEntriesList->selectedItems().isEmpty()) {
        QApplication::clipboard()->setText(lstEntriesList->currentItem()->text());
    }
}

