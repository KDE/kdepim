/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>
    Copyright (C) 2013 Laurent Montel <montel@kde.org>

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
//krazy:excludeall=qmethods due to use of KStatusBar::showMessage()

#include "toolbox.h"
#include "dbman.h"
#include "entriescountdialog.h"
#include "addeditblog.h"
#include "backend.h"
#include "bilbopost.h"
#include "bilboblog.h"
#include "catcheckbox.h"
#include "settings.h"

#include <KMenu>
#include <KIcon>
#include <KAction>
#include <KToolInvocation>
#include <QClipboard>
#include <QTimer>
#include <kstatusbar.h>
#include <kdebug.h>
#include <kxmlguiwindow.h>
#include <kmessagebox.h>
#include <kdatetime.h>
#include <kurl.h>

class Toolbox::Private
{
public:
    QList<CatCheckBox*> listCategoryCheckBoxes;
    int mCurrentBlogId;
    KStatusBar *statusbar;
};
Toolbox::Toolbox( QWidget *parent )
    : QWidget( parent ), d(new Private)
{
    d->mCurrentBlogId = -1;
//QT5
#if 0
    if ( parent )
        d->statusbar = qobject_cast<KXmlGuiWindow*>( parent )->statusBar();
    else
#endif
        d->statusbar = new KStatusBar( this );
    setupUi( this );
    setButtonsIcon();
    frameCat->layout()->setAlignment( Qt::AlignTop );
    optionsDate->setDate( QDateTime::currentDateTime().date() );
    optionsTime->setTime( QDateTime::currentDateTime().time() );

    connect( btnCatReload, SIGNAL(clicked()), this, SLOT(slotReloadCategoryList()) );
    connect( btnEntriesUpdate, SIGNAL(clicked()), this, SLOT(slotUpdateEntries()) );
    connect( btnEntriesClear, SIGNAL(clicked(bool)), this, SLOT(clearEntries()) );

    connect( lstEntriesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
             this, SLOT(slotEntrySelected(QListWidgetItem*)) );
    connect( btnEntriesRemove, SIGNAL(clicked(bool)), this, SLOT(slotRemoveSelectedEntryFromServer()) );

    connect( btnOptionsNow, SIGNAL(clicked(bool)), this, SLOT(setDateTimeNow()) );
    connect( localEntries, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
             this, SLOT(slotLocalEntrySelected(QTreeWidgetItem*,int)) );
    connect( btnLocalRemove, SIGNAL(clicked(bool)) , this, SLOT(slotRemoveLocalEntry()) );

    lblOptionsTrackBack->setVisible( false );
    txtOptionsTrackback->setVisible( false );
    btnCatAdd->setVisible( false );

    lstEntriesList->setContextMenuPolicy( Qt::CustomContextMenu );
    connect( lstEntriesList, SIGNAL(customContextMenuRequested(QPoint)),
             this, SLOT(requestEntriesListContextMenu(QPoint)) );

    QTimer::singleShot(1000, this, SLOT(reloadLocalPosts()));
}

Toolbox::~Toolbox()
{
    delete d;
}


void Toolbox::setCurrentBlogId( int blog_id )
{
    if ( d->mCurrentBlogId == blog_id )
        return;
    d->mCurrentBlogId = blog_id;
    if ( blog_id <= 0 )
        return;
    slotLoadCategoryListFromDB( blog_id );
    slotLoadEntriesFromDB( blog_id );
    Qt::LayoutDirection ll = DBMan::self()->blogList().value( blog_id )->direction();
    frameCat->setLayoutDirection( ll );
    lstEntriesList->setLayoutDirection( ll );
}

void Toolbox::slotReloadCategoryList()
{
    if ( d->mCurrentBlogId == -1 ) {
        KMessageBox::sorry( this, i18n( "No blog has been selected: you have to select a blog from the Blogs page before asking for the list of categories." ) );
        return;
    }

    Backend *b = new Backend( d->mCurrentBlogId );
    connect( b, SIGNAL(sigCategoryListFetched(int)), this, SLOT(slotLoadCategoryListFromDB(int)) );
    connect( b, SIGNAL(sigError(QString)), this, SIGNAL(sigError(QString)) );
    emit sigBusy( true );
    d->statusbar->showMessage( i18n( "Requesting list of categories..." ) );
    b->getCategoryListFromServer();
}

