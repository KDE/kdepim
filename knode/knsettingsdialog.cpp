/***************************************************************************
                          knsettingsdialog.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qlayout.h>
#include <qwidgetstack.h>
#include <qsplitter.h>
#include <qheader.h>
#include <qhbox.h>

#include <klocale.h>
#include <kconfig.h>
#include <kseparator.h>
#include <kiconloader.h>

#include "knnetaccess.h"
#include "knarticlewidget.h"
#include "knodeview.h"
#include "knfetcharticlemanager.h"
#include "knsavedarticlemanager.h"
#include "kngroupmanager.h"
#include "knaccountmanager.h"
#include "knfoldermanager.h"
#include "knfiltermanager.h"
#include "knaccnewssettings.h"
#include "knaccmailsettings.h"
#include "knreadgensettings.h"
#include "knreadhdrsettings.h"
#include "knappsettings.h"
#include "knfiltersettings.h"
#include "knpostcomsettings.h"
#include "knposttechsettings.h"
#include "knpostspellsettings.h"
#include "kncleanupsettings.h"
#include "knusersettings.h"
#include "knglobals.h"
#include "utilities.h"
#include "knsettingsdialog.h"


//==============================================================================================


KNSettingsWidget::KNSettingsWidget(QWidget *parent) : QWidget(parent)
{
}



KNSettingsWidget::~KNSettingsWidget()
{
}


//==============================================================================================


KNSettingsDialog::KNSettingsDialog(QWidget *parent, const char *name)
  : KDialogBase(TreeList, i18n("Preferences"), Ok|Apply|Cancel|Help, Ok, parent, name, false, true)
{
  setShowIconsInTreeList(true);
  //  setRootIsDecorated(false);

  QStringList list;

  // Set up the folder bitmaps
  list << i18n("Accounts");
  setFolderIcon(list, UserIcon("server"));

  list.clear();
  list << i18n("Reading News");
  setFolderIcon(list, BarIcon("arrow_right"));

  list.clear();
  list << i18n("Posting News");
  setFolderIcon(list, BarIcon("arrow_right"));

  // Identity
  QFrame *frame = addHBoxPage(i18n(" Identity"),i18n("Personal Information"), UserIcon("smile"));
  widgets.append(new KNUserSettings(frame));

  // Accounts / News
  list.clear();
  list << i18n("Accounts") << i18n(" News");
  frame = addHBoxPage(list, i18n("Newsgroups Servers"), UserIcon("group"));
  
  widgets.append(new  KNAccNewsSettings(frame, knGlobals.accManager, knGlobals.gManager));
  
  // Accounts / Mail
  list.clear();
  list << i18n("Accounts") << i18n(" Mail");
  frame = addHBoxPage(list, i18n("Mail Server"), UserIcon("accmail"));
  widgets.append(new KNAccMailSettings(frame));

  // Appearance
  frame = addHBoxPage(i18n("Appearance"), i18n("Customize visual appearance"), BarIcon("arrow_right"));
  widgets.append(new KNAppSettings(frame));
  
  // Read News / General
  list.clear();
  list << i18n("Reading News") << i18n("General");
  frame = addHBoxPage(list, i18n("General Options"), BarIcon("arrow_right"));
  widgets.append(new KNReadGenSettings(frame));

  // Read News // Headers
  list.clear();
  list << i18n("Reading News") << i18n("Headers");
  frame = addHBoxPage(list, i18n("Customize displayed article headers"), BarIcon("arrow_right"));
  widgets.append(new KNReadHdrSettings(frame));

  // Read News / Filters
  list.clear();
  list << i18n("Reading News") << i18n(" Filters");
  frame = addHBoxPage(list,i18n("Article Filters"),BarIcon("fltrblue"));
  widgets.append(new KNFilterSettings(knGlobals.fiManager, frame));

  // Post News / Technical
  list.clear();
  list << i18n("Posting News") << i18n("Technical");
  frame = addHBoxPage(list, i18n("Technical Settings"), BarIcon("arrow_right"));
  widgets.append(new KNPostTechSettings(frame));

  // Post News / Composer
  list.clear();
  list << i18n("Posting News") << i18n("Composer");
  frame = addHBoxPage(list, i18n("Customize composer behaviour"), BarIcon("arrow_right"));
  widgets.append(new KNPostComSettings(frame));

  // Post News / Spelling
  list.clear();
  list << i18n("Posting News") << i18n("Spelling");
  frame = addHBoxPage(list, i18n("Spell checker behavior"), BarIcon("arrow_right"));
  widgets.append(new KNPostSpellSettings(frame));

  // Cleanup
  frame = addHBoxPage(i18n("Cleanup"),i18n("Preserving disk space"), BarIcon("arrow_right"));
  widgets.append(new KNCleanupSettings(frame));

  restoreWindowSize("settingsDlg", this, QSize(508,424));
}


KNSettingsDialog::~KNSettingsDialog()
{
  saveWindowSize("settingsDlg", this->size());
}


void KNSettingsDialog::slotOk()
{
  slotApply();
  KDialogBase::slotOk();
}


void KNSettingsDialog::slotApply()
{
  KNSettingsWidget *sw;
  for(unsigned int i=0; i<widgets.count(); i++) {   // write config...
    sw = widgets.at(i);
    if(sw) sw->apply();
  }

  knGlobals.accManager->readConfig();               // read changed config...
  knGlobals.sArtManager->readConfig();
  knGlobals.gManager->readConfig();
  knGlobals.fArtManager->saveOptions();
  knGlobals.fArtManager->readOptions();
  KNArticleWidget::saveOptions();            // important: remember full header on/off
  KNArticleWidget::readOptions();
  KNArticleWidget::updateInstances();
}

//--------------------------------

#include "knsettingsdialog.moc"
