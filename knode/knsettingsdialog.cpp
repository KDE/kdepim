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

#include "knode.h"
#include "knaccnewssettings.h"
#include "knaccmailsettings.h"
#include "knreadgensettings.h"
#include "knreadhdrsettings.h"
#include "knreadappsettings.h"
#include "knfiltersettings.h"
#include "knpostcomsettings.h"
#include "knposttechsettings.h"
#include "kncleanupsettings.h"
#include "knkeysettings.h"
#include "knusersettings.h"
#include "knglobals.h"
#include "utilities.h"
#include "knsettingsdialog.h"


KNSettingsDialog::KNSettingsDialog() : KDialogBase(TreeList, i18n("Settings"), Ok|Cancel|Help, Ok,
                                                   knGlobals.top, 0, true, true)
{
  setShowIconsInTreeList(true);
  //  setRootIsDecorated(false);

  QStringList list;

  // Set up the folder bitmaps
  list << i18n("Accounts");
  setFolderIcon(list, BarIcon("arrow_right"));

  list.clear();
  list << i18n("Read News");
  setFolderIcon(list, BarIcon("arrow_right"));

  list.clear();
  list << i18n("Post News");
  setFolderIcon(list, BarIcon("arrow_right"));
  

  // Accounts / News
  list.clear();
  list << i18n("Accounts") << i18n("News"); 
  QFrame *frame = addHBoxPage(list, i18n("News"), BarIcon("arrow_right"));
  
  widgets.append(new  KNAccNewsSettings(frame, knGlobals.accManager));
  
  // Accounts / Mail
  list.clear();
  list << i18n("Accounts") << i18n("Mail");
  frame = addHBoxPage(list, i18n("Mail"), BarIcon("arrow_right"));
  widgets.append(new KNAccMailSettings(frame));
  

  // User
  frame = addHBoxPage(i18n("User"),i18n("User"), BarIcon("arrow_right"));
  widgets.append(new KNUserSettings(frame));

  // Read News / General
  list.clear();
  list << i18n("Read News") << i18n("General");
  frame = addHBoxPage(list, i18n("General"), BarIcon("arrow_right"));
  widgets.append(new KNReadGenSettings(frame));

  // Read News // Headers
  list.clear();
  list << i18n("Read News") << i18n("Headers");
  frame = addHBoxPage(list, i18n("Headers"), BarIcon("arrow_right"));
  widgets.append(new KNReadHdrSettings(frame));

  // Read News / Appearance
  list.clear();
  list << i18n("Read News") << i18n("Appearance");
  frame = addHBoxPage(list, i18n("Appearance"), BarIcon("arrow_right"));
  widgets.append(new KNReadAppSettings(frame));

  // Read News / Filters
  list.clear();
  list << i18n("Read News") << i18n("Filters");
  frame = addHBoxPage(list,i18n("Filters"),BarIcon("fltrblue"));
  widgets.append(new KNFilterSettings(knGlobals.fiManager, frame));

  // Post News / Technical
  list.clear();
  list << i18n("Post News") << i18n("Technical");
  frame = addHBoxPage(list, i18n("Technical"), BarIcon("arrow_right"));
  widgets.append(new KNPostTechSettings(frame));

  // Post News / Composer
  list.clear();
  list << i18n("Post News") << i18n("Composer");
  frame = addHBoxPage(list, i18n("Composer"), BarIcon("arrow_right"));
  widgets.append(new KNPostComSettings(frame));

  // Cleanup
  frame = addHBoxPage(i18n("Cleanup"),i18n("Cleanup"), BarIcon("arrow_right"));
  widgets.append(new KNCleanupSettings(frame));
  
  // Key bindings
  frame = addHBoxPage(i18n("Key bindings"), i18n("Key bindings"), BarIcon("arrow_right"));
  widgets.append(new KNKeySettings(frame));

  restoreWindowSize("settingsDlg", this, sizeHint());
}
  
KNSettingsDialog::~KNSettingsDialog()
{
  saveWindowSize("settingsDlg", this->size());
}

  
  
void KNSettingsDialog::apply()
{
  KNSettingsWidget *sw;
  for(unsigned int i=0; i<widgets.count(); i++) {
    sw = widgets.at(i);
    if(sw) sw->apply();
  }
}
  
void KNSettingsDialog::slotHelp()
{
  qDebug("Remember to implement the help facilities");
}


//--------------------------------

#include "knsettingsdialog.moc"
