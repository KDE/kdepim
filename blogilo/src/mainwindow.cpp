/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2009 Golnaz Nilieh <g382nilieh@gmail.com>

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
#include "composer/multilinetextedit.h"
#include "blogsettings.h"
// #include "htmleditor.h"

#include "ui_advancedsettingsbase.h"
#include "ui_settingsbase.h"
#include "ui_editorsettingsbase.h"

#include <ktabwidget.h>
#include <ksystemtrayicon.h>
#include <kstatusbar.h>
#include <KToggleAction>
#include <kactioncollection.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <KDE/KLocale>
#include <QDir>
#include <QMenu>
#include <QDockWidget>
#include <QProgressBar>
#include <KSelectAction>
#include <kimagefilepreview.h>
#include "uploadmediadialog.h"
#include <QTimer>

#define TIMEOUT 5000
#include <KToolInvocation>

MainWindow::MainWindow()
    : KXmlGuiWindow(), mCurrentBlogId(__currentBlogId)
{
    kDebug();
    previousActivePostIndex = -1;
    activePost = 0;
    systemTray = 0;
    busyNumber = 0;
    progress = 0;
    this->setWindowTitle( i18n("Blogilo") );

    tabPosts = new KTabWidget( this );
    tabPosts->setElideMode( Qt::ElideRight );///TODO make this Optional!
    tabPosts->setTabCloseActivatePrevious( true );
    setCentralWidget( tabPosts );
//     this->setDockOptions( QMainWindow::ForceTabbedDocks);

    toolbox = new Toolbox( this );
    toolboxDock = new QDockWidget( i18n( "Toolbox" ), this );
    toolboxDock->setAllowedAreas( Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea );
    toolboxDock->setFeatures( QDockWidget::AllDockWidgetFeatures );
    toolboxDock->setObjectName( "dock_toolbox" );
    toolbox->setObjectName( "toolbox" );
    toolboxDock->setWidget( toolbox );
    this->addDockWidget( Qt::RightDockWidgetArea, toolboxDock );

    btnRemovePost = new QToolButton( tabPosts );
    btnRemovePost->setIcon( KIcon( "tab-close" ) );
    btnRemovePost->setToolTip( i18n( "Close tab" ) );
    tabPosts->setCornerWidget( btnRemovePost, Qt::TopRightCorner );
    connect( btnRemovePost, SIGNAL( clicked( bool ) ), this, SLOT( sltRemovePostEntry() ) );

    // then, setup our actions
    setupActions();

    // add a status bar
    statusBar()->show();
//     toolbox->show();

    // a call to KXmlGuiWindow::setupGUI() populates the GUI
    // with actions, using KXMLGUI.
    // It also applies the saved mainwindow settings, if any, and ask the
    // mainwindow to automatically save settings if changed: window size,
    // toolbar position, icon size, etc.
    setupGUI();

    toolbox->setVisible( Settings::showToolboxOnStart() );
    actionCollection()->action("toggle_toolbox")->setChecked( Settings::showToolboxOnStart() );

    setupSystemTray();

    connect( tabPosts, SIGNAL( currentChanged( int ) ), this, SLOT( sltActivePostChanged( int ) ) );
    connect( toolbox, SIGNAL( sigEntrySelected( BilboPost &, int ) ), this, SLOT( sltNewPostOpened( BilboPost&, int ) ) );
//     connect( toolbox, SIGNAL( sigCurrentBlogChanged( int ) ), this, SLOT( sltCurrentBlogChanged( int ) ) );
    connect( toolbox, SIGNAL( sigError( const QString& ) ), this, SLOT( sltError( const QString& ) ) );
    connect( toolbox, SIGNAL( sigBusy(bool) ), this, SLOT( slotBusy(bool) ));

    QList<BilboBlog*> blogList = DBMan::self()->blogList().values();
    int count = blogList.count();
    for(int i=0; i < count; ++i) {
        QAction *act = new QAction( blogList[i]->title(), blogs );
        act->setCheckable( true );
        act->setData( blogList[i]->id() );
        blogs->addAction( act );
    }
    connect( blogs, SIGNAL(triggered( QAction* )), this, SLOT(currentBlogChanged(QAction*)) );
    QTimer::singleShot( 0, this, SLOT( loadTempPosts() ) );
}

