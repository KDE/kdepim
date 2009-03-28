/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#include <QStackedWidget>
#include <QLabel>
#include <QCheckBox>
#include <QByteArray>
#include <QListWidget>
#include <QTextStream>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>

#include <klocale.h>
#include <kfiledialog.h>
#include <kseparator.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <klineedit.h>
#include <kcomponentdata.h>
#include <kpushbutton.h>

#include <kmime/kmime_util.h>

#include "knconvert.h"
#include "resource.h"


bool KNConvert::needToConvert(const QString &oldVersion)
{
  bool ret=(
              (oldVersion.left(3)=="0.3") ||
              (oldVersion.left(3)=="0.4")
           );

  return ret;
}


KNConvert::KNConvert(const QString &version)
  : KDialog(0), l_ogList(0), c_onversionDone(false), v_ersion(version)
{
  setCaption(i18n("Conversion"));
  QVBoxLayout *topL=new QVBoxLayout(this);
  topL->setSpacing(5);
  topL->setMargin(5);
  s_tack=new QStackedWidget(this);
  topL->addWidget(s_tack, 1);
  topL->addWidget(new KSeparator(this));

  QHBoxLayout *btnL=new QHBoxLayout();
  topL->addLayout( btnL );
  btnL->setSpacing(5);
  s_tartBtn=new QPushButton(i18n("Start Conversion..."), this);
  s_tartBtn->setDefault(true);
  btnL->addStretch(1);
  btnL->addWidget(s_tartBtn);
  c_ancelBtn=new KPushButton(KStandardGuiItem::cancel(), this);
  btnL->addWidget(c_ancelBtn);

  connect(s_tartBtn, SIGNAL(clicked()), this, SLOT(slotStart()));
  connect(c_ancelBtn, SIGNAL(clicked()), this, SLOT(reject()));

  w_1=new QWidget(s_tack);
  s_tack->insertWidget(1,w_1);
  QGridLayout *w1L=new QGridLayout(w_1);
  w1L->setSpacing(5);
  w1L->setMargin(5);

  QLabel *l1=new QLabel(i18n(
"<b>Congratulations, you have upgraded to KNode version %1.</b><br />\
Unfortunately this version uses a different format for some data-files, so \
in order to keep your existing data it is necessary to convert it first. This is \
now done automatically by KNode. If you want to, a backup of your existing data \
will be created before the conversion starts.", QString(KNODE_VERSION)), w_1);
  w1L->addWidget(l1, 0, 0, 1, 3 );

  c_reateBkup=new QCheckBox(i18n("Create backup of old data"), w_1);
  w1L->addWidget(c_reateBkup, 2, 0, 1, 3 );
  connect(c_reateBkup, SIGNAL(toggled(bool)), this, SLOT(slotCreateBkupToggled(bool)));

  b_ackupPathLabel=new QLabel(i18n("Save backup in:"),w_1);
  w1L->addWidget(b_ackupPathLabel, 3,0);

  b_ackupPath=new KLineEdit(QDir::homePath()+QString("/knodedata-")+v_ersion+".tar.gz", w_1);
  w1L->addWidget(b_ackupPath, 3,1);

  b_rowseBtn= new QPushButton(i18n("Browse..."), w_1);
  connect(b_rowseBtn, SIGNAL(clicked()), this, SLOT(slotBrowse()));
  w1L->addWidget(b_rowseBtn, 3,2);
  w1L->setColumnStretch(1,1);
  w1L->addItem( new QSpacerItem( 0,15), 1, 0 );
  w1L->setRowStretch(4,1);
  w1L->addItem( new QSpacerItem( 0,15), 4, 0 );

  w_2=new QLabel(s_tack);
  w_2->setText(i18n("<b>Converting, please wait...</b>"));
  w_2->setAlignment(Qt::AlignCenter);
  s_tack->insertWidget(2,w_2);

  w_3=new QWidget(s_tack);
  s_tack->insertWidget(3,w_3);
  QVBoxLayout *w3L=new QVBoxLayout(w_3);
  w3L->setSpacing(5);
  w3L->setMargin(5);

  r_esultLabel=new QLabel(w_3);
  w3L->addWidget(r_esultLabel);
  QLabel *l2=new QLabel(i18n("Processed tasks:"),w_3);
  l_ogList = new QListWidget( w_3 );
  w3L->addSpacing(15);
  w3L->addWidget(l2);
  w3L->addWidget(l_ogList, 1);

  s_tack->setCurrentWidget(w_1);
  slotCreateBkupToggled(false);
}


KNConvert::~KNConvert()
{
  for ( QList<Converter*>::Iterator it = mConverters.begin(); it != mConverters.end(); ++it )
    delete (*it);
}


