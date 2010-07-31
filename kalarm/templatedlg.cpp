/*
 *  templatedlg.cpp  -  dialogue to create, edit and delete alarm templates
 *  Program:  kalarm
 *  Copyright © 2004-2006 by David Jarvie <software@astrojar.org.uk>
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

#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqwhatsthis.h>

#include <klocale.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <kaccel.h>
#include <kdebug.h>

#include "editdlg.h"
#include "alarmcalendar.h"
#include "functions.h"
#include "templatelistview.h"
#include "undo.h"
#include "templatedlg.moc"

static const char TMPL_DIALOG_NAME[] = "TemplateDialog";


TemplateDlg* TemplateDlg::mInstance = 0;


TemplateDlg::TemplateDlg(TQWidget* parent, const char* name)
	: KDialogBase(KDialogBase::Plain, i18n("Alarm Templates"), Close, Ok, parent, name, false, true)
{
	TQWidget* topWidget = plainPage();
	TQBoxLayout* topLayout = new TQHBoxLayout(topWidget);
	topLayout->setSpacing(spacingHint());

	TQBoxLayout* layout = new TQVBoxLayout(topLayout);
	mTemplateList = new TemplateListView(true, i18n("The list of alarm templates"), topWidget);
	mTemplateList->setSelectionMode(TQListView::Extended);
	mTemplateList->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding, TQSizePolicy::Expanding));
	connect(mTemplateList, TQT_SIGNAL(selectionChanged()), TQT_SLOT(slotSelectionChanged()));
	layout->addWidget(mTemplateList);

	layout = new TQVBoxLayout(topLayout);
	TQPushButton* button = new TQPushButton(i18n("&New..."), topWidget);
	connect(button, TQT_SIGNAL(clicked()), TQT_SLOT(slotNew()));
	TQWhatsThis::add(button, i18n("Create a new alarm template"));
	layout->addWidget(button);

	mEditButton = new TQPushButton(i18n("&Edit..."), topWidget);
	connect(mEditButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotEdit()));
	TQWhatsThis::add(mEditButton, i18n("Edit the currently highlighted alarm template"));
	layout->addWidget(mEditButton);

	mCopyButton = new TQPushButton(i18n("Co&py"), topWidget);
	connect(mCopyButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotCopy()));
	TQWhatsThis::add(mCopyButton,
	      i18n("Create a new alarm template based on a copy of the currently highlighted template"));
	layout->addWidget(mCopyButton);

	mDeleteButton = new TQPushButton(i18n("&Delete"), topWidget);
	connect(mDeleteButton, TQT_SIGNAL(clicked()), TQT_SLOT(slotDelete()));
	TQWhatsThis::add(mDeleteButton, i18n("Delete the currently highlighted alarm template"));
	layout->addWidget(mDeleteButton);

	KAccel* accel = new KAccel(this);
	accel->insert(KStdAccel::SelectAll, mTemplateList, TQT_SLOT(slotSelectAll()));
	accel->insert(KStdAccel::Deselect, mTemplateList, TQT_SLOT(slotDeselect()));
	accel->readSettings();

	mTemplateList->refresh();
	slotSelectionChanged();          // enable/disable buttons as appropriate

	TQSize s;
	if (KAlarm::readConfigWindowSize(TMPL_DIALOG_NAME, s))
		resize(s);
}

/******************************************************************************
*  Destructor.
*/
TemplateDlg::~TemplateDlg()
{
	mInstance = 0;
}

/******************************************************************************
*  Create an instance, if none already exists.
*/
TemplateDlg* TemplateDlg::create(TQWidget* parent, const char* name)
{
	if (mInstance)
		return 0;
	mInstance = new TemplateDlg(parent, name);
	return mInstance;
}

/******************************************************************************
*  Called when the New Template button is clicked to create a new template
*  based on the currently selected alarm.
*/
void TemplateDlg::slotNew()
{
	createTemplate(0, this, mTemplateList);
}