void Toolbox::slotUpdateEntries(int count)
{
    if ( d->mCurrentBlogId == -1 ) {
        KMessageBox::sorry( this, i18n( "No blog has been selected: you have to select a blog from the Blogs page before asking for the list of entries." ) );
        return;
    }
    if (count == 0) {
        count = Settings::updateEntriesCount();
        if ( Settings::showUpdateEntriesDialog() ) {
            QPointer<EntriesCountDialog> dia = new EntriesCountDialog( this );
            if ( !dia->exec() ) {
                delete dia;
                return;
            }
            count = dia->count();
            dia->deleteLater();
        }
    }
    Backend *entryB = new Backend( d->mCurrentBlogId, this);
    entryB->getEntriesListFromServer( count );
    connect( entryB, SIGNAL(sigEntriesListFetched(int)), this, SLOT(slotLoadEntriesFromDB(int)) );
    connect( entryB, SIGNAL(sigError(QString)), this, SIGNAL(sigError(QString)) );
    d->statusbar->showMessage( i18n( "Requesting list of entries..." ) );
    setCursor( Qt::BusyCursor );
    emit sigBusy( true );
}

void Toolbox::slotLoadEntriesFromDB( int blog_id )
{
    if ( blog_id == -1 ) {
        kDebug() << "Blog Id doesn't set correctly";
        return;
    }
    lstEntriesList->clear();
    const QList<QVariantMap> listEntries = DBMan::self()->listPostsInfo( blog_id );
    const int count = listEntries.count();
    for ( int i=0; i < count; ++i ) {
        QListWidgetItem *lstItem = new QListWidgetItem( listEntries[i].value(QLatin1String("title")).toString() );
        lstItem->setToolTip(listEntries.at(i).value(QLatin1String("c_time")).toDateTime().toString());
        if (listEntries.at(i).value(QLatin1String("is_private")).toBool()) {
            lstItem->setForeground(QBrush(Qt::blue));
            lstItem->setToolTip(i18n("%1 (Draft)",lstItem->toolTip()));
        }
        lstItem->setData( BlogEntryID, listEntries.at(i).value(QLatin1String("id")).toInt() );
        lstEntriesList->addItem( lstItem );
    }
    d->statusbar->showMessage( i18n( "List of entries received." ), STATUSTIMEOUT );
    unsetCursor();
    emit sigBusy( false );
}

void Toolbox::slotLoadCategoryListFromDB( int blog_id )
{
    if ( blog_id == -1 ) {
        kDebug() << "Blog Id do not sets correctly";
        return;
    }
    clearCatList();
    QList<Category> listCategories;
    listCategories = DBMan::self()->listCategories( blog_id );

    QList<Category>::const_iterator i;
    QList<Category>::const_iterator endIt = listCategories.constEnd();
    for ( i = listCategories.constBegin(); i != endIt; ++i ) {
        CatCheckBox *cb = new CatCheckBox( i->name, this );
        cb->setCategory( *i );
        d->listCategoryCheckBoxes.append( cb );
        frameCat->layout()->addWidget( cb );
    }
    d->statusbar->showMessage( i18n( "List of categories received." ), STATUSTIMEOUT );
    unsetCursor();
    emit sigBusy( false );
}

void Toolbox::slotRemoveSelectedEntryFromServer()
{
    if (lstEntriesList->selectedItems().count() < 1)
        return;
    if ( KMessageBox::warningYesNoCancel(this, i18n( "Removing a post from your blog cannot be undone.\nAre you sure you want to remove the post with title \"%1\" from your blog?", lstEntriesList->currentItem()->text() ))
    == KMessageBox::Yes) {
        BilboPost *post = new BilboPost( DBMan::self()->getPostInfo( lstEntriesList->currentItem()->
                                                                     data(BlogEntryID).toInt() ) );
        Backend *b = new Backend( d->mCurrentBlogId, this);
        connect(b, SIGNAL(sigPostRemoved(int,BilboPost)), this, SLOT(slotPostRemoved(int,BilboPost)) );
        connect(b, SIGNAL(sigError(QString)), this, SLOT(slotError(QString)));
        b->removePost(post);
        d->statusbar->showMessage( i18n( "Removing post..." ) );
    }
}

