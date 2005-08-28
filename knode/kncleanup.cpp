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

#include <stdlib.h>

#include <qdir.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qprogressbar.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <kseparator.h>
#include <kapplication.h>
#include <kdebug.h>

#include "knfolder.h"
#include "knglobals.h"
#include "kncleanup.h"
#include "knconfig.h"
#include "knfoldermanager.h"
#include "kngroupmanager.h"
#include "knarticlemanager.h"
#include "knnntpaccount.h"


KNCleanUp::KNCleanUp() :  d_lg(0)
{
}


KNCleanUp::~KNCleanUp()
{
  delete d_lg;
}


void KNCleanUp::start()
{
  if ( mColList.isEmpty() )
    return;

  d_lg = new ProgressDialog( mColList.count() );
  d_lg->show();

  for ( QValueList<KNArticleCollection*>::Iterator it = mColList.begin(); it != mColList.end(); ++it ) {
    if ( (*it)->type() == KNCollection::CTgroup ) {
      d_lg->showMessage( i18n("Deleting expired articles in <b>%1</b>").arg( (*it)->name() ) );
      kapp->processEvents();
      expireGroup( static_cast<KNGroup*>( (*it) ) );
      d_lg->doProgress();
    }
    else if ( (*it)->type() == KNCollection::CTfolder ) {
      d_lg->showMessage( i18n("Compacting folder <b>%1</b>").arg( (*it)->name() ) );
      kapp->processEvents();
      compactFolder( static_cast<KNFolder*>( (*it) ) );
      d_lg->doProgress();
    }
  }

  delete d_lg;
  d_lg=0;
}


void KNCleanUp::reset()
{
  mColList.clear();
  if(d_lg) {
    delete d_lg;
    d_lg=0;
  }
}


void KNCleanUp::expireGroup( KNGroup *g, bool showResult )
{
  int expDays=0, idRef=0, foundId=-1, delCnt=0, leftCnt=0, newCnt=0, firstArtNr=g->firstNr(), firstNew=-1;
  bool unavailable=false;
  KNRemoteArticle *art, *ref;

  if (!g)
    return;

  KNConfig::Cleanup *conf = g->activeCleanupConfig();

  g->setNotUnloadable(true);

  if (!g->isLoaded() && !knGlobals.groupManager()->loadHeaders(g)) {
    g->setNotUnloadable(false);
    return;
  }

  //find all expired
  for(int i=0; i<g->length(); i++) {
    art=g->at(i);
    if(art->isRead())
      expDays = conf->maxAgeForRead();
    else
      expDays = conf->maxAgeForUnread();

    unavailable = false;
    if ((art->articleNumber() != -1) && conf->removeUnavailable())
      unavailable = (art->articleNumber() < firstArtNr);

    art->setExpired( (art->date()->ageInDays() >= expDays) || unavailable );
  }

  //save threads
  if (conf->preserveThreads()) {
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
      if (art->hasContent())
        knGlobals.articleManager()->unloadArticle(art, true);
    }
    else if(art->isNew() && !art->isRead()) {
      if(firstNew==-1)
        firstNew=i;
      newCnt++;
    }
  }

  g->setNotUnloadable(false);

  if(delCnt>0) {
    g->saveStaticData(g->length(), true);
    g->saveDynamicData(g->length(), true);
    g->decCount(delCnt);
    g->setNewCount(newCnt);
    g->setFirstNewIndex(firstNew);
    g->saveInfo();
    knGlobals.groupManager()->unloadHeaders(g, true);
  }
  else
    g->syncDynamicData();

  conf->setLastExpireDate();
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

  if (!f)
    return;

  QDir dir(f->path());

  if(!dir.exists())
    return;

  f->setNotUnloadable(true);

  if (!f->isLoaded() && !knGlobals.folderManager()->loadHeaders(f)) {
    f->setNotUnloadable(false);
    return;
  }

  f->closeFiles();
  QFileInfo info(f->m_boxFile);
  QString oldName=info.fileName();
  QString newName=oldName+".new";
  KNFile newMBoxFile(info.dirPath(true)+"/"+newName);

  if( (f->m_boxFile.open(IO_ReadOnly)) && (newMBoxFile.open(IO_WriteOnly)) ) {
    QTextStream ts(&newMBoxFile);
    ts.setEncoding(QTextStream::Latin1);
    for(int idx=0; idx<f->length(); idx++) {
      art=f->at(idx);
      if(f->m_boxFile.at(art->startOffset())) {
        ts << "From aaa@aaa Mon Jan 01 00:00:00 1997\n";
        art->setStartOffset(newMBoxFile.at());
        while(f->m_boxFile.at() < (uint)art->endOffset())
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

  f->setNotUnloadable(false);
}


//===============================================================================================


KNCleanUp::ProgressDialog::ProgressDialog(int steps)
 : QDialog(knGlobals.topWidget, 0, true)
{
  const int w=400,
            h=160;

  p_rogress=0;
  s_teps=steps;

  setCaption(kapp->makeStdCaption(i18n("Cleaning Up")));

  setFixedSize(w,h);
  QFrame *top=new QFrame(this);
  top->setGeometry(0,0, w,h);

  QVBoxLayout *topL=new QVBoxLayout(top, 10);

  QLabel *l=new QLabel(i18n("Cleaning up. Please wait..."), top);
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


void KNCleanUp::ProgressDialog::showMessage(const QString &s)
{
  m_sg->setText(s);
}


void KNCleanUp::ProgressDialog::doProgress()
{
  p_rogress++;
  p_bar->setProgress(p_rogress*100);
}


void KNCleanUp::ProgressDialog::closeEvent(QCloseEvent *)
{
  // do nothing => prevent that the user closes the window
}

// kate: space-indent on; indent-width 2;