MainWindow::~MainWindow()
{
    kDebug();
    int count = tabPosts->count();
    for(int i =0; i<count; ++i)
        qobject_cast<PostEntry*>(tabPosts->widget(i))->aboutToQuit();
    writeConfigs();
    qApp->quit();
}

void MainWindow::setupActions()
{
    KStandardAction::quit( this, SLOT( close() ), actionCollection() );

    KStandardAction::preferences( this, SLOT( optionsPreferences() ), actionCollection() );

    // custom menu and menu item
    KAction *actNewPost = new KAction( KIcon( "document-new" ), i18n( "New Post" ), this );
    actionCollection()->addAction( QLatin1String( "new_post" ), actNewPost );
    actNewPost->setShortcut( Qt::CTRL + Qt::Key_N );
    connect( actNewPost, SIGNAL( triggered( bool ) ), this, SLOT( sltCreateNewPost() ) );

    KAction *actAddBlog = new KAction( KIcon( "list-add" ), i18n( "Add Blog..." ), this );
    actionCollection()->addAction( QLatin1String( "add_blog" ), actAddBlog );
    connect( actAddBlog, SIGNAL( triggered( bool ) ), this, SLOT( addBlog() ) );

    KAction *actPublish = new KAction( KIcon( "arrow-up" ), i18n( "Submit..." ), this );
    actionCollection()->addAction( QLatin1String( "publish_post" ), actPublish );
    connect( actPublish, SIGNAL( triggered( bool ) ), this, SLOT( sltPublishPost() ) );

    KAction *actUpload = new KAction( KIcon( "upload-media" ), i18n( "Upload Media..." ), this );
    actionCollection()->addAction( QLatin1String( "upload_media" ), actUpload );
    connect( actUpload, SIGNAL( triggered( bool ) ), this, SLOT( uploadMediaObject() ) );

    KAction *actSaveLocally = new KAction( KIcon( "document-save" ), i18n( "Save Locally" ), this );
    actionCollection()->addAction( QLatin1String( "save_locally" ), actSaveLocally );
    actSaveLocally->setShortcut( Qt::CTRL + Qt::Key_S );
    connect( actSaveLocally, SIGNAL( triggered( bool ) ), this, SLOT( sltSavePostLocally() ) );

    KToggleAction *actToggleToolboxVisible = new KToggleAction( i18n( "Show Toolbox" ), this );
    actionCollection()->addAction( QLatin1String( "toggle_toolbox" ), actToggleToolboxVisible );
    actToggleToolboxVisible->setShortcut( Qt::CTRL + Qt::Key_T );
    connect( actToggleToolboxVisible, SIGNAL( toggled( bool ) ),
             this, SLOT( sltToggleToolboxVisible( bool ) ) );
    connect( toolboxDock, SIGNAL(visibilityChanged(bool)),
             this, SLOT( slotToolboxVisibilityChanged(bool) ) );

    KAction *actClearImageCache = new KAction( KIcon( "edit-clear" ), i18n( "Clear Cached Images" ), this );
    actionCollection()->addAction( QLatin1String( "clear_image_cache" ), actClearImageCache );
    connect( actClearImageCache, SIGNAL( triggered( bool ) ), this, SLOT( sltClearCache() ) );

    blogs = new KSelectAction( this );
    actionCollection()->addAction( QLatin1String( "blogs_list" ), blogs );

    KAction *actOpenBlog = new KAction(KIcon("applications-internet"), i18n("Open in browser"), this);
    actionCollection()->addAction( QLatin1String("open_blog_in_browser"), actOpenBlog);
    actOpenBlog->setToolTip(i18n("Open current blog in browser"));
    connect( actOpenBlog, SIGNAL(triggered(bool)), this, SLOT(slotOpenCurrentBlogInBrowser()) );
}

