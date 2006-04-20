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

#include <qgroupbox.h>
#include <qlayout.h>
#include <q3vbox.h>
#include <qcheckbox.h>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QGridLayout>
#include <QComboBox>

#include <kcharsets.h>
#include <klocale.h>
#include <klineedit.h>
#include <kvbox.h>

#include "knglobals.h"
#include "knconfigmanager.h"
#include "knconfigwidgets.h"
#include "utilities.h"
#include "kngroup.h"
#include "kngrouppropdlg.h"
#include <qlabel.h>


KNGroupPropDlg::KNGroupPropDlg( KNGroup *group, QWidget *parent )
  : KDialogBase( Tabbed, i18n("Properties of %1", group->groupname()),
                 Ok|Cancel|Help, Ok, parent ),
    g_rp(group), n_ickChanged(false)
{

  // General tab ===============================================

  QWidget *page = addPage(i18n("&General"));
  QVBoxLayout *pageL = new QVBoxLayout(page, 3);

  // settings
  QGroupBox *gb = new QGroupBox(i18n("Settings"), page);
  pageL->addWidget(gb);
  QGridLayout *grpL=new QGridLayout(gb, 3, 3, 15, 5);

  grpL->addRowSpacing(0, fontMetrics().lineSpacing()-9);

  n_ick=new KLineEdit(gb);
  if (g_rp->hasName())
    n_ick->setText(g_rp->name());
  QLabel *l=new QLabel(n_ick, i18n("&Nickname:"), gb);
  grpL->addWidget(l,1,0);
  grpL->addWidget(n_ick,1,1, 1,2);

  u_seCharset=new QCheckBox(i18n("&Use different default charset:"), gb);
  u_seCharset->setChecked(g_rp->useCharset());
  grpL->addWidget(u_seCharset,2,0, 1, 2 );

  c_harset=new QComboBox(false, gb);
  c_harset->insertStringList( KGlobal::charsets()->availableEncodingNames() );
  c_harset->setCurrentIndex( c_harset->findText( g_rp->defaultCharset().toLower() ) );
  c_harset->setEnabled(g_rp->useCharset());
  connect(u_seCharset, SIGNAL(toggled(bool)), c_harset, SLOT(setEnabled(bool)));
  grpL->addWidget(c_harset, 2,2);

  grpL->setColumnStretch(1,1);
  grpL->setColumnStretch(2,2);

  // group name & description
  gb = new QGroupBox(i18n("Description"), page);
  pageL->addWidget(gb);
  grpL=new QGridLayout(gb, 4, 3, 15, 5);

  grpL->addRowSpacing(0, fontMetrics().lineSpacing()-9);

  l=new QLabel(i18n("Name:"), gb);
  grpL->addWidget(l,1,0);
  l=new QLabel(group->groupname(),gb);
  grpL->addWidget(l,1,2);

  l=new QLabel(i18n("Description:"), gb);
  grpL->addWidget(l,2,0);
  l=new QLabel(g_rp->description(),gb);
  grpL->addWidget(l,2,2);

  l=new QLabel(i18n("Status:"), gb);
  grpL->addWidget(l,3,0);
  QString status;
  switch (g_rp->status()) {
    case KNGroup::unknown:  status=i18n("unknown");
                            break;
    case KNGroup::readOnly: status=i18n("posting forbidden");
                            break;
    case KNGroup::postingAllowed:  status=i18n("posting allowed");
                                   break;
    case KNGroup::moderated:       status=i18n("moderated");
                                   break;
  }
  l=new QLabel(status,gb);
  grpL->addWidget(l,3,2);

  grpL->addColSpacing(1,20);
  grpL->setColumnStretch(2,1);

  // statistics
  gb = new QGroupBox(i18n("Statistics"), page);
  pageL->addWidget(gb);
  grpL=new QGridLayout(gb, 6, 3, 15, 5);

  grpL->addRowSpacing(0, fontMetrics().lineSpacing()-9);

  l=new QLabel(i18n("Articles:"), gb);
  grpL->addWidget(l,1,0);
  l=new QLabel(QString::number(g_rp->count()),gb);
  grpL->addWidget(l,1,2);

  l=new QLabel(i18n("Unread articles:"), gb);
  grpL->addWidget(l,2,0);
  l=new QLabel(QString::number(g_rp->count()-g_rp->readCount()),gb);
  grpL->addWidget(l,2,2);

  l=new QLabel(i18n("New articles:"), gb);
  grpL->addWidget(l,3,0);
  l=new QLabel(QString::number(g_rp->newCount()),gb);
  grpL->addWidget(l,3,2);

  l=new QLabel(i18n("Threads with unread articles:"), gb);
  grpL->addWidget(l,4,0);
  l=new QLabel(QString::number(g_rp->statThrWithUnread()),gb);
  grpL->addWidget(l,4,2);

  l=new QLabel(i18n("Threads with new articles:"), gb);
  grpL->addWidget(l,5,0);
  l=new QLabel(QString::number(g_rp->statThrWithNew()),gb);
  grpL->addWidget(l,5,2);

  grpL->addColSpacing(1,20);
  grpL->setColumnStretch(2,1);

  pageL->addStretch(1);

  // Specfic Identity tab =========================================
  i_dWidget = new KNode::IdentityWidget( g_rp->identity(), knGlobals.instance(), addVBoxPage( i18n("&Identity") ) );

  // per server cleanup configuration
  QFrame* cleanupPage = addPage( i18n("&Cleanup") );
  QVBoxLayout *cleanupLayout = new QVBoxLayout( cleanupPage, KDialog::spacingHint() );
  mCleanupWidget = new KNode::GroupCleanupWidget( g_rp->cleanupConfig(), cleanupPage );
  mCleanupWidget->load();
  cleanupLayout->addWidget( mCleanupWidget );
  cleanupLayout->addStretch( 1 );

  KNHelper::restoreWindowSize("groupPropDLG", this, sizeHint());
}



KNGroupPropDlg::~KNGroupPropDlg()
{
  KNHelper::saveWindowSize("groupPropDLG", size());
}



void KNGroupPropDlg::slotOk()
{
  if( !(g_rp->name()==n_ick->text()) ) {
    g_rp->setName(n_ick->text());
    n_ickChanged=true;
  }

  i_dWidget->save();
  mCleanupWidget->save();

  g_rp->setUseCharset(u_seCharset->isChecked());
  g_rp->setDefaultCharset(c_harset->currentText().toLatin1());

  accept();
}
