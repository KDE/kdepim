/* dbRecordEditor.cc                KPilot
**
** Copyright (C) 2003 Reinhold Kainhofer <reinhold@kainhofer.com>
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "options.h"

#include <qtextedit.h>
#include <qlabel.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include "dbAppInfoEditor.h"

#include <khexedit/byteseditinterface.h>
using namespace KHE;


/*************************************************
**************************************************/

DBAppInfoEditor::DBAppInfoEditor(char*appInfoData, int l, QWidget *parent) :
	KDialogBase(parent, "AppBlock Editor",false,
		i18n("Edit AppInfo Block"),
		Ok|Cancel), 
	appInfo(appInfoData), 
	len(l)
{
	fAppInfoEdit = KHE::createBytesEditWidget( this, "fAppInfoEdit" );
	if( fAppInfoEdit )
	{
		 // fetch the editor interface
		KHE::BytesEditInterface* fAppInfoEditIf = KHE::bytesEditInterface( fAppInfoEdit );
		Q_ASSERT( fAppInfoEditIf ); // This should not fail!
		if( fAppInfoEditIf )
		{
			fAppInfoEditIf->setData( (char*)appInfoData, l );
			fAppInfoEditIf->setMaxDataSize( l );
			// TODO_RK: Make the app info editable. I need to find a way 
			// to sync the appInfoBlock to the handheld
			fAppInfoEditIf->setReadOnly( true );
		}
	}
	else
	{
		QLabel*tmpW = new QLabel( i18n("To view the Application info block data, please install a hex editor (e.g. khexedit from kdeutils)."), this );
		tmpW->setBackgroundMode( Qt::PaletteMid );
		tmpW->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter | Qt::WordBreak);
		tmpW->setFrameShape( QFrame::Panel );
		tmpW->setFrameShadow( QFrame::Sunken );
		fAppInfoEdit = tmpW;
	}
	setMainWidget( fAppInfoEdit );
	fillWidgets();
}


DBAppInfoEditor::~DBAppInfoEditor()
{
}

void DBAppInfoEditor::slotOk()
{
	KMessageBox::sorry(this, i18n("Changing the AppInfo block isn't yet supported by KPilot!"));
/*	if (KMessageBox::questionYesNo(this, i18n("Changing the AppInfo block "
		"might corrupt the whole database. \n\nReally assign the new AppInfo "
		"block?"), i18n("Changing AppInfo Block"), i18n("Assign"), KStdGuiItem::cancel())==KMessageBox::Yes)
	{
		// TODO: Copy the data over
		// TODO: set the length
		// (*len)=..;
	}*/
	KDialogBase::slotOk();
}

void DBAppInfoEditor::fillWidgets()
{
	// FUNCTIONSETUP
}



#include "dbAppInfoEditor.moc"