void MainWindow::loadTempPosts()
{
    kDebug();
    QMap<BilboPost*, int> tempList = DBMan::self()->listTempPosts();
    int count = tempList.count();
    if( count > 0 ){
        QMap<BilboPost*, int>::ConstIterator it = tempList.constBegin();
        QMap<BilboPost*, int>::ConstIterator endIt = tempList.constEnd();
        for( ; it != endIt; ++it ) {
            createPostEntry(it.value(), (*it.key()));
        }
    } else {
        sltCreateNewPost();
    }
//     activePost = qobject_cast<PostEntry*>( tabPosts->currentWidget() );
    previousActivePostIndex = 0;
    if( activePost )
        setCurrentBlog( activePost->currentPostBlogId() );
}

void MainWindow::setCurrentBlog( int blog_id )
{
    kDebug()<<blog_id;
    if(blog_id == -1) {
        blogs->setCurrentItem( -1 );
        toolbox->setCurrentBlogId( blog_id );
//         actionCollection()->action("publish_post")->setEnabled( false );
        return;
    }
    int count = blogs->items().count();
    for (int i=0; i<count; ++i) {
        if( blogs->action(i)->data().toInt() == blog_id ) {
            blogs->setCurrentItem( i );
            currentBlogChanged( blogs->action( i ) );
            break;
        }
    }
}

void MainWindow::currentBlogChanged( QAction *act )
{
    if( act ) {
        if( mCurrentBlogId == act->data().toInt() )
            return;
        mCurrentBlogId = act->data().toInt();
//         __currentBlogId = mCurrentBlogId;
        if( activePost ) {
//             actionCollection()->action("publish_post")->setEnabled( true );
            activePost->setCurrentPostBlogId( mCurrentBlogId );
        } else {
//             actionCollection()->action("publish_post")->setEnabled( false );
        }
        blogs->setToolTip( DBMan::self()->blogList().value( mCurrentBlogId )->blogUrl() );
    } else {
        mCurrentBlogId = -1;
        if( activePost )
            activePost->setCurrentPostBlogId( mCurrentBlogId );
    }
    toolbox->setCurrentBlogId( mCurrentBlogId );
}

