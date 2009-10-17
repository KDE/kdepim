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

AddEditBlog::AddEditBlog( int blog_id, QWidget *parent, Qt::WFlags flags )
        : KDialog( parent, flags ), wait(0)
{
    kDebug();
    mainW = new KTabWidget( this );
    ui.setupUi( mainW );
    this->setMainWidget( mainW );
    this->setWindowTitle( i18n( "Add a new blog" ) );
    isNewBlog = true;
    mFetchAPITimer = mFetchBlogIdTimer = mFetchProfileIdTimer = 0;

    connect( ui.txtId, SIGNAL( textChanged( const QString& ) ), this, SLOT( enableOkButton( const QString& ) ) );
    connect( ui.txtUrl, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableAutoConfBtn() ) );
    connect( ui.txtUser, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableAutoConfBtn() ) );
    connect( ui.txtPass, SIGNAL( textChanged( const QString & ) ), this, SLOT( enableAutoConfBtn() ) );
    connect( ui.btnAutoConf, SIGNAL( clicked() ), this, SLOT( autoConfigure() ) );
    connect( ui.btnFetch, SIGNAL( clicked() ), this, SLOT( fetchBlogId() ) );
    connect( ui.comboApi, SIGNAL( currentIndexChanged(int) ), this, SLOT( slotComboApiChanged(int) ) );
    connect( ui.txtUrl, SIGNAL( returnPressed() ), this, SLOT( sltReturnPressed() ) );
    connect( ui.txtUser, SIGNAL( returnPressed() ), this, SLOT( sltReturnPressed() ) );
    connect( ui.txtPass, SIGNAL( returnPressed() ), this, SLOT( sltReturnPressed() ) );
    connect( ui.txtId, SIGNAL( returnPressed() ), this, SLOT( sltReturnPressed() ) );

    if ( blog_id > -1 ) {
        this->setWindowTitle( i18n( "Edit blog settings" ) );
        this->enableButtonOk( true );
        ui.btnFetch->setEnabled( true );
        ui.btnAutoConf->setEnabled( true );
        isNewBlog = false;
        bBlog = new BilboBlog( DBMan::self()->blog( blog_id ) );
        ui.txtUrl->setText( bBlog->url().url() );
        ui.txtUser->setText( bBlog->username() );
        ui.txtPass->setText( bBlog->password() );
        ui.txtId->setText( bBlog->blogid() );
        ui.txtTitle->setText( bBlog->title() );
        ui.comboApi->setCurrentIndex( bBlog->api() );
        ui.comboDir->setCurrentIndex( bBlog->direction() );
        ui.txtTitle->setEnabled(true);
    } else {
        bBlog = new BilboBlog( this );
        bBlog->setBlogId( 0 );
        this->enableButtonOk( false );
        ui.txtTitle->setEnabled(false);
    }

    slotComboApiChanged( ui.comboApi->currentIndex() );
    ui.txtUrl->setFocus();
}

void AddEditBlog::enableAutoConfBtn()
{
    if ( ui.txtUrl->text().isEmpty() || ui.txtUser->text().isEmpty() || ui.txtPass->text().isEmpty() ) {
        ui.btnAutoConf->setEnabled( false );
        ui.btnFetch->setEnabled( false );
    } else {
        ui.btnAutoConf->setEnabled( true );
        ui.btnFetch->setEnabled( true );
    }
}

