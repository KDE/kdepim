/*
    This file is part of KAddressBook.
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qheader.h>
#include <qiconset.h>
#include <qimage.h>
#include <qdragobject.h>
#include <qcombobox.h>
#include <qpainter.h>
#include <qbrush.h>
#include <qevent.h>

#include <klocale.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kurl.h>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kimproxy.h>

#include "kaddressbooktableview.h"

#include "contactlistview.h"

/////////////////////////////////
// DynamicTip Methods

DynamicTip::DynamicTip( ContactListView *parent)
  : QToolTip( parent )
{
}

void DynamicTip::maybeTip( const QPoint &pos )
{
  if (!parentWidget()->inherits( "ContactListView" ))
    return;

  ContactListView *plv = (ContactListView*)parentWidget();
  if (!plv->tooltips())
    return;

  QPoint posVp = plv->viewport()->pos();

  QListViewItem *lvi = plv->itemAt( pos - posVp );
  if (!lvi)
    return;

  ContactListViewItem *plvi = dynamic_cast< ContactListViewItem* >(lvi);
  if (!plvi)
    return;

  QString s;
  QRect r = plv->itemRect( lvi );
  r.moveBy( posVp.x(), posVp.y() );

    //kdDebug(5720) << "Tip rec: " << r.x() << "," << r.y() << "," << r.width()
    //          << "," << r.height() << endl;

  KABC::Addressee a = plvi->addressee();
  if (a.isEmpty())
    return;

  s += i18n("label: value", "%1: %2").arg(a.formattedNameLabel())
                                     .arg(a.formattedName());

  s += '\n';
  s += i18n("label: value", "%1: %2").arg(a.organizationLabel())
                                       .arg(a.organization());

  QString notes = a.note().stripWhiteSpace();
  if ( !notes.isEmpty() ) {
    notes += '\n';
    s += '\n' + i18n("label: value", "%1: \n").arg(a.noteLabel());
    QFontMetrics fm( font() );

    // Begin word wrap code based on QMultiLineEdit code
    int i = 0;
    bool doBreak = false;
    int linew = 0;
    int lastSpace = -1;
    int a = 0;
    int lastw = 0;

    while ( i < int(notes.length()) ) {
      doBreak = FALSE;
      if ( notes[i] != '\n' )
        linew += fm.width( notes[i] );

      if ( lastSpace >= a && notes[i] != '\n' )
        if  (linew >= parentWidget()->width()) {
          doBreak = TRUE;
          if ( lastSpace > a ) {
            i = lastSpace;
            linew = lastw;
          }
          else
            i = QMAX( a, i-1 );
        }

      if ( notes[i] == '\n' || doBreak ) {
        s += notes.mid( a, i - a + (doBreak?1:0) ) +"\n";

        a = i + 1;
        lastSpace = a;
        linew = 0;
      }

      if ( notes[i].isSpace() ) {
        lastSpace = i;
        lastw = linew;
      }

      if ( lastSpace <= a ) {
        lastw = linew;
      }

      ++i;
    }
  }

  tip( r, s );
}

///////////////////////////
// ContactListViewItem Methods

ContactListViewItem::ContactListViewItem(const KABC::Addressee &a,
                                         ContactListView *parent,
                                         KABC::AddressBook *doc,
                                         const KABC::Field::List &fields,
                                         KIMProxy *proxy )
  : KListViewItem(parent), mAddressee(a), mFields( fields ),
    parentListView( parent ), mDocument(doc), mIMProxy( proxy )
{
  if ( mIMProxy )
    mHasIM = ( !( mIMProxy->allContacts().find( mAddressee.uid() ) == mIMProxy->allContacts().end() ) );
  else 
    mHasIM = false;
  refresh();
}

QString ContactListViewItem::key(int column, bool ascending) const
{
  // Preserve behaviour of QListViewItem::key(), otherwise we cause a crash if the column does not exist
  if ( column >= parentListView->columns() )
    return QString::null;
    
#if KDE_VERSION >= 319
  Q_UNUSED( ascending )
  if ( parentListView->showIM() ) {
    // in this case, one column is reserved for IM presence
    // so we have to process it differently
    if ( column == parentListView->imColumn() ) {
      // increment by one before converting to string so that -1 is not greater than 1
      // create the sort key by taking the numeric status 0 low, 5 high, and subtracting it from 5
      // so that the default ascending gives online before offline, etc.
      QString key = QString::number( 5 - ( mIMProxy->presenceNumeric( mAddressee.uid() ) + 1 ) );
      return key;
    }
    else {
      return mFields[ column ]->sortKey( mAddressee );
    }
  }
  else 
    return mFields[ column ]->sortKey( mAddressee );
#else
  return QListViewItem::key( column, ascending ).lower();
#endif
}

void ContactListViewItem::paintCell(QPainter * p,
                                    const QColorGroup & cg,
                                    int column,
                                    int width,
                                    int align)
{
  KListViewItem::paintCell(p, cg, column, width, align);

  if ( !p )
    return;

  if (parentListView->singleLine()) {
    p->setPen( parentListView->alternateColor() );
    p->drawLine( 0, height() - 1, width, height() - 1 );
  }
}


ContactListView *ContactListViewItem::parent()
{
  return parentListView;
}


void ContactListViewItem::refresh()
{
  // Update our addressee, since it may have changed else were
  mAddressee = mDocument->findByUid(mAddressee.uid());
  if (mAddressee.isEmpty())
    return;

  int i = 0;
  if ( mHasIM )
    setPixmap( parentListView->imColumn(), mIMProxy->presenceIcon( mAddressee.uid() ) );
  
  KABC::Field::List::ConstIterator it;
  for( it = mFields.begin(); it != mFields.end(); ++it ) {
    if ( (*it)->label() == KABC::Addressee::birthdayLabel() ) {
      QDate date = mAddressee.birthday().date();
      if ( date.isValid() )
        setText( i++, KGlobal::locale()->formatDate( date, true ) );
      else
        setText( i++, "" );
    } else
      setText( i++, (*it)->value( mAddressee ) );
  }
}

void ContactListViewItem::setHasIM( bool hasIM )
{
  mHasIM = hasIM;
}

///////////////////////////////
// ContactListView

ContactListView::ContactListView(KAddressBookTableView *view,
                                 KABC::AddressBook* /* doc */,
                                 QWidget *parent,
                                 const char *name )
  : KListView( parent, name ),
    pabWidget( view ),
    oldColumn( 0 )
{
  mABackground = true;
  mSingleLine = false;
  mToolTips = true;
  mShowIM = true;
  mAlternateColor = KGlobalSettings::alternateBackgroundColor();
  
  setAlternateBackgroundEnabled(mABackground);
  setAcceptDrops( true );
  viewport()->setAcceptDrops( true );
  setAllColumnsShowFocus( true );
  setShowSortIndicator(true);
  setSelectionModeExt( KListView::Extended );
  setDropVisualizer(false);

  connect(this, SIGNAL(dropped(QDropEvent*)),
          this, SLOT(itemDropped(QDropEvent*)));

  new DynamicTip( this );
}

