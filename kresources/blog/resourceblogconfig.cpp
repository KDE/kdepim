/*
  This file is part of the blog resource.

  Copyright (c) 2007 Mike Arthur <mike@mikearthur.co.uk>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "resourceblogconfig.h"

#include <QLabel>
#include <QGridLayout>

#include <KLocale>
#include <KDialog>
#include <KUrlRequester>
#include <KLineEdit>
#include <KComboBox>

#include <kcal/resourcecachedconfig.h>

using namespace KCal;

ResourceBlogConfig::ResourceBlogConfig
( QWidget *parent ) : KRES::ConfigWidget( parent )
{
  //FIXME: Resize to abritary size to fix KOrganizer bug.
  resize( 245, 115 );
  QGridLayout *mainLayout = new QGridLayout( this );
  mainLayout->setSpacing( KDialog::spacingHint() );

  QLabel *label = new QLabel( i18n( "XML-RPC URL:" ), this );
  mUrl = new KUrlRequester( this );
  mUrl->setMode( KFile::File );
  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mUrl, 1, 1 );

  label = new QLabel( i18n( "Username:" ), this );
  mUsername = new KLineEdit( this );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mUsername, 2, 1 );

  label = new QLabel( i18n( "Password:" ), this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( QLineEdit::Password );

  mainLayout->addWidget( label, 3, 0 );
  mainLayout->addWidget( mPassword, 3, 1 );

  label = new QLabel( i18n( "API:" ), this );
  mAPI = new KComboBox( false, this );
  //TODO: When these are more stable/featureful, add them.
  //mAPI->addItem( "Google Blogger Data" );
  //mAPI->addItem( "LiveJournal" );
  //mAPI->addItem( "Movable Type" );
  mAPI->addItem( "MetaWeblog" );
  mAPI->addItem( "Blogger 1.0" );

  mainLayout->addWidget( label, 4, 0 );
  mainLayout->addWidget( mAPI, 4, 1 );

  label = new QLabel( i18n( "Blog:" ), this );
  mBlogs = new KComboBox( false, this );
  mBlogs->setEnabled( false );

  mainLayout->addWidget( label, 5, 0 );
  mainLayout->addWidget( mBlogs, 5, 1 );

  label = new QLabel( i18n( "Posts to download:" ), this );
  mDownloadCount = new KLineEdit( this );
  mDownloadCount->setValidator( new QIntValidator( 1, 1000, mDownloadCount ) );

  mainLayout->addWidget( label, 6, 0 );
  mainLayout->addWidget( mDownloadCount, 6, 1 );

  // Add the subwidget for the cache reload settings.
  mReloadConfig = new ResourceCachedReloadConfig( this );
  mainLayout->addWidget( mReloadConfig, 7, 0, 1, 2 );

  // Add the subwidget for the cache save settings.
  mSaveConfig = new ResourceCachedSaveConfig( this );
  mainLayout->addWidget( mSaveConfig, 8, 0, 1, 2 );
}

void ResourceBlogConfig::loadSettings( KRES::Resource *res )
{
  ResourceBlog *resource = static_cast<ResourceBlog *>( res );
  if ( resource ) {
    mUrl->setUrl( resource->url().url() );
    mUsername->setText( resource->username() );
    mPassword->setText( resource->password() );
    mAPI->setCurrentItem( resource->API(), false );
    mDownloadCount->setText( QString::number( resource->downloadCount() ) );
    QPair<QString, QString> blog = resource->blog();
    if ( !blog.second.isEmpty() ) {
      mBlogs->addItem( blog.second, blog.first );
      mBlogs->setEnabled( true );
    }
    connect ( mAPI, SIGNAL( currentIndexChanged( int ) ),
        this, SLOT( slotBlogAPIChanged( int ) ) );
    mReloadConfig->loadSettings( resource );
    mSaveConfig->loadSettings( resource );
    kDebug( 5700 ) << "ResourceBlogConfig::loadSettings(): reloaded";
  } else {
    kError( 5700 ) <<"ResourceBlogConfig::loadSettings():"
                   << " no ResourceBlog, cast failed";
  }
}

void ResourceBlogConfig::saveSettings( KRES::Resource *res )
{
  ResourceBlog *resource = static_cast<ResourceBlog*>( res );
  if ( resource ) {
    resource->setUrl( mUrl->url().url() );
    resource->setUsername( mUsername->text() );
    resource->setPassword( mPassword->text() );
    resource->setAPI( mAPI->currentText() );
    resource->setDownloadCount( mDownloadCount->text().toInt() );
    QPair<QString, QString> blog = resource->blog();
    if ( !mBlogs->currentText().isEmpty() ) {
      resource->setBlog( mBlogs->itemData( mBlogs->currentIndex() ).toString(),
                         mBlogs->currentText() );
    }
    mReloadConfig->saveSettings( resource );
    mSaveConfig->saveSettings( resource );
    kDebug( 5700 ) << "ResourceBlogConfig::saveSettings(): saved";
  } else {
    kError( 5700 ) <<"ResourceBlogConfig::saveSettings():"
      " no ResourceBlog, cast failed";
  }
}

void ResourceBlogConfig::slotBlogInfoRetrieved(
    const QMap<QString, QString> &blogs )
{
  kDebug( 5700 ) <<"ResourceBlogConfig::slotBlogInfoRetrieved()";
  QMap<QString,QString>::const_iterator i;
  for (i = blogs.constBegin(); i != blogs.constEnd(); ++i) {
    mBlogs->addItem( i.value(), i.key() );
  }
  if ( mBlogs->count() ) {
    mBlogs->setEnabled( true );
  }
}

void ResourceBlogConfig::slotBlogAPIChanged( int index )
{
  kDebug( 5700 ) <<"ResourceBlogConfig::slotBlogAPIChanged";
  //FIXME Delete me somehow?
  ResourceBlog *blog =  new ResourceBlog();
  blog->setUrl( mUrl->url() );
  blog->setUsername( mUsername->text() );
  blog->setPassword( mPassword->text() );
  blog->setAPI( mAPI->itemText( index ) );
  connect ( blog, SIGNAL( signalBlogInfoRetrieved(
                const QMap<QString,QString> & ) ),
            this, SLOT( slotBlogInfoRetrieved(
                        const QMap<QString,QString> & ) ) );
  blog->listBlogs();
  mBlogs->clear();
  mBlogs->setEnabled( false );
}

#include "resourceblogconfig.moc"