void AddEditBlog::autoConfigure()
{
    kDebug();
    if ( ui.txtUrl->text().isEmpty() || ui.txtUser->text().isEmpty() || ui.txtPass->text().isEmpty() ) {
        kDebug() << "Username, Password or Url doesn't set!";
        KMessageBox::sorry( this, i18n( "You have to set the username, password and URL of your blog or website." ),
                            i18n( "Incomplete fields" ) );
        return;
    }
    showWaitWidget( i18n("Trying to guess blog and API type...") );
    ///Guess API with Url:
    if ( ui.txtUrl->text().contains( "xmlrpc.php", Qt::CaseInsensitive ) ) {
        ui.comboApi->setCurrentIndex( 3 );
        fetchBlogId();
        return;
    }
    if ( ui.txtUrl->text().contains( "blogspot", Qt::CaseInsensitive ) ) {
        ui.comboApi->setCurrentIndex( 4 );
        fetchBlogId();
        return;
    }
    if ( ui.txtUrl->text().contains( "wordpress", Qt::CaseInsensitive ) ) {
        ui.comboApi->setCurrentIndex( 3 );
        ui.txtUrl->setText( ui.txtUrl->text() + "/xmlrpc.php" );
        fetchBlogId();
        return;
    }
    if ( ui.txtUrl->text().contains( "livejournal", Qt::CaseInsensitive ) ) {
        ui.comboApi->setCurrentIndex( 0 );
        tmpBlogUrl = ui.txtUrl->text();
        ui.txtUrl->setText( "http://www.livejournal.com/interface/blogger/" );
        ui.txtId->setText( ui.txtUser->text() );
        ui.txtTitle->setText( ui.txtUser->text() );
        hideWaitWidget();
        return;
    }
    kDebug() << "Trying to guess API type by Homepage contents";
    KIO::StoredTransferJob *httpGetJob = KIO::storedGet( ui.txtUrl->text() , KIO::NoReload, KIO::HideProgressInfo );
    connect( httpGetJob, SIGNAL( result( KJob* ) ), this, SLOT( gotHtml( KJob* ) ) );
    mFetchAPITimer = new QTimer( this );
    mFetchAPITimer->setSingleShot( true );
    connect( mFetchAPITimer, SIGNAL( timeout() ), this, SLOT( handleFetchAPITimeout() ) );
    mFetchAPITimer->start( TIMEOUT );
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

    QRegExp rxGData( QString( "content='blogger' name='generator'" ) );
    if ( rxGData.indexIn( httpData ) != -1 ) {
        kDebug() << "content='blogger' name='generator' matched";
        mFetchAPITimer->deleteLater();
        ui.comboApi->setCurrentIndex( 4 );
        QRegExp rxBlogId( QString( "BlogID=(\\d+)" ) );
        ui.txtId->setText( rxBlogId.cap( 1 ) );
        hideWaitWidget();
        return;
    }

    QRegExp rxLiveJournal( QString( "rel=\"openid.server\" href=\"http://www.livejournal.com/openid/server.bml\"" ) );
    if ( rxLiveJournal.indexIn( httpData ) != -1 ) {
        kDebug() << " rel=\"openid.server\" href=\"http://www.livejournal.com/openid/server.bml\" matched";
        mFetchAPITimer->deleteLater();
        ui.comboApi->setCurrentIndex( 0 );
        ui.txtUrl->setText( "http://www.liverjournal.com/interface/blogger/" );
        ui.txtId->setText( ui.txtUser->text() );
        hideWaitWidget();
        return;
    }

    QRegExp rxWordpress( QString( "name=\"generator\" content=\"WordPress" ) );
    if ( rxWordpress.indexIn( httpData ) != -1 ) {
        kDebug() << "name=\"generator\" content=\"WordPress matched";
        mFetchAPITimer->deleteLater();
        ui.comboApi->setCurrentIndex( 3 );
        ui.txtUrl->setText( ui.txtUrl->text() + "/xmlrpc.php" );
        fetchBlogId();
        return;
    }

    // add MT for WordpressBuggy -> URL/xmlrpc.php exists
    KIO::StoredTransferJob *testXmlRpcJob = KIO::storedGet( ui.txtUrl->text() + "/xmlrpc.php",
                                                            KIO::NoReload, KIO::HideProgressInfo );

    connect( testXmlRpcJob, SIGNAL( result( KJob* ) ), this, SLOT( gotXmlRpcTest( KJob* ) ) );
}

void AddEditBlog::gotXmlRpcTest( KJob *job )
{
    kDebug();
    mFetchAPITimer->deleteLater();
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
    ui.comboApi->setCurrentIndex( 2 );
    ui.txtUrl->setText( ui.txtUrl->text() + "/xmlrpc.php" );
    fetchBlogId();
}

