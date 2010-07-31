/*
 *  templatemenuaction.cpp  -  menu action to select a template
 *  Program:  kalarm
 *  Copyright © 2005,2008 by David Jarvie <djarvie@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kalarm.h"

#include <kactionclasses.h>
#include <kpopupmenu.h>
#include <kdebug.h>

#include "alarmcalendar.h"
#include "alarmevent.h"
#include "functions.h"
#include "templatemenuaction.moc"


TemplateMenuAction::TemplateMenuAction(const TQString& label, const TQString& icon, TQObject* receiver,
                                       const char* slot, KActionCollection* actions, const char* name)
	: KActionMenu(label, icon, actions, name)
{
	setDelayed(false);
	connect(popupMenu(), TQT_SIGNAL(aboutToShow()), TQT_SLOT(slotInitMenu()));
	connect(popupMenu(), TQT_SIGNAL(activated(int)), TQT_SLOT(slotSelected(int)));
	connect(this, TQT_SIGNAL(selected(const KAEvent&)), receiver, slot);
}

/******************************************************************************
* Called when the New From Template action is clicked.
* Creates a popup menu listing all alarm templates, in sorted name order.
*/
void TemplateMenuAction::slotInitMenu()
{
	KPopupMenu* menu = popupMenu();
	menu->clear();
	mOriginalTexts.clear();
	TQValueList<KAEvent> templates = KAlarm::templateList();
	for (TQValueList<KAEvent>::ConstIterator it = templates.constBegin();  it != templates.constEnd();  ++it)
	{
		TQString name = (*it).templateName();
		// Insert the template in sorted order
		TQStringList::Iterator tit;
		for (tit = mOriginalTexts.begin();
		     tit != mOriginalTexts.end()  &&  TQString::localeAwareCompare(name, *tit) > 0;
		     ++tit);
		mOriginalTexts.insert(tit, name);
	}
	for (TQStringList::ConstIterator tit = mOriginalTexts.constBegin();  tit != mOriginalTexts.constEnd();  ++tit)
		menu->insertItem(*tit);
}

/******************************************************************************
*  Called when a template is selected from the New From Template popup menu.
*  Executes a New Alarm dialog, preset from the selected template.
*/
void TemplateMenuAction::slotSelected(int id)
{
	KPopupMenu* menu = popupMenu();
	TQString item = mOriginalTexts[menu->indexOf(id)];
	if (!item.isEmpty())
	{
		AlarmCalendar* cal = AlarmCalendar::templateCalendarOpen();
		if (cal)
		{
			KAEvent templ = KAEvent::findTemplateName(*cal, item);
			emit selected(templ);
		}
	}
}
