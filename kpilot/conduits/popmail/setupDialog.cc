#include <sys/types.h>
#include <sys/stat.h>
#include <kconfig.h>
#include <kapp.h>
#include "setupDialog.moc"

PopMailOptions::PopMailOptions()
  : QTabDialog(0L, "popmailOptions")
{
  resize(400, 550);
  setCancelButton();
  setCaption(klocale->translate("PopMail Conduit v1.0"));
  setupWidget();
  connect(this, SIGNAL(applyButtonPressed()), this, SLOT(commitChanges()));
  connect(this, SIGNAL(cancelButtonPressed()), this, SLOT(cancelChanges()));
}

PopMailOptions::~PopMailOptions()
{
}
  
void
PopMailOptions::commitChanges()
{
  KConfig* config = kapp->getConfig();
  config->setGroup("Email Conduit");
  config->writeEntry("EmailAddress", fEmailFrom->text());
  config->writeEntry("Signature", fSignature->text());
  config->writeEntry("SendmailCmd", fSendmailCmd->text());
  config->writeEntry("SMTPServer", fSMTPServer->text());
  config->writeEntry("SMTPPort", fSMTPPort->text());
  config->writeEntry("UseSMTP", (int)fUseSMTP->isChecked());
  config->writeEntry("PopServer", fPopServer->text());
  config->writeEntry("PopPort", atoi(fPopPort->text()));
  config->writeEntry("PopUser", fPopUser->text());
  config->writeEntry("LeaveMail", (int)fLeaveMail->isChecked());
  config->writeEntry("SyncIncoming", (int)fSyncIncoming->isChecked());
  config->writeEntry("SendOutgoing", (int)fSendOutgoing->isChecked());
  config->writeEntry("StorePass", (int)fStorePass->isChecked());
  config->writeEntry("PopPass", fPopPass->text());
  config->sync(); // So that the daemon can get at the changes.
  if(fStorePass->isChecked()) // Make sure permissions are safe (still not a good idea)
    chmod(kapp->localconfigdir() + "/popmail-conduitrc", 0600);
  kapp->quit(); // So that the conduit exits.
  close();
}

void
PopMailOptions::togglePopPass()
{
  if(fStorePass->isChecked())
    fPopPass->setEnabled(true);
  else
    fPopPass->setEnabled(false);
}


void PopMailOptions::toggleUseSMTP()
{
  if(fUseSMTP->isChecked()) {
    fSendmailCmd->setEnabled(false);
    fSMTPServer->setEnabled(true);
    fSMTPPort->setEnabled(true);
  } else {
    fSendmailCmd->setEnabled(true);
    fSMTPServer->setEnabled(false);
    fSMTPPort->setEnabled(false);
  }
}

void 
PopMailOptions::cancelChanges()
{
    kapp->quit(); // So the conduit exits
    close();
}

