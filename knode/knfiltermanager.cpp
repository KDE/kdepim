/***************************************************************************
                          knfiltermanager.cpp  -  description
                             -------------------

    copyright            : (C) 1999 by Christian Thurner
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


#include <stdlib.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qbitarray.h>

#include <ktabctl.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstddirs.h>
#include <kpopupmenu.h>
#include <ktoolbarbutton.h>
#include <kiconloader.h>
#include <ktoolbar.h>
#include <ksimpleconfig.h>

#include "utilities.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knstringfilter.h"
#include "knarticlefilter.h"
#include "knfilterdialog.h"
#include "knfiltersettings.h"
#include "knfiltermanager.h"


KNFilterSelectAction::KNFilterSelectAction( const QString& text, const QString& pix,
                                            int accel, QObject* parent, const char* name )
 : KAction(text,pix,accel,parent,name), currentItem(-42)
{
  if (!pix.isEmpty())
    p_ixmap = BarIcon(pix);
  p_opup = new KPopupMenu;
  p_opup->setCheckable(true);
  connect(p_opup,SIGNAL(activated(int)),this,SLOT(slotMenuActivated(int)));
}



KNFilterSelectAction::~KNFilterSelectAction()
{
  delete p_opup;
}



int KNFilterSelectAction::plug(QWidget* widget, int index)
{
  if ( widget->inherits("QPopupMenu") ) {
    QPopupMenu* menu = static_cast<QPopupMenu*>( widget );
    int id;
    if ( !pixmap().isNull() ) {
      id = menu->insertItem( pixmap(), p_opup, -1, index );
    } else {
      if ( hasIconSet() )
        id = menu->insertItem( iconSet(), text(), p_opup, -1, index );
      else
        id = menu->insertItem( text(), p_opup, -1, index );
    }

    menu->setItemEnabled( id, isEnabled() );
    menu->setWhatsThis( id, whatsThis() );

    addContainer( menu, id );
    connect( menu, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );
    return containerCount() - 1;
  }
  else if ( widget->inherits("KToolBar"))  {
    KToolBar *bar = static_cast<KToolBar *>( widget );

    int id_ = getToolButtonID();
    bar->insertButton( p_ixmap, id_, p_opup, isEnabled(), plainText(), index);

    QWhatsThis::add( bar->getButton(id_), whatsThis() );
    addContainer( bar, id_ );

    connect( bar, SIGNAL( destroyed() ), this, SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  qDebug("Can not plug KFilterSelectAction in %s", widget->className() );
  return -1;
}



void KNFilterSelectAction::setCurrentItem(int id)
{
  p_opup->setItemChecked(currentItem, false);
  p_opup->setItemChecked(id, true);
  currentItem = id;
}



void KNFilterSelectAction::slotMenuActivated(int id)
{
  setCurrentItem(id);
  emit(activated(id));
}



//=========================================================================================



KNFilterManager::KNFilterManager( QObject * parent, const char * name)
 : QObject(parent,name), fset(0), currFilter(0)
{
	fList.setAutoDelete(true);
	
	actFilter = new KNFilterSelectAction(i18n("&Filter"), "filter", 0 , &actionCollection, "view_Filter");
	connect(actFilter, SIGNAL(activated(int)), this,	SLOT(slotMenuActivated(int)));
	
	loadFilters();	
	readOptions();	
}



KNFilterManager::~KNFilterManager()
{
}



void KNFilterManager::readOptions()
{
  KConfig *conf=KGlobal::config();   	
  conf->setGroup("READNEWS");
  setFilter(conf->readNumEntry("lastFilterID", 1));
}



void KNFilterManager::saveOptions()
{
  if (currFilter) {
    KConfig *conf=KGlobal::config();   	
    conf->setGroup("READNEWS");
    conf->writeEntry("lastFilterID", currFilter->id());
  }
}



void KNFilterManager::loadFilters()
{
	QString fname(KGlobal::dirs()->findResource("appdata","filters/filters.rc"));
	if (fname != QString::null) {
    KSimpleConfig conf(fname,true);
	
    QValueList<int> activeFilters = conf.readIntListEntry("Active");
    menuOrder = conf.readIntListEntry("Menu");
	
    QValueList<int>::Iterator it = activeFilters.begin();
    while (it != activeFilters.end()) {
    	KNArticleFilter *f=new KNArticleFilter((*it));
		  if (f->loadInfo())
  			addFilter(f);
  		else
  			delete f;	
  		it++;
    }	
  }
 	updateMenu();	
}



void KNFilterManager::saveFilterLists()
{
	QString dir(KGlobal::dirs()->saveLocation("appdata","filters/"));
	if (dir==QString::null) {
		displayInternalFileError();
		return;
	}
	KSimpleConfig conf(dir+"filters.rc");
	
	QValueList<int> activeFilters;
  QListIterator<KNArticleFilter> it(fList);
	for ( ; it.current(); ++it )
		activeFilters << it.current()->id();

	conf.writeEntry("Active",activeFilters);
	conf.writeEntry("Menu",menuOrder);
}



void KNFilterManager::startConfig(KNFilterSettings *fs)
{
	fset=fs;
	for(KNArticleFilter *f=fList.first(); f; f=fList.next())
	  fset->addItem(f);	
	
	QValueList<int>::Iterator it = menuOrder.begin();
  while (it != menuOrder.end()) {
  	if ((*it)!=-1)
  		fset->addMenuItem(byID((*it)));
		else
			fset->addMenuItem(0);
		++it;
	}
}



void KNFilterManager::endConfig()
{
	QValueList<int> newOrder(fset->menuOrder());
	if (newOrder != menuOrder) {  // save only on demand, to avoid making a local
		menuOrder = newOrder;        // copy of the config-file as long as possible
		saveFilterLists();
	}
		
	if(currFilter)
    if(!currFilter->isEnabled()) currFilter=0;

 	updateMenu();

	emit filterChanged(currFilter);
	
	fset=0;
}



void KNFilterManager::newFilter()
{
	KNArticleFilter *f=new KNArticleFilter();
	editFilter(f);
}



void KNFilterManager::addFilter(KNArticleFilter *f)
{
	if(f->id()==-1) {      // new filter, find suitable ID
		QValueList<int> activeFilters;
  	QListIterator<KNArticleFilter> it(fList);  // ok, this is a ugly hack:
		for ( ; it.current(); ++it )               // we want to reuse old id's, so we
			activeFilters << it.current()->id();     // try to find the first unused id.
		int newId = 1;
		while (activeFilters.contains(newId)>0)
			newId++;
		f->setId(newId);
	}
	fList.append(f);
}



void KNFilterManager::editFilter(KNArticleFilter *f)
{
	if(!f->loaded() && f->id()!=-1) f->load();
	KNFilterDialog *fdlg=new KNFilterDialog(0,0,f);
	connect(fdlg, SIGNAL(editDone(KNFilterDialog*)),
		this, SLOT(slotEditDone(KNFilterDialog*)));
	
	fdlg->show();
}



void KNFilterManager::deleteFilter(KNArticleFilter *f)
{
	if(KMessageBox::questionYesNo(0,i18n("Do you really want to delete this filter?"))==KMessageBox::Yes) {
	  if(fList.removeRef(f)) {     // does not delete surplus config files
  		if(fset) {                  // we reuse ids to reduce the number of dead files
  			fset->removeItem(f);
  			fset->removeMenuItem(f);
  		}
  		if(currFilter==f) currFilter=0;
	 	}
	}
}



KNArticleFilter* KNFilterManager::setFilter(const int id)
{
	KNArticleFilter *bak=currFilter;
	
	currFilter=byID(id);	
	
	if(currFilter) {
	  actFilter->setCurrentItem(currFilter->id());
    emit(filterChanged(currFilter));
	}	else
	  currFilter=bak;
	
	return currFilter;		
}



KNArticleFilter* KNFilterManager::byID(int id)
{
	KNArticleFilter *ret=0;
	
	for(ret=fList.first(); ret; ret=fList.next())
		if(ret->id()==id) break;
		
	return ret;	
}



bool KNFilterManager::nameIsOK(KNArticleFilter *f)
{
	KNArticleFilter *var=fList.first();
	bool found=false;
	
	while(var && !found) {
		if(var!=f) found=(f->name()==var->name());
		var=fList.next();
	}
	
	return (!found);
}



void KNFilterManager::updateMenu()
{
	actFilter->popupMenu()->clear();
	KNArticleFilter *f=0;
	
	QValueList<int>::Iterator it = menuOrder.begin();
  while (it != menuOrder.end()) {
	 	if ((*it)!=-1) {
			if ((f=byID((*it))))
				actFilter->popupMenu()->insertItem(f->name(), f->id());
		}
		else actFilter->popupMenu()->insertSeparator();
		++it;
  }
	
	if(currFilter)
		actFilter->setCurrentItem(currFilter->id());
	actFilter->setEnabled(!menuOrder.isEmpty());
}



void KNFilterManager::slotEditDone(KNFilterDialog *d)
{
	KNArticleFilter *f=d->filter();
	
	if(!nameIsOK(f)) {
		#warning on error we quietly delete the changes made by the user: FIX IT
		KMessageBox::error(0, i18n("This name exists already.\nPlease choose a different one."));
		return;    // uhmmm, memory leak? a new filter with a invalid name => no delete
	}
			
	if(f->id()==-1) {
		addFilter(f);
		f->setLoaded(true);
		if(fset) {
			fset->addItem(f);
			if(f->isEnabled()) fset->addMenuItem(f);
		}
	}
	else if(fset) {
		if(f->isEnabled()) fset->addMenuItem(f);
		else fset->removeMenuItem(f);
		fset->updateItem(f);
	}
			
	f->save();	
		
	delete d;					
}



void KNFilterManager::slotMenuActivated(int id)
{
	KNArticleFilter *f=setFilter(id);
	
	if (!f)
    KMessageBox::error(0, i18n("ERROR : no such filter!"));
}




//--------------------------------

#include "knfiltermanager.moc"


/*/==============================================================================