void KNConvert::convert()
{
  int errors=0;
  for ( QList<Converter*>::Iterator it = mConverters.begin(); it != mConverters.end(); ++it )
    if( !(*it)->doConvert() )
      errors++;

  if(errors>0)
    r_esultLabel->setText(i18n(
"<b>Some errors occurred during the conversion.</b>\
<br />You should now examine the log to find out what went wrong."));
  else
    r_esultLabel->setText(i18n(
"<b>The conversion was successful.</b>\
<br />Have a lot of fun with this new version of KNode. ;-)"));

  s_tartBtn->setText(i18n("Start KNode"));
  s_tartBtn->setEnabled(true);
  c_ancelBtn->setEnabled(true);
  l_ogList->addItems( l_og );
  s_tack->setCurrentWidget(w_3);

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
  s_tack->setCurrentWidget(w_2);

  if(v_ersion.left(3)=="0.3" || v_ersion.left(7)=="0.4beta") {
    //Version 0.4
    mConverters.append( new Converter04( &l_og ) );
  }

  //create backup of old data using "tar"
  if(c_reateBkup->isChecked()) {
    if(b_ackupPath->text().isEmpty()) {
      KMessageBox::error(this, i18n("Please select a valid backup path."));
      return;
    }

    QString dataDir = KStandardDirs::locateLocal( "data", "knode/" );
    t_ar=new KProcess;
    *t_ar << "tar";
    *t_ar << "-cz" << dataDir
          << "-f" << b_ackupPath->text();
    connect(t_ar, SIGNAL(finished (int, QProcess::ExitStatus )), this, SLOT(slotTarExited(int, QProcess::ExitStatus )));
    t_ar->start();
    if(!t_ar->waitForStarted()) {
      slotTarExited(t_ar->exitCode(), t_ar->exitStatus() );
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


void KNConvert::slotTarExited(int exitCode, QProcess::ExitStatus exitStatus )
{
  bool success=true;

  if( (exitStatus == QProcess::CrashExit) || exitCode) {
    success=false;
    if(KMessageBox::Cancel==KMessageBox::warningContinueCancel(this, i18n("<b>The backup failed</b>; do you want to continue anyway?"))) {

      delete t_ar;
      t_ar = 0;
      reject();
      return;
    }
  }

  delete t_ar;
  t_ar = 0;
  if(success)
    l_og.append(i18n("created backup of the old data-files in %1", b_ackupPath->text()));
  else
    l_og.append(i18n("backup failed."));

  // now we actually convert the files
  convert();
}



//============================================================================================



bool KNConvert::Converter04::doConvert()
{
  QString dir = KStandardDirs::locateLocal( "data", "knode/folders/" );
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


int KNConvert::Converter04::convertFolder(const QString &srcPrefix, const QString &dstPrefix)
{
  QFile srcMBox(srcPrefix+".mbox"),
        srcIdx(srcPrefix+".idx"),
        dstMBox(dstPrefix+".mbox"),
        dstIdx(dstPrefix+".idx");
  QTextStream ts(&dstMBox);
  ts.setCodec( "ISO 8859-1" );

  OldFolderIndex oldIdx;
  NewFolderIndex newIdx;
  int lastId=0;
  bool filesOpen;

  //open files
  filesOpen=srcMBox.open(QIODevice::ReadOnly);
  filesOpen=filesOpen && srcIdx.open(QIODevice::ReadOnly);

  if(dstIdx.exists() && dstIdx.size()>0) { //we are converting from 0.4beta*
    if( (filesOpen=filesOpen && dstIdx.open(QIODevice::ReadOnly)) ) {
      dstIdx.seek( dstIdx.size() - sizeof(NewFolderIndex) ); //set filepointer to last entry
      dstIdx.read( (char*)(&newIdx), sizeof(NewFolderIndex) );
      lastId=newIdx.id;
      dstIdx.close();
    }
  }

  filesOpen=filesOpen && dstMBox.open(QIODevice::WriteOnly | QIODevice::Append);
  filesOpen=filesOpen && dstIdx.open(QIODevice::WriteOnly | QIODevice::Append);

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
    srcIdx.read( (char*)(&oldIdx), sizeof(OldFolderIndex));
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
    QByteArray buffer;
    srcMBox.seek( oldIdx.so );
    buffer = srcMBox.read( size );

    //remove "X-KNode-Overview"
    int pos = buffer.indexOf( '\n' );
    if ( pos > -1 )
      buffer.remove( 0, pos + 1 );

    //write mbox-data
    ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
    ts.flush();
    newIdx.so = dstMBox.pos(); //save start-offset
    ts << "X-KNode-Overview: ";
    ts << KMime::extractHeader(buffer, "Subject") << '\t';
    ts << KMime::extractHeader(buffer, "Newsgroups") << '\t';
    ts << KMime::extractHeader(buffer, "To") << '\t';
    ts << KMime::extractHeader(buffer, "Lines") << '\n';
    ts << buffer;
    ts.flush();
    newIdx.eo = dstMBox.pos(); //save end-offset
    ts << '\n';

    //write index-data
    dstIdx.write((char*)(&newIdx), sizeof(NewFolderIndex));
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







