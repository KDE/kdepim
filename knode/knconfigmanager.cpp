/*
    knconfigmanager.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include "knconfigmanager.h"

#include <kiconloader.h>
#include <klocale.h>
#include <kwin.h>

#include <qhbox.h>
#include <qframe.h>

#include "utilities.h"
#include "knglobals.h"
#include "knarticlewidget.h"
#include "knarticlefactory.h"
#include "knodeview.h"


KNConfigManager::KNConfigManager(QObject *p, const char *n) : QObject(p, n), d_ialog(0)
{
  i_dentity           = new KNConfig::Identity();
  a_ppearance         = new KNConfig::Appearance();
  r_eadNewsGeneral    = new KNConfig::ReadNewsGeneral();
  r_eadNewsViewer     = new KNConfig::ReadNewsViewer();
  d_isplayedHeaders   = new KNConfig::DisplayedHeaders();
  p_ostNewsTechnical  = new KNConfig::PostNewsTechnical();
  p_ostNewsCompose    = new KNConfig::PostNewsComposer();
  c_leanup            = new KNConfig::Cleanup();
  p_rivacy            = new KNConfig::Privacy();
}


KNConfigManager::~KNConfigManager()
{
  delete i_dentity;
  delete a_ppearance;
  delete r_eadNewsGeneral;
  delete r_eadNewsViewer;
  delete d_isplayedHeaders;
  delete p_ostNewsTechnical;
  delete p_ostNewsCompose;
  delete c_leanup;
  delete p_rivacy;
}


void KNConfigManager::configure()
{
  if(!d_ialog) {
    d_ialog=new KNConfigDialog(this, knGlobals.topWidget, "Preferences_Dlg");
    connect(d_ialog, SIGNAL(finished()), this, SLOT(slotDialogDone()));
    d_ialog->show();
  }
  else
    KWin::setActiveWindow(d_ialog->winId());
}


void KNConfigManager::slotDialogDone()
{
  d_ialog->delayedDestruct();
  d_ialog=0;
}


//===================================================================================================


KNConfigDialog::KNConfigDialog(KNConfigManager *m, QWidget *p, const char *n)
  : KDialogBase(TreeList, i18n("Preferences"), Ok|Apply|Cancel|Help, Ok, p, n, false, true)
{
  setShowIconsInTreeList(true);
  //  setRootIsDecorated(false);

  QStringList list;

  // Set up the folder bitmaps
  list << QString(" ")+i18n("Accounts");
  setFolderIcon(list, UserIcon("server_big"));

  list.clear();
  list << QString(" ")+i18n("Reading News");
  setFolderIcon(list, BarIcon("mail_get"));

  list.clear();
  list << QString(" ")+i18n("Posting News");
  setFolderIcon(list, BarIcon("mail_forward"));

  // Identity
  QFrame *frame = addHBoxPage(i18n("Identity"),i18n("Personal Information"), BarIcon("identity", KIcon::SizeMedium ));
  w_idgets.append(new KNConfig::IdentityWidget(m->identity(), frame));

  // Accounts / News
  list.clear();
  list << QString(" ")+i18n("Accounts") << i18n(" News");
  frame = addHBoxPage(list, i18n("Newsgroups Servers"), UserIcon("group_big"));
  w_idgets.append(new  KNConfig::NntpAccountListWidget(frame));

  // Accounts / Mail
  list.clear();
  list << QString(" ")+i18n("Accounts") << i18n(" Mail");
  frame = addHBoxPage(list, i18n("Mail Server (SMTP)"), BarIcon("mail_generic"));
  w_idgets.append(new KNConfig::SmtpAccountWidget(frame));

  // Appearance
  frame = addHBoxPage(QString(" ")+i18n("Appearance"), i18n("Customize visual appearance"), BarIcon("appearance"));
  w_idgets.append(new KNConfig::AppearanceWidget(m->appearance(), frame));

  // Read News / General
  list.clear();
  list << QString(" ")+i18n("Reading News") << QString(" ")+i18n("General");
  frame = addHBoxPage(list, i18n("General Options"), BarIcon("misc"));
  w_idgets.append(new KNConfig::ReadNewsGeneralWidget(m->readNewsGeneral(), frame));

  // Read News / Scores
  list.clear();
  list << QString(" ")+i18n("Reading News") << QString(" ")+i18n("Scoring");
  frame = addHBoxPage(list,i18n("Scoring Rules"),BarIcon("misc"));
  w_idgets.append(new KNConfig::ScoreListWidget(frame));

  // Read News / Filters
  list.clear();
  list << QString(" ")+i18n("Reading News") << i18n(" Filters");
  frame = addHBoxPage(list,i18n("Article Filters"),BarIcon("filter"));
  w_idgets.append(new KNConfig::FilterListWidget(frame));

  // Read News / Headers
  list.clear();
  list << QString(" ")+i18n("Reading News") << QString(" ")+i18n("Headers");
  frame = addHBoxPage(list, i18n("Customize displayed article headers"), BarIcon("text_block"));
  w_idgets.append(new KNConfig::DisplayedHeadersWidget(m->displayedHeaders(), frame));

  // Read News / Viewer
  list.clear();
  list << QString(" ")+i18n("Reading News") << QString(" ")+i18n("Viewer");
  frame = addHBoxPage(list, i18n("Customize article viewer behaviour"), BarIcon("contents"));
  w_idgets.append(new KNConfig::ReadNewsViewerWidget(m->readNewsViewer(), frame));

  // Post News / Technical
  list.clear();
  list << QString(" ")+i18n("Posting News") << QString(" ")+i18n("Technical");
  frame = addHBoxPage(list, i18n("Technical Settings"), BarIcon("configure"));
  w_idgets.append(new KNConfig::PostNewsTechnicalWidget(m->postNewsTechnical(), frame));

  // Post News / Composer
  list.clear();
  list << QString(" ")+i18n("Posting News") << QString(" ")+i18n("Composer");
  frame = addHBoxPage(list, i18n("Customize composer behaviour"), BarIcon("signature"));
  w_idgets.append(new KNConfig::PostNewsComposerWidget(m->postNewsComposer(), frame));

  // Post News / Spelling
  list.clear();
  list << QString(" ")+i18n("Posting News") << QString(" ")+i18n("Spelling");
  frame = addHBoxPage(list, i18n("Spell checker behavior"), BarIcon("spellcheck"));
  w_idgets.append(new KNConfig::PostNewsSpellingWidget(frame));

  // Privacy
  frame = addHBoxPage(QString(" ")+i18n("Signing/Verifying"),
                      i18n("Protect your privacy by signing and verifying postings"), BarIcon("password"));
  w_idgets.append(new KNConfig::PrivacyWidget(m->privacy(),frame));

  // Cleanup
  frame = addHBoxPage(QString(" ")+i18n("Cleanup"),i18n("Preserving disk space"), BarIcon("wizard"));
  w_idgets.append(new KNConfig::CleanupWidget(m->cleanup(), frame));

  KNHelper::restoreWindowSize("settingsDlg", this, QSize(508,424));

  setHelp("anc-setting-your-identity");
}


KNConfigDialog::~KNConfigDialog()
{
  KNHelper::saveWindowSize("settingsDlg", this->size());
}


void KNConfigDialog::slotApply()
{
  for(KNConfig::BaseWidget *w=w_idgets.first(); w; w=w_idgets.next())
    w->apply();

  KNArticleWidget::configChanged();
  knGlobals.view->configChanged();
  knGlobals.artFactory->configChanged();
}


void KNConfigDialog::slotOk()
{
  slotApply();
  KDialogBase::slotOk();
}


//-----------------------------
#include "knconfigmanager.moc"