void ContactListView::paintEmptyArea( QPainter * p, const QRect & rect )
{
  QBrush b = palette().brush(QPalette::Active, QColorGroup::Base);

  // Get the brush, which will have the background pixmap if there is one.
  if (b.pixmap())
  {
    p->drawTiledPixmap( rect.left(), rect.top(), rect.width(), rect.height(),
      *(b.pixmap()),
      rect.left() + contentsX(),
      rect.top() + contentsY() );
  }

  else
  {
    // Do a normal paint
    KListView::paintEmptyArea(p, rect);
  }
}

void ContactListView::contentsMousePressEvent(QMouseEvent* e)
{
  presspos = e->pos();
  KListView::contentsMousePressEvent(e);
}


// To initiate a drag operation
void ContactListView::contentsMouseMoveEvent( QMouseEvent *e )
{
  if ((e->state() & LeftButton) && (e->pos() - presspos).manhattanLength() > 4 ) {
    emit startAddresseeDrag();
  }
  else
    KListView::contentsMouseMoveEvent( e );
}

bool ContactListView::acceptDrag(QDropEvent *e) const
{
  return QTextDrag::canDecode(e);
}

void ContactListView::itemDropped(QDropEvent *e)
{
  contentsDropEvent(e);
}

void ContactListView::contentsDropEvent( QDropEvent *e )
{
  emit addresseeDropped(e);
}

void ContactListView::setAlternateBackgroundEnabled(bool enabled)
{
  mABackground = enabled;

  if (mABackground)
  {
    setAlternateBackground(mAlternateColor);
  }
  else
  {
    setAlternateBackground(QColor());
  }
}

void ContactListView::setBackgroundPixmap(const QString &filename)
{
  if (filename.isEmpty())
  {
    unsetPalette();
  }
  else
  {
    setPaletteBackgroundPixmap(QPixmap(filename));
  }
}

void ContactListView::setShowIM( bool enabled )
{
  mShowIM = enabled;
}

bool ContactListView::showIM()
{
  return mShowIM;
}

void ContactListView::setIMColumn( int column )
{
  mInstantMsgColumn = column;
}

int ContactListView::imColumn()
{
  return mInstantMsgColumn;
}

#include "contactlistview.moc"
