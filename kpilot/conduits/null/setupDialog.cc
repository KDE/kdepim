// setupDialog.cc
//
// Copyright (C) 2000 Adriaan de Groot
//
// This file is distributed under the Gnu General Public Licence (GPL).
// The GPL should have been included with this file in a file called
// COPYING. 


// This is the setup dialog for null-conduit.
// Because null-conduit does nothing, the
// setup is fairly simple.
//
//


// KDE standard includes
//
//
#include <kconfig.h>
#include <kapp.h>
#include <stream.h>
// KPilot standard includes
//
//
#include "kpilot.h"
// null-conduit specific includes
//
//
#include "setupDialog.moc"

#define max(a,b) ((a)>(b) ? (a) : (b))

// groupName returns the group that
// our configuration options go in.
//
//
const char *NullOptions::groupName() 
{
	return "NULL Conduit";
}

NullOptions::NullOptions()
	: QTabDialog(0L, "Null-conduit Options"),
	textFieldLabel(0L),
	textField(0L),
	generalLabel(0L)
{
	FUNCTIONSETUP;

	setCancelButton();
	setCaption(klocale->translate("Null-conduit v1.0"));
	setupWidget();
	connect(this, SIGNAL(applyButtonPressed()), 
		this, SLOT(commitChanges()));
	connect(this, SIGNAL(cancelButtonPressed()), 
		this, SLOT(cancelChanges()));
}

NullOptions::~NullOptions()
{
	FUNCTIONSETUP;
}
  
void
NullOptions::commitChanges()
{
	FUNCTIONSETUP;

	KConfig* config = kapp->getConfig();
	config->setGroup(groupName());

	if (debug_level)
	{
		cerr << fname << ": Wrote null-conduit message:\n" <<
			fname << ": " << textField->text() << endl;
	}
	config->writeEntry("Text", textField->text());
	kapp->quit(); // So that the conduit exits.
	close();
}

void 
NullOptions::cancelChanges()
{
	FUNCTIONSETUP;

	kapp->quit(); // So the conduit exits
	close();
}

void
NullOptions::setupWidget()
{
	FUNCTIONSETUP;

	KConfig* config = kapp->getConfig();
	int maxwidth=0;

	generalTab = new QWidget(this);
	config->setGroup(groupName());

	generalLabel=new QLabel(klocale->translate(
		"The NULL conduit doesn't actually do anything."),
		generalTab);
	generalLabel->adjustSize();
	generalLabel->move(10,14);
	maxwidth=max(maxwidth,generalLabel->x()+generalLabel->width()+SPACING);

	textFieldLabel=new QLabel(klocale->translate("Log message:"),
		generalTab);
	textFieldLabel->adjustSize();
	textFieldLabel->move(10,BELOW(generalLabel));

	textField=new QLineEdit(generalTab);
	textField->setText(config->readEntry("Text","NULL conduit was here!"));
	textField->resize(200,textField->height());
	textField->move(RIGHT(textFieldLabel),BELOW(generalLabel));
	maxwidth=max(maxwidth,textField->x()+textField->width()+SPACING);

	generalTab->resize(maxwidth,
		textField->y()+textField->height()+SPACING);
	resize(maxwidth+SPACING,
		generalTab->height()+8*SPACING);
	addTab(generalTab, klocale->translate("&Null Settings"));
}


// $Log$
