/* dbRecordEditor.cc                KPilot
**
** Copyright (C) 2003 by Dan Pilone
** Writeen 2003 by Reinhold Kainhofer
**
**/

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
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtextedit.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include "dbAppInfoEditor.h"

#include "hexviewwidget.h"

/*************************************************
**************************************************/

DBAppInfoEditor::DBAppInfoEditor(unsigned char*appInfoData, int *l, QWidget *parent)
 : KDialogBase(parent, "AppBlock Editor",false,i18n("Edit AppInfo Block"),
              Ok|Cancel), appInfo(appInfoData), len(l)
{
//	CHexBuffer*buff=new CHexBuffer();
//	fAppInfoEdit=new CHexViewWidget(this, "fAppInfoEdit", 0L);
	fAppInfoEdit=new QTextEdit(this);
	setMainWidget(fAppInfoEdit);
	fillWidgets();
}


DBAppInfoEditor::~DBAppInfoEditor()
{
}

void DBAppInfoEditor::slotOk()
{
	if (KMessageBox::questionYesNo(this, i18n("Changing the AppInfo block might corrupt the whole database. \n\nReally assign the new AppInfo block?"), i18n("Changing AppInfo Block"))==KMessageBox::Yes)
	{
		// TODO: Copy the data over
		// TODO: set the length
		// (*len)=..;
		KDialogBase::slotOk();
	}
}

void DBAppInfoEditor::fillWidgets()
{
	// FUNCTIONSETUP
	//CHexBuffer*fBuff=new CHexBuffer();
	//CHexBuffer*fBuff=fHexEdit->hexBuffer();
	//fBuff->assign((char*)appBlock, len);
	//fHexEdit->setBuffer(fBuff);
	// TODO:  Set the value of the CHexViewWidget from the buffer...
}



#include "dbAppInfoEditor.moc"
