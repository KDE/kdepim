/* setupDialog.cc			KPilot
**
** Copyright (C) 2000-2001 by Adriaan de Groot
**
** This file is part of the KNotes conduit, a conduit for KPilot that
** synchronises the Pilot's memo pad application with KNotes.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/
#include "options.h"

#include <iostream.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <kconfig.h>
#include <kdebug.h>
#include "kpilotConfig.h"
#include "setupDialog.moc"
#include "abbrowserConduitConfig.h"

// Something to allow us to check what revision
// the modules are that make up a binary distribution.
//
//
static const char *setupDialog_id="$Id$";

AbbrowserConduitGeneralPage::AbbrowserConduitGeneralPage(setupDialog *p,
							 KConfig& c) :
      setupDialogPage(i18n("General"),p), fConfigWidget(0L)
    {
    FUNCTIONSETUP;
    
    QGridLayout *grid = new QGridLayout(this,1,1);
    fConfigWidget = new AbbrowserConduitConfig(c, this, "fConfigWidget");
    grid->addWidget(fConfigWidget, 0, 0);
    
    }

int AbbrowserConduitGeneralPage::commitChanges(KConfig& c)
    {
    FUNCTIONSETUP;

    fConfigWidget->commitChanges(c);
    
    return 0;
    }


/* static */ const QString AbbrowserConduitOptions::Group("conduitAbbrowser");

AbbrowserConduitOptions::AbbrowserConduitOptions(QWidget *parent) :
      setupDialog(parent,Group,0L)
    {
    FUNCTIONSETUP;
    KConfig& c = KPilotConfig::getConfig(Group);
    
    addPage(new AbbrowserConduitGeneralPage(this,c));
    addPage(new setupInfoPage(this));
    setupWidget();
    
    (void) setupDialog_id;
    }

  
// Revision 1.1  2001/4/05 00:22:28  stern
// New Abbrowser conduit
//
