// $Id$

#include <qlabel.h>
#include <qlistview.h>
#include <qfile.h>
#include <qtextstream.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kabapi.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "atcommand.h"
#include "commandscheduler.h"

#include "mobilegui.h"
#include "mobilegui.moc"

/* 
 *  Constructs a MobileGui which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MobileGui::MobileGui(CommandScheduler *scheduler,QWidget* parent,
                     const char* name,bool modal,WFlags fl)
    : MobileGui_base( parent, name, modal, fl ),
      mScheduler(scheduler)
{
  connect(mScheduler,SIGNAL(commandProcessed(ATCommand *)),
          SLOT(processResult(ATCommand *)));
}

MobileGui::~MobileGui()
{
}

void MobileGui::readModelInformation()
{
  emit sendCommand("+cgmi");
  emit sendCommand("+cgmm");
  emit sendCommand("+cgmr");
  emit sendCommand("+cgsn");
}

void MobileGui::readPhonebook()
{
  emit sendCommand("+cpbr=1,3");
}

void MobileGui::writePhonebook()
{
  kdDebug() << "MobileGui::writePhonebook" << endl;

  KMessageBox::information(this,i18n("Not yet implemented."),
                           i18n("Write to Phone"));
}

void MobileGui::readKab()
{
  kdDebug() << "MobileGui::readKab()" << endl;

  mPhonebookView->clear();

  KabAPI kab(0);
  if (kab.init() != AddressBook::NoError) {
    kdDebug() << "Error initing kab" << endl;
    return;
  }

  std::list<AddressBook::Entry> entries;
  if (kab.getEntries(entries) != AddressBook::NoError) {
    kdDebug() << "Error reading kab entries" << endl;
    return;
  }

  std::list<AddressBook::Entry>::const_iterator it = entries.begin();
  while(it != entries.end()) {
    kdDebug() << "Entry: " << (*it).firstname << " " << (*it).lastname << endl;
    QStringList phones = (*it).telephone;
    for(QStringList::ConstIterator it2 = phones.begin(); it2 != phones.end(); ++it2) {
      kdDebug() << "  " << (*it2) << endl;
    }
    
    QString index,type,name,phonetype,phone;
    
    QStringList::ConstIterator cIt = (*it).custom.begin();
    while(cIt != (*it).custom.end()) {
      if ((*cIt).startsWith("X-KANDY-Index:"))
        index = (*cIt).mid((*cIt).find(":")+1);
      else if ((*cIt).startsWith("X-KANDY-Type:"))
        type = (*cIt).mid((*cIt).find(":")+1);
      else if ((*cIt).startsWith("X-KANDY-Name:"))
        name = (*cIt).mid((*cIt).find(":")+1);
      else if ((*cIt).startsWith("X-KANDY-Phonetype:"))
        phonetype = (*cIt).mid((*cIt).find(":")+1);

      ++cIt;
    }

    if (!index.isEmpty()) {
      for(QStringList::ConstIterator pIt = (*it).telephone.begin();
          pIt != (*it).telephone.end(); ++pIt) {
        if ((*pIt) == phonetype) {
          ++pIt;
          phone = (*pIt);
        } else {
          ++pIt;
        }
      }
      new QListViewItem(mPhonebookView,index,phone,type,name);
    }

    ++it;
  }
}

void MobileGui::writeKab()
{
  kdDebug() << "MobileGui::writeKab()" << endl;

  KabAPI kab(0);
  if (kab.init() != AddressBook::NoError) {
    kdDebug() << "Error initing kab" << endl;
    return;
  }

  QListViewItem *item = mPhonebookView->firstChild();
  while (item) {
    AddressBook::Entry entry;
    
    QString name = item->text(3);
    QString phonenumber = item->text(1);
    QString index = item->text(0);
    QString type = item->text(2);

    QString indexEntry = "X-KANDY-Index:" + index;
    
    entry.fn = name;

    // Try to identify type of phonenumber and write it to the corresponding
    // telephone entry    
    if (phonenumber.left(3) == "017" || phonenumber.left(6) == "+49017") {
      entry.telephone << QString::number(1) << phonenumber;
      entry.custom << "X-KANDY-Phonetype:1";
    } else {
      entry.telephone << QString::number(0) << phonenumber;
      entry.custom << "X-KANDY-Phonetype:0";
    }
    entry.custom << "X-KANDY-Name:" + name;
    entry.custom << "X-KANDY-Type:" + type;
    entry.custom << indexEntry;

    int found = -1;
    KabKey key;

    AddressBook::Entry oldEntry;

    // Search for entry that has same X-KANDY-Index
    int num = kab.addressbook()->noOfEntries();
    for (int i = 0; i < num; ++i) {
      if (AddressBook::NoError != kab.addressbook()->getKey(i,key)) {
        kdDebug() << "Error getting key from kab." << endl;
        break;
      }
      if (AddressBook::NoError != kab.addressbook()->getEntry(key,oldEntry))
      {
        kdDebug() << "Error getting entry from kab." << endl;
        break;
      }
      found = oldEntry.custom.findIndex(indexEntry);
      if (found > 0) {
        break;
      }
    }  

    if (found > 0) {
      AddressBook::ErrorCode error = kab.addressbook()->change(key,entry);
      if (error != AddressBook::NoError) {
        kdDebug() << "kab.change returned with error " << error << endl;
      } else {
        kdDebug() << "Changed " << name << endl;
      }
    } else {
      AddressBook::ErrorCode error = kab.add(entry,key);
      if (error != AddressBook::NoError) {
        kdDebug() << "kab.add returned with error " << error << endl;
      } else {
        kdDebug() << "Added " << name << endl;
      }
    }
    
    item = item->nextSibling();
  }
  
  kab.save(true);
}

void MobileGui::refreshStatus()
{
  // This is a hack. Execution of a command identified by the id should be the
  // job of the CommandScheduler.
  emit sendCommand("+cbc");
  emit sendCommand("+csq");
}

void MobileGui::processResult(ATCommand *command)
{
  if (command->id() == "+cbc") {
    mBatteryChargeLabel->setText(command->resultField(1) + " %");
  } else if (command->id() == "+csq") {
    mSignalQualityLabel->setText(command->resultField(0));
  } else if (command->id() == "+cgmi") {
    mManufacturerLabel->setText(command->resultField(0));
  } else if (command->id() == "+cgmm") {
    mModelLabel->setText(command->resultField(0));
  } else if (command->id() == "+cgmr") {
    mGSMVersionLabel->setText(command->resultField(0));
  } else if (command->id() == "+cgsn") {
    mSerialNumberLabel->setText(command->resultField(0));
  } else if (command->id() == "+cpbr=1,3") {
    fillPhonebook(command);
  }
}

void MobileGui::fillPhonebook(ATCommand *cmd)
{
  kdDebug() << "MobileGui::fillPhonebook()" << endl;
  
//  kdDebug() << "--- " << cmd->resultString() << endl;
  mPhonebookView->clear();
    
  QList<QStringList> *list = cmd->resultFields();
  
  QStringList *fields = list->first();
  while(fields) {
    QListViewItem *item = new QListViewItem(mPhonebookView);
  
    int i=0;
    QStringList::Iterator it;
    for(it = fields->begin();it != fields->end(); ++it) {
//      kdDebug() << "---+++: " << *it << endl;
      item->setText(i++,dequote(*it));
    }
    
    fields = list->next();
  }
  
  emit phonebookRead();
}

QString MobileGui::dequote(const QString &str)
{
  int pos = 0;
  int len = str.length();

  if (str.left(1) == "\"") {
    ++pos;
    --len;
  } 
  if (str.right(1) == "\"") {
    --len;
  }
  
  return str.mid(pos,len);
}

void MobileGui::savePhonebook()
{
  QString fileName = KFileDialog::getSaveFileName("phonebook.csv");

  QFile outFile(fileName);
  if ( outFile.open(IO_WriteOnly) ) {    // file opened successfully
    QTextStream t( &outFile );        // use a text stream

    QListViewItem *item = mPhonebookView->firstChild();
    while (item) {
      t << item->text(0) << "," << item->text(1) << "," << item->text(2) << ","
        << item->text(3) << endl;
      item = item->nextSibling();
    }

    outFile.close();
  }
}