void AddEditBlog::fetchBlogId()
{
    kDebug() << ui.comboApi->currentIndex();

    switch ( ui.comboApi->currentIndex() ) {
        case 0:
        case 1:
        case 2:
        case 3:
            mBlog = new KBlog::Blogger1( KUrl( ui.txtUrl->text() ), this );
            dynamic_cast<KBlog::Blogger1*>( mBlog )->setUsername( ui.txtUser->text() );
            dynamic_cast<KBlog::Blogger1*>( mBlog )->setPassword( ui.txtPass->text() );
            connect( dynamic_cast<KBlog::Blogger1*>( mBlog ) , SIGNAL( listedBlogs( const QList<QMap<QString, QString> >& ) ),
                     this, SLOT( fetchedBlogId( const QList<QMap<QString, QString> >& ) ) );
            mFetchBlogIdTimer = new QTimer( this );
            mFetchBlogIdTimer->setSingleShot( true );
            connect( mFetchBlogIdTimer, SIGNAL( timeout() ), this, SLOT( handleFetchIDTimeout() ) );
            mFetchBlogIdTimer->start( TIMEOUT );
            dynamic_cast<KBlog::Blogger1*>( mBlog )->listBlogs();
            break;

        case 4:
            mBlog = new KBlog::GData( ui.txtUrl->text() , this );
            dynamic_cast<KBlog::GData*>( mBlog )->setUsername( ui.txtUser->text() );
            dynamic_cast<KBlog::GData*>( mBlog )->setPassword( ui.txtPass->text() );
            connect( dynamic_cast<KBlog::GData*>( mBlog ), SIGNAL( fetchedProfileId( const QString& ) ),
                     this, SLOT( fetchedProfileId( const QString& ) ) );
            dynamic_cast<KBlog::GData*>( mBlog )->fetchProfileId();
            mFetchProfileIdTimer = new QTimer( this );
            mFetchProfileIdTimer->setSingleShot( true );
            connect( mFetchProfileIdTimer, SIGNAL( timeout() ), this, SLOT( handleFetchIDTimeout() ) );
            mFetchProfileIdTimer->start( TIMEOUT );
            break;
        default:
            kDebug()<<"Unknown API";
            return;
            break;
    };
    connect( mBlog, SIGNAL( error( KBlog::Blog::ErrorType, const QString& ) ),
             this, SLOT( handleFetchError( KBlog::Blog::ErrorType, const QString& ) ) );
    ui.txtId->setText( i18n( "Please wait..." ) );
    ui.txtId->setEnabled( false );
    showWaitWidget( i18n( "Fetching Blog Id..." ) );
}

void AddEditBlog::handleFetchIDTimeout()
{
    kDebug();
    if ( mFetchBlogIdTimer ) {
        mFetchBlogIdTimer->deleteLater();
        mFetchBlogIdTimer = 0;
    }
    if( mFetchProfileIdTimer ) {
        mFetchProfileIdTimer->deleteLater();
        mFetchProfileIdTimer = 0;
    }
    ui.txtId->setText( QString() );
    ui.txtId->setEnabled( true );
    hideWaitWidget();
    KMessageBox::error( this, i18n( "Fetching the blog id timed out. Check your Internet connection,\
and your homepage URL, username or password.\nNote that the URL has to contain \"http://\"\
\nIf you are using a self-hosted Wordpress blog, you have to enable Remote Publishing in its configuration." ) );
}

void AddEditBlog::handleFetchAPITimeout()
{
    kDebug();
    mFetchAPITimer->deleteLater();
    mFetchAPITimer = 0;
    hideWaitWidget();
    ui.txtId->setEnabled( true );
    KMessageBox::sorry( this, i18n( "The API guess function has failed, \
please check your Internet connection. Otherwise, you have to set the API type manually on the Advanced tab." ),
                        i18n( "Auto Configuration Failed" ) );
}

void AddEditBlog::handleFetchError( KBlog::Blog::ErrorType type, const QString & errorMsg )
{
    kDebug() << " ErrorType: " << type;
    ui.txtId->setEnabled( true );
    hideWaitWidget();
    KMessageBox::detailedError( this, i18n( "Fetching BlogID Failed.\nPlease check your Internet connection." ), errorMsg );
}

