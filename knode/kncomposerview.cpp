/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2007 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/
#include <QFile>
#include <QGridLayout>
#include <klocale.h>
#include <QPushButton>
#include <kmeditor.h>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <Q3Header>
#include <KComboBox>
#include <KTemporaryFile>
#include <QApplication>
#include "knglobals.h"
#include "kncomposer.h"
#include "kncomposereditor.h"
#include "kncomposerview.h"
#include "kncomposerview.moc"

//=====================================================================================


KNComposer::ComposerView::ComposerView( KNComposer *composer )
  : QSplitter( Qt::Vertical, composer ), a_ttWidget(0), a_ttView(0), v_iewOpen(false)
{
  QWidget *main=new QWidget(this);

  //headers
  QFrame *hdrFrame=new QFrame(main);
  hdrFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
  QGridLayout *hdrL=new QGridLayout(hdrFrame);
  hdrL->setSpacing(5);
  hdrL->setMargin(7);
  hdrL->setColumnStretch(1,1);

  //To
  t_o=new KNLineEdit(this, true, hdrFrame);
  mEdtList.append(t_o);

  l_to=new QLabel(i18n("T&o:"),hdrFrame);
  l_to->setBuddy(t_o);
  t_oBtn=new QPushButton(i18n("&Browse..."), hdrFrame);
  hdrL->addWidget(l_to, 0,0);
  hdrL->addWidget(t_o, 0,1);
  hdrL->addWidget(t_oBtn, 0,2);
  connect(t_oBtn, SIGNAL(clicked()), parent(), SLOT(slotToBtnClicked()));

  //Newsgroups
  g_roups=new KNLineEdit(this, false, hdrFrame);
  mEdtList.append(g_roups);

  l_groups=new QLabel(i18n("&Groups:"),hdrFrame);
  l_groups->setBuddy(g_roups);
  g_roupsBtn=new QPushButton(i18n("B&rowse..."), hdrFrame);
  hdrL->addWidget(l_groups, 1,0);
  hdrL->addWidget(g_roups, 1,1);
  hdrL->addWidget(g_roupsBtn, 1,2);
  connect(g_roups, SIGNAL(textChanged(const QString&)),
          parent(), SLOT(slotGroupsChanged(const QString&)));
  connect(g_roupsBtn, SIGNAL(clicked()), parent(), SLOT(slotGroupsBtnClicked()));

  //Followup-To
  f_up2=new KComboBox(true, hdrFrame);
  l_fup2=new QLabel(i18n("Follo&wup-To:"),hdrFrame);
  l_fup2->setBuddy(f_up2);
  hdrL->addWidget(l_fup2, 2,0);
  hdrL->addWidget(f_up2, 2, 1, 1,2);

  //subject
  s_ubject=new KNLineEditSpell(this, false, hdrFrame);
  mEdtList.append(s_ubject);

  QLabel *l=new QLabel(i18n("S&ubject:"),hdrFrame);
  l->setBuddy(s_ubject);
  hdrL->addWidget(l, 3,0);
  hdrL->addWidget(s_ubject, 3, 1, 1,2);
  connect(s_ubject, SIGNAL(textChanged(const QString&)),
          parent(), SLOT(slotSubjectChanged(const QString&)));

  //Editor
  e_dit=new KNComposerEditor(this);
  e_dit->switchToPlainText();
  e_dit->setMinimumHeight(50);
  e_dit->setFont(KGlobalSettings::fixedFont());

  QVBoxLayout *notL=new QVBoxLayout(e_dit);
  notL->addStretch(1);
  n_otification=new QGroupBox(e_dit);
  QHBoxLayout *v2 = new QHBoxLayout( n_otification );
  l=new QLabel(i18n("You are currently editing the article body\nin an external editor. To continue, you have\nto close the external editor."), n_otification);
  c_ancelEditorBtn=new QPushButton(i18n("&Kill External Editor"), n_otification);
  v2->addWidget(l);
  v2->addWidget(c_ancelEditorBtn);
#ifdef __GNUC__
#warning Port me?
#endif
//   n_otification->setFrameStyle(Q3Frame::Panel | Q3Frame::Raised);
//   n_otification->setLineWidth(2);
  n_otification->hide();
  notL->addWidget(n_otification, 0, Qt::AlignHCenter);
  notL->addStretch(1);

  //finish GUI
  QVBoxLayout *topL=new QVBoxLayout(main);
  topL->setSpacing(4);
  topL->setMargin(4);
  topL->addWidget(hdrFrame);
  topL->addWidget(e_dit, 1);
}


KNComposer::ComposerView::~ComposerView()
{
  if(v_iewOpen) {
    KConfigGroup conf( knGlobals.config(), "POSTNEWS");

    conf.writeEntry("Att_Splitter",sizes());   // save splitter pos

    QList<int> lst;                        // save header sizes
    Q3Header *h=a_ttView->header();
    for (int i=0; i<5; i++)
      lst << h->sectionSize(i);
    conf.writeEntry("Att_Headers",lst);
  }
}