/******************************************************************************
*  Called when the Copy button is clicked to edit a copy of an existing alarm,
*  to add to the list.
*/
void TemplateDlg::slotCopy()
{
	TemplateListViewItem* item = mTemplateList->selectedItem();
	if (item)
	{
		KAEvent event = item->event();
		createTemplate(&event, mTemplateList);
	}
}

/******************************************************************************
*  Create a new template.
*  If 'event' is non-zero, base the new template on an existing event or template.
*/
void TemplateDlg::createTemplate(const KAEvent* event, TQWidget* parent, TemplateListView* view)
{
	EditAlarmDlg editDlg(true, i18n("New Alarm Template"), parent, 0, event);
	if (editDlg.exec() == TQDialog::Accepted)
	{
		KAEvent event;
		editDlg.getEvent(event);

		// Add the template to the displayed lists and to the calendar file
		KAlarm::addTemplate(event, view, &editDlg);
		Undo::saveAdd(event);
	}
}

/******************************************************************************
*  Called when the Modify button is clicked to edit the currently highlighted
*  alarm in the list.
*/
void TemplateDlg::slotEdit()
{
	TemplateListViewItem* item = mTemplateList->selectedItem();
	if (item)
	{
		KAEvent event = item->event();
		EditAlarmDlg editDlg(true, i18n("Edit Alarm Template"), this, 0, &event);
		if (editDlg.exec() == TQDialog::Accepted)
		{
			KAEvent newEvent;
			editDlg.getEvent(newEvent);
			TQString id = event.id();
			newEvent.setEventID(id);

			// Update the event in the displays and in the calendar file
			KAlarm::updateTemplate(newEvent, mTemplateList, &editDlg);
			Undo::saveEdit(event, newEvent);
		}
	}
}

/******************************************************************************
*  Called when the Delete button is clicked to delete the currently highlighted
*  alarms in the list.
*/
void TemplateDlg::slotDelete()
{
	TQValueList<EventListViewItemBase*> items = mTemplateList->selectedItems();
	int n = items.count();
	if (KMessageBox::warningContinueCancel(this, i18n("Do you really want to delete the selected alarm template?",
	                                                  "Do you really want to delete the %n selected alarm templates?", n),
	                                       i18n("Delete Alarm Template", "Delete Alarm Templates", n), KGuiItem(i18n("&Delete"), "editdelete"))
		    != KMessageBox::Continue)
		return;

	int warnErr = 0;
	KAlarm::UpdateStatus status = KAlarm::UPDATE_OK;
	TQValueList<KAEvent> events;
	AlarmCalendar::templateCalendar()->startUpdate();    // prevent multiple saves of the calendar until we're finished
	for (TQValueList<EventListViewItemBase*>::Iterator it = items.begin();  it != items.end();  ++it)
	{
		TemplateListViewItem* item = (TemplateListViewItem*)(*it);
		events.append(item->event());
		KAlarm::UpdateStatus st = KAlarm::deleteTemplate(item->event());
		if (st != KAlarm::UPDATE_OK)
		{
			status = st;
			++warnErr;
		}
	}
	if (!AlarmCalendar::templateCalendar()->endUpdate())    // save the calendar now
	{
		status = KAlarm::SAVE_FAILED;
		warnErr = items.count();
	}
	Undo::saveDeletes(events);
	if (warnErr)
		displayUpdateError(this, status, KAlarm::ERR_TEMPLATE, warnErr);
}

/******************************************************************************
* Called when the group of items selected changes.
* Enable/disable the buttons depending on whether/how many templates are
* currently highlighted.
*/
void TemplateDlg::slotSelectionChanged()
{
	int count = mTemplateList->selectedCount();
	mEditButton->setEnabled(count == 1);
	mCopyButton->setEnabled(count == 1);
	mDeleteButton->setEnabled(count);
}

/******************************************************************************
*  Called when the dialog's size has changed.
*  Records the new size in the config file.
*/
void TemplateDlg::resizeEvent(TQResizeEvent* re)
{
	if (isVisible())
		KAlarm::writeConfigWindowSize(TMPL_DIALOG_NAME, re->size());
	KDialog::resizeEvent(re);
}