void MainWindow::sltCreateNewPost()
{
    kDebug();

    tabPosts->setCurrentWidget( createPostEntry( mCurrentBlogId, BilboPost()) );
    if( mCurrentBlogId == -1 ) {
        if( blogs->items().count() > 0 ) {
            blogs->setCurrentItem( 0 );
            currentBlogChanged( blogs->action( 0 ) );
        }
    }
    if ( this->isVisible() == false ) {
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
    if ( KConfigDialog::showDialog( "settings" ) )  {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog( this, "settings", Settings::self() );
    QWidget *generalSettingsDlg = new QWidget;
    generalSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    Ui::SettingsBase ui_prefs_base;
    Ui::EditorSettingsBase ui_editorsettings_base;
    ui_prefs_base.setupUi( generalSettingsDlg );
    BlogSettings *blogSettingsDlg = new BlogSettings;
    blogSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    connect( blogSettingsDlg, SIGNAL(blogAdded(const BilboBlog &)),
             this, SLOT(slotBlogAdded(const BilboBlog &)) );
    connect( blogSettingsDlg, SIGNAL(blogEdited(const BilboBlog &)),
             this, SLOT(slotBlogEdited(const BilboBlog &)) );
    connect( blogSettingsDlg, SIGNAL(blogRemoved(int)), this, SLOT(slotBlogRemoved(int)) );
    QWidget *editorSettingsDlg = new QWidget;
    editorSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    ui_editorsettings_base.setupUi( editorSettingsDlg );
    QWidget *advancedSettingsDlg = new QWidget;
    advancedSettingsDlg->setAttribute( Qt::WA_DeleteOnClose );
    Ui::AdvancedSettingsBase ui_advancedsettings_base;
    ui_advancedsettings_base.setupUi( advancedSettingsDlg );

//     QWidget *htmlEditorSettings = HtmlEditor::self()->configPage( 0, dialog );
//     dialog->addPage( htmlEditorSettings, i18n( "HTML Editor" ), "configure" );
    dialog->addPage( generalSettingsDlg, i18nc( "Configure Page", "General" ), "configure" );
    dialog->addPage( blogSettingsDlg, i18nc( "Configure Page", "Blogs" ), "document-properties");
    dialog->addPage( editorSettingsDlg, i18nc( "Configure Page", "Editor" ), "accessories-text-editor" );
    dialog->addPage( advancedSettingsDlg, i18nc( "Configure Page", "Advanced" ), "applications-utilities");
    connect( dialog, SIGNAL( settingsChanged( const QString& ) ), this, SIGNAL( settingsChanged() ) );
    connect( dialog, SIGNAL(settingsChanged(const QString& )), this, SLOT(slotSettingsChanged()) );
    connect( dialog, SIGNAL(destroyed(QObject*)), this, SLOT(slotDialogDestroyed(QObject*)));
    dialog->setAttribute( Qt::WA_DeleteOnClose );
    dialog->resize( Settings::configWindowSize() );
    dialog->show();
}

void MainWindow::slotSettingsChanged()
{
    setupSystemTray();
}

void MainWindow::slotDialogDestroyed( QObject *win )
{
    QSize size = qobject_cast<QWidget *>(win)->size();
    QString name = win->objectName();
    if(name == "settings") {
        Settings::setConfigWindowSize( size );
    }
}

void MainWindow::addBlog()
{
    AddEditBlog *addEditBlogWindow = new AddEditBlog( -1, this );
    addEditBlogWindow->setWindowModality( Qt::ApplicationModal );
    addEditBlogWindow->setAttribute( Qt::WA_DeleteOnClose );
    connect( addEditBlogWindow, SIGNAL( sigBlogAdded( const BilboBlog& ) ),
             this, SLOT( slotBlogAdded( const BilboBlog& ) ) );
    addEditBlogWindow->show();
}

void MainWindow::slotBlogAdded( const BilboBlog &blog )
{
    QAction *act = new QAction( blog.title(), blogs );
    act->setCheckable( true );
    act->setData( blog.id() );
    blogs->addAction( act );
    blogs->setCurrentAction( act );
    currentBlogChanged( act );
    toolbox->sltReloadCategoryList();
    toolbox->sltUpdateEntries( 20 );
}

void MainWindow::slotBlogEdited( const BilboBlog &blog )
{
    int count = blogs->actions().count();
    for(int i=0; i< count; ++i){
        if( blogs->action( i )->data().toInt() == blog.id() ) {
            blogs->action( i )->setText( blog.title() );
            break;
        }
    }
}

void MainWindow::slotBlogRemoved( int blog_id )
{
    int count = blogs->actions().count();
    for(int i=0; i< count; ++i){
        if( blogs->action( i )->data().toInt() == blog_id ) {
            if( blogs->currentItem() == i ) {
                blogs->setCurrentItem( i-1 );
                currentBlogChanged( blogs->action( i-1 ) );
            }
            blogs->removeAction( blogs->action( i ) );
            if(blogs->currentItem() == -1)
                toolbox->clearFields();
            break;
        }
    }
}

void MainWindow::setupSystemTray()
{
    if( Settings::enableSysTrayIcon()) {
        if ( !systemTray ) {
            systemTray = new KSystemTrayIcon( this );
            systemTray->setIcon(this->windowIcon());
            systemTray->setToolTip( i18n("Blogilo") );
            systemTray->contextMenu()->addAction( actionCollection()->action("new_post") );
            systemTray->show();
        }
    } else if( systemTray ) {
        systemTray->deleteLater();
        systemTray = 0;
    }
}

void MainWindow::sltUploadAllChanges()
{
    kDebug();
}

void MainWindow::sltPostTitleChanged( const QString& title )
{
//     kDebug();
    tabPosts->setTabText( tabPosts->currentIndex(), title );
}

void MainWindow::sltToggleToolboxVisible( bool isVisible )
{
    toolboxDock->setVisible( isVisible );
}

void MainWindow::slotToolboxVisibilityChanged(bool)
{
    actionCollection()->action(QLatin1String("toggle_toolbox"))->setChecked( toolboxDock->isVisibleTo(this) );
}

void MainWindow::sltActivePostChanged( int index )
{
    kDebug() << "new post index: " << index << "\tPrev Index: " << previousActivePostIndex;

    activePost = qobject_cast<PostEntry*>( tabPosts->widget( index ) );
    PostEntry *prevActivePost = qobject_cast<PostEntry*>( tabPosts->widget( previousActivePostIndex ) );
    int activePostBlogId = -1;
    int prevPostBlogId = -1;

    if (( prevActivePost != 0 ) && ( index != previousActivePostIndex ) ) {
        prevPostBlogId = prevActivePost->currentPostBlogId();
        toolbox->getFieldsValue( *prevActivePost->currentPost() );
        prevActivePost->setCurrentPostBlogId( mCurrentBlogId );
    }

    if ( index >= 0 ) {
        activePostBlogId = activePost->currentPostBlogId();
        if ( activePostBlogId != -1 && activePostBlogId != prevPostBlogId ) {
                setCurrentBlog( activePostBlogId );
        }
        toolbox->setFieldsValue( activePost->currentPost() );
    } else {
        kError() << "ActivePost is NULL! tabPosts Current index is: " << tabPosts->currentIndex() ;
    }
    previousActivePostIndex = index;
}

void MainWindow::sltPublishPost()
{
    kDebug();
    if ( mCurrentBlogId == -1 ) {
        KMessageBox::sorry( this, i18n( "You have to select a blog to publish this post to." ) );
        kDebug() << "Blog id not sets correctly.";
        return;
    }
    if( !activePost || tabPosts->currentIndex() == -1) {
        KMessageBox::sorry( this, i18n( "There is no open post to submit." ) );
        kDebug() << "There isn't any post";
        return;
    }
    BilboPost post = *activePost->currentPost();
    toolbox->getFieldsValue( post );
//     post.setPrivate( false );
    activePost->submitPost( mCurrentBlogId, post );
}

void MainWindow::sltRemovePostEntry( PostEntry *widget )
{
    kDebug();
    if( !widget ) {
        widget = activePost;
    }
    DBMan::self()->removeTempEntry( *widget->currentPost() );
    tabPosts->removePage(widget);
    widget->close();

//     if(tabPosts->count() == 1)
//         tabPosts->setTabBarHidden(true);
//     else
//         tabPosts->setTabBarHidden(false);

//     if( tabPosts->count() == 0 ){
//         sltCreateNewPost();
//         previousActivePostIndex = 0;
//     }
    if( tabPosts->count() < 1 ) {
        activePost = 0;
        toolbox->resetFields();
//         actionCollection()->action("publish_post")->setEnabled( false );
    }
}

void MainWindow::sltNewPostOpened( BilboPost &newPost, int blog_id )
{
    kDebug();
    QWidget * w = createPostEntry( blog_id, newPost );
    tabPosts->setCurrentWidget( w );
}

void MainWindow::sltSavePostLocally()
{
    kDebug();
    if(activePost && tabPosts->count() > 0) {
        toolbox->getFieldsValue(*activePost->currentPost());
        activePost->saveLocally();
        toolbox->reloadLocalPosts();
    }
}

void MainWindow::sltError( const QString & errorMessage )
{
    kDebug() << "Error message: " << errorMessage;
    KMessageBox::detailedError( this, i18n( "An error occurred in the last transaction." ), errorMessage );
    statusBar()->clearMessage();
    slotBusy(false);
}

void MainWindow::writeConfigs()
{
    kDebug();
    if ( toolboxDock->isVisible() )
        Settings::setShowToolboxOnStart( true );
    else
        Settings::setShowToolboxOnStart( false );
}

void MainWindow::keyPressEvent( QKeyEvent * event )
{
    if ( event->modifiers() == Qt::CTRL ) {
        switch ( event->key() ) {
            case  Qt::Key_1:
                toolbox->setCurrentPage( 0 );
                break;
            case Qt::Key_2:
                toolbox->setCurrentPage( 1 );
                break;
            case Qt::Key_3:
                toolbox->setCurrentPage( 2 );
                break;
            case Qt::Key_4:
                toolbox->setCurrentPage( 3 );
                break;
            case Qt::Key_5:
                toolbox->setCurrentPage( 4 );
                break;
            case Qt::Key_W:
                sltRemovePostEntry();
                break;
            default:
                KXmlGuiWindow::keyPressEvent( event );
                break;
        }
    }
}

void MainWindow::postManipulationDone( bool isError, const QString &customMessage )
{
    kDebug();
    if(isError){
        KMessageBox::detailedError(this, i18n("Submitting post failed"), customMessage);
    } else {
        PostEntry *entry = qobject_cast<PostEntry*>(sender());
        if(entry && KMessageBox::questionYesNo(this, i18n("%1\nDo you want to keep the post open?", customMessage),
                    QString(), KStandardGuiItem::yes(), KStandardGuiItem::no(), "KeepPostOpen") == KMessageBox::No ) {
            sltRemovePostEntry(entry);
        } else {
            toolbox->setFieldsValue(entry->currentPost());
        }
        toolbox->sltLoadEntriesFromDB( mCurrentBlogId );
    }
    this->unsetCursor();
    toolbox->unsetCursor();
}

void MainWindow::slotBusy(bool isBusy)
{
    kDebug()<<"isBusy="<<isBusy<<"\tbusyNumber="<<busyNumber;
    if(isBusy){
        this->setCursor(Qt::BusyCursor);
        toolbox->setCursor( Qt::BusyCursor );
        ++busyNumber;
        if(!progress){
            progress = new QProgressBar(statusBar());
            progress->setMinimum( 0 );
            progress->setMaximum( 0 );
            progress->setFixedSize(250, 17);
            statusBar()->addPermanentWidget(progress);
        }
    } else {
        --busyNumber;
        if( busyNumber < 1 ){
            this->unsetCursor();
            toolbox->unsetCursor();
            if(progress){
                statusBar()->removeWidget(progress);
                progress->deleteLater();
                progress = 0;
            }
//             busyNumber = 0;
        }
    }
}

QWidget* MainWindow::createPostEntry(int blog_id, const BilboPost& post)
{
    kDebug();
    PostEntry *temp = new PostEntry( this );
    temp->setAttribute( Qt::WA_DeleteOnClose );
    temp->setCurrentPost(post);
    temp->setCurrentPostBlogId( blog_id );

    connect( temp, SIGNAL( sigTitleChanged( const QString& ) ),
             this, SLOT( sltPostTitleChanged( const QString& ) ) );
    connect( temp, SIGNAL( postPublishingDone( bool, const QString& ) ),
            this, SLOT( postManipulationDone( bool, const QString& ) ) );
    connect( this, SIGNAL( settingsChanged() ), temp, SLOT( settingsChanged() ));
    connect( temp, SIGNAL( showStatusMessage(QString,bool)),
             this, SLOT(slotShowStatusMessage(QString,bool)));

    connect( temp, SIGNAL( sigBusy( bool ) ), this, SLOT( slotBusy( bool ) ) );

    tabPosts->addTab( temp, post.title() );
//     if(tabPosts->count() == 1)
//         tabPosts->setTabBarHidden(true);
//     else
//         tabPosts->setTabBarHidden(false);
    return temp;
}

void MainWindow::sltClearCache()
{
    QDir cacheDir( CACHED_MEDIA_DIR );
    QStringListIterator i( cacheDir.entryList() );
    while ( i.hasNext() ) {
        cacheDir.remove( i.next() );
    }
    MultiLineTextEdit::clearCache();
}

void MainWindow::slotShowStatusMessage(const QString &message, bool isPermanent)
{
    statusBar()->showMessage(message, (isPermanent ? 0 : TIMEOUT));
}

void MainWindow::uploadMediaObject()
{
    UploadMediaDialog *uploadDlg = new UploadMediaDialog(this);
    connect(uploadDlg, SIGNAL(sigBusy(bool)), SLOT(slotBusy(bool)));
    if(mCurrentBlogId == -1)
        uploadDlg->init( 0 );
    else
        uploadDlg->init( &DBMan::self()->blog(mCurrentBlogId) );
}

void MainWindow::slotOpenCurrentBlogInBrowser()
{
    if (mCurrentBlogId > -1)
    {
	KUrl url( DBMan::self()->blog( mCurrentBlogId ).blogUrl() );
	if(url.isValid())
	    KToolInvocation::invokeBrowser(url.url());
	else
	    KMessageBox::sorry(this, i18n("Cannot find current blog URL."));
    }
    ///TODO
    ///else show a massege to the user saying that a blog should be selected before.
}

#include "mainwindow.moc"