void AddEditBlog::fetchedBlogId( const QList< QMap < QString , QString > > & list )
{
    kDebug();
    if( mFetchBlogIdTimer ) {
        mFetchBlogIdTimer->deleteLater();
        mFetchBlogIdTimer = 0;
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
    } else {
        blogId = list.begin()->value("id");
        blogName = list.begin()->value("title");
        blogUrl = list.begin()->value("url");
        apiUrl = list.begin()->value("apiUrl");
    }
    ui.txtId->setText( blogId );
    ui.txtTitle->setText( blogName );
    ui.txtId->setEnabled( true );
    ui.btnFetch->setEnabled( true );
    ui.btnAutoConf->setEnabled( true );

    if( !apiUrl.isEmpty() ){
        ui.txtUrl->setText( apiUrl );
    } else {
        apiUrl = ui.txtUrl->text();
    }
    if( !blogUrl.isEmpty() ) {
        bBlog->setBlogUrl( blogUrl );
    } else {
        if(tmpBlogUrl.isEmpty())
            bBlog->setBlogUrl( apiUrl );
        else
            bBlog->setBlogUrl( tmpBlogUrl );
    }

    bBlog->setUrl( QUrl( apiUrl ) );
    bBlog->setUsername( ui.txtUser->text() );
    bBlog->setPassword( ui.txtPass->text() );
    bBlog->setBlogId( blogId );
    bBlog->setTitle( blogName );
}

void AddEditBlog::fetchedProfileId( const QString &id )
{
    kDebug();
    Q_UNUSED(id);
    mFetchProfileIdTimer->deleteLater();
    mFetchProfileIdTimer = 0;
    connect( dynamic_cast<KBlog::GData*>( mBlog ), SIGNAL( listedBlogs( const QList<QMap<QString, QString> >& ) ),
             this, SLOT( fetchedBlogId( const QList<QMap<QString, QString> >& ) ) );
    connect( dynamic_cast<KBlog::GData*>( mBlog ), SIGNAL( error( KBlog::Blog::ErrorType, const QString& ) ),
             this, SLOT( handleFetchError( KBlog::Blog::ErrorType, const QString& ) ) );
    mFetchBlogIdTimer = new QTimer( this );
    mFetchBlogIdTimer->setSingleShot( true );
    connect( mFetchBlogIdTimer, SIGNAL( timeout() ), this, SLOT( handleFetchIDTimeout() ) );
    mFetchBlogIdTimer->start( TIMEOUT );
    dynamic_cast<KBlog::GData*>( mBlog )->listBlogs();
}

void AddEditBlog::enableOkButton( const QString & txt )
{
    bool check = !txt.isEmpty();
    this->enableButtonOk( check );
    ui.txtTitle->setEnabled( check );
}