void KNComposer::ComposerView::focusNextPrevEdit(const QWidget* aCur, bool aNext)
{
  QList<QWidget*>::Iterator it;

  if ( !aCur ) {
    it = --( mEdtList.end() );
  } else {
    for ( QList<QWidget*>::Iterator it2 = mEdtList.begin(); it2 != mEdtList.end(); ++it2 ) {
      if ( (*it2) == aCur ) {
        it = it2;
        break;
      }
    }
    if ( it == mEdtList.end() )
      return;
    if ( aNext )
      ++it;
    else {
      if ( it != mEdtList.begin() )
        --it;
      else
        return;
    }
  }
  if ( it != mEdtList.end() ) {
    if ( (*it)->isVisible() )
      (*it)->setFocus();
  } else if ( aNext )
    e_dit->setFocus();
}


void KNComposer::ComposerView::setMessageMode(KNComposer::MessageMode mode)
{
  if (mode != KNComposer::news) {
    l_to->show();
    t_o->show();
    t_oBtn->show();
  } else {
    l_to->hide();
    t_o->hide();
    t_oBtn->hide();
  }
  if (mode != KNComposer::mail) {
    l_groups->show();
    l_fup2->show();
    g_roups->show();
    f_up2->show();
    g_roupsBtn->show();

  } else {
    l_groups->hide();
    l_fup2->hide();
    g_roups->hide();
    f_up2->hide();
    g_roupsBtn->hide();
  }
}


void KNComposer::ComposerView::showAttachmentView()
{
  if(!a_ttWidget) {
    a_ttWidget=new QWidget(this);
    QGridLayout *topL=new QGridLayout(a_ttWidget);
    topL->setSpacing(4);
    topL->setMargin(4);

    a_ttView=new AttachmentView(a_ttWidget);
    topL->addWidget(a_ttView, 0, 0, 3, 1);

    //connections
    connect(a_ttView, SIGNAL(currentChanged(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentSelected(Q3ListViewItem*)));
    connect(a_ttView, SIGNAL(clicked ( Q3ListViewItem * )),
            parent(), SLOT(slotAttachmentSelected(Q3ListViewItem*)));

    connect(a_ttView, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)),
            parent(), SLOT(slotAttachmentPopup(K3ListView*, Q3ListViewItem*, const QPoint&)));
    connect(a_ttView, SIGNAL(delPressed(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentRemove(Q3ListViewItem*)));
    connect(a_ttView, SIGNAL(doubleClicked(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(Q3ListViewItem*)));
    connect(a_ttView, SIGNAL(returnPressed(Q3ListViewItem*)),
            parent(), SLOT(slotAttachmentEdit(Q3ListViewItem*)));

    //buttons
    a_ttAddBtn=new QPushButton(i18n("A&dd..."),a_ttWidget);
    connect(a_ttAddBtn, SIGNAL(clicked()), parent(), SLOT(slotAttachFile()));
    topL->addWidget(a_ttAddBtn, 0,1);

    a_ttRemoveBtn=new QPushButton(i18n("&Remove"), a_ttWidget);
    a_ttRemoveBtn->setEnabled(false);
    connect(a_ttRemoveBtn, SIGNAL(clicked()), parent(), SLOT(slotRemoveAttachment()));
    topL->addWidget(a_ttRemoveBtn, 1,1);

    a_ttEditBtn=new QPushButton(i18n("&Properties"), a_ttWidget);
    a_ttEditBtn->setEnabled(false);
    connect(a_ttEditBtn, SIGNAL(clicked()), parent(), SLOT(slotAttachmentProperties()));
    topL->addWidget(a_ttEditBtn, 2,1, Qt::AlignTop);

    topL->setRowStretch(2,1);
    topL->setColumnStretch(0,1);
  }

  if(!v_iewOpen) {
    v_iewOpen=true;
    a_ttWidget->show();

    KConfigGroup conf(knGlobals.config(), "POSTNEWS");

    QList<int> lst = conf.readEntry("Att_Splitter",QList<int>());
    if(lst.count()!=2)
      lst << 267 << 112;
    setSizes(lst);

    lst=conf.readEntry("Att_Headers",QList<int>());
    if(lst.count()==5) {
      QList<int>::Iterator it = lst.begin();

      Q3Header *h=a_ttView->header();
      for(int i=0; i<5; i++) {
        h->resizeSection(i,(*it));
        ++it;
      }
    }
  }
}


void KNComposer::ComposerView::hideAttachmentView()
{
  if(v_iewOpen) {
    a_ttWidget->hide();
    v_iewOpen=false;
  }
}


void KNComposer::ComposerView::showExternalNotification()
{
  e_dit->setReadOnly(true);
  n_otification->show();
}


void KNComposer::ComposerView::hideExternalNotification()
{
  e_dit->setReadOnly(false);
  n_otification->hide();
}
