#include <qdir.h>
#include <qlabel.h>
#include <qpushbt.h>
#include <qdialog.h>
#include <kapp.h>
#include <klocale.h>
#include <kfiledialog.h>
#include "vcal-setup.moc"

VCalSetup::VCalSetup()
  : QDialog(0L)
{
  resize(390, 170);
  setCaption(i18n("VCalConduit v3.5 Setup"));
  setupWidget();
}

VCalSetup::~VCalSetup()
{
}

void
VCalSetup::slotCommitChanges()
{
  KConfig* config = kapp->getConfig();
  config->setGroup("VCal Conduit");
  config->writeEntry("CalFile", fCalendarFile->text());
  if (fPromptYesNo->isChecked())
    config->writeEntry("FirstTime", "true");
  else
    config->writeEntry("FirstTime", "false");
  config->sync();
  close();
}

void
VCalSetup::slotCancelChanges()
{
  close();
}

void
VCalSetup::slotBrowse()
{
  QString fileName = KFileDialog::getOpenFileName(0L, "*.vcs");
  if(fileName.isNull())
    return;
  fCalendarFile->setText(fileName);
}

void
VCalSetup::setupWidget()
{
  QLabel* currentLabel;
  KConfig* config = kapp->getConfig();
  QButton* aButton;

  config->setGroup("VCal Conduit");
  currentLabel = new QLabel(i18n("VCal-Conduit v3.1 by: D. Pilone, P. Brown & H.J. Steehouwer"), 
			    this);
  currentLabel->adjustSize();
  currentLabel->move(10, 10);

  currentLabel = new QLabel(i18n("Calendar File:"),
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
  
  fPromptYesNo = new QCheckBox(i18n("&Prompt before changing data."), this);
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
