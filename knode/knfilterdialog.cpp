/*
    knfilterdialog.cpp

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

#include <qlabel.h>
#include <qcheckbox.h>
#include <qlayout.h>

#include <klocale.h>
#include <kmessagebox.h>
#include <klineedit.h>

#include "knglobals.h"
#include "knfiltermanager.h"
#include "knfilterconfigwidget.h"
#include "knarticlefilter.h"
#include "utilities.h"
#include "knfilterdialog.h"


KNFilterDialog::KNFilterDialog(KNArticleFilter *f, QWidget *parent, const char *name)
  : KDialogBase(Plain, (f->id()==-1)? i18n("New Filter"):i18n("Properties of %1").arg(f->name()),
                Ok|Cancel|Help, Ok, parent, name),
    fltr(f)
{
  QFrame* page=plainPage();

  QGroupBox *gb=new QGroupBox(page);
  fname=new KLineEdit(gb);
  QLabel *l1=new QLabel(fname, i18n("Na&me:"), gb);
  apon=new QComboBox(gb);
  apon->insertItem(i18n("Single Articles"));
  apon->insertItem(i18n("Whole Threads"));
  QLabel *l2=new QLabel(apon, i18n("Apply o&n:"), gb);
  enabled=new QCheckBox(i18n("Sho&w in menu"), gb);

  fw=new KNFilterConfigWidget(page);

  QGridLayout *gbL=new QGridLayout(gb, 2,4,8,5);
  gbL->addWidget(l1, 0,0);
  gbL->addMultiCellWidget(fname, 0,0,1,3);
  gbL->addWidget(enabled, 1,0);
  gbL->addWidget(l2, 1,2);
  gbL->addWidget(apon, 1,3);
  gbL->setColStretch(1,1);

  QVBoxLayout *topL=new QVBoxLayout(page,0,5);

  topL->addWidget(gb);
  topL->addWidget(fw,1);

  enabled->setChecked(f->isEnabled());
  apon->setCurrentItem((int) f->applyOn());
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
  connect( fname,  SIGNAL( textChanged ( const QString & )), this, SLOT( slotTextChanged( const QString & )));
  slotTextChanged( fname->text() );
}



KNFilterDialog::~KNFilterDialog()
{
  KNHelper::saveWindowSize("filterDLG", size());
}

void KNFilterDialog::slotTextChanged( const QString &_text )
{
    enableButtonOK( !_text.isEmpty() );
}

void KNFilterDialog::slotOk()
{
  if (fname->text().isEmpty())
    KMessageBox::sorry(this, i18n("Please provide a name for this filter."));
  else
    if (!knGlobals.filManager->newNameIsOK(fltr,fname->text()))
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
      fltr->setApplyOn(apon->currentItem());

      accept();
    }
}



//--------------------------------

#include "knfilterdialog.moc"
