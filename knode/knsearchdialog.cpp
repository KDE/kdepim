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

#include <QCheckBox>

#include <klocale.h>
#include <kiconloader.h>
#include <kvbox.h>

#include "knfilterconfigwidget.h"
#include "knarticlefilter.h"
#include "utilities.h"
#include "knsearchdialog.h"

using namespace KNode;

KNode::SearchDialog::SearchDialog( searchType type, QWidget *parent ) :
  KDialog( parent )
{
  Q_UNUSED( type );
  setCaption( i18n("Find Articles") );
  setButtons( User1 | User2 | Close );
  setModal( false );
  setDefaultButton( User1 );

  setWindowIcon( KIcon("knode") );
  setButtonsOrientation( Qt::Vertical );
  setButtonGuiItem( User1, KGuiItem( i18n("&Search"), "edit-find" ) );
  setButtonGuiItem( User2, KGuiItem( i18n("C&lear"), "edit-clear" ) );

  connect( this, SIGNAL(closeClicked()), SLOT(slotClose()) );
  connect( this, SIGNAL(user1Clicked()), SLOT(slotUser1()) );
  connect( this, SIGNAL(user2Clicked()), SLOT(slotUser2()) );

  KVBox *filterBox = new KVBox( this );
  setMainWidget( filterBox );

  completeThreads = new QCheckBox( i18n("Sho&w complete threads"), filterBox );
  fcw = new KNFilterConfigWidget( filterBox );
  fcw->reset();

  f_ilter = new KNArticleFilter();
  f_ilter->setLoaded( true );
  f_ilter->setSearchFilter( true );

  setFixedHeight( sizeHint().height() );
  KNHelper::restoreWindowSize( "searchDlg", this, sizeHint() );
  fcw->setStartFocus();
}



KNode::SearchDialog::~SearchDialog()
{
  delete f_ilter;
  KNHelper::saveWindowSize("searchDlg", size());
}


void KNode::SearchDialog::slotUser1()
{
  f_ilter->status=fcw->status->filter();
  f_ilter->score=fcw->score->filter();
  f_ilter->age=fcw->age->filter();
  f_ilter->lines=fcw->lines->filter();
  f_ilter->subject=fcw->subject->filter();
  f_ilter->from=fcw->from->filter();
  f_ilter->messageId=fcw->messageId->filter();
  f_ilter->references=fcw->references->filter();
  f_ilter->setApplyOn(completeThreads->isChecked()? 1:0);
  emit doSearch(f_ilter);
}



void KNode::SearchDialog::slotUser2()
{
  fcw->reset();
}



void KNode::SearchDialog::slotClose()
{
  emit dialogDone();
}


void KNode::SearchDialog::closeEvent( QCloseEvent * )
{
  emit dialogDone();
}


