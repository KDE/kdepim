/*
    This file is part of Blogilo, A KDE Blogging Client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2008-2010 Golnaz Nilieh <g382nilieh@gmail.com>

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

#include "addeditblog.h"

#include <kblog/gdata.h>
#include <kblog/blogger1.h>
#include <kblog/metaweblog.h>
#include <kblog/movabletype.h>
#include <kblog/wordpressbuggy.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <kio/jobclasses.h>
#include <kio/job.h>

#include "waitwidget.h"
#include "bilboblog.h"
#include "dbman.h"
#include <QTableWidget>
#include <QTimer>

static const int TIMEOUT = 45000;

class AddEditBlog::Private
{
public:
    Private()
    : wait(0)
    {}
    Ui::AddEditBlogBase ui;
    KTabWidget *mainW;
    bool isNewBlog;
    BilboBlog *bBlog;
    KBlog::Blog *mBlog;
    QTimer* mFetchProfileIdTimer;
    QTimer* mFetchBlogIdTimer;
    QTimer* mFetchAPITimer;
    bool isIdFetched;
    WaitWidget *wait;
    QString tmpBlogUrl;
};

AddEditBlog::AddEditBlog( int blog_id, QWidget *parent, Qt::WFlags flags )
        : KDialog( parent, flags ), d(new Private)
{
    kDebug();
    d->mainW = new KTabWidget( this );
    d->ui.setupUi( d->mainW );
    this->setMainWidget( d->mainW );
    this->setWindowTitle( i18n( "Add a new blog" ) );
    d->isNewBlog = true;
    d->mFetchAPITimer = d->mFetchBlogIdTimer = d->mFetchProfileIdTimer = 0;

    connect( d->ui.txtId, SIGNAL( textChanged( const QString& ) ), this, SLOT( enableOkButton( const QString& ) ) );
    connect( d->ui.txtUrl, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableAutoConfBtn() ) );
    connect( d->ui.txtUser, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableAutoConfBtn() ) );
    connect( d->ui.txtPass, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableAutoConfBtn() ) );
    connect( d->ui.btnAutoConf, SIGNAL( clicked() ), this, SLOT( autoConfigure() ) );
    connect( d->ui.btnFetch, SIGNAL( clicked() ), this, SLOT( fetchBlogId() ) );
    connect( d->ui.comboApi, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotComboApiChanged(int) ) );
    connect( d->ui.txtUrl, SIGNAL( returnPressed() ), this, SLOT( slotReturnPressed() ) );
    connect( d->ui.txtUser, SIGNAL( returnPressed() ), this, SLOT( slotReturnPressed() ) );
    connect( d->ui.txtPass, SIGNAL( returnPressed() ), this, SLOT( slotReturnPressed() ) );
    connect( d->ui.txtId, SIGNAL( returnPressed() ), this, SLOT( slotReturnPressed() ) );

    if ( blog_id > -1 ) {
        this->setWindowTitle( i18n( "Edit blog settings" ) );
        this->enableButtonOk( true );
        d->ui.btnFetch->setEnabled( true );
        d->ui.btnAutoConf->setEnabled( true );
        d->isNewBlog = false;
        d->bBlog =  DBMan::self()->blog( blog_id );
        d->ui.txtUrl->setText( d->bBlog->url().url() );
        d->ui.txtUser->setText( d->bBlog->username() );
        d->ui.txtPass->setText( d->bBlog->password() );
        d->ui.txtId->setText( d->bBlog->blogid() );
        d->ui.txtTitle->setText( d->bBlog->title() );
        d->ui.comboApi->setCurrentIndex( d->bBlog->api() );
        d->ui.comboDir->setCurrentIndex( d->bBlog->direction() );
        d->ui.txtTitle->setEnabled(true);
    } else {
        d->bBlog = new BilboBlog( this );
        d->bBlog->setBlogId( 0 );
        this->enableButtonOk( false );
        d->ui.txtTitle->setEnabled(false);
    }

    slotComboApiChanged( d->ui.comboApi->currentIndex() );
    d->ui.txtUrl->setFocus();
}

void AddEditBlog::enableAutoConfBtn()
{
    if ( d->ui.txtUrl->text().isEmpty() || d->ui.txtUser->text().isEmpty() || d->ui.txtPass->text().isEmpty() ) {
        d->ui.btnAutoConf->setEnabled( false );
        d->ui.btnFetch->setEnabled( false );
    } else {
        d->ui.btnAutoConf->setEnabled( true );
        d->ui.btnFetch->setEnabled( true );
    }
}

void AddEditBlog::autoConfigure()
{
    kDebug();
    if ( d->ui.txtUrl->text().isEmpty() || d->ui.txtUser->text().isEmpty() || d->ui.txtPass->text().isEmpty() ) {
        kDebug() << "Username, Password or Url doesn't set!";
        KMessageBox::sorry( this, i18n( "You have to set the username, password and URL of your blog or website." ),
                            i18n( "Incomplete fields" ) );
        return;
    }
    showWaitWidget( i18n("Trying to guess blog and API type...") );
    QString textUrl;
    ///Guess API with Url:
    if ( d->ui.txtUrl->text().contains( "xmlrpc.php", Qt::CaseInsensitive ) ) {
        d->ui.comboApi->setCurrentIndex( 3 );
        fetchBlogId();
        return;
    }
    if ( d->ui.txtUrl->text().contains( "blogspot", Qt::CaseInsensitive ) ) {
        d->ui.comboApi->setCurrentIndex( 4 );
        fetchBlogId();
        return;
    }
    if ( d->ui.txtUrl->text().contains( "wordpress", Qt::CaseInsensitive ) ) {
        d->ui.comboApi->setCurrentIndex( 3 );
    textUrl = d->ui.txtUrl->text();
    while (textUrl.endsWith(QChar('/'))) {
        textUrl.remove(textUrl.length()-1, 1);
    }
        d->ui.txtUrl->setText( textUrl + "/xmlrpc.php" );
        fetchBlogId();
        return;
    }
    if ( d->ui.txtUrl->text().contains( "livejournal", Qt::CaseInsensitive ) ) {
        d->ui.comboApi->setCurrentIndex( 0 );
        d->tmpBlogUrl = d->ui.txtUrl->text();
        d->ui.txtUrl->setText( "http://www.livejournal.com/interface/blogger/" );
        d->ui.txtId->setText( d->ui.txtUser->text() );
        d->ui.txtTitle->setText( d->ui.txtUser->text() );
        hideWaitWidget();
        return;
    }
    kDebug() << "Trying to guess API type by Homepage contents";
    KIO::StoredTransferJob *httpGetJob = KIO::storedGet( d->ui.txtUrl->text() , KIO::NoReload, KIO::HideProgressInfo );
    connect( httpGetJob, SIGNAL( result( KJob* ) ), this, SLOT( gotHtml( KJob* ) ) );
    d->mFetchAPITimer = new QTimer( this );
    d->mFetchAPITimer->setSingleShot( true );
    connect( d->mFetchAPITimer, SIGNAL( timeout() ), this, SLOT( handleFetchAPITimeout() ) );
    d->mFetchAPITimer->start( TIMEOUT );
}

void AddEditBlog::gotHtml( KJob *job )
{
    kDebug();
    if ( !job ) return;
    if ( job->error() ) {
        kDebug() << "Auto configuration failed! Error: " << job->errorString();
        hideWaitWidget();
        KMessageBox::sorry(this, i18n("Auto configuration failed. You have to set Blog API on Advanced tab manually."));
        return;
    }
    QString httpData( dynamic_cast<KIO::StoredTransferJob*>( job )->data() );
    job->deleteLater();

    QString textUrl;
    QRegExp rxGData( QString( "content='blogger' name='generator'" ) );
    if ( rxGData.indexIn( httpData ) != -1 ) {
        kDebug() << "content='blogger' name='generator' matched";
        d->mFetchAPITimer->deleteLater();
        d->ui.comboApi->setCurrentIndex( 4 );
        QRegExp rxBlogId( QString( "BlogID=(\\d+)" ) );
        d->ui.txtId->setText( rxBlogId.cap( 1 ) );
        hideWaitWidget();
        return;
    }

    QRegExp rxLiveJournal( QString( "rel=\"openid.server\" href=\"http://www.livejournal.com/openid/server.bml\"" ) );
    if ( rxLiveJournal.indexIn( httpData ) != -1 ) {
        kDebug() << " rel=\"openid.server\" href=\"http://www.livejournal.com/openid/server.bml\" matched";
        d->mFetchAPITimer->deleteLater();
        d->ui.comboApi->setCurrentIndex( 0 );
        d->ui.txtUrl->setText( "http://www.liverjournal.com/interface/blogger/" );
        d->ui.txtId->setText( d->ui.txtUser->text() );
        hideWaitWidget();
        return;
    }

    QRegExp rxWordpress( QString( "name=\"generator\" content=\"WordPress" ) );
    if ( rxWordpress.indexIn( httpData ) != -1 ) {
        kDebug() << "name=\"generator\" content=\"WordPress matched";
        d->mFetchAPITimer->deleteLater();
        d->ui.comboApi->setCurrentIndex( 3 );

    textUrl = d->ui.txtUrl->text();
    while (textUrl.endsWith(QChar('/'))) {
        textUrl.remove(textUrl.length()-1, 1);
    }
        d->ui.txtUrl->setText( textUrl + "/xmlrpc.php" );
        fetchBlogId();
        return;
    }

    // add MT for WordpressBuggy -> URL/xmlrpc.php exists
    textUrl = d->ui.txtUrl->text();
    while (textUrl.endsWith(QChar('/'))) {
    textUrl.remove(textUrl.length()-1, 1);
    }
    KIO::StoredTransferJob *testXmlRpcJob = KIO::storedGet( QString(textUrl + QLatin1String("/xmlrpc.php")),
                                                            KIO::NoReload, KIO::HideProgressInfo );

    connect( testXmlRpcJob, SIGNAL( result( KJob* ) ), this, SLOT( gotXmlRpcTest( KJob* ) ) );
}

void AddEditBlog::gotXmlRpcTest( KJob *job )
{
    kDebug();
    d->mFetchAPITimer->deleteLater();
    if ( !job ) return;
    if ( job->error() ) {
        kDebug() << "Auto configuration failed! Error: " << job->errorString();
        hideWaitWidget();
        KMessageBox::sorry(this, i18n("Auto configuration failed. You have to set Blog API on Advanced tab manually."));
        return;
    }
    KMessageBox::information(this, i18n("The program could not guess the API of your blog, \
but has found an XMLRPC interface and is trying to use it.\
\nThe MovableType API is assumed for now; choose another API if you know the server supports it."));
    d->ui.comboApi->setCurrentIndex( 2 );
    QString textUrl = d->ui.txtUrl->text();
    while (textUrl.endsWith(QChar('/'))) {
        textUrl.remove(textUrl.length()-1, 1);
    }
    d->ui.txtUrl->setText( textUrl + "/xmlrpc.php" );
    fetchBlogId();
}

void AddEditBlog::fetchBlogId()
{
    kDebug() << d->ui.comboApi->currentIndex();

    switch ( d->ui.comboApi->currentIndex() ) {
        case 0:
        case 1:
        case 2:
        case 3:
            d->mBlog = new KBlog::Blogger1( KUrl( d->ui.txtUrl->text() ), this );
            dynamic_cast<KBlog::Blogger1*>( d->mBlog )->setUsername( d->ui.txtUser->text() );
            dynamic_cast<KBlog::Blogger1*>( d->mBlog )->setPassword( d->ui.txtPass->text() );
            connect( dynamic_cast<KBlog::Blogger1*>( d->mBlog ) , SIGNAL( listedBlogs( const QList<QMap<QString, QString> >& ) ),
                     this, SLOT( fetchedBlogId( const QList<QMap<QString, QString> >& ) ) );
            d->mFetchBlogIdTimer = new QTimer( this );
            d->mFetchBlogIdTimer->setSingleShot( true );
            connect( d->mFetchBlogIdTimer, SIGNAL( timeout() ), this, SLOT( handleFetchIDTimeout() ) );
            d->mFetchBlogIdTimer->start( TIMEOUT );
            dynamic_cast<KBlog::Blogger1*>( d->mBlog )->listBlogs();
            break;

        case 4:
            d->mBlog = new KBlog::GData( d->ui.txtUrl->text() , this );
            dynamic_cast<KBlog::GData*>( d->mBlog )->setUsername( d->ui.txtUser->text() );
            dynamic_cast<KBlog::GData*>( d->mBlog )->setPassword( d->ui.txtPass->text() );
            connect( dynamic_cast<KBlog::GData*>( d->mBlog ), SIGNAL( fetchedProfileId( const QString& ) ),
                     this, SLOT( fetchedProfileId( const QString& ) ) );
            dynamic_cast<KBlog::GData*>( d->mBlog )->fetchProfileId();
            d->mFetchProfileIdTimer = new QTimer( this );
            d->mFetchProfileIdTimer->setSingleShot( true );
            connect( d->mFetchProfileIdTimer, SIGNAL( timeout() ), this, SLOT( handleFetchIDTimeout() ) );
            d->mFetchProfileIdTimer->start( TIMEOUT );
            break;
        default:
            kDebug()<<"Unknown API";
            return;
            break;
    };
    connect( d->mBlog, SIGNAL( error( KBlog::Blog::ErrorType, const QString& ) ),
             this, SLOT( handleFetchError( KBlog::Blog::ErrorType, const QString& ) ) );
    d->ui.txtId->setText( i18n( "Please wait..." ) );
    d->ui.txtId->setEnabled( false );
    showWaitWidget( i18n( "Fetching Blog Id..." ) );
}

void AddEditBlog::handleFetchIDTimeout()
{
    kDebug();
    if ( d->mFetchBlogIdTimer ) {
        d->mFetchBlogIdTimer->stop();
    }
    if( d->mFetchProfileIdTimer ) {
        d->mFetchProfileIdTimer->stop();
    }
    d->ui.txtId->setText( QString() );
    d->ui.txtId->setEnabled( true );
    hideWaitWidget();
    KMessageBox::error( this, i18n( "Fetching the blog id timed out. Check your Internet connection,\
and your homepage URL, username or password.\nNote that the URL has to contain \"http://\"\
\nIf you are using a self-hosted Wordpress blog, you have to enable Remote Publishing in its configuration." ) );
}

void AddEditBlog::handleFetchAPITimeout()
{
    kDebug();
    d->mFetchAPITimer->deleteLater();
    d->mFetchAPITimer = 0;
    hideWaitWidget();
    d->ui.txtId->setEnabled( true );
    d->ui.txtId->setText( QString() );
    KMessageBox::sorry( this, i18n( "The API guess function has failed, \
please check your Internet connection. Otherwise, you have to set the API type manually on the Advanced tab." ),
                        i18n( "Auto Configuration Failed" ) );
}

void AddEditBlog::handleFetchError( KBlog::Blog::ErrorType type, const QString & errorMsg )
{
    kDebug() << " ErrorType: " << type;
    d->ui.txtId->setEnabled( true );
    d->ui.txtId->setText( QString() );
    hideWaitWidget();
    KMessageBox::detailedError( this, i18n( "Fetching BlogID Failed.\nPlease check your Internet connection." ), errorMsg );
}

void AddEditBlog::fetchedBlogId( const QList< QMap < QString , QString > > & list )
{
    kDebug();
    if( d->mFetchBlogIdTimer ) {
        d->mFetchBlogIdTimer->deleteLater();
        d->mFetchBlogIdTimer = 0;
    }
    hideWaitWidget();
    QString blogId, blogName, blogUrl, apiUrl;
    if ( list.count() > 1 ) {
        kDebug() << "User has more than ONE blog!";
        KDialog *blogsDialog = new KDialog(this);
        QTableWidget *blogsList = new QTableWidget(blogsDialog);
        blogsList->setSelectionBehavior(QAbstractItemView::SelectRows);
        QList< QMap<QString,QString> >::const_iterator it = list.constBegin();
        QList< QMap<QString,QString> >::const_iterator endIt = list.constEnd();
        int i=0;
        blogsList->setColumnCount(4);
        QStringList headers;
        headers<<"Title"<<"Url";
        blogsList->setHorizontalHeaderLabels(headers);
        blogsList->setColumnHidden(2, true);
        blogsList->setColumnHidden(3, true);
        for(;it != endIt; ++it){
            kDebug()<<it->value("title");
            blogsList->insertRow(i);
            blogsList->setCellWidget(i, 0, new QLabel( it->value("title")) );
            blogsList->setCellWidget(i, 1, new QLabel( it->value("url")) );
            blogsList->setCellWidget(i, 2, new QLabel( it->value("id")) );
            blogsList->setCellWidget(i, 3, new QLabel( it->value("apiUrl")) );
            ++i;
        }
        blogsDialog->setMainWidget(blogsList);
        blogsDialog->setWindowTitle( i18n("Which blog?") );
        if( blogsDialog->exec() ) {
            int row = blogsList->currentRow();
            if( row == -1 )
                return;
            blogId = qobject_cast<QLabel*>( blogsList->cellWidget(row, 2) )->text();
            blogName = qobject_cast<QLabel*>( blogsList->cellWidget(row, 0) )->text();
            blogUrl = qobject_cast<QLabel*>( blogsList->cellWidget(row, 1) )->text();
            apiUrl = qobject_cast<QLabel*>( blogsList->cellWidget(row, 3) )->text();
        } else
            return;
    } else if (list.count() > 0) {
        blogId = list.constBegin()->value("id");
        blogName = list.constBegin()->value("title");
        blogUrl = list.constBegin()->value("url");
        apiUrl = list.constBegin()->value("apiUrl");
    } else {
        KMessageBox::sorry(this, i18n("Sorry, No blog found with the specified account info."));
        return;
    }
    d->ui.txtId->setText( blogId );
    d->ui.txtTitle->setText( blogName );
    d->ui.txtId->setEnabled( true );
    d->ui.btnFetch->setEnabled( true );
    d->ui.btnAutoConf->setEnabled( true );

    if( !apiUrl.isEmpty() ){
        d->ui.txtUrl->setText( apiUrl );
    } else {
        apiUrl = d->ui.txtUrl->text();
    }
    if( !blogUrl.isEmpty() ) {
        d->bBlog->setBlogUrl( blogUrl );
    } else {
        if(d->tmpBlogUrl.isEmpty())
            d->bBlog->setBlogUrl( apiUrl );
        else
            d->bBlog->setBlogUrl( d->tmpBlogUrl );
    }

    d->bBlog->setUrl( QUrl( apiUrl ) );
    d->bBlog->setUsername( d->ui.txtUser->text() );
    d->bBlog->setPassword( d->ui.txtPass->text() );
    d->bBlog->setBlogId( blogId );
    d->bBlog->setTitle( blogName );
}

void AddEditBlog::fetchedProfileId( const QString &id )
{
    kDebug();
    Q_UNUSED(id);
    d->mFetchProfileIdTimer->deleteLater();
    d->mFetchProfileIdTimer = 0;
    connect( dynamic_cast<KBlog::GData*>( d->mBlog ), SIGNAL( listedBlogs( const QList<QMap<QString, QString> >& ) ),
             this, SLOT( fetchedBlogId( const QList<QMap<QString, QString> >& ) ) );
    connect( dynamic_cast<KBlog::GData*>( d->mBlog ), SIGNAL( error( KBlog::Blog::ErrorType, const QString& ) ),
             this, SLOT( handleFetchError( KBlog::Blog::ErrorType, const QString& ) ) );
    d->mFetchBlogIdTimer = new QTimer( this );
    d->mFetchBlogIdTimer->setSingleShot( true );
    connect( d->mFetchBlogIdTimer, SIGNAL( timeout() ), this, SLOT( handleFetchIDTimeout() ) );
    d->mFetchBlogIdTimer->start( TIMEOUT );
    dynamic_cast<KBlog::GData*>( d->mBlog )->listBlogs();
}

void AddEditBlog::enableOkButton( const QString & txt )
{
    bool check = !txt.isEmpty();
    this->enableButtonOk( check );
    d->ui.txtTitle->setEnabled( check );
}

void AddEditBlog::slotReturnPressed()
{
    ///FIXME This function commented temporarilly! check its functionality! and uncomment it!
    if(this->isButtonEnabled(KDialog::Ok)){
        this->setButtonFocus(KDialog::Ok);
    } else {
        if(d->mainW->currentIndex()==0){
            if(d->ui.btnAutoConf->isEnabled()){
                autoConfigure();
            }
        } else {
            fetchBlogId();
        }
    }
}

AddEditBlog::~AddEditBlog()
{
    kDebug();
    delete d;
}

void AddEditBlog::setSupportedFeatures( BilboBlog::ApiType api )
{
    QString yesStyle = "QLabel{color: green;}";
    QString yesText = i18nc( "Supported feature or Not", "Yes" );
    QString noStyle = "QLabel{color: red;}";
    QString noText = i18nc( "Supported feature or Not", "No, API does not support it" );
    QString notYetText = i18nc( "Supported feature or Not", "No, Blogilo does not yet support it" );

    d->ui.featureCreatePost->setText( yesText );
    d->ui.featureCreatePost->setStyleSheet( yesStyle );
    d->ui.featureRemovePost->setText( yesText );
    d->ui.featureRemovePost->setStyleSheet( yesStyle );
    d->ui.featurRecentPosts->setText( yesText );
    d->ui.featurRecentPosts->setStyleSheet( yesStyle );

    d->ui.featureCreateCategory->setStyleSheet( noStyle );

    switch( api ) {
        case BilboBlog::BLOGGER1_API:
            d->ui.featureUploadMedia->setText( noText );
            d->ui.featureUploadMedia->setStyleSheet( noStyle );
            d->ui.featureCategories->setText( noText );
            d->ui.featureCategories->setStyleSheet( noStyle );
            d->ui.featureMultipagedPosts->setText( noText );
            d->ui.featureMultipagedPosts->setStyleSheet( noStyle );
            d->ui.featureCreateCategory->setText( noText );
            d->ui.featureTags->setText( noText );
            d->ui.featureTags->setStyleSheet( noStyle );
            break;
        case BilboBlog::METAWEBLOG_API:
            d->ui.featureUploadMedia->setText( yesText );
            d->ui.featureUploadMedia->setStyleSheet( yesStyle );
            d->ui.featureCategories->setText( noText );
            d->ui.featureCategories->setStyleSheet( noStyle );
            d->ui.featureMultipagedPosts->setText( noText );
            d->ui.featureMultipagedPosts->setStyleSheet( noStyle );
            d->ui.featureCreateCategory->setText( noText );
            d->ui.featureTags->setText( noText );
            d->ui.featureTags->setStyleSheet( noStyle );
            break;
        case BilboBlog::MOVABLETYPE_API:
            d->ui.featureUploadMedia->setText( yesText );
            d->ui.featureUploadMedia->setStyleSheet( yesStyle );
            d->ui.featureCategories->setText( yesText );
            d->ui.featureCategories->setStyleSheet( yesStyle );
            d->ui.featureMultipagedPosts->setText( yesText );
            d->ui.featureMultipagedPosts->setStyleSheet( yesStyle );
            d->ui.featureCreateCategory->setText( noText );
            d->ui.featureTags->setText( yesText );
            d->ui.featureTags->setStyleSheet( yesStyle );
            break;
        case BilboBlog::WORDPRESSBUGGY_API:
            d->ui.featureUploadMedia->setText( yesText );
            d->ui.featureUploadMedia->setStyleSheet( yesStyle );
            d->ui.featureCategories->setText( yesText );
            d->ui.featureCategories->setStyleSheet( yesStyle );
            d->ui.featureMultipagedPosts->setText( yesText );
            d->ui.featureMultipagedPosts->setStyleSheet( yesStyle );
            d->ui.featureCreateCategory->setText( notYetText );
            d->ui.featureTags->setText( yesText );
            d->ui.featureTags->setStyleSheet( yesStyle );
            break;
        case BilboBlog::GDATA_API:
            d->ui.featureUploadMedia->setText( noText );
            d->ui.featureUploadMedia->setStyleSheet( noStyle );
            d->ui.featureCategories->setText( noText );
            d->ui.featureCategories->setStyleSheet( noStyle );
            d->ui.featureMultipagedPosts->setText( noText );
            d->ui.featureMultipagedPosts->setStyleSheet( noStyle );
            d->ui.featureCreateCategory->setText( noText );
            d->ui.featureTags->setText( yesText );
            d->ui.featureTags->setStyleSheet( yesStyle );
            break;
    };
}

void AddEditBlog::slotComboApiChanged( int index )
{
    ///This wrapper is to change api if needed!
    setSupportedFeatures( (BilboBlog::ApiType) index );
}

void AddEditBlog::slotButtonClicked( int button )
{
    kDebug();
    if ( button == KDialog::Ok ) {
        if ( d->bBlog->blogid().isEmpty() && d->ui.txtId->text().isEmpty() ) {
            KMessageBox::sorry( this, i18n( "Blog ID has not yet been retrieved.\
\nYou can fetch the blog ID by clicking on \"Auto Configure\" or the \"Fetch ID\" button; otherwise, you have\
 to insert your blog ID manually." )
                                            );
            return;
        }
        d->bBlog->setApi(( BilboBlog::ApiType )d->ui.comboApi->currentIndex() );
        d->bBlog->setDirection(( Qt::LayoutDirection )d->ui.comboDir->currentIndex() );
        d->bBlog->setTitle( d->ui.txtTitle->text() );
        d->bBlog->setPassword( d->ui.txtPass->text() );
        d->bBlog->setUsername( d->ui.txtUser->text() );
        d->bBlog->setBlogId( d->ui.txtId->text() );
        d->bBlog->setUrl( KUrl( d->ui.txtUrl->text() ) );
        if(d->bBlog->blogUrl().isEmpty())
            d->bBlog->setBlogUrl(d->ui.txtUrl->text());

        if ( d->isNewBlog ) {
            int blog_id = DBMan::self()->addBlog( *d->bBlog );
            d->bBlog->setId( blog_id );
            if ( blog_id != -1 ) {
                kDebug() << "Emitting sigBlogAdded() ...";
                Q_EMIT sigBlogAdded( *d->bBlog );
            } else {
                kDebug() << "Cannot add blog";
            }
        } else {
            if ( DBMan::self()->editBlog( *d->bBlog ) ) {
                kDebug() << "Emitting sigBlogEdited() ...";
                Q_EMIT sigBlogEdited( *d->bBlog );
            } else {
                kDebug() << "Cannot edit blog with id " << d->bBlog->id();
            }
        }
        accept();
    } else
        KDialog::slotButtonClicked( button );
}

void AddEditBlog::showWaitWidget( QString text )
{
    d->ui.btnAutoConf->setEnabled( false );
    d->ui.btnFetch->setEnabled( false );
    if( !d->wait ) {
        d->wait = new WaitWidget(this);
        d->wait->setWindowModality( Qt::WindowModal );
        d->wait->setBusyState();
    }
    d->wait->setText( text );
    d->wait->show();
}

void AddEditBlog::hideWaitWidget()
{
    d->ui.btnAutoConf->setEnabled( true );
    d->ui.btnFetch->setEnabled( true );
    if( d->wait )
        d->wait->deleteLater();
    d->wait = 0;
}

#include "addeditblog.moc"
