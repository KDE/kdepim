#include "options.h"
#include <sys/time.h>
#include <stream.h>
#include <pi-appinfo.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <kdebug.h>
#include "pilotComponent.moc"
#include "kpilot.h"

inline
PilotComponent::PilotComponent(QWidget* parent)
  : QWidget(parent)
    {
    }



int PilotComponent::findSelectedCategory(QComboBox *fCatList,
	struct CategoryAppInfo *info,
	bool AllIsUnfiled)
{
	FUNCTIONSETUP;

	// Semantics of currentCatID are: 
	//
	// >=0		is a specific category based on the text -> 
	//		category number mapping defined by the Pilot, 
	// ==-1 	means "All" category selected when 
	//		AllIsUnfiled is true.
	// == 0		== Unfiled means "All" category selected when 
	//		AllIsUnfiled is false.
	//
	//
	int currentCatID = 0;

	// If a category is deleted after others have been added, none of the
	// category numbers are changed.  So we need to find the category number
	// for this category (this category is represented by the selected
	// *text*).
	//
	//
	// The top entry in the list is "All", so if the top item is
	// selected we can indicate that we are using the "All" category.
	//
	//
	if (fCatList->currentItem()==0)
	{
		currentCatID=-1;
		if (debug_level & UI_MINOR)
		{
			kdDebug() << fname <<
				": Category 'All' selected.\n" ;
		}
	}
	else
	{
		QString selectedCategory=fCatList->text(fCatList->currentItem());
		if (debug_level & UI_MINOR)
		{
			kdDebug() << fname << 
				": List item " << fCatList->currentItem() <<
				" selected, text=" <<
				selectedCategory << endl ;
		}

		currentCatID=0;
		while(strcmp(info->name[currentCatID], 
		       selectedCategory.local8Bit()) && 
			(currentCatID < fCatList->count()))
		{
			if (debug_level & UI_TEDIOUS)
			{
				kdDebug() << fname <<
					": Didn't match category " <<
					currentCatID << '=' <<
					info->name[currentCatID]
					<< endl ;
			}

			currentCatID++;
		}

		if (currentCatID < fCatList->count())
		{
			if (debug_level&UI_MINOR)
			{
				kdDebug() << fname << 
					": Matched category " <<
					currentCatID << '=' <<
					info->name[currentCatID]
					<< endl ;
			}
		}
		else
		{
			kdDebug() << fname 
				<< ": Selected category didn't match "
				"any name!\n" ;
			currentCatID=-1;
		}
	}

	if ((currentCatID==-1) && AllIsUnfiled) currentCatID=0;
	return currentCatID;
}


void PilotComponent::populateCategories(QComboBox *c,
	struct CategoryAppInfo *info)
{
	FUNCTIONSETUP;
	int i;

	c->clear();

	if (!info) goto CategoryAll;

	// Fill up the categories list box with
	// the categories defined by the user. 
	// These presumably are in the language 
	// the user uses, so no translation is necessary.
	//
	//
	for(i = 0; i < 15; i++)
	{
		if(strlen(info->name[i]))
		{
			if (debug_level & UI_MINOR)
			{
				kdDebug() << fname
				<< ": Adding category: "
				<< info->name[i]
				<< " with ID: " 
				<< (int)info->ID[i] 
				<< endl;
			}
			c->insertItem(info->name[i]);
		}
	}

CategoryAll:
	c->insertItem(i18n("All"),0);
}
