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
#include <QCheckBox>

#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include "knglobals.h"
#include "knfiltermanager.h"
#include "knfilterconfigwidget.h"
#include "knarticlefilter.h"
#include "utilities.h"
#include "knfilterdialog.h"


KNFilterDialog::KNFilterDialog( KNArticleFilter *f, QWidget *parent )
  : KDialog( parent ),
    fltr(f)
{
  if ( f->id() == -1 )
    setCaption( i18n("New Filter") );
  else
    setCaption( i18n("Properties of %1", f->name()) );
  setButtons( Ok|Cancel|Help );
  setDefaultButton( Ok );

  QFrame* page = new QFrame( this );
  setMainWidget( page );

  QGroupBox *gb = new QGroupBox( page );
  fname=new KLineEdit(gb);
  QLabel *l1=new QLabel(i18n("Na&me:"),gb);
  l1->setBuddy(fname);
  apon=new QComboBox(gb);
  apon->addItem(i18n("Single Articles"));
  apon->addItem(i18n("Whole Threads"));
  QLabel *l2=new QLabel(i18n("Apply o&n:"),gb);
  l2->setBuddy(apon);
  enabled=new QCheckBox(i18n("Sho&w in menu"), gb);

  fw=new KNFilterConfigWidget(page);

  QGridLayout *gbL=new QGridLayout(gb);
  gbL->setSpacing(5);
  gbL->setMargin(8);
  gbL->addWidget(l1, 0,0);
  gbL->addWidget(fname, 0,1, 1,3);
  gbL->addWidget(enabled, 1,0);
  gbL->addWidget(l2, 1,2);
  gbL->addWidget(apon, 1,3);
  gbL->setColumnStretch(1,1);

  QVBoxLayout *topL=new QVBoxLayout(page);
  topL->setSpacing(5);
  topL->setMargin(0);

  topL->addWidget(gb);
  topL->addWidget(fw,1);

  enabled->setChecked(f->isEnabled());
  apon->setCurrentIndex((int) f->applyOn());
  fname->setText(f->translatedName());

  fw->status->setFilter(f->status);
  fw->lines->setFilter(f->lines);
  fw->age->setFilter(f->age);
  fw->score->setFilter(f->score);
  fw->subject->setFilter(f->subject);
  fw->from->setFilter(f->from);
  fw->messageId->setFilter(f->messageId);
  fw->references->setFilter(f->references);

  setFixedHeight(sizeHint().height());
  KNHelper::restoreWindowSize("filterDLG", this, sizeHint());

  setHelp("anc-using-filters");
  connect( fname,  SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
  connect( this, SIGNAL(okClicked()),this,SLOT(slotOk()));
  slotTextChanged( fname->text() );
}



KNFilterDialog::~KNFilterDialog()
{
  KNHelper::saveWindowSize("filterDLG", size());
}

void KNFilterDialog::slotTextChanged( const QString &_text )
{
    enableButtonOk( !_text.isEmpty() );
}

void KNFilterDialog::slotOk()
{
  if (fname->text().isEmpty())
    KMessageBox::sorry(this, i18n("Please provide a name for this filter."));
  else
    if (!knGlobals.filterManager()->newNameIsOK(fltr,fname->text()))
      KMessageBox::sorry(this, i18n("A filter with this name exists already.\nPlease choose a different name."));
    else {
      fltr->setTranslatedName(fname->text());
      fltr->setEnabled(enabled->isChecked());
      fltr->status=fw->status->filter();
      fltr->score=fw->score->filter();
      fltr->age=fw->age->filter();
      fltr->lines=fw->lines->filter();
      fltr->subject=fw->subject->filter();
      fltr->from=fw->from->filter();
      fltr->messageId=fw->messageId->filter();
      fltr->references=fw->references->filter();
      fltr->setApplyOn(apon->currentIndex());

      accept();
    }
}



//--------------------------------

#include "knfilterdialog.moc"