void Toolbox::slotPostRemoved( int blog_id, const BilboPost &post )
{
    KMessageBox::information( this, i18nc( "Post removed from Blog", "Post with title \"%1\" removed from \"%2\".",
                                          post.title(), DBMan::self()->blogList().value(blog_id)->title() ) );
    slotLoadEntriesFromDB( blog_id );
    d->statusbar->showMessage( i18n( "Post removed" ), STATUSTIMEOUT );
    sender()->deleteLater();
}

void Toolbox::slotError(const QString& errorMessage)
{
    KMessageBox::detailedError( this, i18n( "An error occurred in the latest transaction." ), errorMessage );
    d->statusbar->showMessage( i18nc( "Operation failed", "Failed" ), STATUSTIMEOUT );
    sender()->deleteLater();
}

void Toolbox::clearFields()
{
    clearCatList();
    lstEntriesList->clear();
    txtCatTags->clear();
    chkOptionsTime->setChecked( false );
    optionsDate->setDate( QDateTime::currentDateTime().date() );
    optionsTime->setTime( QDateTime::currentDateTime().time() );
    txtOptionsTrackback->clear();
    txtSlug->clear();
    txtSummary->clear();
    chkOptionsComments->setChecked( true );
    chkOptionsTrackback->setChecked( true );
    comboOptionsStatus->setCurrentIndex( 0 );
}

void Toolbox::resetFields()
{
    unCheckCatList();
    txtCatTags->clear();
    chkOptionsTime->setChecked( false );
    optionsDate->setDate( QDateTime::currentDateTime().date() );
    optionsTime->setTime( QDateTime::currentDateTime().time() );
    txtOptionsTrackback->clear();
    txtSlug->clear();
    txtSummary->clear();
    chkOptionsComments->setChecked( true );
    chkOptionsTrackback->setChecked( true );
    comboOptionsStatus->setCurrentIndex( 0 );
}

void Toolbox::clearCatList()
{
    foreach( CatCheckBox* cat, d->listCategoryCheckBoxes ){
        cat->deleteLater();
    }
    d->listCategoryCheckBoxes.clear();
}

void Toolbox::getFieldsValue( BilboPost* currentPost )
{
    currentPost->setCategoryList( selectedCategories() );
    currentPost->setTags( currentTags() );
    currentPost->setModifyTimeStamp( chkOptionsTime->isChecked() );
    if ( currentPost->status() == KBlog::BlogPost::New ) {
        if ( chkOptionsTime->isChecked() ) {
            currentPost->setModificationDateTime( KDateTime( optionsDate->date(), optionsTime->time() ) );
            currentPost->setCreationDateTime( KDateTime( optionsDate->date(), optionsTime->time() ) );
        } else {
            currentPost->setCreationDateTime( KDateTime::currentLocalDateTime() );
            currentPost->setModificationDateTime( KDateTime::currentLocalDateTime() );
        }
    } else {
        currentPost->setCreationDateTime( KDateTime( optionsDate->date(), optionsTime->time() ) );
        currentPost->setModificationDateTime( KDateTime( optionsDate->date(), optionsTime->time() ) );
    }
    if ( currentPost->creationDateTime().isUtc() || currentPost->modificationDateTime().isUtc() ){
        kDebug()<<"creationDateTime was UTC!";
        currentPost->setCreationDateTime( KDateTime( currentPost->creationDateTime().dateTime(),
                                                    KDateTime::LocalZone ) );
        currentPost->setModificationDateTime( KDateTime( currentPost->modificationDateTime().dateTime(),
                                                    KDateTime::LocalZone ) );
    }
    currentPost->setSlug( txtSlug->text() );
    currentPost->setPrivate(( comboOptionsStatus->currentIndex() == 1 ) ? true : false );
    currentPost->setCommentAllowed( chkOptionsComments->isChecked() );
    currentPost->setTrackBackAllowed( chkOptionsTrackback->isChecked() );
    currentPost->setSummary( txtSummary->toPlainText() );
}

