/*  -*- c++ -*-

  kwidgetlister.cpp

  This file is part of libkdepim.
  Copyright (c) 2001 Marc Mutz <mutz@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License,
  version 2, as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

  In addition, as a special exception, the copyright holders give
  permission to link the code of this library with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "kwidgetlister.h"

#include <KDebug>
#include <KDialog>
#include <KLocale>
#include <KGuiItem>
#include <KHBox>
#include <KPushButton>

#include <QPushButton>
#include <QVBoxLayout>

#include <assert.h>

using namespace KPIM;

class KWidgetLister::Private
{
  public:
    Private( KWidgetLister *qq )
      : q( qq ),
        mBtnMore( 0 ),
        mBtnFewer( 0 ),
        mBtnClear( 0 ),
        mLayout( 0 ),
        mButtonBox( 0 ),
        mMinWidgets( 0 ),
        mMaxWidgets( 0 )

      
    {
    }

    ~Private()
    {
      qDeleteAll( mWidgetList );
      mWidgetList.clear();
    }

    void enableControls();

    KWidgetLister *q;
    QPushButton *mBtnMore, *mBtnFewer, *mBtnClear;
    QVBoxLayout *mLayout;
    KHBox       *mButtonBox;
    QList<QWidget*> mWidgetList;
    int mMinWidgets;
    int mMaxWidgets;
};

void KWidgetLister::Private::enableControls()
{
  const int count = mWidgetList.count();
  const bool isMaxWidgets = ( count >= mMaxWidgets );
  const bool isMinWidgets = ( count <= mMinWidgets );
  if ( mBtnMore )
    mBtnMore->setEnabled( !isMaxWidgets );
  if ( mBtnFewer )
    mBtnFewer->setEnabled( !isMinWidgets );
}

KWidgetLister::KWidgetLister( bool fewerMoreButton, int minWidgets, int maxWidgets, QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  d->mMinWidgets = qMax( minWidgets, 1 );
  d->mMaxWidgets = qMax( maxWidgets, d->mMinWidgets + 1 );
  init( fewerMoreButton );
}

KWidgetLister::KWidgetLister( int minWidgets, int maxWidgets, QWidget *parent )
  : QWidget( parent ), d( new Private( this ) )
{
  d->mMinWidgets = qMax( minWidgets, 1 );
  d->mMaxWidgets = qMax( maxWidgets, d->mMinWidgets + 1 );
  init();
}

KWidgetLister::~KWidgetLister()
{
  delete d;
}

void KWidgetLister::init( bool fewerMoreButton )
{
    //--------- the button box
  d->mLayout = new QVBoxLayout( this );
  d->mLayout->setMargin( 0 );
  d->mLayout->setSpacing( 4 );

  d->mButtonBox = new KHBox( this );
  d->mButtonBox->setSpacing( KDialog::spacingHint() );
  d->mLayout->addWidget( d->mButtonBox );

  if ( fewerMoreButton )
  {
    d->mBtnMore = new KPushButton( KGuiItem( i18nc( "more widgets", "More" ),
                                           QLatin1String("list-add") ), d->mButtonBox );
    d->mButtonBox->setStretchFactor( d->mBtnMore, 0 );

    d->mBtnFewer = new KPushButton( KGuiItem( i18nc( "fewer widgets", "Fewer" ),
                                            QLatin1String("list-remove") ), d->mButtonBox );
    d->mButtonBox->setStretchFactor( d->mBtnFewer, 0 );
  }
  QWidget *spacer = new QWidget( d->mButtonBox );
  d->mButtonBox->setStretchFactor( spacer, 1 );

  d->mBtnClear = new KPushButton( KStandardGuiItem::clear(), d->mButtonBox );
  // FIXME a useful whats this. KStandardGuiItem::clear() returns a text with an edit box
  d->mBtnClear->setWhatsThis( QString() );
  d->mButtonBox->setStretchFactor( d->mBtnClear, 0 );

  //---------- connect everything
  if ( fewerMoreButton )
  {
    connect( d->mBtnMore, SIGNAL(clicked()),
             this, SLOT(slotMore()) );
    connect( d->mBtnFewer, SIGNAL(clicked()),
             this, SLOT(slotFewer()) );
  }
  
  connect( d->mBtnClear, SIGNAL(clicked()),
           this, SLOT(slotClear()) );

  d->enableControls();

}

void KWidgetLister::slotMore()
{
  // the class should make certain that slotMore can't
  // be called when mMaxWidgets are on screen.
  assert( (int)d->mWidgetList.count() < d->mMaxWidgets );

  addWidgetAtEnd();
  //  adjustSize();
  d->enableControls();
}

void KWidgetLister::slotFewer()
{
  // the class should make certain that slotFewer can't
  // be called when mMinWidgets are on screen.
  assert( (int)d->mWidgetList.count() > d->mMinWidgets );

  removeLastWidget();
  //  adjustSize();
  d->enableControls();
}

void KWidgetLister::slotClear()
{
  setNumberOfShownWidgetsTo( d->mMinWidgets );

  // clear remaining widgets
  foreach ( QWidget *widget, d->mWidgetList )
    clearWidget( widget );

  //  adjustSize();
  d->enableControls();
  emit clearWidgets();
}

void KWidgetLister::addWidgetAtEnd( QWidget *widget )
{
  if ( !widget )
    widget = this->createWidget( this );

  d->mLayout->insertWidget( d->mLayout->indexOf( d->mButtonBox ), widget );
  d->mWidgetList.append( widget );
  widget->show();

  d->enableControls();
  emit widgetAdded();
  emit widgetAdded( widget );
}

void KWidgetLister::removeLastWidget()
{
  // The layout will take care that the
  // widget is removed from screen, too.
  delete d->mWidgetList.takeLast();
  d->enableControls();
  emit widgetRemoved();
}

void KWidgetLister::clearWidget( QWidget *widget )
{
  Q_UNUSED( widget );
}

QWidget *KWidgetLister::createWidget( QWidget *parent )
{
  return new QWidget( parent );
}

void KWidgetLister::setNumberOfShownWidgetsTo( int aNum )
{
  int superfluousWidgets = qMax( (int)d->mWidgetList.count() - aNum, 0 );
  int missingWidgets     = qMax( aNum - (int)d->mWidgetList.count(), 0 );

  // remove superfluous widgets
  for ( ; superfluousWidgets ; superfluousWidgets-- ) {
    removeLastWidget();
  }

  // add missing widgets
  for ( ; missingWidgets ; missingWidgets-- ) {
    addWidgetAtEnd();
  }
}

QList<QWidget*> KWidgetLister::widgets() const
{
  return d->mWidgetList;
}

int KWidgetLister::widgetsMinimum() const
{
  return d->mMinWidgets;
}

int KWidgetLister::widgetsMaximum() const
{
  return d->mMaxWidgets;
}

void KWidgetLister::removeWidget(QWidget*widget)
{
  // The layout will take care that the
  // widget is removed from screen, too.

  if ( d->mWidgetList.count()  <= widgetsMinimum() )
    return;
  
  const int index = d->mWidgetList.indexOf( widget );  
  QWidget* w =  d->mWidgetList.takeAt(index);
  w->deleteLater();
  w = 0;
  d->enableControls();
  emit widgetRemoved( widget );
  emit widgetRemoved();

}

void KWidgetLister::addWidgetAfterThisWidget(QWidget*currentWidget, QWidget* widget)
{
  if ( !widget )
    widget = this->createWidget( this );

  int index = d->mLayout->indexOf( currentWidget ? currentWidget :  d->mButtonBox )+1;
  d->mLayout->insertWidget( index, widget );
  index = 0;
  if (currentWidget) {
      index = d->mWidgetList.indexOf(currentWidget);
      d->mWidgetList.insert(index+1, widget);
  } else {
      d->mWidgetList.append(widget);
  }
  widget->show();

  d->enableControls();
  emit widgetAdded();
  emit widgetAdded( widget );  
}


