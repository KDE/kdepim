#include <sys/types.h>
#include <sys/stat.h>
#include <kconfig.h>
#include <kapp.h>
#include "setupDialog.moc"


const char *NullOptions::groupName() const
{
	return "NULL Conduit";
}

NullOptions::PopMailOptions()
	: QTabDialog(0L, "Null-conduit Options")
{
	textField=textFieldLabel=generalLabel=NULL;

	resize(400, 550);
	setCancelButton();
	setCaption(klocale->translate("Null-conduit v1.0"));
	setupWidget();
	connect(this, SIGNAL(applyButtonPressed()), 
		this, SLOT(commitChanges()));
	connect(this, SIGNAL(cancelButtonPressed()), 
		this, SLOT(cancelChanges()));
}

NullOptions::~PopMailOptions()
{
	SAFEDELETE(textField);
	SAFEDELETE(textFieldLabel);
	SAFEDELETE(generalLabel);
}
  
void
NullOptions::commitChanges()
{
	KConfig* config = kapp->getConfig();
	config->setGroup(groupName());
	config->writeEntry("Text", textField->text());
	kapp->quit(); // So that the conduit exits.
	close();
}

void 
NullOptions::cancelChanges()
{
	kapp->quit(); // So the conduit exits
	close();
}

void
NullOptions::setupWidget()
{
	KConfig* config = kapp->getConfig();
	QWidget* currentWidget;

	currentWidget = new QWidget(this);
	config->setGroup(groupName());

	generalLabel=new QLabel(klocale->translate(
		"The NULL conduit doesn't actually do anything."));
	generalLabel->adjustSize();
	generalLabel->move(10,14);

	textFieldLabel=new QLabel(klocale->translage("Log message:"));
	textFieldLabel->adjustSize();
	textFieldLabel->move(10,BELOW(generalLabel));

	textField=new QLineEdit(currentWidget);
	textField->setText(config->readEntry("Text","NULL conduit was here!"));
	textField->resize(200,textField->height());
	textField->move(RIGHT(textFieldLabel),BELOW(generalLabel));

	addTab(currentWidget, klocale->translate("&Email Settings"));
}