void Toolbox::setFieldsValue( BilboPost* post )
{
    if ( post == 0 ) {
        resetFields();
        kDebug()<<"post is NULL";
        return;
    }

    setSelectedCategories( post->categories() );
    txtCatTags->setText( post->tags().join( QLatin1String(", ") ) );
//     kDebug() << "Post status is: " << post->status();
    if ( post->status() == KBlog::BlogPost::New )
        comboOptionsStatus->setCurrentIndex( 2 );
    else if ( post->isPrivate() )
        comboOptionsStatus->setCurrentIndex( 1 );
    else
        comboOptionsStatus->setCurrentIndex( 0 );
    chkOptionsComments->setChecked( post->isCommentAllowed() );
    chkOptionsTrackback->setChecked( post->isTrackBackAllowed() );
    chkOptionsTime->setChecked( post->isModifyTimeStamp() );
    if ( post->creationDateTime().isUtc() || post->modificationDateTime().isUtc() ){
        kDebug()<<"creationDateTime was UTC!";
        post->setCreationDateTime(KDateTime(post->creationDateTime().dateTime(), KDateTime::LocalZone));
        post->setModificationDateTime(KDateTime(post->modificationDateTime().dateTime(), KDateTime::LocalZone));
    }
    optionsTime->setTime( post->creationDateTime().time() );
    optionsDate->setDate( post->creationDateTime().date() );
    txtSlug->setText( KUrl::fromPercentEncoding( post->slug().toLatin1() ) );
    txtSummary->setPlainText( post->summary() );
}

QList< Category > Toolbox::selectedCategories() const
{
    QList<Category> list;
    const int count = d->listCategoryCheckBoxes.count();
    for ( int i = 0; i < count; ++i ) {
        if ( d->listCategoryCheckBoxes.at(i)->isChecked() )
            list.append( d->listCategoryCheckBoxes.at(i)->category() );
    }
    return list;
}

QStringList Toolbox::selectedCategoriesTitle() const
{
    QStringList list;
    const int count = d->listCategoryCheckBoxes.count();
    for ( int i = 0; i < count; ++i ) {
        if ( d->listCategoryCheckBoxes.at(i)->isChecked() )
            list.append( d->listCategoryCheckBoxes.at(i)->category().name );
    }
    return list;
}

void Toolbox::setSelectedCategories( const QStringList &list )
{
    unCheckCatList();
    int count = d->listCategoryCheckBoxes.count();
    for ( int i = 0; i < count; ++i ) {
        if ( list.contains( d->listCategoryCheckBoxes.at(i)->category().name, Qt::CaseInsensitive ) )
            d->listCategoryCheckBoxes.at(i)->setChecked( true );
    }
}

QStringList Toolbox::currentTags()
{
    QStringList t = txtCatTags->text().split( QRegExp( QString::fromUtf8(",|،") ), QString::SkipEmptyParts );
    for ( int i = 0; i < t.count() ; ++i ) {
        t[i] = t[i].trimmed();
    }
    return t;
}

void Toolbox::slotEntrySelected( QListWidgetItem * item )
{
    BilboPost post = DBMan::self()->getPostInfo( item->data( BlogEntryID ).toInt() );
    kDebug() << "Emiting sigEntrySelected...";
    Q_EMIT sigEntrySelected( post, d->mCurrentBlogId );
}

void Toolbox::setCurrentPage( int index )
{
    box->setCurrentIndex( index );
}

void Toolbox::slotEntriesCopyUrl()
{
    if ( lstEntriesList->currentItem() == 0 ) {
        return;
    }
    BilboPost post = DBMan::self()->getPostInfo( lstEntriesList->currentItem()->data( BlogEntryID ).toInt() );
    if ( !post.permaLink().isEmpty() )
        QApplication::clipboard()->setText( post.permaLink().prettyUrl() );
    else if ( !post.link().isEmpty() )
        QApplication::clipboard()->setText( post.link().prettyUrl() );
    else
        KMessageBox::sorry(this, i18n( "No link field is available in the database for this entry." ) );
}

void Toolbox::unCheckCatList()
{
    const int count = d->listCategoryCheckBoxes.count();
    for ( int j = 0; j < count; ++j ) {
        d->listCategoryCheckBoxes.at(j)->setChecked( false );
    }
}

void Toolbox::setButtonsIcon()
{
    btnEntriesUpdate->setIcon( KIcon( QLatin1String("view-refresh") ) );
    btnEntriesRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
    btnEntriesClear->setIcon( KIcon( QLatin1String("edit-clear") ) );
    btnCatReload->setIcon( KIcon( QLatin1String("view-refresh") ) );
    btnCatAdd->setIcon( KIcon( QLatin1String("list-add") ) );
    btnLocalRemove->setIcon( KIcon( QLatin1String("list-remove") ) );
    ///TODO Add option for selecting only text or only Icon for Toolbox buttons!
}