KNFilterListDialog::KNFilterListDialog(QWidget *parent=0, const char *name=0) :
	QSemiModal(parent, name, true)
{
	
	setCaption(i18n("Configure filters"));
	
	
	KTabCtl *tabs=new KTabCtl(this);
	QPushButton *helpBtn=new QPushButton(i18n("Help"), this);
	//helpBtn->setEnabled(false);
	QPushButton *closeBtn=new QPushButton(i18n("Close"), this);
	SIZE(closeBtn); SIZE(helpBtn);
		
	
	
  QWidget *w1=new QWidget(tabs);
	
	list1=new KNListBox(w1);
	
	QButtonGroup *btns1=new QButtonGroup(w1);
	
	QPushButton *newBtn=new QPushButton(i18n("new"), btns1);
	QPushButton *delBtn=new QPushButton(i18n("delete"), btns1);
	QPushButton *editBtn=new QPushButton(i18n("edit"), btns1);
	SIZE(newBtn); SIZE(delBtn); SIZE(editBtn);
	
	QGridLayout *w1L=new QGridLayout(w1, 2,2,10);
	QVBoxLayout *btnL1=new QVBoxLayout(btns1,5);
		
	btnL1->addWidget(newBtn);
	btnL1->addWidget(delBtn);
	btnL1->addWidget(editBtn);
	
	w1L->addWidget(btns1, 0,1);
	w1L->addMultiCellWidget(list1, 0,1,0,0);
	w1L->setColStretch(0,1);
	w1L->setRowStretch(1,1);
	w1L->addColSpacing(0,200);
	
	
	
	
	
	QWidget *w2=new QWidget(tabs);
	
	list2=new KNListBox(w2);
	
	QButtonGroup *btns2=new QButtonGroup(w2);
	QPushButton *upBtn=new QPushButton(i18n("up"), btns2);
	QPushButton *downBtn=new QPushButton(i18n("down"), btns2);
	QPushButton *insSepBtn=new QPushButton(i18n("insert\nSeparator"), btns2);
	QPushButton *remSepBtn=new QPushButton(i18n("remove\nSeparator"), btns2);
  SIZE(upBtn); SIZE(downBtn); SIZE(insSepBtn); SIZE(remSepBtn);
	
	QGridLayout *w2L=new QGridLayout(w2, 2,2,10);
	QVBoxLayout *btnL2=new QVBoxLayout(btns2,5);
	
	btnL2->addWidget(upBtn);
	btnL2->addWidget(downBtn);
	btnL2->addSpacing(35);
	btnL2->addWidget(insSepBtn);
	btnL2->addWidget(remSepBtn);
	
	w2L->addWidget(btns2, 0,1);
	w2L->addMultiCellWidget(list2, 0,1,0,0);
	w2L->setColStretch(0,1);
	w2L->setRowStretch(1,1);
	w2L->addColSpacing(0,200);
		
	
	
	tabs->addTab(w1, i18n("Filters"));
	tabs->addTab(w2, i18n("Menu"));

		
	QVBoxLayout *topL=new QVBoxLayout(this, 10);
	QHBoxLayout *chBtnL=new QHBoxLayout(5);
	
	topL->addWidget(tabs, 1);
	topL->addLayout(chBtnL);
	chBtnL->addWidget(helpBtn);
	chBtnL->addStretch(1);
	chBtnL->addWidget(closeBtn);
  	
	topL->activate();
	
	
	connect(closeBtn, SIGNAL(clicked()), this, SLOT(slotCloseBtn()));
	connect(helpBtn, SIGNAL(clicked()), this, SLOT(slotHelpBtn()));
	
	connect(btns1, SIGNAL(clicked(int)), this, SLOT(slotButtons1(int)));
	connect(btns2, SIGNAL(clicked(int)), this, SLOT(slotButtons2(int)));
	
	connect(list1, SIGNAL(selected(int)), this, SLOT(slotList1Selected(int)));
	
	setDialogSize("fListDLG", this);
	
	
}



