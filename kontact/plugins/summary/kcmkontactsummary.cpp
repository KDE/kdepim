/*
    This file is part of KDE Kontact.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <kiconloader.h>
#include <klocale.h>
#include <plugin.h>
#include <kplugininfo.h>
#include <ktrader.h>

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqpixmap.h>

#include "kcmkontactsummary.h"

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kontactsummary( TQWidget *parent, const char * ) {
    return new KCMKontactSummary( parent, "kcmkontactsummary" );
  }
}

class PluginItem : public QCheckListItem
{
  public:
    PluginItem( KPluginInfo *info, KListView *parent )
      : TQCheckListItem( parent, TQString::null, TQCheckListItem::CheckBox ),
        mInfo( info )
    {
      TQPixmap pm = KGlobal::iconLoader()->loadIcon( mInfo->icon(), KIcon::Small );
      setPixmap( 0, pm );
    }

    KPluginInfo* pluginInfo() const
    {
      return mInfo;
    }

    virtual TQString text( int column ) const
    {
      if ( column == 0 )
        return mInfo->name();
      else if ( column == 1 )
        return mInfo->comment();
      else
        return TQString::null;
    }

  private:
    KPluginInfo *mInfo;
};

PluginView::PluginView( TQWidget *parent, const char *name )
  : KListView( parent, name )
{
  addColumn( i18n( "Name" ) );
  setAllColumnsShowFocus( true );
  setFullWidth( true );
}

PluginView::~PluginView()
{
}

KCMKontactSummary::KCMKontactSummary( TQWidget *parent, const char *name )
  : KCModule( parent, name )
{
  TQVBoxLayout *layout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Here you can select which summary plugins to have visible in your summary view." ), this );
  layout->addWidget( label );

  mPluginView = new PluginView( this );
  layout->addWidget( mPluginView );

  layout->setStretchFactor( mPluginView, 1 );

  connect( mPluginView, TQT_SIGNAL( clicked( TQListViewItem* ) ),
           this, TQT_SLOT( itemClicked( TQListViewItem* ) ) );
  load();

  KAboutData *about = new KAboutData( I18N_NOOP( "kontactsummary" ),
                                      I18N_NOOP( "KDE Kontact Summary" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2004 Tobias Koenig" ) );

  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKontactSummary::load()
{
  KTrader::OfferList offers = KTrader::self()->query(
      TQString::fromLatin1( "Kontact/Plugin" ),
      TQString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );

  TQStringList activeSummaries;

  KConfig config( "kontact_summaryrc" );
  if ( !config.hasKey( "ActiveSummaries" ) ) {
    activeSummaries << "kontact_kaddressbookplugin";
    activeSummaries << "kontact_specialdatesplugin";
    activeSummaries << "kontact_korganizerplugin";
    activeSummaries << "kontact_todoplugin";
    activeSummaries << "kontact_kpilotplugin";
    activeSummaries << "kontact_weatherplugin";
    activeSummaries << "kontact_newstickerplugin";
  } else {
    activeSummaries = config.readListEntry( "ActiveSummaries" );
  }

  mPluginView->clear();
  mPluginList.clear();

  mPluginList = KPluginInfo::fromServices( offers, &config, "Plugins" );
  KPluginInfo::List::Iterator it;
  KConfig *conf = new KConfig("kontactrc");
  KConfigGroup *cg = new KConfigGroup( conf, "Plugins" );
  for ( it = mPluginList.begin(); it != mPluginList.end(); ++it ) {
      (*it)->load( cg );

    if ( !(*it)->isPluginEnabled() )
      continue;

    TQVariant var = (*it)->property( "X-KDE-KontactPluginHasSummary" );
    if ( !var.isValid() )
      continue;

    if ( var.toBool() == true ) {
      PluginItem *item = new PluginItem( *it, mPluginView );

      if ( activeSummaries.find( (*it)->pluginName() ) != activeSummaries.end() )
        item->setOn( true );
    }
  }
}

void KCMKontactSummary::save()
{
  TQStringList activeSummaries;

  TQListViewItemIterator it( mPluginView, TQListViewItemIterator::Checked );
  while ( it.current() ) {
    PluginItem *item = static_cast<PluginItem*>( it.current() );
    activeSummaries.append( item->pluginInfo()->pluginName() );
    ++it;
  }

  KConfig config( "kontact_summaryrc" );
  config.writeEntry( "ActiveSummaries", activeSummaries );
}

void KCMKontactSummary::defaults()
{
  emit changed( true );
}

void KCMKontactSummary::itemClicked( TQListViewItem* )
{
  emit changed( true );
}

#include "kcmkontactsummary.moc"