void Toolbox::reloadLocalPosts()
{
    kDebug();

    localEntries->clear();

    QList<QVariantMap> localList = DBMan::self()->listLocalPosts();
    const int count = localList.count();

    for (int i=0; i < count; ++i){
        const QString postTitle = localList.at(i).value( QLatin1String("post_title") ).toString();
        const QString blogTitle = localList.at(i).value( QLatin1String("blog_title") ).toString();

        QTreeWidgetItem *item = new QTreeWidgetItem( localEntries, QStringList()<<postTitle<<blogTitle );
        item->setData(0, LocalEntryID, localList.at(i).value( QLatin1String("local_id") ).toInt());
        item->setData(1, LocalEntryID, localList.at(i).value( QLatin1String("blog_id") ).toInt());
    }
}

void Toolbox::slotLocalEntrySelected( QTreeWidgetItem* item,int column )
{
    kDebug()<<"Emitting sigEntrySelected...";
    Q_UNUSED(column);
    BilboPost post = DBMan::self()->localPost(item->data(0, LocalEntryID).toInt());
    emit sigEntrySelected( post, item->data(1, LocalEntryID).toInt() );
}

void Toolbox::slotRemoveLocalEntry()
{
    if (localEntries->currentItem()) {
        if ( KMessageBox::warningYesNo(this, i18n("Are you sure you want to remove the selected local entry?"))
            == KMessageBox::No )
            return;
        const int local_id = localEntries->currentItem()->data(0,LocalEntryID).toInt();
        if ( DBMan::self()->removeLocalEntry(local_id) ) {
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
    if ( d->mCurrentBlogId == -1 )
        return;
    if ( KMessageBox::warningContinueCancel(this, i18n("Are you sure you want to clear the list of entries?")) ==
         KMessageBox::Cancel )
        return;
    if ( DBMan::self()->clearPosts( d->mCurrentBlogId ) )
        lstEntriesList->clear();
    else
        KMessageBox::detailedSorry(this, i18n( "Cannot clear the list of entries." ) , DBMan::self()->lastErrorText());
}

void Toolbox::setDateTimeNow()
{
    optionsDate->setDate( QDate::currentDate() );
    optionsTime->setTime( QTime::currentTime() );
}

void Toolbox::requestEntriesListContextMenu( const QPoint & pos )
{
    if ( lstEntriesList->selectedItems().isEmpty() )
        return;
    Q_UNUSED(pos);
    KMenu *entriesContextMenu = new KMenu;
    KAction *actEntriesOpenInBrowser = new KAction( KIcon(QLatin1String("applications-internet")),
                                                    i18n("Open in browser"), entriesContextMenu );
    connect( actEntriesOpenInBrowser, SIGNAL(triggered()), this, SLOT(openPostInBrowser()) );
    KAction *actEntriesCopyUrl = new KAction( KIcon(QLatin1String("edit-copy")),
                                              i18n("Copy URL"), entriesContextMenu );
    connect( actEntriesCopyUrl, SIGNAL(triggered(bool)), this, SLOT(slotEntriesCopyUrl()) );
    KAction *actEntriesCopyTitle = new KAction( KIcon(QLatin1String("edit-copy")),
                                                i18n("Copy title"), entriesContextMenu );
    connect( actEntriesCopyTitle, SIGNAL(triggered(bool)), this, SLOT(copyPostTitle()) );
    entriesContextMenu->addAction( actEntriesOpenInBrowser );
    entriesContextMenu->addAction( actEntriesCopyUrl );
    entriesContextMenu->addAction( actEntriesCopyTitle );
    entriesContextMenu->exec( QCursor::pos() );
    delete entriesContextMenu;
}

void Toolbox::openPostInBrowser()
{
    if ( lstEntriesList->selectedItems().isEmpty() )
        return;
    BilboPost post = DBMan::self()->getPostInfo( lstEntriesList->currentItem()->data( BlogEntryID ).toInt() );
    QString url;
    if ( !post.permaLink().isEmpty() )
        url = post.permaLink().pathOrUrl();
    else if ( !post.link().isEmpty() )
        url = post.link().pathOrUrl();
    else
        url = DBMan::self()->blogList().value( d->mCurrentBlogId )->blogUrl();
    KToolInvocation::invokeBrowser ( url );
}

void Toolbox::copyPostTitle()
{
    if ( !lstEntriesList->selectedItems().isEmpty() )
        QApplication::clipboard()->setText( lstEntriesList->currentItem()->text() );
}

