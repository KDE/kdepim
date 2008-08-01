/*
   This file is part of KAddressBook.
   Copyright (C) 2007 Mathias Soeken <msoeken@tzi.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
#include "comboboxheaderview.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QEvent>
#include <QtGui/QComboBox>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QTableWidget>

class ComboBoxHeaderView::ComboBox : public QComboBox {
  friend class ComboBoxHeaderView;

  public:
    ComboBox( int index, ComboBoxHeaderView *parent ) 
      : QComboBox( parent ), mIndex( index ), mParent( parent ) 
    {
    }

  protected:
    virtual void enterEvent( QEvent * ) {
      setCurrentIndex(
        findText( 
          mParent->model()->headerData( mIndex, 
                                        mParent->orientation() ).toString() ) );

      mParent->setCurrentIndex( mIndex );
    }

  private:
    int mIndex;
    ComboBoxHeaderView *mParent;
};

//@cond PRIVATE
class ComboBoxHeaderView::Private {
  public:
    Private() :
      mComboBox( 0 ),
      mCurrentIndex( -1 ),
      mHoverStyle( true ),
      mMargin( 1 ) {}

    QStringList mItems;

    QComboBox *mComboBox;
    QList<ComboBox*> mBoxes;

    int mCurrentIndex;

    bool mHoverStyle;
    int mMargin;
};
//@endcond

ComboBoxHeaderView::ComboBoxHeaderView( QStringList items,
                                        QTableWidget *parent,
                                        bool hoverStyle ) :
  QHeaderView( Qt::Horizontal, parent ), d( new Private() )
{
  d->mItems = items;
  d->mHoverStyle = hoverStyle;

  initialize();

  connect( this, SIGNAL( sectionCountChanged( int, int ) ),
           this, SLOT( initialize() ) );
  connect( this, SIGNAL( sectionResized( int, int, int ) ),
           this, SLOT( initialize() ) );
}

ComboBoxHeaderView::~ComboBoxHeaderView()
{
  delete d;
}

QString ComboBoxHeaderView::headerLabel( int logicalIndex ) const
{
  return model()->headerData( logicalIndex, orientation() ).toString();
}

QStringList ComboBoxHeaderView::items() const
{
  return d->mItems;
}

int ComboBoxHeaderView::margin() const 
{
  return d->mMargin;
}

void ComboBoxHeaderView::setMargin( int margin)
{
  d->mMargin = margin;

  if ( !( d->mHoverStyle ) ) {
    initialize();
  }
}

int ComboBoxHeaderView::indexOfHeaderLabel( int logicalIndex ) const
{
  return d->mItems.indexOf( headerLabel( logicalIndex ) );
}

QString ComboBoxHeaderView::valueOfHeaderLabel( int logicalIndex ) const
{
  return d->mItems[ indexOfHeaderLabel( logicalIndex ) ];
}

QRect ComboBoxHeaderView::sectionRect( int logicalIndex ) const
{
  return QRect( sectionPosition( logicalIndex ) + d->mMargin, 0, 
                sectionSize( logicalIndex ) - 2 * d->mMargin, height() );
}

void ComboBoxHeaderView::adjustComboBoxIndex( QComboBox *comboBox, 
                                              int logicalIndex )
{
  comboBox->setCurrentIndex(
    comboBox->findText( 
      model()->headerData( logicalIndex, orientation() ).toString() ) );
}

void ComboBoxHeaderView::adjustComboBoxIndex( int logicalIndex )
{
  if ( d->mHoverStyle ) {
    adjustComboBoxIndex( d->mComboBox, logicalIndex );
  } else {
    adjustComboBoxIndex( static_cast< QComboBox* >( d->mBoxes[ logicalIndex ] ), 
                         logicalIndex );
  }
}

bool ComboBoxHeaderView::isViewVisible() const
{
  if ( d->mHoverStyle ) {
    if ( d->mComboBox->view()->isVisible() ) {
      return true;
    }
  } else {
    Q_FOREACH ( ComboBox *box, d->mBoxes ) {
      if ( box->view()->isVisible() ) {
        return true;
      }
    }
  }

  return false;
}

void ComboBoxHeaderView::initialize()
{
  Q_FOREACH( ComboBox *box, d->mBoxes ) {
    box->setVisible( false );
  }

  if ( d->mHoverStyle ) {
    if ( !( d->mComboBox ) ) {
      d->mComboBox = new QComboBox( this );
      d->mComboBox->addItems( d->mItems );
      d->mComboBox->setVisible( false );

      connect( d->mComboBox, SIGNAL( activated( int ) ),
               d->mComboBox, SLOT( hide() ) );
      connect( d->mComboBox, SIGNAL( activated( const QString & ) ),
               this, SLOT( slotActivated( const QString & ) ) );
    }
  } else {
    ComboBox *box = 0;
    bool toBeAdded;

    for ( int i = 0; i < count(); ++i ) {
      toBeAdded = ( i >= d->mBoxes.count() );

      if ( toBeAdded ) {
        box = new ComboBox( i, this );
        box->addItems( d->mItems );
        adjustComboBoxIndex( static_cast< QComboBox* >( box ), i );
        d->mBoxes.append( box );
        connect( box, SIGNAL( activated( const QString & ) ),
                 this, SLOT( slotActivated( const QString & ) ) );
      } else {
        box = d->mBoxes[ i ];
      }

      box->setGeometry( sectionRect( i ) );
      box->setVisible( true );
    }
  }
}

void ComboBoxHeaderView::slotActivated( const QString &text )
{
  // FIXME a solution with QAbstractItemModel::setHeaderData
  //       would be nicer, but that seems not work at moment.
  QTableWidget *view = static_cast<QTableWidget *>( parent() );
  if ( view && d->mCurrentIndex >= 0 ) {
    QTableWidgetItem *item = view->horizontalHeaderItem( d->mCurrentIndex );
    if ( !item ) {
      item = new QTableWidgetItem;
      view->setHorizontalHeaderItem( d->mCurrentIndex, item );
    }
    item->setText( text );
  }
}

void ComboBoxHeaderView::setCurrentIndex( int index )
{
  if ( !( isViewVisible() ) ) {
    d->mCurrentIndex = index;
  }
}

void ComboBoxHeaderView::slotResetTexts()
{
  if ( !( d->mHoverStyle ) ) {
    for ( int i = 0; i < count(); ++i ) {
      adjustComboBoxIndex( i );
    }
  }
}

void ComboBoxHeaderView::mouseMoveEvent( QMouseEvent *event ) 
{
  /* do not do this, when a popup is open */
  if ( !( d->mHoverStyle ) || isViewVisible() ) {
    QHeaderView::mouseMoveEvent( event );
    return;
  }

  bool found = false;
  d->mCurrentIndex = -1;

  int index = logicalIndexAt( event->pos() );
  if ( index >= 0 ) {
    found = true;
    d->mCurrentIndex = index;
  }

  if ( found ) {
    d->mComboBox->setGeometry( sectionRect( index ) );
    adjustComboBoxIndex( d->mComboBox, index );
  }
  d->mComboBox->setVisible( found );

  QHeaderView::mouseMoveEvent( event );
}

void ComboBoxHeaderView::leaveEvent( QEvent *event )
{
  if ( d->mHoverStyle && !( d->mComboBox->view()->isVisible() ) ) {
    d->mComboBox->setVisible( false );
    d->mCurrentIndex = -1;
  }

  QHeaderView::leaveEvent( event );
}

void ComboBoxHeaderView::resizeEvent( QResizeEvent *event )
{
  if ( !( d->mHoverStyle ) ) {
    initialize();
  }

  QHeaderView::resizeEvent( event );
}

void ComboBoxHeaderView::setModel( QAbstractItemModel *model )
{
  QHeaderView::setModel( model );
  connect( model, SIGNAL( headerDataChanged( Qt::Orientation, int, int ) ),
           this, SLOT( slotResetTexts() ) );
}

#include "comboboxheaderview.moc"
