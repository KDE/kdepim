/***************************************************************************
                          knarticlemanager.cpp  -  description
                             -------------------

    copyright            : (C) 2000 by Christian Thurner
    email                : cthurner@freepage.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kfiledialog.h>
#include <kmessagebox.h>
#include <krun.h>

#include "knarticlemanager.h"
#include "knglobals.h"
#include "utilities.h"

QStringList KNArticleManager::tempFiles;


KNArticleManager::KNArticleManager(KNListView *v)
{
	view=v;
	mainArtWidget=KNArticleWidget::mainWidget();
}



KNArticleManager::~KNArticleManager()
{
}



void KNArticleManager::deleteTempFiles()
{
  QStringList::Iterator it;
  QCString cmd;
  for(it=tempFiles.begin(); it!=tempFiles.end(); ++it) {
    cmd="rm -f "+(*it).local8Bit();
    system(cmd.data());
  }
}



void KNArticleManager::saveContentToFile(KNMimeContent *c)
{
	QString fName;
	DwString data;
	QFile f;
	fName=KFileDialog::getSaveFileName(c->ctName().data(), 0, xTop, 0);
	
	if(!fName.isEmpty()) {
		f.setName(fName);
		if(f.open(IO_WriteOnly)) {
			data=c->decodedData();
			f.writeBlock(data.data(), data.size());
			f.close();
		}
		else displayExternalFileError();
	}	
}



void KNArticleManager::saveArticleToFile(KNArticle *a)
{
	QString fName;
	DwString tmp;
	QFile f;
		
	fName=KFileDialog::getSaveFileName(a->subject().data(), 0, xTop, 0);
	
	if(!fName.isEmpty()) {
		tmp="";
	  for(char *line=a->firstHeaderLine(); line; line=a->nextHeaderLine()) {
  		tmp+=line;
  		tmp+="\n";
  	}
  	tmp+="\n";
  	tmp+=a->mainContent()->decodedData();
		f.setName(fName);
		if(f.open(IO_WriteOnly)) {
			f.writeBlock(tmp.data(), tmp.size());
			f.close();
		}
		else displayExternalFileError();
	}
}



QString KNArticleManager::saveContentToTemp(KNMimeContent *c)
{
  QString path;
	QCString tmp;
	DwString data;
	QFile f;
	
	tmp=c->headerLine("X-KNode-Tempfile");
	if(!tmp.isEmpty()) {
	  path=QString(tmp);
	  return path;
	}
	path="/tmp/"+KApplication::randomString(10);
	path+=c->ctName();
	tempFiles.append(path);
	f.setName(path);
 	if(f.open(IO_WriteOnly)) {
 		data=c->decodedData();
 		f.writeBlock(data.data(), data.size());
 		f.close();
 	  c->setHeader(KNArticleBase::HTxkntempfile, path.local8Bit());
 	  return path;
 	}
 	else {
 	  displayExternalFileError();
    return QString::null;
  }	
}



void KNArticleManager::openContent(KNMimeContent *c)
{
  QString path=saveContentToTemp(c);
  KURL url;
  KRun *run;
  if(path.isNull()) return;
  url.setPath(KURL::encode_string(path));
  run=new KRun(url, 0, true, true); //auto-deletion enabled
}



void KNArticleManager::showArticle(KNArticle *a, bool force)
{
	KNArticleWidget *aw;
	if(!force) KNArticleWidget::showArticle(a);
	else {
		aw=KNArticleWidget::find(a);
		if(aw) aw->updateContents();
	}
}



void KNArticleManager::showError(KNArticle *a, const QString &error)
{
	KNArticleWidget *aw=KNArticleWidget::find(a);
	if(aw) aw->showErrorMessage(error);
}

