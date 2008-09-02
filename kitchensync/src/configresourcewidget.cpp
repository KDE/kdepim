/*
    This file is part of KitchenSync.

    Copyright (c) 2008 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "configresourcewidget.h"

#include <QtGui/QButtonGroup>
#include <QtGui/QLabel>
#include <QtGui/QRadioButton>
#include <QtGui/QStackedLayout>

#include <KComboBox>
#include <KLineEdit>
#include <KLocale>
#include <KUrlRequester>

ResourceWidget::ResourceWidget( const QSync::PluginResource &resource, QWidget *parent )
  : QWidget( parent ),
    mResource( resource ),
    mName( 0 ), mPath( 0 ), mUrl( 0 )
{
  QHBoxLayout *layout = new QHBoxLayout( this );

  if ( resource.isOptionSupported( QSync::PluginResource::NameOption ) ) {
    mName = new KLineEdit( this );
    layout->addWidget( mName );
  }

  if ( resource.isOptionSupported( QSync::PluginResource::PathOption ) ) {
    mPath = new KUrlRequester( this );
    mPath->setMode( KFile::File | KFile::Directory | KFile::LocalOnly );
    layout->addWidget( mPath );
  }

  if ( resource.isOptionSupported( QSync::PluginResource::UrlOption ) ) {
    mUrl = new KUrlRequester( this );
    layout->addWidget( mUrl );
  }
}

ResourceWidget::~ResourceWidget()
{
}

QSync::PluginResource ResourceWidget::resource() const
{
  return mResource;
}

void ResourceWidget::load()
{
  if ( mName )
    mName->setText( mResource.name() );

  if ( mPath )
    mPath->setUrl( mResource.path() );

  if ( mUrl )
    mUrl->setUrl( mResource.url() );
}

void ResourceWidget::save()
{
  if ( mName )
    mResource.setName( mName->text() );

  if ( mPath )
    mResource.setPath( mPath->url().path() );

  if ( mUrl )
    mResource.setUrl( mUrl->url().url() );
}



TypeWidget::TypeWidget( const QSync::PluginResource::List &resources, QWidget *parent )
  : QWidget( parent )
{
  QGridLayout *layout = new QGridLayout( this );

  QButtonGroup *group = new QButtonGroup( this );

  int row = 0;
  for ( ; row < resources.count(); ++row ) {
    QRadioButton *button = new QRadioButton( this );
    group->addButton( button );

    if ( resources.count() == 1 ) {
      button->setChecked( true );
      button->hide();
    }

    ResourceWidget *wdg = new ResourceWidget( resources.at( row ), this );

    mMap.insert( button, wdg );

    layout->addWidget( button, row, 0 );
    layout->addWidget( wdg, row, 1 );

    mResourceWidgets.append( wdg );
  }

  layout->setRowStretch( row, 1 );
}

TypeWidget::~TypeWidget()
{
}

void TypeWidget::load()
{
  // load configuration
  for ( int i = 0; i < mResourceWidgets.count(); ++i )
    mResourceWidgets.at( i )->load();

  // set active resource
  QMapIterator<QRadioButton*, ResourceWidget*> it( mMap );
  while ( it.hasNext() ) {
    it.next();
    if ( it.value()->resource().enabled() ) {
      it.key()->setChecked( true );
      break;
    }
  }
}

void TypeWidget::save()
{
  // store active resource
  QMapIterator<QRadioButton*, ResourceWidget*> it( mMap );
  while ( it.hasNext() ) {
    it.next();
    if ( it.key()->isChecked() ) {
      it.value()->resource().setEnabled( true );
      break;
    }
  }

  // save configuration
  for ( int i = 0; i < mResourceWidgets.count(); ++i )
    mResourceWidgets.at( i )->save();
}



ConfigResourceWidget::ConfigResourceWidget( const QSync::PluginResource::List &resources, QWidget *parent )
  : QWidget( parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );

  QHBoxLayout *typeLayout = new QHBoxLayout;
  layout->addLayout( typeLayout );

  QLabel *label = new QLabel( i18n( "Type:" ), this );
  mType = new KComboBox( this );
  typeLayout->addWidget( label );
  typeLayout->addWidget( mType );

  mStack = new QStackedLayout;
  layout->addLayout( mStack );

  QMap<QString, QSync::PluginResource::List> sortMap;

  for ( int i = 0; i < resources.count(); ++i )
    sortMap[ resources.at( i ).objectType() ].append( resources.at( i ) );

  QMapIterator<QString, QSync::PluginResource::List> it( sortMap );
  while ( it.hasNext() ) {
    it.next();

    TypeWidget *wdg = new TypeWidget( it.value(), this );
    mType->addItem( it.key() );
    mTypeWidgetMap.insert( it.key(), wdg );
    mTypeWidgetList.append( wdg );

    mStack->addWidget( wdg );
  }

  typeChanged( 0 );
}

ConfigResourceWidget::~ConfigResourceWidget()
{
}

void ConfigResourceWidget::load()
{
  TypeWidget::MapIterator it( mTypeWidgetMap );
  while ( it.hasNext() ) {
    it.next();
    it.value()->load();
  }
}

void ConfigResourceWidget::save()
{
  TypeWidget::MapIterator it( mTypeWidgetMap );
  while ( it.hasNext() ) {
    it.next();
    it.value()->save();
  }
}

void ConfigResourceWidget::typeChanged( int index )
{
  mStack->setCurrentWidget( mTypeWidgetList.at( index ) );
}

#include "configresourcewidget.moc"
