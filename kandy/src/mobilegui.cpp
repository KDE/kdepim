// $Id$

#include <qlabel.h>
#include <qlistview.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qmessagebox.h>

#include <kdebug.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kapplication.h>

#include <kabc/stdaddressbook.h>

#include "atcommand.h"
#include "commandscheduler.h"

#include "mobilegui.h"
#include "mobilegui.moc"

class SyncEntry {
  public:
    SyncEntry() { mOn = true; }
  
    bool mOn;
};

class SyncEntryKab : public SyncEntry {
  public:
    SyncEntryKab(bool on,const QString &index,const QString &name,
                 const QString &type,const QString &phone,const QString &phonetype)
    {
      mOn = on;
      mIndex = index;
      mName = name;
      mType = type;
      mPhone = phone;
      mPhonetype = phonetype;
    }
  
    QString mIndex;
    QString mName;
    QString mType;
    QString mPhone;
    QString mPhonetype;

    KABC::Addressee mAddressee;
};

class SyncEntryMobile : public SyncEntry {
  public:
    SyncEntryMobile(bool on,const QString &index,const QString &phone,
                    const QString &type,const QString &name)
    {
      mOn = on;
      mIndex = index;
      mName = name;
      mType = type;
      mPhone = phone;
    }
    
    QString mIndex;
    QString mName;
    QString mType;
    QString mPhone;
};

class SyncEntryCommon : public SyncEntry {
  public:
    SyncEntryCommon(bool on,SyncEntryKab *kabEntry,SyncEntryMobile *mobileEntry)
    {
      mOn = on;
      mKabEntry = kabEntry;
      mMobileEntry = mobileEntry;
    }
    
    SyncEntryKab *mKabEntry;
    SyncEntryMobile *mMobileEntry;
};

class AddressSyncer {
  public:
    AddressSyncer()
    {
      mKabEntries.setAutoDelete(true);
      mMobileEntries.setAutoDelete(true);
      mCommonEntries.setAutoDelete(true);
    }
  
    QPtrList<SyncEntryKab> mKabEntries;
    QPtrList<SyncEntryMobile> mMobileEntries;
    QPtrList<SyncEntryCommon> mCommonEntries; 
};


class PhoneBookItem : public QCheckListItem {
  public:
    PhoneBookItem(QListView *v) : QCheckListItem(v,"",QCheckListItem::CheckBox)
    {
      mSyncEntry = 0;
    }
    PhoneBookItem(QListView *v,SyncEntry *syncEntry,const QString &index,
                  const QString &phone,
                  const QString &type, const QString &name) :
      QCheckListItem(v,index,QCheckListItem::CheckBox)
    {
      mSyncEntry = syncEntry;
      
      setText(1,phone);
      setText(2,type);
      setText(3,name);
    }

    void setItem(const QString &index,const QString &phone,
                 const QString &type, const QString &name)
    {
      setText(0,index);
      setText(1,phone);
      setText(2,type);
      setText(3,name);
    }

    void setIndex(int i) { setText(0,QString::number(i)); }
    QString index() { return text(0); }
    QString phone() { return text(1); }
    QString type() { return text(2); }
    QString name() { return text(3); }

    SyncEntry *syncEntry() { return mSyncEntry; }

  private:
    SyncEntry *mSyncEntry;
};


