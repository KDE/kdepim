/*
    kncleanup.cpp

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

#include <stdlib.h>

#include <qdatetime.h>
#include <qtextstream.h>
#include <qdir.h>
#include <qlayout.h>
#include <qframe.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kseparator.h>
#include <kapp.h>
#include <kdebug.h>

#include "kngroup.h"
#include "knfolder.h"
#include "knglobals.h"
#include "kncleanup.h"
#include "knconfig.h"
#include "knarticlewidget.h"



KNCleanUp::KNCleanUp(KNConfig::Cleanup *cfg) :  d_lg(0) , c_onfig(cfg)
{
  c_olList.setAutoDelete(false);
}


KNCleanUp::~KNCleanUp()
{
  delete d_lg;
}


void KNCleanUp::start()
{
  d_lg=new ProgressDialog(c_olList.count());
  d_lg->show();

  for(KNArticleCollection *c=c_olList.first(); c; c=c_olList.next()) {
    if(c->type()==KNCollection::CTgroup) {
      d_lg->showMessage(i18n("deleting expired articles in <b>%1</b>").arg(c->name()));
      kapp->processEvents();
      expireGroup(static_cast<KNGroup*>(c));
      d_lg->doProgress();
    }
    else if(c->type()==KNCollection::CTfolder) {
      d_lg->showMessage(i18n("compacting folder <b>%1</b>").arg(c->name()));
      kapp->processEvents();
      compactFolder(static_cast<KNFolder*>(c));
      d_lg->doProgress();
    }
  }

  delete d_lg;
  d_lg=0;
}


void KNCleanUp::reset()
{
  c_olList.clear();
  if(d_lg) {
    delete d_lg;
    d_lg=0;
  }
}


void KNCleanUp::expireGroup(KNGroup *g, bool showResult)
{
  int expDays=0, idRef=0, foundId=-1, delCnt=0, leftCnt=0, newCnt=0;
  KNRemoteArticle *art, *ref;

  if(!g->loadHdrs()) return;


  //find all expired
  for(int i=0; i<g->length(); i++) {
    art=g->at(i);
    if(art->isRead())
      expDays=c_onfig->maxAgeForRead();
    else
      expDays=c_onfig->maxAgeForUnread();

    art->setExpired( (art->date()->ageInDays() >= expDays) );
  }

  //save threads
  if(c_onfig->preserveThreads()) {
    for(int i=0; i<g->length(); i++) {
      art=g->at(i);
      if(!art->isExpired()) {
        idRef=art->idRef();
        while(idRef!=0) {
          ref=g->byId(idRef);
          ref->setExpired(false);
          idRef=ref->idRef();
        }
      }
    }
  }

  //restore threading
  for(int i=0; i<g->length(); i++) {
    art=g->at(i);
    if(!art->isExpired()) {
      idRef=art->idRef();
      foundId=0;
      while(foundId==0 && idRef!=0) {
        ref=g->byId(idRef);
        if(!ref->isExpired()) foundId=ref->id();
        idRef=ref->idRef();
      }
      art->setIdRef(foundId);
    }
  }

  //delete expired
  for(int i=0; i<g->length(); i++) {
    art=g->at(i);
    if(art->isExpired()) {
      if(art->isRead())
        g->decReadCount();
      delCnt++;
      art->clear();
      KNArticleWidget::articleRemoved(art);
    }
    else if(art->isNew())
      newCnt++;
  }

  if(delCnt>0) {
    g->saveStaticData(g->length(), true);
    g->saveDynamicData(g->length(), true);
    g->decCount(delCnt);
    g->setNewCount(newCnt);
    g->clear();
  }
  else
    g->syncDynamicData();

  g->saveInfo();
  leftCnt=g->count();

  kdDebug(5003) << "KNCleanUp::expireGroup() : " << g->groupname() << ": "
    << delCnt << " deleted , " << leftCnt << " left" << endl;

  if(showResult)
    KMessageBox::information(knGlobals.topWidget,
      i18n("<b>%1</b><br>expired: %2<br>left: %3").arg(g->groupname()).arg(delCnt).arg(leftCnt));
}


void KNCleanUp::compactFolder(KNFolder *f)
{
  KNLocalArticle *art;
  QDir dir(f->path());

  if(!dir.exists())
    return;

  if(!f->loadHdrs())
    return;

  f->closeFiles();
  QFileInfo info(f->m_boxFile);
  QString oldName=info.fileName();
  QString newName=oldName+".new";
  KNFile newMBoxFile(info.dirPath(true)+"/"+newName);

  if( (f->m_boxFile.open(IO_ReadOnly)) && (newMBoxFile.open(IO_WriteOnly)) ) {
    QTextStream ts(&newMBoxFile);
    for(int idx=0; idx<f->length(); idx++) {
      art=f->at(idx);
      if(f->m_boxFile.at(art->startOffset())) {
        ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
        art->setStartOffset(newMBoxFile.at());
        while(f->m_boxFile.at() < art->endOffset())
          ts << f->m_boxFile.readLineWnewLine();
        art->setEndOffset(newMBoxFile.at());
        newMBoxFile.putch('\n');
      }
    }

    f->syncIndex(true);
    newMBoxFile.close();
    f->closeFiles();

    dir.remove(oldName);
    dir.rename(newName, oldName);
  }
}


//===============================================================================================


KNCleanUp::ProgressDialog::ProgressDialog(int steps)
  : QSemiModal(knGlobals.topWidget,0, true, WStyle_Customize | WStyle_Tool | WStyle_NoBorder)
{
  const int w=400,
            h=160;

  p_rogress=0;
  s_teps=steps;

  setFixedSize(w,h);
  QFrame *top=new QFrame(this);
  top->setFrameStyle(QFrame::WinPanel | QFrame::Raised);
  top->setGeometry(0,0, w,h);

  QVBoxLayout *topL=new QVBoxLayout(top, 10);

  QLabel *l=new QLabel(i18n("Cleaning up. Please wait ..."), top);
  topL->addWidget(l);

  KSeparator *sep=new KSeparator(top);
  topL->addWidget(sep);

  m_sg=new QLabel(top);
  topL->addWidget(m_sg);

  p_bar=new QProgressBar(top);
  topL->addWidget(p_bar);
  p_bar->setTotalSteps(100*s_teps);
  p_bar->setProgress(1);


  if(knGlobals.topWidget->isVisible()) {
    int x, y;
    x=(knGlobals.topWidget->width()-w)/2;
    y=(knGlobals.topWidget->height()-h)/2;
    if(x<0 || y<0) {
      x=0;
      y=0;
    }
    x+=knGlobals.topWidget->x();
    y+=knGlobals.topWidget->y();
    move(x,y);
  }
}


KNCleanUp::ProgressDialog::~ProgressDialog()
{
}


void KNCleanUp::ProgressDialog::doProgress()
{
  p_rogress++;
  p_bar->setProgress(p_rogress*100);
}



/*
void KNCleanUp::group(KNGroup *g, bool withGUI)
{




void KNCleanUp::folder(KNFolder *f)
{
  KNSavedArticle *art;
  if(!f->loadHdrs()) return;
    
  QDir dir(f->path());
  if (!dir.exists())
    return; 
  
  QString oldName(QString("folder%1.mbox").arg(f->id()));
  KNFile oldFile(f->path()+oldName);
  QString newName(QString("folder%1.mbox.new").arg(f->id()));
  KNFile newFile(f->path()+newName);  

  if( (oldFile.open(IO_ReadOnly)) && (newFile.open(IO_WriteOnly)) ) {
    QTextStream ts(&newFile);
    for(int idx=0; idx<f->length(); idx++) {
      art=f->at(idx);
      if(oldFile.at(art->startOffset())) {
        ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
        art->setStartOffset(newFile.at());
        while(oldFile.at() < art->endOffset())
          ts << oldFile.readLineWnewLine();     
        art->setEndOffset(newFile.at());
        newFile.putch('\n');
      }
    }
    newFile.close();
    oldFile.close();
    f->syncDynamicData(true);
    f->saveInfo();
    
    dir.remove(oldName);  
    dir.rename(newName,oldName);
  }
}
*/


