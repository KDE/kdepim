/*
    knconvert.cpp

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#include <qlayout.h>
#include <qwidgetstack.h>
#include <qlabel.h>
#include <qcheckbox.h>

#include <klocale.h>
#include <kfiledialog.h>
#include <kseparator.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kprocess.h>
#include <kapplication.h>

#include <kmime_util.h>

#include "knconvert.h"
#include "resource.h"
#include <qpushbutton.h>


bool KNConvert::needToConvert(const QString &oldVersion)
{
  bool ret=(
              (oldVersion.left(3)=="0.3") ||
              (oldVersion.left(3)=="0.4")
           );

  return ret;
}


KNConvert::KNConvert(const QString &version)
  : QDialog(0,0,true), l_ogList(0), c_onversionDone(false), v_ersion(version)
{
  setCaption(kapp->makeStdCaption(i18n("Conversion")));
  QVBoxLayout *topL=new QVBoxLayout(this, 5,5);
  s_tack=new QWidgetStack(this);
  topL->addWidget(s_tack, 1);
  topL->addWidget(new KSeparator(this));

  QHBoxLayout *btnL=new QHBoxLayout(topL, 5);
  s_tartBtn=new QPushButton(i18n("Start Conversion..."), this);
  s_tartBtn->setDefault(true);
  btnL->addStretch(1);
  btnL->addWidget(s_tartBtn);
  c_ancelBtn=new QPushButton(i18n("Cancel"), this);
  btnL->addWidget(c_ancelBtn);

  connect(s_tartBtn, SIGNAL(clicked()), this, SLOT(slotStart()));
  connect(c_ancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

  w_1=new QWidget(s_tack);
  s_tack->addWidget(w_1, 1);
  QGridLayout *w1L=new QGridLayout(w_1, 5,3, 5,5);

  QLabel *l1=new QLabel(i18n(
"<b>Congratulations, you have upgraded to KNode version %1.</b><br>\
Unfortunately this version uses a different format for some data-files, so \
in order to keep your existing data it is necessary to convert it first. This is \
now done automatically by KNode. If you want to, a backup of your existing data \
will be created before the conversion starts.").arg(KNODE_VERSION), w_1);
  w1L->addMultiCellWidget(l1, 0,0, 0,2);

  c_reateBkup=new QCheckBox(i18n("Create backup of old data"), w_1);
  w1L->addMultiCellWidget(c_reateBkup, 2,2, 0,2);
  connect(c_reateBkup, SIGNAL(toggled(bool)), this, SLOT(slotCreateBkupToggled(bool)));

  b_ackupPathLabel=new QLabel(i18n("Save backup in:"), w_1);
  w1L->addWidget(b_ackupPathLabel, 3,0);

  b_ackupPath=new KLineEdit(QDir::homeDirPath()+QString("/knodedata-")+v_ersion+".tar.gz", w_1);
  w1L->addWidget(b_ackupPath, 3,1);

  b_rowseBtn= new QPushButton(i18n("Browse..."), w_1);
  connect(b_rowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  w1L->addWidget(b_rowseBtn, 3,2);
  w1L->setColStretch(1,1);
  w1L->addRowSpacing(1,15);
  w1L->setRowStretch(4,1);
  w1L->addRowSpacing(4,15);

  w_2=new QLabel(s_tack);
  w_2->setText(i18n("<b>Converting, please wait...</b>"));
  w_2->setAlignment(AlignCenter);
  s_tack->addWidget(w_2, 2);

  w_3=new QWidget(s_tack);
  s_tack->addWidget(w_3, 3);
  QVBoxLayout *w3L=new QVBoxLayout(w_3, 5,5);

  r_esultLabel=new QLabel(w_3);
  w3L->addWidget(r_esultLabel);
  QLabel *l2=new QLabel(i18n("Processed tasks:"), w_3);
  l_ogList=new QListBox(w_3);
  w3L->addSpacing(15);
  w3L->addWidget(l2);
  w3L->addWidget(l_ogList, 1);

  s_tack->raiseWidget(w_1);
  slotCreateBkupToggled(false);
}


KNConvert::~KNConvert()
{
}


void KNConvert::convert()
{
  int errors=0;
  for(Converter *c=c_onverters.first(); c; c=c_onverters.next())
    if(!c->doConvert())
      errors++;

  if(errors>0)
    r_esultLabel->setText(i18n(
"<b>Some errors occurred during the conversion.</b>\
<br>You should now examine the log to find out what went wrong."));
  else
    r_esultLabel->setText(i18n(
"<b>The conversion was successful.</b>\
<br>Have a lot of fun with this new version of KNode. ;-)"));

  s_tartBtn->setText(i18n("Start KNode"));
  s_tartBtn->setEnabled(true);
  c_ancelBtn->setEnabled(true);
  l_ogList->insertStringList(l_og);
  s_tack->raiseWidget(w_3);

  c_onversionDone=true;
}


void KNConvert::slotStart()
{
  if(c_onversionDone) {
    accept();
    return;
  }

  s_tartBtn->setEnabled(false);
  c_ancelBtn->setEnabled(false);
  s_tack->raiseWidget(w_2);

  c_onverters.setAutoDelete(true);

  if(v_ersion.left(3)=="0.3" || v_ersion.left(7)=="0.4beta") {
    //Version 0.4
    c_onverters.append(new Converter04(&l_og));
  }

  //create backup of old data using "tar"
  if(c_reateBkup->isChecked()) {
    if(b_ackupPath->text().isEmpty()) {
      KMessageBox::error(this, i18n("Please select a valid backup path."));
      return;
    }

    QString dataDir=locateLocal("data","knode/");
    t_ar=new KProcess;
    *t_ar << "tar";
    *t_ar << "-cz" << dataDir
          << "-f" << b_ackupPath->text();
    connect(t_ar, SIGNAL(processExited(KProcess*)), this, SLOT(slotTarExited(KProcess*)));
    if(!t_ar->start()) {
      delete t_ar;
      t_ar = 0;
      slotTarExited(0);
    }
  }
  else
    convert(); //convert files without backup
}


void KNConvert::slotCreateBkupToggled(bool b)
{
  b_ackupPathLabel->setEnabled(b);
  b_ackupPath->setEnabled(b);
  b_rowseBtn->setEnabled(b);
}


void KNConvert::slotBrowse()
{
  QString newPath=KFileDialog::getSaveFileName(b_ackupPath->text());

  if(!newPath.isEmpty())
    b_ackupPath->setText(newPath);
}


void KNConvert::slotTarExited(KProcess *proc)
{
  bool success=true;

  if(!proc || !proc->normalExit() || proc->exitStatus()!=0) {
    success=false;
    if(KMessageBox::No==KMessageBox::warningYesNo(this, i18n("<b>The backup failed!</b>. Do you want to continue anyway?"))) {

      delete t_ar;
      t_ar = 0;
      reject();
      return;
    }
  }

  delete t_ar;
  t_ar = 0;
  if(success)
    l_og.append(i18n("created backup of the old data-files in %1").arg(b_ackupPath->text()));
  else
    l_og.append(i18n("backup failed."));

  // now we actually convert the files
  convert();
}



//============================================================================================



bool KNConvert::Converter04::doConvert()
{
  QString dir=locateLocal("data","knode/")+"folders/";
  int num;
  bool error=false;

  //Drafts
  if(QFile::exists(dir+"folder1.idx")) {
    num=convertFolder(dir+"folder1", dir+"drafts_1");
    if(num==-1) {
      error=true;
      l_og->append(i18n("conversion of folder \"Drafts\" to version 0.4 failed."));
    }
    else {
      l_og->append(i18n("converted folder \"Drafts\" to version 0.4"));
    }
  }
  else
    l_og->append(i18n("nothing to be done for folder \"Drafts\""));

  //Outbox
  if(QFile::exists(dir+"folder2.idx")) {
    num=convertFolder(dir+"folder2", dir+"outbox_2");
    if(num==-1) {
      error=true;
      l_og->append(i18n("conversion of folder \"Outbox\" to version 0.4 failed."));
    }
    else {
      l_og->append(i18n("converted folder \"Outbox\" to version 0.4"));
    }
  }
  else
    l_og->append(i18n("nothing to be done for folder \"Outbox\""));

  //Sent
  if(QFile::exists(dir+"folder3.idx")) {
    num=convertFolder(dir+"folder3", dir+"sent_3");
    if(num==-1) {
      error=true;
      l_og->append(i18n("conversion of folder \"Sent\" to version 0.4 failed."));
    }
    else {
      l_og->append(i18n("converted folder \"Sent\" to version 0.4"));
    }
  }
  else
    l_og->append(i18n("nothing to be done for folder \"Sent\""));

  //remove old info-files
  QFile::remove(dir+"standard.info");
  QFile::remove(dir+".standard.info");
  return (!error);
}


int KNConvert::Converter04::convertFolder(QString srcPrefix, QString dstPrefix)
{
  QFile srcMBox(srcPrefix+".mbox"),
        srcIdx(srcPrefix+".idx"),
        dstMBox(dstPrefix+".mbox"),
        dstIdx(dstPrefix+".idx");
  QTextStream ts(&dstMBox);
  ts.setEncoding(QTextStream::Latin1);

  OldFolderIndex oldIdx;
  NewFolderIndex newIdx;
  int lastId=0;
  bool filesOpen;

  //open files
  filesOpen=srcMBox.open(IO_ReadOnly);
  filesOpen=filesOpen && srcIdx.open(IO_ReadOnly);

  if(dstIdx.exists() && dstIdx.size()>0) { //we are converting from 0.4beta*
    if( (filesOpen=filesOpen && dstIdx.open(IO_ReadOnly)) ) {
      dstIdx.at( dstIdx.size()-sizeof(NewFolderIndex) ); //set filepointer to last entry
      dstIdx.readBlock( (char*)(&newIdx), sizeof(NewFolderIndex) );
      lastId=newIdx.id;
      dstIdx.close();
    }
  }

  filesOpen=filesOpen && dstMBox.open(IO_WriteOnly | IO_Append);
  filesOpen=filesOpen && dstIdx.open(IO_WriteOnly | IO_Append);

  if(!filesOpen) {
    srcMBox.close();
    srcIdx.close();
    dstMBox.close();
    dstIdx.close();
    return -1;
  }

  //conversion starts here
  while(!srcIdx.atEnd()) {

    //read index data
    srcIdx.readBlock( (char*)(&oldIdx), sizeof(OldFolderIndex));
    newIdx.id=++lastId;
    newIdx.sId=oldIdx.sId;
    newIdx.ti=oldIdx.ti;

    switch(oldIdx.status) {
      case 0: //AStoPost
        newIdx.flags[0]=false;  //doMail()
        newIdx.flags[1]=false;  //mailed()
        newIdx.flags[2]=true;   //doPost()
        newIdx.flags[3]=false;  //posted()
        newIdx.flags[4]=false;  //canceled()
        newIdx.flags[5]=false;  //editDisabled()
      break;

      case 1: //AStoMail
        newIdx.flags[0]=true;   //doMail()
        newIdx.flags[1]=false;  //mailed()
        newIdx.flags[2]=false;  //doPost()
        newIdx.flags[3]=false;  //posted()
        newIdx.flags[4]=false;  //canceled()
        newIdx.flags[5]=false;  //editDisabled()
      break;

      case 2: //ASposted
        newIdx.flags[0]=false;  //doMail()
        newIdx.flags[1]=false;  //mailed()
        newIdx.flags[2]=true;   //doPost()
        newIdx.flags[3]=true;   //posted()
        newIdx.flags[4]=false;  //canceled()
        newIdx.flags[5]=true;   //editDisabled()
      break;

      case 3: //ASmailed
        newIdx.flags[0]=true;   //doMail()
        newIdx.flags[1]=true;   //mailed()
        newIdx.flags[2]=false;  //doPost()
        newIdx.flags[3]=false;  //posted()
        newIdx.flags[4]=false;  //canceled()
        newIdx.flags[5]=true;   //editDisabled()
      break;

      case 6: //AScanceled
        newIdx.flags[0]=false;  //doMail()
        newIdx.flags[1]=false;  //mailed()
        newIdx.flags[2]=true;   //doPost()
        newIdx.flags[3]=true;   //posted()
        newIdx.flags[4]=true;   //canceled()
        newIdx.flags[5]=true;   //editDisabled()
      break;

      default: //what the ..
        newIdx.flags[0]=false;  //doMail()
        newIdx.flags[1]=false;  //mailed()
        newIdx.flags[2]=false;   //doPost()
        newIdx.flags[3]=false;  //posted()
        newIdx.flags[4]=false;  //canceled()
        newIdx.flags[5]=false;  //editDisabled()
      break;
    }


    //read mbox-data
    unsigned int size=oldIdx.eo-oldIdx.so;
    QCString buff(size+10);
    srcMBox.at(oldIdx.so);
    int readBytes=srcMBox.readBlock(buff.data(), size);
    buff.at(readBytes)='\0'; //terminate string;

    //remove "X-KNode-Overview"
    int pos=buff.find('\n');
    if(pos>-1)
      buff.remove(0, pos+1);

    //write mbox-data
    ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
    newIdx.so=dstMBox.at(); //save start-offset
    ts << "X-KNode-Overview: ";
    ts << KMime::extractHeader(buff, "Subject") << '\t';
    ts << KMime::extractHeader(buff, "Newsgroups") << '\t';
    ts << KMime::extractHeader(buff, "To") << '\t';
    ts << KMime::extractHeader(buff, "Lines") << '\n';
    ts << buff;
    newIdx.eo=dstMBox.at(); //save end-offset
    ts << '\n';

    //write index-data
    dstIdx.writeBlock((char*)(&newIdx), sizeof(NewFolderIndex));
  }

  //close/remove files and return number of articles in the new folder
  srcMBox.remove();
  srcIdx.remove();
  dstMBox.close();
  dstIdx.close();
  return ( dstIdx.size()/sizeof(NewFolderIndex) );
}




//-----------------------------
#include "knconvert.moc"