void
PopMailOptions::setupWidget()
{
  KConfig* config = kapp->getConfig();
  QWidget* currentWidget;
  QLabel* currentLabel;

  currentWidget = new QWidget(this);
  config->setGroup("Email Conduit");
  
  currentLabel = new QLabel(klocale->translate("Email Address: "), 
			    currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 14);
  fEmailFrom = new QLineEdit(currentWidget);
  fEmailFrom->setText(config->readEntry("EmailAddress", "$USER"));
  fEmailFrom->resize(200, fEmailFrom->height());
  fEmailFrom->move(110,10);
  
  currentLabel = new QLabel(klocale->translate("Signature File: "), 
			    currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 54);
  fSignature = new QLineEdit(currentWidget);
  fSignature->setText(config->readEntry("Signature", ""));
  fSignature->resize(200, fSignature->height());
  fSignature->move(110, 50);
  
  currentLabel = new QLabel(klocale->translate("Sendmail Cmd:"), currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 94);
  fSendmailCmd = new QLineEdit(currentWidget);
  fSendmailCmd->setText(config->readEntry("SendmailCmd", "/usr/lib/sendmail -t -i"));
  fSendmailCmd->resize(200, fSignature->height());
  fSendmailCmd->move(110, 90);
  
  currentLabel = new QLabel(klocale->translate("SMTP Server:"), currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 134);
  fSMTPServer = new QLineEdit(currentWidget);
  fSMTPServer->setText(config->readEntry("SMTPServer", "mail"));
  fSMTPServer->resize(200, fSignature->height());
  fSMTPServer->move(110, 130);

  currentLabel = new QLabel(klocale->translate("SMTP Port:"), currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 174);
  fSMTPPort = new QLineEdit(currentWidget);
  fSMTPPort->setText(config->readEntry("SMTPPort", "25"));
  fSMTPPort->resize(200, fSignature->height());
  fSMTPPort->move(110, 170);
  
  currentLabel = new QLabel(klocale->translate("POP Server:"), currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 224);
  fPopServer = new QLineEdit(currentWidget);
  fPopServer->setText(config->readEntry("PopServer", "pop"));
  fPopServer->resize(200, fPopServer->height());
  fPopServer->move(110, 220);
  
  currentLabel = new QLabel(klocale->translate("POP Port:"), currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 264);
  fPopPort = new QLineEdit(currentWidget);
  fPopPort->setText(config->readEntry("PopPort", "110"));
  fPopPort->resize(200, fPopPort->height());
  fPopPort->move(110, 260);
  
  currentLabel = new QLabel(klocale->translate("POP Username:"), currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 304);
  fPopUser = new QLineEdit(currentWidget);
  fPopUser->setText(config->readEntry("PopUser", "$USER"));
  fPopUser->resize(200, fPopUser->height());
  fPopUser->move(110, 300);
  
  currentLabel = new QLabel(klocale->translate("Pop Password:"), currentWidget);
  currentLabel->adjustSize();
  currentLabel->move(10, 344);
  fPopPass = new QLineEdit(currentWidget);
  fPopPass->setEchoMode(QLineEdit::Password);
  fPopPass->setText(config->readEntry("PopPass", ""));
  fPopPass->resize(200, fPopPass->height());
  fPopPass->move(110, 340);
  fPopPass->setEnabled(config->readNumEntry("StorePass", 0));

  fLeaveMail = new QCheckBox(klocale->translate("&Leave mail on server."), currentWidget);
  fLeaveMail->adjustSize();
  fLeaveMail->setChecked(config->readNumEntry("LeaveMail", 1));
  fLeaveMail->move(10, 380);
  
  fSyncIncoming = new QCheckBox(klocale->translate("Sync &Incoming mail."), currentWidget);
  fSyncIncoming->adjustSize();
  fSyncIncoming->setChecked(config->readNumEntry("SyncIncoming", 1));
  fSyncIncoming->move(200, 380);
  
  fSendOutgoing = new QCheckBox(klocale->translate("Send &Outgoing mail."), currentWidget);
  fSendOutgoing->adjustSize();
  fSendOutgoing->setChecked(config->readNumEntry("SendOutgoing", 1));
  fSendOutgoing->move(200, 410);
  
  fStorePass = new QCheckBox(klocale->translate("Save &Pop password."), currentWidget);
  connect(fStorePass, SIGNAL(clicked()), this, SLOT(togglePopPass()));
  fStorePass->adjustSize();
  fStorePass->setChecked(config->readNumEntry("StorePass", 0));
  fStorePass->move(10, 410);

  fUseSMTP = new QCheckBox(klocale->translate("Use a &SMTP Server."), currentWidget);
  connect(fUseSMTP, SIGNAL(clicked()), this, SLOT(toggleUseSMTP()));
  fUseSMTP->adjustSize();
  fUseSMTP->setChecked(config->readNumEntry("UseSMTP", 0));
  fUseSMTP->move(10, 440);
  toggleUseSMTP(); //to set everything

  addTab(currentWidget, klocale->translate("&Email Settings"));
}
