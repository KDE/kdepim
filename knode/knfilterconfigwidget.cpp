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

#include <QLabel>

#include <QVBoxLayout>

#include <klocale.h>

#include "knstringfilter.h"
#include "knstatusfilter.h"
#include "knrangefilter.h"
#include "knfilterconfigwidget.h"

using namespace KNode;

KNFilterConfigWidget::KNFilterConfigWidget( QWidget *parent ) :
  QTabWidget( parent )
{
  QWidget *sf, *idW, *add;
  sf=new QWidget(this);
  QVBoxLayout *sfL=new QVBoxLayout(sf);
  sfL->setSpacing(5);
  sfL->setMargin(8);
  subject = new StringFilterWidget( i18n("Subject"), sf );
  sfL->addWidget(subject);
  from = new StringFilterWidget( i18n("From"), sf );
  sfL->addWidget(from);
  QLabel *l = new QLabel(i18n("The following placeholders are supported:\n%MYNAME=own name, %MYEMAIL=own email address"),sf);
  sfL->addWidget(l);
  sfL->addStretch(1);
  addTab(sf, i18n("Subject && &From"));

  idW=new QWidget(this);
  QVBoxLayout *idL=new QVBoxLayout(idW);
  idL->setSpacing(5);
  idL->setMargin(8);
  messageId = new StringFilterWidget( i18n("Message-ID"), idW );
  idL->addWidget(messageId);
  references = new StringFilterWidget( i18n("References"), idW );
  idL->addWidget(references);
  idL->addStretch(1);
  addTab(idW, i18n("M&essage-IDs"));

  status = new StatusFilterWidget( this );
  addTab(status, i18n("&Status"));

  add=new QWidget(this);
  QVBoxLayout *addL=new QVBoxLayout(add);
  addL->setSpacing(5);
  addL->setMargin(8);
  score = new RangeFilterWidget( i18n("Score"), -99999, 99999, add );
  addL->addWidget(score);
  age = new RangeFilterWidget( i18n("Age"), 0, 999, add, i18n(" days") );
  addL->addWidget(age);
  lines = new RangeFilterWidget( i18n("Lines"), 0, 99999, add );
  addL->addWidget(lines);
  addL->addStretch(1);
  addTab(add, i18n("&Additional"));
}


KNFilterConfigWidget::~KNFilterConfigWidget()
{
}


void KNFilterConfigWidget::reset()
{
  from->clear();
  subject->clear();
  messageId->clear();
  references->clear();
  age->clear();
  lines->clear();
  score->clear();
  status->clear();
}


void KNFilterConfigWidget::setStartFocus()
{
  subject->setStartFocus();
}

// -----------------------------------------------------------------------------

