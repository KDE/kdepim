/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>
    Copyright (C) 2013-2016 Laurent Montel <montel@kde.org>

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

#include "mainwindow.h"
#include "global.h"
#include "dbman.h"
#include "toolbox.h"
#include "postentry.h"
#include "addeditblog.h"
#include "backend.h"
#include "bilbomedia.h"
#include "settings.h"
#include "bilboblog.h"
#include "blogsettings.h"
#include "poststabwidget.h"
#include "uploadmediadialog.h"
#include "configuredialog.h"

#include "ui_advancedsettingsbase.h"
#include "ui_settingsbase.h"
#include "ui_editorsettingsbase.h"

#include "Libkdepim/ProgressDialog"
#include <Libkdepim/StatusbarProgressWidget>
#include <Libkdepim/ProgressStatusBarWidget>

#include <qtabwidget.h>
#include <KStatusNotifierItem>
#include <qstatusbar.h>
#include <KToggleAction>
#include <kactioncollection.h>
#include <KActionMenu>
#include <QAction>
#include <kconfigdialog.h>
#include "blogilo_debug.h"
#include <kmessagebox.h>
#include <KLocalizedString>
#include <KSelectAction>
#include <kimagefilepreview.h>
#include <QMenu>
#include <QUrl>

#include <QDockWidget>
#include <QProgressBar>
#include <QTimer>
#include <QKeyEvent>
#include <QDesktopServices>

#define TIMEOUT 5000

