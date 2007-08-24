/* todoviewer_page.cc			KPilot
**
** Copyright (C) 2007 Bertjan Broeksema <b.broeksema@kdemail.net>
**
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
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/

#include "memoviewer_page.h"

#include "options.h"
#include "pilotDatabase.h"
#include "pilotMemo.h"
#include "kpilotConfig.h"

MemoViewerPage::MemoViewerPage( QWidget *parent, const QString &dbPath )
	: ViewerPageBase( parent, dbPath, CSL1( "MemoDB" ), i18n( "Memo Viewer" ) )
{

}

PilotAppInfoBase* MemoViewerPage::loadAppInfo()
{
	if( database()->isOpen() )
	{
		return new PilotMemoInfo( database() );
	}
	
	return 0L;
}

QListWidgetItem* MemoViewerPage::getListWidgetItem( PilotRecord *rec )
{
	if( !( rec->isDeleted() ) &&
		( !( rec->isSecret() ) || KPilotSettings::showSecrets() ) )
	{
		PilotMemo *memo = new PilotMemo( rec );
		if( memo )
		{
			QListWidgetItem *newItem = new QListWidgetItem;
			newItem->setText( memo->getTitle() );
			newItem->setFlags( Qt::ItemIsSelectable | Qt::ItemIsEnabled );
			return newItem;
		}
	}
	
	return 0L;
}

QString MemoViewerPage::getRecordInfo( PilotRecord *rec )
{
	if( !( rec->isDeleted() ) &&
		( !( rec->isSecret() ) || KPilotSettings::showSecrets() ) )
	{
		PilotMemo *memo = new PilotMemo( rec );
		if( memo )
		{
			return memo->text();
		}
	}
	
	return QString();
}
