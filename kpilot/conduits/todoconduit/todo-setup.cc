#include <qdir.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qdialog.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "todo-setup.moc"

TodoSetup::TodoSetup()
  : QDialog(0L)
{
  resize(390, 170);
  setCaption(klocale->translate("TodoConduit v1.0 Setup"));
  setupWidget();
}

TodoSetup::~TodoSetup()
{
}

void
TodoSetup::slotCommitChanges()
{
  KConfig* config = kapp->getConfig();
  config->setGroup("Todo Conduit");
  config->writeEntry("CalFile", fCalendarFile->text());
  if (fPromptYesNo->isChecked())
    config->writeEntry("FirstTime", "true");
  else
    config->writeEntry("FirstTime", "false");
  config->sync();
  close();
}

void
TodoSetup::slotCancelChanges()
{
  close();
}

void
TodoSetup::slotBrowse()
{
  QString fileName = KFileDialog::getOpenFileName(0L, "*.vcs");
  if(fileName.isNull())
    return;
  fCalendarFile->setText(fileName);
}

void
TodoSetup::setupWidget()
{
  QLabel* currentLabel;
  KConfig* config = kapp->getConfig();
  QPushButton* aButton;

  config->setGroup("Todo Conduit");
  currentLabel = new QLabel(klocale->translate("Todo-Conduit v1.0 by Preston Brown"), 
			    this);
  currentLabel->adjustSize();
  currentLabel->move(10, 10);

  currentLabel = new QLabel(klocale->translate("Calendar File:"),
			    this);
  currentLabel->adjustSize();
  currentLabel->move(10, 56);
  
  fCalendarFile = new QLineEdit(this);
  fCalendarFile->setText(config->readEntry("CalFile", ""));
  fCalendarFile->resize(200, fCalendarFile->height());
  fCalendarFile->move(120, 50);

  aButton = new QPushButton("...", this);
  aButton->adjustSize();
  aButton->move(325, 52);
  connect(aButton, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  
  fPromptYesNo = new QCheckBox(klocale->translate("&Prompt before changing data."), this);
  fPromptYesNo->adjustSize();
  fPromptYesNo->setChecked(config->readBoolEntry("FirstTime", TRUE));
  fPromptYesNo->move(80, 90);

  aButton = new QPushButton("OK", this);
  aButton->resize(60,aButton->height());
  aButton->move(80, 125);
  connect(aButton, SIGNAL(clicked()), this, SLOT(slotCommitChanges()));

  aButton = new QPushButton("Cancel", this);
  aButton->resize(60,aButton->height());
  aButton->move(220, 125);
  connect(aButton, SIGNAL(clicked()), this, SLOT(slotCancelChanges()));
}