void AddEditBlog::sltReturnPressed()
{
    ///FIXME This function commented temporarilly! check its functionality! and uncomment it!
    if(this->isButtonEnabled(KDialog::Ok)){
        this->setButtonFocus(KDialog::Ok);
    } else {
        if(mainW->currentIndex()==0){
            if(ui.btnAutoConf->isEnabled()){
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
}

void AddEditBlog::setSupportedFeatures( BilboBlog::ApiType api )
{
    QString yesStyle = "QLabel{color: green;}";
    QString yesText = i18nc( "Supported feature or Not", "Yes" );
    QString noStyle = "QLabel{color: red;}";
    QString noText = i18nc( "Supported feature or Not", "No, API does not support it" );
    QString notYetText = i18nc( "Supported feature or Not", "No, Blogilo does not yet support it" );

    ui.featureCreatePost->setText( yesText );
    ui.featureCreatePost->setStyleSheet( yesStyle );
    ui.featureRemovePost->setText( yesText );
    ui.featureRemovePost->setStyleSheet( yesStyle );
    ui.featurRecentPosts->setText( yesText );
    ui.featurRecentPosts->setStyleSheet( yesStyle );

    ui.featureCreateCategory->setStyleSheet( noStyle );

    switch( api ) {
        case BilboBlog::BLOGGER1_API:
            ui.featureUploadMedia->setText( noText );
            ui.featureUploadMedia->setStyleSheet( noStyle );
            ui.featureCategories->setText( noText );
            ui.featureCategories->setStyleSheet( noStyle );
            ui.featureMultipagedPosts->setText( noText );
            ui.featureMultipagedPosts->setStyleSheet( noStyle );
            ui.featureCreateCategory->setText( noText );
            ui.featureTags->setText( noText );
            ui.featureTags->setStyleSheet( noStyle );
            break;
        case BilboBlog::METAWEBLOG_API:
            ui.featureUploadMedia->setText( yesText );
            ui.featureUploadMedia->setStyleSheet( yesStyle );
            ui.featureCategories->setText( noText );
            ui.featureCategories->setStyleSheet( noStyle );
            ui.featureMultipagedPosts->setText( noText );
            ui.featureMultipagedPosts->setStyleSheet( noStyle );
            ui.featureCreateCategory->setText( noText );
            ui.featureTags->setText( noText );
            ui.featureTags->setStyleSheet( noStyle );
            break;
        case BilboBlog::MOVABLETYPE_API:
            ui.featureUploadMedia->setText( yesText );
            ui.featureUploadMedia->setStyleSheet( yesStyle );
            ui.featureCategories->setText( yesText );
            ui.featureCategories->setStyleSheet( yesStyle );
            ui.featureMultipagedPosts->setText( yesText );
            ui.featureMultipagedPosts->setStyleSheet( yesStyle );
            ui.featureCreateCategory->setText( noText );
            ui.featureTags->setText( yesText );
            ui.featureTags->setStyleSheet( yesStyle );
            break;
        case BilboBlog::WORDPRESSBUGGY_API:
            ui.featureUploadMedia->setText( yesText );
            ui.featureUploadMedia->setStyleSheet( yesStyle );
            ui.featureCategories->setText( yesText );
            ui.featureCategories->setStyleSheet( yesStyle );
            ui.featureMultipagedPosts->setText( yesText );
            ui.featureMultipagedPosts->setStyleSheet( yesStyle );
            ui.featureCreateCategory->setText( notYetText );
            ui.featureTags->setText( yesText );
            ui.featureTags->setStyleSheet( yesStyle );
            break;
        case BilboBlog::GDATA_API:
            ui.featureUploadMedia->setText( noText );
            ui.featureUploadMedia->setStyleSheet( noStyle );
            ui.featureCategories->setText( noText );
            ui.featureCategories->setStyleSheet( noStyle );
            ui.featureMultipagedPosts->setText( noText );
            ui.featureMultipagedPosts->setStyleSheet( noStyle );
            ui.featureCreateCategory->setText( noText );
            ui.featureTags->setText( yesText );
            ui.featureTags->setStyleSheet( yesStyle );
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
        if ( bBlog->blogid().isEmpty() && ui.txtId->text().isEmpty() ) {
            KMessageBox::sorry( this, i18n( "Blog ID has not yet been retrieved.\
\nYou can fetch the blog ID by clicking on \"Auto Configure\" or the \"Fetch ID\" button; otherwise, you have\
 to insert your blog ID manually." )
                                            );
            return;
        }
        bBlog->setApi(( BilboBlog::ApiType )ui.comboApi->currentIndex() );
        bBlog->setDirection(( Qt::LayoutDirection )ui.comboDir->currentIndex() );
        bBlog->setTitle( ui.txtTitle->text() );
        bBlog->setPassword( ui.txtPass->text() );
        bBlog->setUsername( ui.txtUser->text() );
        bBlog->setBlogId( ui.txtId->text() );
        bBlog->setUrl( KUrl( ui.txtUrl->text() ) );
        if(bBlog->blogUrl().isEmpty())
            bBlog->setBlogUrl(ui.txtUrl->text());

        if ( isNewBlog ) {
            int blog_id = DBMan::self()->addBlog( *bBlog );
            bBlog->setId( blog_id );
            if ( blog_id != -1 ) {
                kDebug() << "Emitting sigBlogAdded() ...";
                Q_EMIT sigBlogAdded( *bBlog );
            } else {
                kDebug() << "Cannot add blog";
            }
        } else {
            if ( DBMan::self()->editBlog( *bBlog ) ) {
                kDebug() << "Emitting sigBlogEdited() ...";
                Q_EMIT sigBlogEdited( *bBlog );
            } else {
                kDebug() << "Cannot edit blog with id " << bBlog->id();
            }
        }
        accept();
    } else
        KDialog::slotButtonClicked( button );
}

void AddEditBlog::showWaitWidget( QString text )
{
    ui.btnAutoConf->setEnabled( false );
    ui.btnFetch->setEnabled( false );
    if( !wait ) {
        wait = new WaitWidget(this);
        wait->setWindowModality( Qt::WindowModal );
        wait->setBusyState();
    }
    wait->setText( text );
    wait->show();
}

void AddEditBlog::hideWaitWidget()
{
    ui.btnAutoConf->setEnabled( true );
    ui.btnFetch->setEnabled( true );
    if( wait )
        wait->deleteLater();
    wait = 0;
}

#include "addeditblog.moc"
