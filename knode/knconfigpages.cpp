/*
    knconfigpages.cpp

    KNode, the KDE newsreader
    Copyright (c) 2004 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qlayout.h>

#include <kcmodule.h>
#include <kdebug.h>
#include <klocale.h>

#include "knglobals.h"
#include "knconfig.h"
#include "knconfigmanager.h"
#include "knconfigpages.h"


//
// common config page with tabs (code mostly taken from kmail)
//
KNConfig::BasePageWithTabs::BasePageWithTabs( QWidget * parent, const char * name )
  : KCModule( parent, name )
{
  QVBoxLayout *vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  mTabWidget = new QTabWidget( this );
  vlay->addWidget( mTabWidget );
}

void KNConfig::BasePageWithTabs::addTab( KCModule* tab, const QString & title ) {
  mTabWidget->addTab( tab, title );
  connect( tab, SIGNAL(changed( bool )), this, SIGNAL(changed( bool )) );
}

void KNConfig::BasePageWithTabs::load() {
  for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    KCModule *tab = (KCModule*) mTabWidget->page(i);
    if ( tab )
      tab->load();
  }
}

void KNConfig::BasePageWithTabs::save() {
  for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    KCModule *tab = (KCModule*) mTabWidget->page(i);
    if ( tab )
      tab->save();
  }
}

void KNConfig::BasePageWithTabs::defaults() {
  for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    KCModule *tab = (KCModule*) mTabWidget->page(i);
    if ( tab )
      tab->defaults();
  }
}



//
// identity page
//
extern "C"
{
  KDE_EXPORT KCModule *create_knode_config_identity( QWidget *parent, const char * )
  {
    KNConfig::IdentityWidget *page = new KNConfig::IdentityWidget( 
      knGlobals.configManager()->identity(), 
      parent, 
      "kcmknode_config_identity" );
    return page;
  }
}



//
// accounts page
//
extern "C"
{
  KCModule *create_knode_config_accounts( QWidget *parent, const char * )
  {
    KNConfig::AccountsPage *page = new KNConfig::AccountsPage( parent, "kcmknode_config_accounts" );
    return page;
  }
}

KNConfig::AccountsPage::AccountsPage(QWidget *parent, const char *name)
  : BasePageWithTabs(parent, name) {
  
  addTab(new KNConfig::NntpAccountListWidget(this), i18n("Newsgroup Servers"));
  addTab(new KNConfig::SmtpAccountWidget(this), i18n("Mail Server (SMTP)"));
}



//
// appearance page
//
extern "C"
{
  KCModule *create_knode_config_appearance( QWidget *parent, const char * )
  {
    KNConfig::AppearanceWidget *page = new KNConfig::AppearanceWidget( parent, "kcmknode_config_appearance" );
    return page;
  }
}



//
// read news page
//
extern "C"
{
  KCModule *create_knode_config_read_news( QWidget *parent, const char * )
  {
    KNConfig::ReadNewsPage *page = new KNConfig::ReadNewsPage( parent, "kcmknode_config_read_news" );
    return page;
  }
}

KNConfig::ReadNewsPage::ReadNewsPage(QWidget *parent, const char *name)
  : BasePageWithTabs(parent, name) {
  
  KNConfigManager *cfgMgr = knGlobals.configManager();
  addTab(new KNConfig::ReadNewsGeneralWidget(cfgMgr->readNewsGeneral(), this), i18n("General"));
  addTab(new KNConfig::ReadNewsNavigationWidget(cfgMgr->readNewsNavigation(), this), i18n("Navigation"));
  addTab(new KNConfig::ScoringWidget(cfgMgr->scoring(), this), i18n("Scoring"));
  addTab(new KNConfig::FilterListWidget(this), i18n("Filters"));
  addTab(new KNConfig::DisplayedHeadersWidget(cfgMgr->displayedHeaders(), this), i18n("Headers"));
  addTab(new KNConfig::ReadNewsViewerWidget(cfgMgr->readNewsViewer(), this), i18n("Viewer"));
}



//
// post news page
//
extern "C"
{
  KCModule *create_knode_config_post_news( QWidget *parent, const char * )
  {
    KNConfig::PostNewsPage *page = new KNConfig::PostNewsPage( parent, "kcmknode_config_post_news" );
    return page;
  }
}

KNConfig::PostNewsPage::PostNewsPage(QWidget *parent, const char *name)
  : BasePageWithTabs(parent, name) {
  
  KNConfigManager *cfgMgr = knGlobals.configManager();
  addTab(new KNConfig::PostNewsTechnicalWidget(cfgMgr->postNewsTechnical(), this), i18n("Technical"));
  addTab(new KNConfig::PostNewsComposerWidget(cfgMgr->postNewsComposer(), this), i18n("Composer"));
  addTab(new KNConfig::PostNewsSpellingWidget(this), i18n("Spelling"));
}



//
// privacy page
//
extern "C"
{
  KCModule *create_knode_config_privacy( QWidget *parent, const char * )
  {
    KNConfig::PrivacyWidget *page = new KNConfig::PrivacyWidget( parent, "kcmknode_config_privacy" );
    return page;
  }
}



//
// cleanup page
//
extern "C"
{
  KCModule *create_knode_config_cleanup( QWidget *parent, const char * )
  {
    KNConfig::CleanupWidget *page = new KNConfig::CleanupWidget( parent, "kcmknode_config_cleanup" );
    return page;
  }
}


#include "knconfigpages.moc"
