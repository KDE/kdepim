/*
    KNode, the KDE newsreader
    Copyright (c) 2004-2005 Volker Krause <volker.krause@rwth-aachen.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <qlayout.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kcmodule.h>
#include <kdebug.h>
#include <klocale.h>

#include "knglobals.h"
#include "knconfig.h"
#include "knconfigmanager.h"
#include "knconfigpages.h"
#include "knconfigwidgets.h"

#include <kdepimmacros.h>

//
// common config page with tabs (code mostly taken from kmail)
//
KNode::KCMTabContainer::KCMTabContainer( KInstance *inst, QWidget * parent )
  : KCModule( inst, parent )
{
  QVBoxLayout *vlay = new QVBoxLayout( this, 0, KDialog::spacingHint() );
  mTabWidget = new QTabWidget( this );
  vlay->addWidget( mTabWidget );
}

void KNode::KCMTabContainer::addTab( KCModule* tab, const QString & title ) {
  mTabWidget->addTab( tab, title );
  connect( tab, SIGNAL(changed( bool )), this, SIGNAL(changed( bool )) );
}

void KNode::KCMTabContainer::load() {
  for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    KCModule *tab = (KCModule*) mTabWidget->page(i);
    if ( tab )
      tab->load();
  }
}

void KNode::KCMTabContainer::save() {
  for ( int i = 0 ; i < mTabWidget->count() ; ++i ) {
    KCModule *tab = (KCModule*) mTabWidget->page(i);
    if ( tab )
      tab->save();
  }
}

void KNode::KCMTabContainer::defaults()
{
  KCModule *tab = static_cast<KCModule*>( mTabWidget->currentWidget() );
  if ( tab )
    tab->defaults();
}



//
// identity page
//
extern "C"
{
  KDE_EXPORT KCModule *create_knode_config_identity( QWidget *parent )
  {
    KNode::IdentityWidget *page = new KNode::IdentityWidget(
        knGlobals.configManager()->identity(), knGlobals.instance(), parent );
    return page;
  }
}



//
// accounts page
//
extern "C"
{
  KCModule *create_knode_config_accounts( QWidget *parent )
  {
    KNode::AccountsPage *page = new KNode::AccountsPage( knGlobals.instance(), parent );
    return page;
  }
}

KNode::AccountsPage::AccountsPage( KInstance *inst,QWidget *parent )
  : KCMTabContainer( inst,parent ) {

  addTab( new NntpAccountListWidget( knGlobals.instance(), this ), i18n("Newsgroup Servers") );
  addTab( new SmtpAccountWidget( knGlobals.instance(), this ), i18n("Mail Server (SMTP)") );
}



//
// appearance page
//
extern "C"
{
  KCModule *create_knode_config_appearance( QWidget *parent )
  {
    KNode::AppearanceWidget *page = new KNode::AppearanceWidget( knGlobals.instance(), parent );
    return page;
  }
}



//
// read news page
//
extern "C"
{
  KCModule *create_knode_config_read_news( QWidget *parent )
  {
    KNode::ReadNewsPage *page = new KNode::ReadNewsPage( knGlobals.instance(), parent );
    return page;
  }
}

KNode::ReadNewsPage::ReadNewsPage( KInstance *inst,QWidget *parent )
  : KCMTabContainer( inst, parent )
{
  addTab( new ReadNewsGeneralWidget( knGlobals.instance(), this ), i18n("General") );
  addTab( new ReadNewsNavigationWidget( knGlobals.instance(), this ), i18n("Navigation") );
  addTab( new ScoringWidget( knGlobals.instance(), this ), i18n("Scoring") );
  addTab( new FilterListWidget( knGlobals.instance(), this ), i18n("Filters") );
  addTab( new DisplayedHeadersWidget( knGlobals.configManager()->displayedHeaders(), knGlobals.instance(), this ), i18n("Headers") );
  addTab( new ReadNewsViewerWidget( knGlobals.instance(), this ), i18n("Viewer") );
}



//
// post news page
//
extern "C"
{
  KCModule *create_knode_config_post_news( QWidget *parent )
  {
    KNode::PostNewsPage *page = new KNode::PostNewsPage( knGlobals.instance(), parent );
    return page;
  }
}

KNode::PostNewsPage::PostNewsPage( KInstance *inst, QWidget *parent )
  : KCMTabContainer( inst, parent )
{
  addTab( new PostNewsTechnicalWidget( knGlobals.configManager()->postNewsTechnical(), knGlobals.instance(), this ), i18n("Technical") );
  addTab( new PostNewsComposerWidget( knGlobals.instance(), this ), i18n("Composer") );
  addTab( new PostNewsSpellingWidget( knGlobals.instance(), this ), i18n("Spelling") );
}



//
// privacy page
//
extern "C"
{
  KCModule *create_knode_config_privacy( QWidget *parent )
  {
    KNode::PrivacyWidget *page = new KNode::PrivacyWidget( knGlobals.instance(), parent );
    return page;
  }
}



//
// cleanup page
//
extern "C"
{
  KCModule *create_knode_config_cleanup( QWidget *parent )
  {
    KNode::CleanupWidget *page = new KNode::CleanupWidget( knGlobals.instance(), parent );
    return page;
  }
}


#include "knconfigpages.moc"