KNFilterListDialog::~KNFilterListDialog()
{
	saveDialogSize("fListDLG", this->size());
}



void KNFilterListDialog::addItem(KNArticleFilter *f)
{
	QPixmap pm;
	if(f->isEnabled()) pm=ICON("fltrblue.xpm");
	else pm=ICON("fltrgrey.xpm");
	
	KNLBoxItem *it=new KNLBoxItem(f->name(),f,&pm);
	list1->inSort(it);
	
}



void KNFilterListDialog::addMenuItem(KNArticleFilter *f)
{
	if(f) {
		if(findItem(list2, f)==-1)
			list2->insertItem(new KNLBoxItem(f->name(), f,0));
	}
	
	else  list2->insertItem(new KNLBoxItem("===", 0,0));
}



void KNFilterListDialog::removeItem(KNArticleFilter *f)
{
	int i=findItem(list1, f);
  if(i!=-1) list1->removeItem(i);
}



void KNFilterListDialog::removeMenuItem(KNArticleFilter *f)
{
	int i=findItem(list2, f);
	if(i!=-1) list2->removeItem(i);		
}



void KNFilterListDialog::updateItem(KNArticleFilter *f)
{
	int i=findItem(list1, f);
	
	if(i!=-1) {
		QPixmap pm;
		if(f->isEnabled()) pm=ICON("fltrblue.xpm");	
	  else pm=ICON("fltrgrey.xpm");
	  list1->changeItem(new KNLBoxItem(f->name(),f,&pm), i);
	}

	if(f->isEnabled())	
		list2->changeItem(new KNLBoxItem(f->name(),f,0), findItem(list2, f));			
}



