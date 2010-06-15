/*
  This file is part of libkdepim.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>
  Copyright (c) 2010 Bertjan Broeksema <b.broeksema@home.nl>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "kcheckcombobox.h"

#include <QtGui/QAbstractItemView>
#include <QtGui/QKeyEvent>
#include <QtGui/QLineEdit>
#include <QtGui/QStandardItemModel>

using namespace KPIM;

/// Class KCheckComboBox::Private

namespace KPIM {

class KCheckComboBox::Private
{
  KCheckComboBox *q;

  public:
    Private( KCheckComboBox *qq )
      : q( qq )
      , mSeparator( QLatin1String( "," ) )
      , mIgnoreHide( false )
    { }

    void makeInsertedItemsCheckable(const QModelIndex &, int start, int end);
    void updateCheckedItems( const QModelIndex &topLeft = QModelIndex(),
                             const QModelIndex &bottomRight = QModelIndex() );
    void toggleCheckState( int pos );
    void toggleCheckState( const QModelIndex &index );

  public:
    QString mSeparator;
    QString mDefaultText;
    bool mIgnoreHide;
};

}

void KCheckComboBox::Private::makeInsertedItemsCheckable(const QModelIndex &parent, int start, int end)
{
  QStandardItemModel *model = qobject_cast<QStandardItemModel *>( q->model() );
  Q_ASSERT( model );
  for ( int r = start; r <= end; ++r ) {
    QStandardItem *item = model->item( r, 0 );
    item->setCheckable( true );
  }
}

void KCheckComboBox::Private::updateCheckedItems( const QModelIndex &topLeft,
                                                  const QModelIndex &bottomRight )
{
  Q_UNUSED( topLeft );
  Q_UNUSED( bottomRight );

  QStringList items = q->checkedItems();
  QString text;
  if ( items.isEmpty() ) {
    text = mDefaultText;
  } else {
    text = items.join( mSeparator );
  }

  q->setEditText( text );

  emit q->checkedItemsChanged( items );
}

void KCheckComboBox::Private::toggleCheckState( const QModelIndex &index )
{
  QVariant value = index.data( Qt::CheckStateRole );
  if ( value.isValid() ) {
    Qt::CheckState state = static_cast<Qt::CheckState>( value.toInt() );
    q->model()->setData( index, state == Qt::Unchecked ? Qt::Checked : Qt::Unchecked,
                         Qt::CheckStateRole );
  }
}

void KCheckComboBox::Private::toggleCheckState( int pos )
{
  Q_UNUSED( pos );
  toggleCheckState( q->view()->currentIndex() );
}

/// Class KCheckComboBox

KCheckComboBox::KCheckComboBox( QWidget *parent )
  : KComboBox( parent )
  , d( new KCheckComboBox::Private( this ) )
{
  connect( this, SIGNAL(activated(int)), this, SLOT(toggleCheckState(int)) );
  connect( model(), SIGNAL(rowsInserted (const QModelIndex &, int, int)),
           SLOT(makeInsertedItemsCheckable(const QModelIndex &, int, int)) );
  connect( model(), SIGNAL(dataChanged(const QModelIndex &, const QModelIndex &)),
           this, SLOT(updateCheckedItems(const QModelIndex &, const QModelIndex &)) );

  // read-only contents
  setEditable( true );
  lineEdit()->setReadOnly( true );
  setInsertPolicy( KComboBox::NoInsert );

  view()->installEventFilter( this );
  view()->viewport()->installEventFilter( this );

  lineEdit()->installEventFilter( this );

  d->updateCheckedItems();
}

KCheckComboBox::~KCheckComboBox()
{
  delete d;
}

void KCheckComboBox::hidePopup()
{
  if ( !d->mIgnoreHide ) {
    KComboBox::hidePopup();
  }
  d->mIgnoreHide = false;
}

Qt::CheckState KCheckComboBox::itemCheckState( int index ) const
{
  return static_cast<Qt::CheckState>( itemData( index, Qt::CheckStateRole ).toInt() );
}

void KCheckComboBox::setItemCheckState( int index, Qt::CheckState state )
{
  setItemData( index, state, Qt::CheckStateRole );
}

QStringList KCheckComboBox::checkedItems() const
{
  QStringList items;
  if ( model() ) {
    QModelIndex index = model()->index( 0, modelColumn(), rootModelIndex() );
    QModelIndexList indexes = model()->match( index, Qt::CheckStateRole,
                                              Qt::Checked, -1, Qt::MatchExactly );
    foreach ( const QModelIndex &index, indexes ) {
      items += index.data().toString();
    }
  }
  return items;
}

void KCheckComboBox::setCheckedItems( const QStringList &items )
{
  for ( int r = 0; r < model()->rowCount( rootModelIndex() ); ++r ) {
    QModelIndex indx = model()->index( r, modelColumn(), rootModelIndex() );
    QString text = indx.data().toString();
    bool found = items.contains( text );
    model()->setData( indx, found ? Qt::Checked : Qt::Unchecked, Qt::CheckStateRole );
  }
  d->updateCheckedItems();
}

QString KCheckComboBox::defaultText() const
{
  return d->mDefaultText;
}

void KCheckComboBox::setDefaultText( const QString &text )
{
  if ( d->mDefaultText != text ) {
    d->mDefaultText = text;
    d->updateCheckedItems();
  }
}

QString KCheckComboBox::separator() const
{
  return d->mSeparator;
}

void KCheckComboBox::setSeparator( const QString &separator )
{
  if ( d->mSeparator != separator ) {
    d->mSeparator = separator;
    d->updateCheckedItems();
  }
}

void KCheckComboBox::keyPressEvent( QKeyEvent *event )
{
  switch ( event->key() ) {
    case Qt::Key_Up:
    case Qt::Key_Down:
      showPopup();
      event->accept();
      break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
    case Qt::Key_Escape:
      hidePopup();
      event->accept();
      break;
    default:
      break;
  }
  // don't call base class implementation, we don't need all that stuff in there
}

void KCheckComboBox::wheelEvent( QWheelEvent *event )
{
  // discard mouse wheel events on the combo box
  event->accept();
}

bool KCheckComboBox::eventFilter( QObject *receiver, QEvent *event )
{
  switch ( event->type() ) {
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::ShortcutOverride:
    {
      switch ( static_cast<QKeyEvent *>( event )->key() ) {
        case Qt::Key_Space:
          if ( event->type() == QEvent::KeyPress ) {
            d->toggleCheckState( view()->currentIndex() );
            return true;
          }
          break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
        case Qt::Key_Escape:
          // ignore Enter keys, they would normally select items.
          // but we select with Space, because multiple selection is possible
          // we simply close the popup on Enter/Escape
          hidePopup();
          return true;
      }
    }
      break;
    case QEvent::MouseButtonDblClick:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
      d->mIgnoreHide = true;

      if ( receiver == lineEdit() ) {
        showPopup();
        return true;
      }

      break;
    default:
      break;
  }
  return KComboBox::eventFilter( receiver, event );
}

#include "kcheckcombobox.moc"
