// -*- c-basic-offset: 2; -*-

/***************************************************************************
                     kngrouplistwidget.cpp - description
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

#include <qlayout.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qtimer.h>

#include "kngrouplistwidget.h"
#include "knlboxitem.h"
#include "utilities.h"
#include "knglobals.h"

KNGroupListWidget::KNGroupListWidget(KNNntpAccount *a, QWidget *parent)
	: QWidget(parent,0)
{
  //init Dialog
  m_list=new QListView(this);
  m_list->addColumn(i18n("Groupname"));
  m_list->setRootIsDecorated(true);
  m_list->setAllColumnsShowFocus(true);

  filterEdit=new QLineEdit(this);
  QLabel *filterLab=new QLabel(i18n("Filter:"), this);

  QVBoxLayout *topLayout=new QVBoxLayout(this,0,10);
  QHBoxLayout *fltrLayout=new QHBoxLayout(10);
  QVBoxLayout *tabL=new QVBoxLayout(0);

  topLayout->addLayout(fltrLayout,0);
  topLayout->addLayout(tabL,1);

  fltrLayout->addWidget(filterLab, 0);
  fltrLayout->addWidget(filterEdit, 2);

  tabL->addWidget(m_list,1);	

  topLayout->activate();

  connect(m_list, SIGNAL(selectionChanged(QListViewItem*)),this, SLOT(itemSelected(QListViewItem*)));
  connect(m_list, SIGNAL(expanded(QListViewItem*)), this, SLOT(itemExpand(QListViewItem*)));
  connect(filterEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotFilter(const QString&)));

  subList=new QStrList(true);
  subList->setAutoDelete(true);
  xTop->gManager()->getSubscribed(a, subList);
  allList=new QStrList(true);
  allList->setAutoDelete(true);
  a_ccount=a;
  QTimer::singleShot(2, this, SLOT(loadList()));
}

QString KNGroupListWidget::currentText() const
{
  if(m_list->currentItem())
    return m_list->currentItem()->text(0);

  return QString::null;
}

void KNGroupListWidget::itemSelected(QListViewItem* item)
{
  qDebug("itemSelected *%s*", item->text(0).latin1());

  if(!parent() || !static_cast<QCheckListItem*>(item)->isOn())
    return;

  emit itemSelected(item->text(0));
}

void KNGroupListWidget::itemExpand(QListViewItem* i)
{
  if(!i) return;

  qDebug("itemExpand");

  if(i->childCount()) {
    qDebug("has already been expanded, returning");
    return;
  }

  QCString gn(i->text(0).latin1());
  QCString filtertxt(filterEdit->text().local8Bit());
  int gnlen = gn.length();

  for(char *var=allList->first(); var; var=allList->next()) {
    char* p;

    if(!strstr(var, filtertxt.data()))
      continue;

    if(!strnicmp(var, gn.data(), gnlen)) {
      QCheckListItem* item = new QCheckListItem(i, QString(var), QCheckListItem::CheckBox);
      item->setOn(subList->contains(var));
    }
    else
      if(i->childCount())
        break;
  }
}

KNGroupListWidget::~KNGroupListWidget()
{
	delete allList;
	delete subList;
}


void KNGroupListWidget::loadList()
{
  KNFile f;
  f.setName(a_ccount->path()+"groups");
  QCString line;

  if(f.open(IO_ReadOnly)) {
    xTop->setStatusMsg(i18n("loading list of groups ..."));
    allList->clear();			

    while(!f.atEnd()) {
      line = f.readLine();

      allList->append(line.copy().data());

      if((allList->count() & 1023)==0)
        kapp->processEvents();
    }

    f.close();
    xTop->setStatusMsg();
    filterEdit->setText("");
    slotFilter("");
  }	
  else
    qWarning("unable to open %s, reason %d", f.name().latin1(), f.status());
}


void KNGroupListWidget::slotFilter(const QString& filter)
{
  qDebug("slotfilter");
  m_list->clear();

  QCString tlgroupname("");
  QCString filtertxt(filterEdit->text().local8Bit());
  int groupnamelen = tlgroupname.length();
  bool expandit = false;
  QListViewItem* previous = 0;

  for(char *var=allList->first(); var; var=allList->next()) {
    char* p;

    if(!strstr(var, filtertxt.data()))
      continue;

    if(!expandit || strnicmp(var, tlgroupname.data(), groupnamelen)) {
      if((p = strstr(var, "."))) {
        groupnamelen = p - var + 1;
        expandit = true;
      }
      else {
        groupnamelen = strlen(var);
        expandit = false;
      }

      tlgroupname = var;
      tlgroupname.truncate(groupnamelen);

      QListViewItem* i = new QListViewItem(m_list, previous);
      i->setExpandable(expandit);
      i->setText(0, tlgroupname);

      previous = i;
    }
  }
}



//--------------------------------

#include "kngrouplistwidget.moc"