int KNFilterListDialog::findItem(KNListBox *l, KNArticleFilter *f)
{
	int idx=0;
	bool found=false;
	
	while(!found && idx < (int) l->count()) {
		found=(l->itemAt(idx)->data()==f);
		if(!found) idx++;
	}
	
	
	if(found) return idx;
	else return -1;	
	
}



void KNFilterListDialog::slotButtons1(int id)
{
	
	int c=list1->currentItem();
	KNArticleFilter *f=0;
	
	if(c!=-1) f=(KNArticleFilter*) list1->itemAt(c)->data();	

	
	switch(id) {
		
		case 0: emit newFilter(); 				break;
		case 1: if(f) emit delFilter(f); 	break;
		case 2: if(f) emit editFilter(f); break;
	};
	 	
}



void KNFilterListDialog::slotButtons2(int id)
{
	int c=list2->currentItem();
	KNArticleFilter *f=0;
	
	if(c!=-1) f=(KNArticleFilter*) list2->itemAt(c)->data();
	
	switch(id) {
	
		case 0:
			if(c!=0) {
				if(f) list2->insertItem(new KNLBoxItem(f->name(), f,0), c-1);
				else  list2->insertItem(new KNLBoxItem("===", 0,0), c-1);
				list2->removeItem(c+1);
				list2->setCurrentItem(c-1);
			}
		break;
		
		case 1:
			if(c!= (int) list2->count()-1) {
				if(f) list2->insertItem(new KNLBoxItem(f->name(), f,0), c+2);
				else  list2->insertItem(new KNLBoxItem("===", 0,0), c+2);
				list2->removeItem(c);
				list2->setCurrentItem(c+1);
			}
		break;
		
		case 2:
			if(c!=-1) list2->insertItem(new KNLBoxItem("===", 0,0), c);
		break;
		
		case 3:
			if(c!=-1 && f==0) list2->removeItem(c);
		break;			
	}
				 		

}



int* KNFilterListDialog::menuOrder()
{
	KNArticleFilter *f;
	int *ret=new int[list2->count()+1];
	
	ret[0]=list2->count();
	
	for(uint i=0; i<list2->count(); i++) {
		f=(KNArticleFilter*) list2->itemAt(i)->data();
		
		if(f) ret[i+1]=f->id();
		else ret[i+1]=-1;
	}

 return ret;
	
}



void KNFilterListDialog::slotList1Selected(int id)
{
	emit editFilter((KNArticleFilter*) list1->itemAt(id)->data());	
}



void KNFilterListDialog::slotCloseBtn()
{
	emit dialogDone();
}



void KNFilterListDialog::slotHelpBtn()
{
	kapp->invokeHTMLHelp("knode/working-1.html", "4.5");
}
*/


























































