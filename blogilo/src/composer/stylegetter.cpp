/*
    This file is part of Blogilo, A KDE Blogging Client

    It is a modified version of "weblogstylegetter.h" from
    KBlogger project. it has been modified for use in Blogilo, at
    February 2009.

    Copyright (C) 2007-2008-2008 by Christian Weilbach <christian_weilbach@web.de>
    Copyright (C) 2007-2008 Antonio Aloisio <gnuton@gnuton.org>
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

#include "stylegetter.h"
#include "bilbopost.h"
#include "bilboblog.h"
#include "backend.h"
#include "dbman.h"

#include <kio/job.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocalizedstring.h>
#include <qdebug.h>
#include <kdatetime.h>

#include <QFile>


static const char POST_TITLE[] = "Temporary-Post-Used-For-Style-Detection-Title-";
static const char  POST_CONTENT[] = "Temporary-Post-Used-For-Style-Detection-Content-";

StyleGetter::StyleGetter( const int blogid, QObject *parent )
    : QObject( parent )
{
    qDebug();
    BilboBlog *tempBlog = DBMan::self()->blog( blogid );
    if ( tempBlog->isError() ) {
        KMessageBox::detailedError( mParent, i18n( "Cannot fetch the selected blog style."),
                            DBMan::self()->lastErrorText()  );
        return;
    }
    // sets cachePath to ~/.kde4/share/apps/bilbo/blog_host_name/
//     QString blogDir = DBMan::self()->getBlogInfo( blogid ).url().host();
//     QString blogDir = tempBlog.url().host();
//     qDebug() << blogDir;
//     mCachePath = KStandardDirs::locateLocal( "data", "bilbo/" + blogDir + '/' , true );
    QString url = QString::fromLatin1("blogilo/%1/" ).arg( blogid );
    mCachePath = KStandardDirs::locateLocal( "data", url , true );
    generateRandomPostStrings();
    mParent = qobject_cast< QWidget* >( parent );
    Q_ASSERT( mParent );

    // create temp post

    mTempPost = new BilboPost();
    mTempPost->setTitle( mPostTitle );
    mTempPost->setContent( mPostContent );
    mTempPost->setPrivate( false );

    if ( ( tempBlog->api() == BilboBlog::MOVABLETYPE_API ) ||
         ( tempBlog->api() == BilboBlog::WORDPRESSBUGGY_API ) ) {
        mTempPost->setCreationDateTime( KDateTime( QDate(2000, 1, 1), QTime(0, 0), KDateTime::UTC ) );
    }

    b = new Backend( blogid );
    connect( b, SIGNAL(sigPostPublished(int,BilboPost*)), this,
             SLOT(slotTempPostPublished(int,BilboPost*)) );
    connect( b, SIGNAL(sigError(QString)), this, SLOT(slotError(QString)) );

    Q_EMIT sigGetStyleProgress( 10 );

    b->publishPost( mTempPost );
}

StyleGetter::~StyleGetter()
{
    qDebug();
}

QString StyleGetter::styledHtml( const int blogid,
                                      const QString &title,
                                      const QString &content )
{
    qDebug();
//     BilboBlog tempBlog = DBMan::self()->getBlogInfo( blogid );
//     if ( tempBlog.isError() ) {
//         qDebug() << DBMan::self()->lastErrorText();
//         return "<html><body><b>" + title + "</b><br>" + content + "</html>";
//     }
//
//     QString blogDir = tempBlog.url().host();
    //QString url = QString( "bilbo/%1/" ).arg( blogid );
    QString url = QString::fromLatin1("blogilo/%1/" ).arg( blogid );
    url = KStandardDirs::locateLocal( "data", url , true );
    KUrl dest( url );
    dest.addPath(QLatin1String("style.html"));
    dest.setScheme(QLatin1String("file"));

    if ( !dest.isValid() ) {
        return QLatin1String("<html><body><h2 align='center'>") + title + QLatin1String("</h2><br>") + content + QLatin1String("</html>");
    }
    QFile file( dest.pathOrUrl() );
    if ( !file.open( QIODevice::ReadOnly | QIODevice::Text ) ) {
        return QLatin1String("<html><body><h2 align='center'>") + title + QLatin1String("</h2><br>") + content + QLatin1String("</html>");
    }

    QString buffer;
    while ( !file.atEnd() ) {
        QByteArray line = file.readLine();
        buffer.append( QString::fromUtf8( line ) );
    }

    QRegExp typeRx ( QLatin1String( "(TYPE[^>]+>)" ) );
    buffer.remove( typeRx );

    QRegExp titleRx( QString::fromLatin1( "%1[\\d]*" ).arg( QLatin1String(POST_TITLE) ) );
    QRegExp contentRx( QString::fromLatin1( "%1[\\d]*" ).arg( QLatin1String(POST_CONTENT )) );

    buffer.replace( titleRx, title );
    buffer.replace( contentRx, content );

    return buffer;
}

void StyleGetter::slotTempPostPublished( int blogId, BilboPost* post )
{
    qDebug();

    KUrl postUrl;
//     postUrl = post->permaLink();
    postUrl = post->link();
    if ( postUrl.isEmpty() ) {
        qDebug() << "link was empty";
//         postUrl = post->link();
        postUrl = post->permaLink();
        if ( postUrl.isEmpty() ) {
            qDebug() << "permaLink was empty";
            postUrl = KUrl( DBMan::self()->blog(blogId)->blogUrl() );
        }
    }

    Q_EMIT sigGetStyleProgress( 30 );

    mTempPost = post;
    KIO::StoredTransferJob *job = KIO::storedGet( postUrl, KIO::NoReload, KIO::HideProgressInfo );
    connect( job, SIGNAL(result(KJob*)),
            this, SLOT(slotHtmlCopied(KJob*)) );

}

void StyleGetter::slotHtmlCopied( KJob *job )
{
    qDebug();
    if ( job->error() ) {
        KMessageBox::detailedError( mParent, i18n( "Cannot get html file."),
                            job->errorString() );
        sender()->deleteLater();
        return;
    }

    Q_EMIT sigGetStyleProgress( 50 );

    QByteArray httpData( qobject_cast<KIO::StoredTransferJob*>( job )->data() );

    QString href( mTempPost->permaLink().url() );
    int filenameOffset = href.lastIndexOf( QLatin1String("/") );
    href = href.remove( filenameOffset + 1, 255 );
    QString base( QLatin1String("<base href=\"")+href+QLatin1String("\"/>") );

    QRegExp rxBase( QLatin1String("(<base\\shref=[^>]+>)") );
    if ( rxBase.indexIn( QLatin1String(httpData) ) != -1 ) {
         httpData.replace( rxBase.cap( 1 ).toLatin1(), base.toLatin1() );
    }
    else {
        int headOffset = httpData.indexOf( "<head>" );
        httpData.insert( headOffset + 6, base.toLatin1() );
    }

    QFile file( mCachePath + QLatin1String("style.html") );
//     Q_ASSERT( dest.isValid() );
    if ( file.exists() ) {
        file.remove();
    }
    if (!file.open( QIODevice::WriteOnly ) ) {
        KMessageBox::error( mParent,
                            i18n( "Cannot write data to file %1", file.fileName() ) );
        return;
    }
    if ( file.write( httpData ) == -1 ) {
        KMessageBox::error( mParent,
                            i18n( "Cannot write data to file %1", file.fileName() ) );

        file.close();
        return;
    }
    file.close();
    Q_EMIT sigGetStyleProgress( 70 );
//     Q_EMIT sigStyleFetched();


    //Remove temp post from the server.
    connect( b, SIGNAL(sigPostRemoved(int,BilboPost)), this,
             SLOT(slotTempPostRemoved(int,BilboPost)) );
    b->removePost( mTempPost );
}

void StyleGetter::slotTempPostRemoved( int blog_id, const BilboPost &post)
{
    Q_UNUSED( blog_id );
    Q_UNUSED( post );

    delete mTempPost;
    b->deleteLater();

    Q_EMIT sigGetStyleProgress( 100 );
    Q_EMIT sigStyleFetched();
}

void StyleGetter::generateRandomPostStrings()
{
    qDebug();
    srand( time( 0 ) );
    int postRandomNumber = rand();
    mPostTitle = QString::fromLatin1("%1%2" ).arg( QLatin1String(POST_TITLE) ).arg( postRandomNumber );
    mPostContent = QString::fromLatin1( "%1%2" ).arg( QLatin1String(POST_CONTENT) ).arg( postRandomNumber );
}

void StyleGetter::slotError( const QString & errMsg )
{
    qDebug();
    KMessageBox::detailedError( mParent, i18n( "An error occurred in the latest transaction." ), errMsg );
    b->deleteLater();
}

