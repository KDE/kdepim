/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
                                                                        
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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.           
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#include <qcheckbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qpushbutton.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktrader.h>

#include "extensionwidget.h"
#include "kabprefs.h"

#include "kabconfigwidget.h"

class ExtensionItem : public QCheckListItem
{
  public:
    ExtensionItem( QListView *parent, const QString &text );

    void setService( const KService::Ptr &ptr );
    bool configWidgetAvailable() const;
    ConfigureWidget *configWidget( QWidget *parent ) const;
    QString libraryName() const;

    virtual QString text( int column ) const;

  private:
    KService::Ptr mPtr;
};

KABConfigWidget::KABConfigWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this, KDialog::marginHint(),
                                            KDialog::spacingHint() );

  QGroupBox *groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "General" ), this );
  QVBoxLayout *boxLayout = new QVBoxLayout( groupBox->layout() );
  boxLayout->setAlignment( Qt::AlignTop );

  mViewsSingleClickBox = new QCheckBox( i18n( "Honor KDE single click" ), groupBox, "msingle" );
  boxLayout->addWidget( mViewsSingleClickBox );

  mNameParsing = new QCheckBox( i18n( "Automatic name parsing for new addressees" ), groupBox, "mparse" );
  boxLayout->addWidget( mNameParsing );

  topLayout->addWidget( groupBox );

  groupBox = new QGroupBox( 0, Qt::Vertical, i18n( "Extensions" ), this );
  boxLayout = new QVBoxLayout( groupBox->layout() );
  boxLayout->setAlignment( Qt::AlignTop );

  mExtensionView = new KListView( groupBox );
  mExtensionView->setAllColumnsShowFocus( true );
  mExtensionView->addColumn( i18n( "Name" ) );
  mExtensionView->addColumn( i18n( "Description" ) );
  boxLayout->addWidget( mExtensionView );

  mConfigureButton = new QPushButton( i18n( "Configure..." ), groupBox );
  mConfigureButton->setEnabled( false );
  boxLayout->addWidget( mConfigureButton );

  topLayout->addWidget( groupBox );

  connect( mNameParsing, SIGNAL( toggled( bool ) ), this, SLOT( modified() ) );
  connect( mViewsSingleClickBox, SIGNAL( toggled( bool ) ), this, SLOT( modified() ) );
  connect( mExtensionView, SIGNAL( selectionChanged( QListViewItem* ) ),
           SLOT( selectionChanged( QListViewItem* ) ) );
  connect( mExtensionView, SIGNAL( clicked( QListViewItem* ) ),
           SLOT( itemClicked( QListViewItem* ) ) );
  connect( mConfigureButton, SIGNAL( clicked() ),
           SLOT( configureExtension() ) );
}

void KABConfigWidget::restoreSettings()
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  mNameParsing->setChecked( KABPrefs::instance()->mAutomaticNameParsing );
  mViewsSingleClickBox->setChecked( KABPrefs::instance()->mHonorSingleClick );

  restoreExtensionSettings();

  blockSignals( blocked );

  emit changed( false );
}

void KABConfigWidget::saveSettings()
{
  kdDebug() << "KABConfigWidget::save()" << endl;

  KABPrefs::instance()->mAutomaticNameParsing = mNameParsing->isChecked();
  KABPrefs::instance()->mHonorSingleClick = mViewsSingleClickBox->isChecked();

  KABPrefs::instance()->writeConfig();

  saveExtensionSettings();

  emit changed( false );
}

void KABConfigWidget::defaults()
{
  mNameParsing->setChecked( true );
  mViewsSingleClickBox->setChecked( false );

  emit changed( true );
}

void KABConfigWidget::modified()
{
  emit changed( true );
}

void KABConfigWidget::restoreExtensionSettings()
{
  KConfig config( "kaddressbookrc" );
  config.setGroup( "Extensions_General" );

  QStringList activeExtensions = config.readListEntry( "activeExtensions" );

  mExtensionView->clear();

  KTrader::OfferList plugins = KTrader::self()->query( "KAddressBook/Extension" );
  KTrader::OfferList::ConstIterator it;
  for ( it = plugins.begin(); it != plugins.end(); ++it ) {
    if ( !(*it)->hasServiceType( "KAddressBook/Extension" ) )
      continue;

    ExtensionItem *item = new ExtensionItem( mExtensionView, (*it)->name() );
    item->setService( *it );
    if ( activeExtensions.contains( (*it)->library() ) )
      item->setOn( true );
  }
}

void KABConfigWidget::saveExtensionSettings()
{
  KConfig config( "kaddressbookrc" );
  config.deleteGroup( "Extensions_General" );
  config.setGroup( "Extensions_General" );

  QStringList activeExtensions;

  QPtrList<QListViewItem> list;
  QListViewItemIterator it( mExtensionView );
  while ( it.current() ) {
    ExtensionItem *item = static_cast<ExtensionItem*>( it.current() );
    if ( item ) {
      if ( item->isOn() )
        activeExtensions.append( item->libraryName() );
    }
    ++it;
  }

  config.writeEntry( "activeExtensions", activeExtensions );
}

void KABConfigWidget::configureExtension()
{
  ExtensionItem *item = static_cast<ExtensionItem*>( mExtensionView->currentItem() );
  if ( !item )
    return;

  ConfigureWidget *wdg = item->configWidget( this );
  if ( !wdg ) {
    KMessageBox::sorry( this, i18n( "No configure dialog available for this plugin." ) );
    return;
  }
}

void KABConfigWidget::selectionChanged( QListViewItem *i )
{
  ExtensionItem *item = static_cast<ExtensionItem*>( i );
  if ( !item )
    return;

  mConfigureButton->setEnabled( item->configWidgetAvailable() );
}

void KABConfigWidget::itemClicked( QListViewItem *item )
{
  if ( item != 0 )
    modified();
}



ExtensionItem::ExtensionItem( QListView *parent, const QString &text )
  : QCheckListItem( parent, text, CheckBox )
{
}

void ExtensionItem::setService( const KService::Ptr &ptr )
{
  mPtr = ptr;
}

bool ExtensionItem::configWidgetAvailable() const
{
  KLibFactory *factory = KLibLoader::self()->factory( mPtr->library() );
  if ( !factory )
    return false;

  ExtensionFactory *extensionFactory = static_cast<ExtensionFactory*>( factory );
  if ( !extensionFactory )
    return false;

  return extensionFactory->configureWidgetAvailable();
}

ConfigureWidget *ExtensionItem::configWidget( QWidget *parent ) const
{
  KLibFactory *factory = KLibLoader::self()->factory( mPtr->library() );
  if ( !factory )
    return 0;

  ExtensionFactory *extensionFactory = static_cast<ExtensionFactory*>( factory );
  if ( !extensionFactory )
    return 0;

  return extensionFactory->configureWidget( parent );
}

QString ExtensionItem::libraryName() const
{
  return mPtr->library();
}

QString ExtensionItem::text( int column ) const
{
  if ( column == 0 )
    return mPtr->name();
  else if ( column == 1 )
    return mPtr->comment();
  else
    return QString::null;
}

#include "kabconfigwidget.moc"