/* 
 *  Constructs a MobileGui which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MobileGui::MobileGui(CommandScheduler *scheduler,QWidget* parent,
                     const char* name,WFlags fl)
    : DCOPObject("KandyIface"),
      MobileGui_base(parent,name,fl)
{
  mSyncing = false;
  mScheduler = scheduler;
  mSyncer = new AddressSyncer;

  connect(mScheduler,SIGNAL(commandProcessed(ATCommand *)),
          SLOT(processResult(ATCommand *)));
}

MobileGui::~MobileGui()
{
  delete mSyncer;
}

void MobileGui::exit()
{
  kapp->quit();
}

void MobileGui::readModelInformation()
{
  mScheduler->executeId("+cgmi");
  mScheduler->executeId("+cgmm");
  mScheduler->executeId("+cgmr");
  mScheduler->executeId("+cgsn");
}

void MobileGui::readPhonebook()
{
  mScheduler->executeId("+cpbr=1,150");
  
  emit statusMessage(i18n("Reading mobile phonebook..."));
}

void MobileGui::writePhonebook()
{
  kdDebug() << "MobileGui::writePhonebook" << endl;

  for(uint i=0;i<mSyncer->mMobileEntries.count();++i) {
    SyncEntryMobile *entry = mSyncer->mMobileEntries.at(i);

//    kdDebug() << "Writing " << entry->mIndex << " " << entry->mName
//              << " " << entry->mPhone << endl;

    QString id = "+cpbw=" + entry->mIndex;
    mLastWriteId = id;
    ATCommand *cmd = new ATCommand(id);
    cmd->setAutoDelete(true);
    cmd->addParameter(new ATParameter(quote(entry->mPhone)));
    cmd->addParameter(new ATParameter(entry->mType));
    cmd->addParameter(new ATParameter(quote(entry->mName)));

    kdDebug() << "  " << cmd->cmd() << endl;
    kdDebug() << "  id: " << cmd->id() << endl;
    
    mScheduler->execute(cmd);
  }

  emit statusMessage(i18n("Writing mobile phonebook..."));  
}

void MobileGui::readKabc()
{
  kdDebug() << "MobileGui::readKabc()" << endl;

  mSyncer->mKabEntries.clear();

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();

  KABC::AddressBook::Iterator it;
  for( it = addressBook->begin(); it != addressBook->end(); ++it ) {
    
    QString index = (*it).custom("KANDY","Index");
    QString type = (*it).custom("KANDY","Type");
    QString name = (*it).custom("KANDY","Name");
    QString phonetype = (*it).custom("KANDY","Phonetype");
    
    // Get phonenumber according to phonetype. If no number of this type was
    // found use the first phone number as default
    KABC::PhoneNumber phoneNumber;
    if ( phonetype.isEmpty() ) {
      KABC::PhoneNumber::List phoneNumbers = (*it).phoneNumbers();
      phoneNumber = phoneNumbers.first();
    } else {
      phoneNumber = (*it).phoneNumber( phonetype.toInt() );
    }
    QString phone = phoneNumber.number();

    SyncEntryKab *kabEntry;
    if (!index.isEmpty()) {
      // This entry was already stored on the phone at some time.
      kabEntry = new SyncEntryKab(true,index,name,type,phone,phonetype);
    } else {
      // This entry has never been on the phone.
      index = "";
      name = (*it).realName();
      if (phone.left(1) == "+") type = "145";
      else type = "129";
      
      kabEntry = new SyncEntryKab(false,index,name,type,phone,phonetype);
    }

    kabEntry->mAddressee = (*it);

    mSyncer->mKabEntries.append(kabEntry);
  }

  // Display kab entries
  updateKabBook();
  
  emit transientStatusMessage(i18n("Read KDE address book."));
}

void MobileGui::writeKabc()
{
  kdDebug() << "MobileGui::writeKabc()" << endl;

  KABC::AddressBook *addressBook = KABC::StdAddressBook::self();

  KABC::Ticket *ticket = addressBook->requestSaveTicket();

  if ( !ticket ) {
    kdDebug() << "Error! No ticket to save." << endl;
    return;
  }

  for(uint i=0;i<mSyncer->mKabEntries.count();++i) {
    SyncEntryKab *kabEntry = mSyncer->mKabEntries.at(i);

    KABC::Addressee entry = kabEntry->mAddressee;
    
    QString name = kabEntry->mName;
    QString phonenumber = kabEntry->mPhone;
    QString index = kabEntry->mIndex;
    QString type = kabEntry->mType;

    entry.insertCustom( "KANDY", "Index", index );
    
    entry.setFormattedName( name );

    // Try to identify type of phonenumber and write it to the corresponding
    // telephone entry    
    int phoneType = 0;
    if (phonenumber.left(3) == "017" || phonenumber.left(6) == "+49017") {
      phoneType = KABC::PhoneNumber::Cell;
    }
    entry.insertPhoneNumber( KABC::PhoneNumber( phonenumber, phoneType ) );
    entry.insertCustom( "KANDY", "Phonetype", QString::number( phoneType ) );
    
    entry.insertCustom( "KANDY", "Name", name );
    entry.insertCustom( "KANDY", "Type", type );

    addressBook->insertAddressee( entry );
  }
  
  addressBook->save( ticket );
  
  emit transientStatusMessage(i18n("Wrote KDE address book"));
}

void MobileGui::refreshStatus()
{
  mScheduler->executeId("+cbc");
  mScheduler->executeId("+csq");
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
  } else if (command->id() == "+cpbr=1,150") {
    fillPhonebook(command);
  } else if (command->id() == mLastWriteId) {
    mLastWriteId = "";
    emit transientStatusMessage(i18n("Wrote mobile phonebook."));
  }
  if (command->id() == mSyncReadId) {
    mSyncReadId = "";
    mergePhonebooks();
    writeKabc();
    writePhonebook();
    mSyncWriteId = mLastWriteId;
  }
  if (command->id() == mSyncWriteId) {
    mSyncWriteId = "";
    emit transientStatusMessage(i18n("Synced phonebooks."));
    mSyncing = false;
  }
}

void MobileGui::fillPhonebook(ATCommand *cmd)
{
  kdDebug() << "MobileGui::fillPhonebook()" << endl;

//  kdDebug() << "--- " << cmd->resultString() << endl;

  mSyncer->mMobileEntries.clear();
    
  QPtrList<QStringList> *list = cmd->resultFields();
  
  QStringList *fields = list->first();
  while(fields) {
    if (fields->count() != 4) {
      kdDebug() << "Error! Unexpected number of address fields." << endl;
    } else {
      QString index = (*fields)[0];
      QString phone = (*fields)[1];
      QString type = (*fields)[2];
      QString name = (*fields)[3];
      SyncEntryMobile *phoneEntry = new SyncEntryMobile(true,dequote(index),
          dequote(phone),dequote(type),dequote(name));
      mSyncer->mMobileEntries.append(phoneEntry);
    }
    fields = list->next();
  }

  // Display mobile entries
  updateMobileBook();

  emit transientStatusMessage(i18n("Read mobile phonebook."));
  
  emit phonebookRead();
}

QString MobileGui::quote(const QString &str)
{
  if (str.left(1) == "\"" && str.right(1) == "\"") return str;
  
  return "\"" + str + "\"";
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

    for(uint i=0;i<mSyncer->mMobileEntries.count();++i) {
      SyncEntryMobile *e = mSyncer->mMobileEntries.at(i);
      t << e->mIndex << "," << e->mPhone << "," << e->mType << ","
        << e->mName << endl;
    }

    outFile.close();
  }
}

void MobileGui::mergePhonebooks()
{
  kdDebug() << "MobileGui::mergePhonebooks()" << endl;

  // Update selection state from GUI.
  PhoneBookItem *item = (PhoneBookItem *)mKabBook->firstChild();
  while(item) {
    item->syncEntry()->mOn = item->isOn();
    item = (PhoneBookItem *)item->nextSibling();
  }
  item = (PhoneBookItem *)mMobileBook->firstChild();
  while(item) {
    item->syncEntry()->mOn = item->isOn();
    item = (PhoneBookItem *)item->nextSibling();
  }

  mSyncer->mCommonEntries.clear();

//  kdDebug() << " Insert kab list" << endl;

  // Put Kab list into common list
  for(uint i=0;i<mSyncer->mKabEntries.count();++i) {
    if (mSyncer->mKabEntries.at(i)->mOn) {
      mSyncer->mCommonEntries.append(new SyncEntryCommon(true,mSyncer->mKabEntries.at(i),0));
    }
  }

//  kdDebug() << " Insert mobile list" << endl;

  // Put mobile list into common list. Merge equivalent entries.
  for(uint i=0;i<mSyncer->mMobileEntries.count();++i) {
    SyncEntryMobile *mobileEntry = mSyncer->mMobileEntries.at(i);
//    kdDebug() << "--- Inserting " << mobileEntry->mName << endl;
  
    uint j=0;
    for(;j<mSyncer->mCommonEntries.count();++j) {
      if (mSyncer->mCommonEntries.at(j)->mKabEntry) {
        if (mSyncer->mCommonEntries.at(j)->mKabEntry->mIndex ==
            mobileEntry->mIndex) {
          // Equivalent entry is already there. Merge entries.
          mSyncer->mCommonEntries.at(j)->mMobileEntry = mobileEntry;
          break;
        }
      }
    }
    if (j == mSyncer->mCommonEntries.count()) {
      if (mobileEntry->mOn) {
        // Entry wasn't found
        mSyncer->mCommonEntries.append(new SyncEntryCommon(true,0,mobileEntry));
      }
    }
  }
  
//  kdDebug() << " Resolve conflicts" << endl;

  // Resolve conflicts
  bool kabUpdated = false;
  bool mobileUpdated = false;
  for(uint i=0;i<mSyncer->mCommonEntries.count();++i) {
    SyncEntryCommon *entry = mSyncer->mCommonEntries.at(i);
    SyncEntryKab *kabEntry = entry->mKabEntry;
    SyncEntryMobile *mobileEntry = entry->mMobileEntry;
    if (kabEntry && mobileEntry) {
      if (mobileEntry->mPhone == kabEntry->mPhone &&
          mobileEntry->mName == kabEntry->mName) {
        // Entries are identical. Do nothing.
      } else {
        // Merge mobileEntrys
        // This alters the mobile and kab lists. Perhaps we should reflect this in the GUI.
        QString text = "<qt><b>" + i18n("Kab Entry:") + "</b><br>";
        text += "  " + kabEntry->mName + " " + kabEntry->mPhone + "<br>";
        text += "<b>" + i18n("Mobile Entry:") + "</b><br>";
        text += "  " + mobileEntry->mName + " " + mobileEntry->mPhone;
        text += "</qt>";
      
        QMessageBox *msg = new QMessageBox(i18n("Conflicting Entries"),text,
                                         QMessageBox::Warning,1,2,0,this);
        msg->setButtonText(1,i18n("Use Kab Entry"));
        msg->setButtonText(2,i18n("Use Mobile Entry"));
        switch (msg->exec()) {
          case 1: // use kab entry
            mobileEntry->mPhone = kabEntry->mPhone;
            mobileEntry->mName = kabEntry->mName;
            mobileUpdated = true;
            break;
          case 2: // use mobile entry
            kabEntry->mPhone = mobileEntry->mPhone;
            kabEntry->mName = mobileEntry->mName;
            kabUpdated = true;
            break;
        }
      }
    }
  }

//  kdDebug() << " Create new entries" << endl;

  // Create new entries
  for(uint i=0;i<mSyncer->mCommonEntries.count();++i) {
    SyncEntryCommon *entry = mSyncer->mCommonEntries.at(i);
    SyncEntryKab *kabEntry = entry->mKabEntry;
    SyncEntryMobile *mobileEntry = entry->mMobileEntry;

    if (kabEntry && !mobileEntry) {
      kdDebug() << "Creating mobile entry for " << kabEntry->mPhone << endl;
      // Create mobile entry
      // The type should be generated here.
      // The values should be checked for validity.
      entry->mMobileEntry = new SyncEntryMobile(true,"",kabEntry->mPhone,kabEntry->mType,
                                                kabEntry->mName);
      mSyncer->mMobileEntries.append(entry->mMobileEntry);

      // Create new index
      QString index;
      for(uint j=1;j<150;++j) {
        uint k = 0;
        for(;k<mSyncer->mMobileEntries.count();++k) {
          if (mSyncer->mMobileEntries.at(k)->mIndex == QString::number(j)) {
            break;
          }
        }
        if (k == mSyncer->mMobileEntries.count()) {
          index = QString::number(j);
          break;
        }
      }
      entry->mMobileEntry->mIndex = index;
      
      kabEntry->mIndex = index;

      kabUpdated = true;
      mobileUpdated = true;
    } else if (mobileEntry && !kabEntry) {
      // Create kab entry
      QString phonetype = "0";
      entry->mKabEntry = new SyncEntryKab(true,mobileEntry->mIndex,mobileEntry->mName,
                                          mobileEntry->mType,mobileEntry->mPhone,
                                          phonetype);
      mSyncer->mKabEntries.append(entry->mKabEntry);

      kabUpdated = true;
    }
  }

//  kdDebug() << "Update gui" << endl;

  // Update kab and mobile entries
  if (kabUpdated) updateKabBook();
  if (mobileUpdated) updateMobileBook();

  kdDebug() << "MobileGui::mergePhonebooks() done." << endl;
}

void MobileGui::syncPhonebooks()
{
  if (mSyncing) return;

  mSyncing = true;
  readKabc();
  readPhonebook();
  mSyncReadId = "+cpbr=1,150";
}

void MobileGui::updateKabBook()
{
  mKabBook->clear();
  for(uint i=0;i<mSyncer->mKabEntries.count();++i) {
    SyncEntryKab *kabEntry = mSyncer->mKabEntries.at(i);
    PhoneBookItem *item = new PhoneBookItem(mKabBook,kabEntry,kabEntry->mIndex,
        kabEntry->mPhone,kabEntry->mType,kabEntry->mName);
    item->setOn(kabEntry->mOn);
  }
}

void MobileGui::updateMobileBook()
{
  mMobileBook->clear();
  for(uint i=0;i<mSyncer->mMobileEntries.count();++i) {
    SyncEntryMobile *entry = mSyncer->mMobileEntries.at(i);
    PhoneBookItem *item = new PhoneBookItem(mMobileBook,entry,entry->mIndex,
        entry->mPhone,entry->mType,entry->mName);
    item->setOn(entry->mOn);
  }
}