MainWindow::MainWindow()
    : KXmlGuiWindow(),
      activePost(Q_NULLPTR),
      systemTray(Q_NULLPTR),
      previousActivePostIndex(-1),
      busyNumber(0),
      progress(Q_NULLPTR),
      mCurrentBlogId(__currentBlogId)
{
    setWindowTitle(i18n("Blogilo"));

    tabPosts = new PostsTabWidget(this);
    setCentralWidget(tabPosts);
    connect(tabPosts, &PostsTabWidget::createNewPost, this, &MainWindow::slotCreateNewPost);
    connect(tabPosts, &PostsTabWidget::closeTabClicked, this, &MainWindow::slotCloseTabClicked);
    connect(tabPosts, &PostsTabWidget::tabCloseRequested, this, &MainWindow::slotRemovePostEntry);
    connect(tabPosts, &PostsTabWidget::tabRemoveAllExclude, this, &MainWindow::slotRemoveAllExclude);

    toolbox = new Toolbox(this);
    toolboxDock = new QDockWidget(i18n("Toolbox"), this);
    toolboxDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
    toolboxDock->setFeatures(QDockWidget::AllDockWidgetFeatures);
    toolboxDock->setWidget(toolbox);

    toolboxDock->setObjectName(QStringLiteral("dock_toolbox"));
    toolbox->setObjectName(QStringLiteral("toolbox"));

    this->addDockWidget(Qt::RightDockWidgetArea, toolboxDock);

    // then, setup our actions
    setupActions();
    setupStatusBar();

    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();

    toolbox->setVisible(Settings::showToolboxOnStart());
    actionCollection()->action(QStringLiteral("toggle_toolbox"))->setChecked(Settings::showToolboxOnStart());

    setupSystemTray();

    connect(tabPosts, &PostsTabWidget::currentChanged, this, &MainWindow::slotActivePostChanged);
    connect(toolbox, &Toolbox::sigEntrySelected, this, &MainWindow::slotNewPostOpened);
    connect(toolbox, &Toolbox::sigError, this, &MainWindow::slotError);
    connect(toolbox, &Toolbox::sigBusy, this, &MainWindow::slotBusy);

    foreach (BilboBlog *blog, DBMan::self()->blogList()) {
        QAction *act = new QAction(blog->title(), blogs);
        act->setCheckable(true);
        act->setData(blog->id());
        blogs->addAction(act);
    }
    connect(blogs, static_cast<void (KSelectAction::*)(QAction *)>(&KSelectAction::triggered), this, &MainWindow::currentBlogChanged);
    QTimer::singleShot(0, this, &MainWindow::loadTempPosts);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupStatusBar()
{
}

void MainWindow::slotUploadFileDone(const QString &serviceName, const QString &link)
{
    Q_UNUSED(serviceName);
    KMessageBox::information(this, i18n("File uploaded. You can access to it at this url %1", link), i18n("Upload File"));
}

void MainWindow::slotUploadFileFailed(const QString &serviceName, const QString &filename)
{
    Q_UNUSED(serviceName);
    Q_UNUSED(filename);
    KMessageBox::error(this, i18n("Error during upload."), i18n("Upload File"));
}

void MainWindow::slotActionFailed(const QString &serviceName, const QString &error)
{
    KMessageBox::error(this, i18n("%1 return an error '%2'", serviceName, error), i18n("Error"));
}

void MainWindow::slotCloseTabClicked()
{
    const int currentIndex = tabPosts->currentIndex();
    if (currentIndex != -1) {
        slotRemovePostEntry(currentIndex);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    writeConfigs();
    if (!DBMan::self()->clearTempEntries()) {
        qCDebug(BLOGILO_LOG) << "Could not erase temp_post table: " << DBMan::self()->lastErrorText();
    }
    const int count = tabPosts->count();
    if (count > 0) {
        toolbox->getFieldsValue(activePost->currentPost());
        for (int i = 0; i < count; ++i) {
            PostEntry *pst = qobject_cast<PostEntry *>(tabPosts->widget(i));
            DBMan::self()->saveTempEntry(*pst->currentPost(), pst->currentPostBlogId());
        }
    }
    event->accept();
}

void MainWindow::setupActions()
{
    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

    KStandardAction::preferences(this, &MainWindow::optionsPreferences, actionCollection());

    // custom menu and menu item
    QAction *actNewPost = new QAction(QIcon::fromTheme(QStringLiteral("document-new")), i18n("New Post"), this);
    actionCollection()->addAction(QStringLiteral("new_post"), actNewPost);
    actionCollection()->setDefaultShortcut(actNewPost, QKeySequence(Qt::CTRL + Qt::Key_N));
    connect(actNewPost, &QAction::triggered, this, &MainWindow::slotCreateNewPost);

    QAction *actAddBlog = new QAction(QIcon::fromTheme(QStringLiteral("list-add")), i18n("Add Blog..."), this);
    actionCollection()->addAction(QStringLiteral("add_blog"), actAddBlog);
    connect(actAddBlog, &QAction::triggered, this, &MainWindow::addBlog);

    QAction *actPublish = new QAction(QIcon::fromTheme(QStringLiteral("arrow-up")), i18n("Submit..."), this);
    actionCollection()->addAction(QStringLiteral("publish_post"), actPublish);
    connect(actPublish, &QAction::triggered, this, &MainWindow::slotPublishPost);

    QAction *actUpload = new QAction(QIcon::fromTheme(QStringLiteral("upload-media")), i18n("Upload Media..."), this);
    actionCollection()->addAction(QStringLiteral("upload_media"), actUpload);
    connect(actUpload, &QAction::triggered, this, &MainWindow::uploadMediaObject);

    QAction *actSaveLocally = new QAction(QIcon::fromTheme(QStringLiteral("document-save")), i18n("Save Locally"), this);
    actionCollection()->addAction(QStringLiteral("save_locally"), actSaveLocally);
    actionCollection()->setDefaultShortcut(actSaveLocally, QKeySequence(Qt::CTRL + Qt::Key_S));
    connect(actSaveLocally, &QAction::triggered, this, &MainWindow::slotSavePostLocally);

    KToggleAction *actToggleToolboxVisible = new KToggleAction(i18n("Show Toolbox"), this);
    actionCollection()->addAction(QStringLiteral("toggle_toolbox"), actToggleToolboxVisible);
    actionCollection()->setDefaultShortcut(actToggleToolboxVisible, QKeySequence(Qt::CTRL + Qt::Key_T));
    connect(actToggleToolboxVisible, &KToggleAction::toggled, this, &MainWindow::slotToggleToolboxVisible);
    connect(toolboxDock, &QDockWidget::visibilityChanged, this, &MainWindow::slotToolboxVisibilityChanged);

    blogs = new KSelectAction(this);
    actionCollection()->addAction(QStringLiteral("blogs_list"), blogs);

    QAction *actOpenBlog = new QAction(QIcon::fromTheme(QStringLiteral("applications-internet")), i18n("Open in browser"), this);
    actionCollection()->addAction(QStringLiteral("open_blog_in_browser"), actOpenBlog);
    actOpenBlog->setToolTip(i18n("Open current blog in browser"));
    connect(actOpenBlog, &QAction::triggered, this, &MainWindow::slotOpenCurrentBlogInBrowser);

}

void MainWindow::loadTempPosts()
{
    QMap<BilboPost *, int> tempList = DBMan::self()->listTempPosts();
    const int count = tempList.count();
    if (count > 0) {
        QMap<BilboPost *, int>::ConstIterator it = tempList.constBegin();
        QMap<BilboPost *, int>::ConstIterator endIt = tempList.constEnd();
        for (; it != endIt; ++it) {
            createPostEntry(it.value(), (*it.key()));
        }
    } else {
        slotCreateNewPost();
    }
//     activePost = qobject_cast<PostEntry*>( tabPosts->currentWidget() );
    previousActivePostIndex = 0;
    if (activePost) {
        setCurrentBlog(activePost->currentPostBlogId());
    }
}

void MainWindow::setCurrentBlog(int blog_id)
{
    qCDebug(BLOGILO_LOG) << blog_id;
    if (blog_id == -1) {
        blogs->setCurrentItem(-1);
        toolbox->setCurrentBlogId(blog_id);
//         actionCollection()->action("publish_post")->setEnabled( false );
        return;
    }
    const int count = blogs->items().count();
    for (int i = 0; i < count; ++i) {
        if (blogs->action(i)->data().toInt() == blog_id) {
            blogs->setCurrentItem(i);
            currentBlogChanged(blogs->action(i));
            break;
        }
    }
}

void MainWindow::currentBlogChanged(QAction *act)
{
    if (act) {
        if (mCurrentBlogId == act->data().toInt()) {
            return;
        }
        mCurrentBlogId = act->data().toInt();
//         __currentBlogId = mCurrentBlogId;
        if (activePost) {
//             actionCollection()->action("publish_post")->setEnabled( true );
            activePost->setCurrentPostBlogId(mCurrentBlogId);
        } else {
//             actionCollection()->action("publish_post")->setEnabled( false );
        }
        blogs->setToolTip(DBMan::self()->blogList().value(mCurrentBlogId)->blogUrl());
    } else {
        mCurrentBlogId = -1;
        if (activePost) {
            activePost->setCurrentPostBlogId(mCurrentBlogId);
        }
    }
    toolbox->setCurrentBlogId(mCurrentBlogId);
}

void MainWindow::slotCreateNewPost()
{
    tabPosts->setCurrentWidget(createPostEntry(mCurrentBlogId, BilboPost()));
    if (mCurrentBlogId == -1) {
        if (!blogs->items().isEmpty()) {
            blogs->setCurrentItem(0);
            currentBlogChanged(blogs->action(0));
        }
    }
    if (this->isVisible() == false) {
        this->show();
    }
}

void MainWindow::optionsPreferences()
{
    // The preference dialog is derived from prefs_base.ui
    //
    // compare the names of the widgets in the .ui file
    // to the names of the variables in the .kcfg file
    //avoid having 2 dialogs shown
    if (KConfigDialog::showDialog(QStringLiteral("settings")))  {
        return;
    }
    ConfigureDialog *dialog = new ConfigureDialog(this, QStringLiteral("settings"), Settings::self());
    connect(dialog, &ConfigureDialog::blogAdded, this, &MainWindow::slotBlogAdded);
    connect(dialog, &ConfigureDialog::blogEdited, this, &MainWindow::slotBlogEdited);
    connect(dialog, &ConfigureDialog::blogRemoved, this, &MainWindow::slotBlogRemoved);
    connect(dialog, &KConfigDialog::settingsChanged, this, &MainWindow::settingsChanged);
    connect(dialog, &KConfigDialog::settingsChanged, this, &MainWindow::slotSettingsChanged);
    connect(dialog, &ConfigureDialog::dialogDestroyed, this, &MainWindow::slotDialogDestroyed);
    connect(dialog, &ConfigureDialog::configurationChanged, this, &MainWindow::slotSettingsChanged);
    dialog->show();
}

void MainWindow::slotSettingsChanged()
{
    setupSystemTray();
}

void MainWindow::slotDialogDestroyed(QObject *win)
{
    const QSize size = qobject_cast<QWidget *>(win)->size();
    const QString name = win->objectName();
    if (name == QLatin1String("settings")) {
        Settings::setConfigWindowSize(size);
    }
}

void MainWindow::addBlog()
{
    AddEditBlog *addEditBlogWindow = new AddEditBlog(-1, this);
    addEditBlogWindow->setWindowModality(Qt::ApplicationModal);
    addEditBlogWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(addEditBlogWindow, &AddEditBlog::sigBlogAdded, this, &MainWindow::slotBlogAdded);
    addEditBlogWindow->show();
}

void MainWindow::slotBlogAdded(const BilboBlog &blog)
{
    QAction *act = new QAction(blog.title(), blogs);
    act->setCheckable(true);
    act->setData(blog.id());
    blogs->addAction(act);
    blogs->setCurrentAction(act);
    currentBlogChanged(act);
    toolbox->slotReloadCategoryList();
    toolbox->slotUpdateEntries(20);
}

void MainWindow::slotBlogEdited(const BilboBlog &blog)
{
    const int count = blogs->actions().count();
    for (int i = 0; i < count; ++i) {
        if (blogs->action(i)->data().toInt() == blog.id()) {
            blogs->action(i)->setText(blog.title());
            break;
        }
    }
}

void MainWindow::slotBlogRemoved(int blog_id)
{
    const int count = blogs->actions().count();
    for (int i = 0; i < count; ++i) {
        if (blogs->action(i)->data().toInt() == blog_id) {
            if (blogs->currentItem() == i) {
                blogs->setCurrentItem(i - 1);
                currentBlogChanged(blogs->action(i - 1));
            }
            blogs->removeAction(blogs->action(i));
            if (blogs->currentItem() == -1) {
                toolbox->clearFields();
            }
            break;
        }
    }
}

void MainWindow::setupSystemTray()
{
    if (Settings::enableSysTrayIcon()) {
        if (!systemTray) {
            systemTray = new KStatusNotifierItem(this);
            systemTray->setIconByName(QStringLiteral("blogilo"));
            systemTray->setToolTip(QStringLiteral("blogilo"), i18n("Blogilo"), i18n("A KDE Blogging Client"));
            systemTray->contextMenu()->addAction(actionCollection()->action(QStringLiteral("new_post")));
            systemTray->setCategory(KStatusNotifierItem::ApplicationStatus);
            systemTray->setStatus(KStatusNotifierItem::Active);
        }
    } else if (systemTray) {
        systemTray->deleteLater();
        systemTray = Q_NULLPTR;
    }
}

void MainWindow::slotPostTitleChanged(const QString &title)
{
    tabPosts->setTabText(tabPosts->currentIndex(), title);
}

void MainWindow::slotToggleToolboxVisible(bool isVisible)
{
    toolboxDock->setVisible(isVisible);
}

void MainWindow::slotToolboxVisibilityChanged(bool)
{
    actionCollection()->action(QStringLiteral("toggle_toolbox"))->setChecked(toolboxDock->isVisibleTo(this));
}

void MainWindow::slotActivePostChanged(int index)
{
    qCDebug(BLOGILO_LOG) << "new post index: " << index << "\tPrev Index: " << previousActivePostIndex;

    activePost = qobject_cast<PostEntry *>(tabPosts->widget(index));
    PostEntry *prevActivePost = qobject_cast<PostEntry *>(tabPosts->widget(previousActivePostIndex));
    int activePostBlogId = -1;
    int prevPostBlogId = -1;

    if ((prevActivePost != Q_NULLPTR) && (index != previousActivePostIndex)) {
        prevPostBlogId = prevActivePost->currentPostBlogId();
        toolbox->getFieldsValue(prevActivePost->currentPost());
        prevActivePost->setCurrentPostBlogId(mCurrentBlogId);
    }

    if (index >= 0) {
        activePostBlogId = activePost->currentPostBlogId();
        if (activePostBlogId != -1 && activePostBlogId != prevPostBlogId) {
            setCurrentBlog(activePostBlogId);
        }
        toolbox->setFieldsValue(activePost->currentPost());
    } else {
        qCCritical(BLOGILO_LOG) << "ActivePost is NULL! tabPosts Current index is: " << tabPosts->currentIndex();
    }
    previousActivePostIndex = index;
}

void MainWindow::slotPublishPost()
{
    if (mCurrentBlogId == -1) {
        KMessageBox::sorry(this, i18n("You have to select a blog to publish this post to."));
        return;
    }
    if (!activePost || tabPosts->currentIndex() == -1) {
        KMessageBox::sorry(this, i18n("There is no open post to submit."));
        return;
    }
    toolbox->getFieldsValue(activePost->currentPost());
    activePost->submitPost(mCurrentBlogId, *activePost->currentPost());
}

void MainWindow::slotRemoveAllExclude(int pos)
{
    for (int i = tabPosts->count() - 1; i >= 0; --i) {
        if (i == pos) {
            continue;
        }
        PostEntry *widget = qobject_cast<PostEntry *>(tabPosts->widget(i));
        if (!widget) {
            if (activePost) {
                widget = activePost;
            } else {
                return;
            }
        }
        DBMan::self()->removeTempEntry(*widget->currentPost());
        tabPosts->removeTab(tabPosts->indexOf(widget));
        widget->close();
    }
    if (tabPosts->count() < 1) {
        activePost = Q_NULLPTR;
        toolbox->resetFields();
//         actionCollection()->action("publish_post")->setEnabled( false );
    }
}

void MainWindow::slotRemovePostEntry(int pos)
{


    PostEntry *widget = qobject_cast<PostEntry *>(tabPosts->widget(pos));

    if (!widget) {
        if (activePost) {
            widget = activePost;
        } else {
            return;
        }
    }
    DBMan::self()->removeTempEntry(*widget->currentPost());
    tabPosts->removeTab(tabPosts->indexOf(widget));
    widget->close();

    if (tabPosts->count() < 1) {
        activePost = Q_NULLPTR;
        toolbox->resetFields();
//         actionCollection()->action("publish_post")->setEnabled( false );
    }
}

void MainWindow::slotNewPostOpened(BilboPost &newPost, int blog_id)
{

    QWidget *w = createPostEntry(blog_id, newPost);
    tabPosts->setCurrentWidget(w);
}

void MainWindow::slotSavePostLocally()
{

    if (activePost && (tabPosts->count() > 0)) {
        toolbox->getFieldsValue(activePost->currentPost());
        activePost->saveLocally();
        toolbox->reloadLocalPosts();
    }
}

void MainWindow::slotError(const QString &errorMessage)
{
    qCDebug(BLOGILO_LOG) << "Error message: " << errorMessage;
    KMessageBox::detailedError(this, i18n("An error occurred in the last transaction."), errorMessage);
    statusBar()->clearMessage();
    slotBusy(false);
}

void MainWindow::writeConfigs()
{

    if (toolboxDock->isVisible()) {
        Settings::setShowToolboxOnStart(true);
    } else {
        Settings::setShowToolboxOnStart(false);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->modifiers() == Qt::CTRL) {
        switch (event->key()) {
        case  Qt::Key_1:
            toolbox->setCurrentPage(0);
            break;
        case Qt::Key_2:
            toolbox->setCurrentPage(1);
            break;
        case Qt::Key_3:
            toolbox->setCurrentPage(2);
            break;
        case Qt::Key_4:
            toolbox->setCurrentPage(3);
            break;
        case Qt::Key_5:
            toolbox->setCurrentPage(4);
            break;
        case Qt::Key_W:
            slotRemovePostEntry(tabPosts->currentIndex());
            break;
        default:
            KXmlGuiWindow::keyPressEvent(event);
            break;
        }
    }
}

void MainWindow::postManipulationDone(bool isError, const QString &customMessage)
{

    if (isError) {
        KMessageBox::detailedError(this, i18n("Submitting post failed"), customMessage);
    } else {
        PostEntry *entry = qobject_cast<PostEntry *>(sender());
        if (entry) {
            if (KMessageBox::questionYesNo(this, i18n("%1\nDo you want to keep the post open?", customMessage),
                                           QString(), KStandardGuiItem::yes(), KStandardGuiItem::no(), QStringLiteral("KeepPostOpen")) == KMessageBox::No) {
                slotRemovePostEntry(tabPosts->indexOf(entry));
            } else {
                toolbox->setFieldsValue(entry->currentPost());
            }
        }
        toolbox->slotLoadEntriesFromDB(mCurrentBlogId);
    }
    this->unsetCursor();
    toolbox->unsetCursor();
}

void MainWindow::slotBusy(bool isBusy)
{
    qCDebug(BLOGILO_LOG) << "isBusy=" << isBusy << "\tbusyNumber=" << busyNumber;
    if (isBusy) {
        this->setCursor(Qt::BusyCursor);
        toolbox->setCursor(Qt::BusyCursor);
        ++busyNumber;
        if (!progress) {
            progress = new QProgressBar(statusBar());
            progress->setMinimum(0);
            progress->setMaximum(0);
            progress->setFixedSize(250, 17);
            statusBar()->addPermanentWidget(progress);
        }
    } else {
        --busyNumber;
        if (busyNumber < 1) {
            this->unsetCursor();
            toolbox->unsetCursor();
            if (progress) {
                statusBar()->removeWidget(progress);
                progress->deleteLater();
                progress = Q_NULLPTR;
            }
//             busyNumber = 0;
        }
    }
}

QWidget *MainWindow::createPostEntry(int blog_id, const BilboPost &post)
{

    PostEntry *temp = new PostEntry(this);
    temp->setAttribute(Qt::WA_DeleteOnClose);
    temp->setCurrentPost(post);
    temp->setCurrentPostBlogId(blog_id);

    connect(temp, &PostEntry::postTitleChanged, this, &MainWindow::slotPostTitleChanged);
    connect(temp, &PostEntry::postPublishingDone, this, &MainWindow::postManipulationDone);
    connect(this, &MainWindow::settingsChanged, temp, &PostEntry::settingsChanged);
    connect(temp, &PostEntry::showStatusMessage, this, &MainWindow::slotShowStatusMessage);

    connect(temp, &PostEntry::sigBusy, this, &MainWindow::slotBusy);

    tabPosts->addTab(temp, post.title());
    return temp;
}

void MainWindow::slotShowStatusMessage(const QString &message, bool isPermanent)
{
    statusBar()->showMessage(message, (isPermanent ? 0 : TIMEOUT));
}

void MainWindow::uploadMediaObject()
{
    UploadMediaDialog *uploadDlg = new UploadMediaDialog(this);
    connect(uploadDlg, &UploadMediaDialog::sigBusy, this, &MainWindow::slotBusy);
    if (mCurrentBlogId == -1) {
        uploadDlg->init(Q_NULLPTR);
    } else {
        uploadDlg->init(DBMan::self()->blog(mCurrentBlogId));
    }
}

void MainWindow::slotOpenCurrentBlogInBrowser()
{
    if (mCurrentBlogId > -1) {
        QUrl url(DBMan::self()->blog(mCurrentBlogId)->blogUrl());
        if (url.isValid()) {
            QDesktopServices::openUrl(url);
        } else {
            KMessageBox::sorry(this, i18n("Cannot find current blog URL."));
        }
    }
    ///TODO
    ///else show a message to the user saying that a blog should be selected before.
}

