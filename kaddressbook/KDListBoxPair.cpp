/* -*- Mode: C++ -*-

 $Id$

 KDGear - useful widgets for Qt

 Copyright (C) 2001 by Klarälvdalens Datakonsult AB
*/

#include "KDListBoxPair.h"

#include <qapplication.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qdragobject.h>

#include <kiconloader.h>
#include <kglobal.h>
#include <kdialog.h>

//#include "icons/up.xpm"
//#include "icons/down.xpm"
//#include "icons/left.xpm"
//#include "icons/right.xpm"

/*!
  \class KDListBoxPair KDListBoxPair.h
  \brief This class implements a pair of reorderable listboxes.

  \image html kdlistboxpair.png

  Items can be moved between the listboxes and also within one
  listbox. For this, six buttons are provided, two for each listbox
  for moving an item up and down within the listbox and two for moving
  an item between the two listboxes. These buttons can be turned on
  and off so that it is e.g. possible to allow moving items between
  listboxes but not within one listbox.

  All manipulations of the listboxes is done by retrieving the
  pointers to the QListBox pointers with leftListBox() and
  rightListBox(). You can insert and remove items as usual here and
  also connect to the listbox's signal (but note that some user
  interactions will result in both a signal from the listbox as well
  as from this widget being sent).

  Notice that if you use a custom QListBoxItem subclass, then all
  information that cannot be retrieved via the virtual methods
  QListBoxItem::text() and QListBoxItem::pixmap() will get lost when
  the item is moved between the listboxes since the item needs to be
  deleted and recreated. Since the typical item in a listbox is just
  text (possibly annotated by a small pixmap), this is not a very
  strong restriction. You can even work around it by copying the
  additional data in a slot connected to the \a itemMovedLeftToRight
  and \a itemMovedRightToLeft signals.

  \code
  KDListBoxPair* pair = new KDListBoxPair( true, true, tr("Available Stocks"), tr("Your Stocks"), this );
  QListBox* left = pair->leftListBox();
  QStringList list;
  list << "Stock 1" << "Stock 2" << "Stock 3" << "Stock 4" << "Stock 5" << "Stock 6" << "Stock 7" << "Stock 8";
  left->insertStringList(list);
  \endcode
*/

/*!
  \internal.
  Handles DnD between listboxes.
 */
#ifndef DOXYGEN_SKIP_INTERNAL
class KDDnDListBox : public QListBox
{
public:

class KDDnDListBoxItemDrag : public QDragObject
    {
public:
        KDDnDListBoxItemDrag( QWidget* source, const char* name = 0 )
: QDragObject( source, name )
        {}
        virtual const char* format( int = 0 ) const
        {
            return 0;
        }
        virtual QByteArray encodedData( const char* ) const
        {
            return QByteArray();
        }
    };

    KDDnDListBox( QWidget* parent = 0, const char* name = 0 )
: QListBox( parent, name ), mate( 0 )
    {
        setDndEnabled( true );
    }

    void setDndEnabled( bool enable )
    {
        dndEnabled = enable;
        viewport()->setAcceptDrops( enable );
    }

    bool isDndEnabled() const
    {
        return dndEnabled;
    }

    void setMate( KDDnDListBox* _mate )
    {
        mate = _mate;
    }
protected:
    virtual void viewportMousePressEvent( QMouseEvent* e )
    {
        dragStart = e->pos();
        QListBox::viewportMousePressEvent( e );
    }

    virtual void viewportMouseReleaseEvent( QMouseEvent* e )
    {
        dragStart = QPoint();
        QListBox::viewportMouseReleaseEvent( e );
    }

    virtual void viewportMouseMoveEvent( QMouseEvent* e )
    {
        if ( !dragStart.isNull() && ( dragStart - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) {
            if ( itemAt( dragStart ) != 0 ) {
                dragStart = QPoint();
                KDDnDListBoxItemDrag* drag = new KDDnDListBoxItemDrag( this );
                drag->dragMove();
            }
        }
        QListBox::viewportMouseMoveEvent( e );
    }

    // We need to do this in viewportMoveEvent to work around a Qt bug in 2.3.0
    // Just overriding viewportDragEnterEvent wont work.
    virtual void viewportDragMoveEvent( QDragMoveEvent* e )
    {
        if ( e->source() == mate )
            e->accept();
	else
	    e->ignore(); // This one is required!
    }

    virtual void viewportDropEvent( QDropEvent* e )
    {
        if ( e->source() != 0 && e->source() != this && e->source() == mate ) {
            int after = index( itemAt( e->pos() ) );
            QListBoxItem* it = mate->firstItem();
            KDListBoxPair* parent = ( KDListBoxPair* ) parentWidget();
            while ( it != 0 ) {
                QListBoxItem * tmp = it;
                it = it->next();
#if QT_VERSION >= 300
                if( tmp->isSelected() ) {
#else                    
                if ( tmp->selected() ) {
#endif
                    if ( tmp->pixmap() != 0 )
                        insertItem( *tmp->pixmap(), tmp->text(), after );
                    else
                        insertItem( tmp->text(), after );
                    if ( parent->leftListBox() == this ) {
                        emit parent->itemMovedRightToLeft( tmp, item( after + 1 ) );
                    } else {
                        emit parent->itemMovedLeftToRight( tmp, item( after + 1 ) );
                    }
                    delete tmp;
                }
            }
            e->acceptAction();
        } else
            e->ignore();
    }

    QPoint dragStart;
    bool dndEnabled;
    KDDnDListBox* mate;
};
#endif // DOXYGEN_SKIP_INTERNAL


/*!
  Private data for KDListBoxPair
*/
#ifndef DOXYGEN_SKIP_INTERNAL
struct KDListBoxPair::KDListBoxPairPrivate
{
    QLabel* leftLabel;
    QLabel* rightLabel;

    KDDnDListBox* leftListBox;
    KDDnDListBox* rightListBox;

    QToolButton* leftUpButton;
    QToolButton* leftDownButton;

    QToolButton* rightUpButton;
    QToolButton* rightDownButton;

    QToolButton* leftButton;
    QToolButton* rightButton;
};
#endif // DOXYGEN_SKIP_INTERNAL

/*!
  Constructor. Optionally with reorder buttons on the left and right listboxes.
  Assigns labels for the two listboxes that will appear
  above each respective listbox.

  \param labelLeft the label that will appear above the left listbox
  \param labelRight the label that will appear above the right listbox
  \param parent the Qt parent widget
  \param name the Qt object name
*/
KDListBoxPair::KDListBoxPair( bool leftReorder, bool rightReorder,
                              const QString& labelLeft,
                              const QString& labelRight,
                              QWidget* parent, const char* name )
: QWidget( parent, name ), d( new KDListBoxPairPrivate )
{
    init( leftReorder, rightReorder );
    setLeftLabel( labelLeft );
    setRightLabel( labelRight );
}

/*!
  Constructor. Assigns labels for the two listboxes that will appear
  above each respective listbox. Reordering is enabled for both listboxes.

  \param labelLeft the label that will appear above the left listbox
  \param labelRight the label that will appear above the right listbox
  \param parent the Qt parent widget
  \param name the Qt object name
*/
KDListBoxPair::KDListBoxPair( const QString& labelLeft,
                              const QString& labelRight,
                              QWidget* parent, const char* name )
: QWidget( parent, name ), d( new KDListBoxPairPrivate )
{
    init( true, true );
    setLeftLabel( labelLeft );
    setRightLabel( labelRight );
}


/*!
  Constructor. Assigns empty labels for the two listboxes.
  Reordering is enabled for both listboxes.

  \param parent the Qt parent widget
  \param name the Qt object name
*/
KDListBoxPair::KDListBoxPair( QWidget* parent, const char* name )
: QWidget( parent, name ), d( new KDListBoxPairPrivate )
{
    init( true, true );
}

/*!
  Destructor. Deletes the private data
*/
KDListBoxPair::~KDListBoxPair()
{
    delete d;
}

/*!
  Sets up the widgets that make up the listbox pair.
*/
void KDListBoxPair::init( bool leftReorder, bool rightReorder )
{
  // Prepare and load the icons
   KIconLoader* arrows = KGlobal::iconLoader();
   QIconSet downarrow = arrows->loadIconSet("down", KIcon::Toolbar);
   QIconSet uparrow = arrows->loadIcon("up", KIcon::Toolbar);
   QIconSet forwardarrow = arrows->loadIcon("forward", KIcon::Toolbar);
   QIconSet backarrow = arrows->loadIcon("back", KIcon::Toolbar);
 

  QGridLayout * topLayout = new QGridLayout( this, 5, 2 );
  topLayout->setMargin( 6 );
  topLayout->setSpacing( KDialog::spacingHint() );
  {
    // Left side
    d->leftLabel = new QLabel( this );
    topLayout->addWidget( d->leftLabel, 0, 1 );
    d->leftListBox = new KDDnDListBox( this, "leftListBox" );
    topLayout->addWidget( d->leftListBox, 1, 1 );
    d->leftLabel->setBuddy( d->leftListBox );
  }
  {
    // Left Buttons
    QVBoxLayout* l = new QVBoxLayout();
    l->setSpacing( KDialog::spacingHint() );
    topLayout->addLayout( l, 1, 0 );
    d->leftUpButton = new QToolButton( this );
    // d->leftUpButton->setPixmap( QPixmap( up_xpm ) );
    d->leftUpButton->setIconSet( uparrow );
    //d->leftUpButton->setFixedSize( d->leftUpButton->pixmap()->size() );
    //d->leftUpButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    l->addWidget( d->leftUpButton );
    d->leftDownButton = new QToolButton( this );
    // d->leftDownButton->setPixmap( QPixmap( down_xpm ) );
    d->leftDownButton->setIconSet( downarrow );
    //d->leftDownButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    l->addWidget( d->leftDownButton );
    l->addStretch( 2 );
    setLeftReorderButtonsVisible( leftReorder );
  }
  {
    // Right side
    d->rightLabel = new QLabel( this );
    topLayout->addWidget( d->rightLabel, 0, 3 );
    d->rightListBox = new KDDnDListBox( this, "rightListBox" );
    topLayout->addWidget( d->rightListBox , 1, 3 );
    d->rightLabel->setBuddy( d->rightListBox );
  }
  {
    // Right Buttons
    QVBoxLayout* l = new QVBoxLayout();
    l->setSpacing( KDialog::spacingHint() );
    topLayout->addLayout( l, 1, 4 );
    d->rightUpButton = new QToolButton( this );
    //d->rightUpButton->setPixmap( QPixmap( up_xpm ) );
    d->rightUpButton->setIconSet( uparrow );
    //d->rightUpButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    l->addWidget( d->rightUpButton );
    d->rightDownButton = new QToolButton( this );
    //d->rightDownButton->setPixmap( QPixmap( down_xpm ) );
    d->rightDownButton->setIconSet( downarrow );
    //d->rightDownButton->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
    l->addWidget( d->rightDownButton );
    l->addStretch( 2 );
    setRightReorderButtonsVisible( rightReorder );
  }
  {
    // Middle buttons
    QBoxLayout* l = new QVBoxLayout();
    l->setSpacing( KDialog::spacingHint() );
    topLayout->addLayout( l, 1, 2 );
    d->rightButton = new QToolButton( this );
    //d->rightButton->setPixmap( QPixmap( right_xpm ) );
    d->rightButton->setIconSet( forwardarrow ); 
    l->addWidget( d->rightButton );
    d->leftButton = new QToolButton( this );
    //d->leftButton->setPixmap( QPixmap( left_xpm ) );
    d->leftButton->setIconSet( backarrow );
    l->addWidget( d->leftButton );
    l->addStretch( 2 );
  }

  d->leftListBox->setMate( d->rightListBox );
  d->rightListBox->setMate( d->leftListBox );

  connect( d->leftUpButton, SIGNAL( clicked() ),
	   this, SLOT( moveLeftUp() ) );
  connect( d->leftDownButton, SIGNAL( clicked() ),
	   this, SLOT( moveLeftDown() ) );

  connect( d->rightUpButton, SIGNAL( clicked() ),
	   this, SLOT( moveRightUp() ) );
  connect( d->rightDownButton, SIGNAL( clicked() ),
	   this, SLOT( moveRightDown() ) );

  connect( d->leftButton, SIGNAL( clicked() ),
	   this, SLOT( moveRightToLeft() ) );
  connect( d->rightButton, SIGNAL( clicked() ),
	   this, SLOT( moveLeftToRight() ) );

}

QListBoxItem* KDListBoxPair::moveItem( QListBox* l, int i1, int i2 )
{
  QListBoxItem * sitem = l->item( i1 );
  l->takeItem( sitem );
  l->insertItem( sitem, i2 );
  l->setCurrentItem( i2 );
  return sitem;
}


/*!
  Moves the current item in the left listbox one place up
*/
void KDListBoxPair::moveLeftUp()
{
  if ( leftListBox() ->selectionMode() == QListBox::Single ||
       leftListBox() ->selectionMode() == QListBox::NoSelection ) {
    int i = leftListBox() ->currentItem();
    if ( i <= 0 )
      return ;
    QListBoxItem *sitem = moveItem( leftListBox(), i, i - 1 );
    emit itemReorderedLeft( sitem, i, i - 1 );
  } else {
    for ( unsigned int i = 1; i < leftListBox() ->count(); ++i ) {
      QListBoxItem *sitem = leftListBox() ->item( i );
#if QT_VERSION >= 300
      if( sitem->isSelected() ) {
#else                    
	if ( sitem->selected() ) {
#endif
	  moveItem( leftListBox(), i, i - 1 );
	  emit itemReorderedLeft( sitem, i, i - 1 );
	}
      }
    }
  }

  /*!
    Moves the current item in the left listbox one place down
  */
  void KDListBoxPair::moveLeftDown()
    {
      if ( leftListBox() ->selectionMode() == QListBox::Single ||
	   leftListBox() ->selectionMode() == QListBox::NoSelection ) {
        int i = leftListBox() ->currentItem();
        if ( i < 0 || i == ( int ) leftListBox() ->count() - 1 )
	  return ;
        QListBoxItem *sitem = moveItem( leftListBox(), i, i + 1 );
        emit itemReorderedLeft( sitem, i, i + 1 );
      } else {
        for ( int i = ( int ) leftListBox() ->count() - 2; i >= 0 ; --i ) {
	  QListBoxItem *sitem = leftListBox() ->item( i );
#if QT_VERSION >= 300
	  if( sitem->isSelected() ) {
#else                    
            if ( sitem->selected() ) {
#endif
	      moveItem( leftListBox(), i, i + 1 );
	      emit itemReorderedLeft( sitem, i, i + 1 );
            }
	  }
	}
      }

      /*!
	Moves the current item in the right listbox one place up
      */
      void KDListBoxPair::moveRightUp()
	{
	  if ( rightListBox() ->selectionMode() == QListBox::Single ||
	       rightListBox() ->selectionMode() == QListBox::NoSelection ) {
	    int i = rightListBox() ->currentItem();
	    if ( i <= 0 )
	      return ;
	    QListBoxItem *sitem = moveItem( rightListBox(), i, i - 1 );
	    emit itemReorderedLeft( sitem, i, i - 1 );
	  } else {
	    for ( int i = 1; i < ( int ) rightListBox() ->count(); ++i ) {
	      QListBoxItem *sitem = rightListBox() ->item( i );
#if QT_VERSION >= 300
	      if( sitem->isSelected() ) {
#else                    
		if ( sitem->selected() ) {
#endif
		  moveItem( rightListBox(), i, i - 1 );
		  emit itemReorderedRight( sitem, i, i - 1 );
		}
	      }
	    }
	  }

	  /*!
	    Moves the current item in the right listbox one place down
	  */
	  void KDListBoxPair::moveRightDown()
	    {
	      if ( rightListBox() ->selectionMode() == QListBox::Single ||
		   rightListBox() ->selectionMode() == QListBox::NoSelection ) {
		int i = rightListBox() ->currentItem();
		if ( i < 0 || i == ( int ) rightListBox() ->count() - 1 )
		  return ;
		QListBoxItem *sitem = moveItem( rightListBox(), i, i + 1 );
		emit itemReorderedLeft( sitem, i, i + 1 );
	      } else {
		for ( int i = rightListBox() ->count() - 2; i >= 0; --i ) {
		  QListBoxItem *sitem = rightListBox() ->item( i );
#if QT_VERSION >= 300
		  if( sitem->isSelected() ) {
#else                    
		    if ( sitem->selected() ) {
#endif
		      moveItem( rightListBox(), i, i + 1 );
		      emit itemReorderedLeft( sitem, i, i + 1 );
		    }
		  }
		}
	      }

	      /*!
		Moves the current item from the left listbox to the right
	      */
	      void KDListBoxPair::moveLeftToRight()
		{
		  if ( rightListBox() ->selectionMode() == QListBox::Single ||
		       rightListBox() ->selectionMode() == QListBox::NoSelection ) {
		    int i = leftListBox() ->currentItem();
		    if ( i == -1 )
		      return ;
		    QListBoxItem *olditem = leftListBox() ->item( i );
		    if ( olditem->pixmap() )
		      rightListBox() ->insertItem( *olditem->pixmap(), olditem->text() );
		    else
		      rightListBox() ->insertItem( olditem->text() );
		    emit itemMovedLeftToRight( olditem, rightListBox() ->item( rightListBox() ->count() - 1 ) );
		    delete olditem;
		  } else {
		    QListBoxItem *it = leftListBox() ->firstItem();
		    while ( it ) {
		      QListBoxItem * olditem = it;
		      it = it->next();
#if QT_VERSION >= 300
		      if( olditem->isSelected() ) {
#else                    
			if ( olditem->selected() ) {
#endif
			  if ( olditem->pixmap() )
			    rightListBox() ->insertItem( *olditem->pixmap(), olditem->text() );
			  else
			    rightListBox() ->insertItem( olditem->text() );
			  emit itemMovedLeftToRight( olditem, rightListBox() ->item( rightListBox() ->count() - 1 ) );
			  delete olditem;
			}
		      }
		    }
		  }

		  /*!
		    Moves the current item from the right listbox to the left
		  */
		  void KDListBoxPair::moveRightToLeft()
		    {
		      if ( rightListBox() ->selectionMode() == QListBox::Single ||
			   rightListBox() ->selectionMode() == QListBox::NoSelection ) {
			int i = rightListBox() ->currentItem();
			if ( i == -1 )
			  return ;
			QListBoxItem *olditem = rightListBox() ->item( i );
			if ( olditem->pixmap() )
			  leftListBox() ->insertItem( *olditem->pixmap(), olditem->text() );
			else
			  leftListBox() ->insertItem( olditem->text() );
			emit itemMovedRightToLeft( olditem, leftListBox() ->item( leftListBox() ->count() - 1 ) );
			delete olditem;
		      } else {
			QListBoxItem *it = rightListBox() ->firstItem();
			while ( it ) {
			  QListBoxItem * olditem = it;
			  it = it->next();
#if QT_VERSION >= 300
			  if( olditem->isSelected() ) {
#else                    
			    if ( olditem->selected() ) {
#endif
			      if ( olditem->pixmap() )
				leftListBox() ->insertItem( *olditem->pixmap(), olditem->text() );
			      else
				leftListBox() ->insertItem( olditem->text() );
			      emit itemMovedRightToLeft( olditem, leftListBox() ->item( rightListBox() ->count() - 1 ) );
			      delete olditem;
			    }
			  }
			}
		      }

		      /*!
			Assigns a text that will be displayed above the left listbox.

			\param label the text to be displayed
			\sa leftLabel, setRightLabel, rightLabel
		      */
		      void KDListBoxPair::setLeftLabel( const QString& label )
			{
			  d->leftLabel->setText( label );
			}


		      /*!
			Returns a text that is displayed above the left listbox.

			\return the text that is displayed above the left listbox
			\sa setLeftLabel, setRightLabel, rightLabel
		      */
		      QString KDListBoxPair::leftLabel() const
			{
			  return d->leftLabel->text();
			}


		      /*!
			Assigns a text that will be displayed above the right listbox.

			\param label the text to be displayed
			\sa rightLabel, setLeftLabel, leftLabel
		      */
		      void KDListBoxPair::setRightLabel( const QString& label )
			{
			  d->rightLabel->setText( label );
			}


		      /*!
			Returns a text that is displayed above the right listbox.

			\return the text that is displayed above the right listbox
			\sa setRightLabel, setLeftLabel, leftLabel
		      */
		      QString KDListBoxPair::rightLabel() const
			{
			  return d->rightLabel->text();
			}


		      /*!
			Returns a pointer the left listbox. Use this pointer to insert items
			etc.

			\return a pointer to the left listbox
			\sa rightListBox
		      */
		      QListBox* KDListBoxPair::leftListBox() const
			{
			  return d->leftListBox;
			}


		      /*!
			Returns a pointer the right listbox. Use this pointer to insert items
			etc.

			\return a pointer to the right listbox
			\sa leftListBox
		      */
		      QListBox* KDListBoxPair::rightListBox() const
			{
			  return d->rightListBox;
			}


		      /*!
			Turns on or off dragging and dropping between the two listboxes. See
			the introduction to this class for the limitation on moving items
			between listboxes. If you turn off the buttons for moving items,
			items can still be moved by drag and drop and vice versa; to
			completely prevent moving items between the two listboxes, turn off
			both drag and drop and the move buttons.

			\param enable true to enable drag and drop, false to disable
			\sa isDragAndDropEnabled, setMoveButtonsEnabled, moveButtonsEnabled
		      */
		      void KDListBoxPair::setDragAndDropEnabled( bool enable )
			{
			  d->leftListBox->setDndEnabled( enable );
			  d->rightListBox->setDndEnabled( enable );
			}


		      /*!
			Returns whether drag and drop between the two listboxes is enabled.

			\return true if drag and drop is enabled, false otherwise
			\sa setDragAndDropEnabled, setMoveButtonsEnabled, moveButtonsEnabled
		      */
		      bool KDListBoxPair::isDragAndDropEnabled() const
			{
			  return d->leftListBox->isDndEnabled() && d->rightListBox->isDndEnabled();
			}

		      /*!
			Shows/hides the left reorder buttons
		      */
		      void KDListBoxPair::setLeftReorderButtonsVisible( bool visible )
			{
			  if (visible) {
			    d->leftUpButton->show();
			    d->leftDownButton->show();
			  }
			  else {
			    d->leftUpButton->hide();
			    d->leftDownButton->hide();
			  }
			}

		      /*!
			returns true if the left reorder buttons is visible
		      */
		      bool KDListBoxPair::isLeftReorderButtonsVisible() const
			{
			  return d->leftUpButton->isVisible();
			}

		      /*!
			Shows/hides the right reorder buttons
		      */
		      void KDListBoxPair::setRightReorderButtonsVisible( bool visible )
			{
			  if (visible) {
			    d->rightUpButton->show();
			    d->rightDownButton->show();
			  }
			  else {
			    d->rightUpButton->hide();
			    d->rightDownButton->hide();
			  }
			}

		      /*!
			returns true if the right reorder buttons is visible
		      */
		      bool KDListBoxPair::isRightReorderButtonsVisible() const
			{
			  return d->rightUpButton->isVisible();
			}



		      /*!
			Turns on or off the up and down buttons for the left listbox.

			\param enable true to turn the up and down buttons, false to turn
			off
			\sa isLeftReorderButtonsEnabled, setRightReorderButtonsEnabled,
			isRightReorderButtonsEnabled
		      */
		      void KDListBoxPair::setLeftReorderButtonsEnabled( bool enable )
			{
			  d->leftUpButton->setEnabled( enable );
			  d->leftDownButton->setEnabled( enable );
			}


		      /*!
			Returns whether the up and down buttons for reordering the items in
			the left listbox are enabled.

			\return true if the buttons are enabled, false otherwise
			\sa setLeftReorderButtonsEnabled, setRightReorderButtonsEnabled,
			isRightReorderButtonsEnabled
		      */
		      bool KDListBoxPair::isLeftReorderButtonsEnabled() const
			{
			  return  d->leftUpButton->isEnabled() && d->leftDownButton->isEnabled();
			}


		      /*!
			Turns on or off the up and down buttons for the right listbox.

			\param enable true to turn the up and down buttons, false to turn
			off
			\sa isRightReorderButtonsEnabled, setLeftReorderButtonsEnabled,
			isLeftReorderButtonsEnabled
		      */
		      void KDListBoxPair::setRightReorderButtonsEnabled( bool enable )
			{
			  d->rightUpButton->setEnabled( enable );
			  d->rightDownButton->setEnabled( enable );
			}


		      /*!
			Returns whether the up and down buttons for reordering the items in
			the right listbox are enabled.

			\return true if the buttons are enabled, false otherwise
			\sa setRightReorderButtonsEnabled, setLeftReorderButtonsEnabled,
			isLeftReorderButtonsEnabled
		      */
		      bool KDListBoxPair::isRightReorderButtonsEnabled() const
			{
			  return d->rightUpButton->isEnabled() && d->rightDownButton->isEnabled();
			}


		      /*!
			Assigns a pixmap to be used for the up button for the left
			listbox. The default is an up arrow.

			\param pixmap the new pixmap to be used
			\sa leftUpPixmap, setLeftDownPixmap, leftDownPixmap,
			setRightUpPixmap, rightUpPixmap, setRightDownPixmap, rightDownPixmap
		      */
		      void KDListBoxPair::setLeftUpPixmap( const QPixmap& pixmap )
			{
			  d->leftUpButton->setPixmap( pixmap );
			}


		      /*!
			Returns the pixmap that is displayed in the up button for the left
			listbox. The default is an up arrow.

			\return the pixmap displayed
			\sa setLeftUpPixmap, setLeftDownPixmap, leftDownPixmap,
			setRightUpPixmap, rightUpPixmap, setRightDownPixmap, rightDownPixmap
		      */
		      QPixmap KDListBoxPair::leftUpPixmap() const
			{
			  return *(d->leftUpButton->pixmap());
			}


		      /*!
			Assigns a pixmap to be used for the down button for the left
			listbox. The default is an down arrow.

			\param pixmap the new pixmap to be used
			\sa leftDownPixmap, setLeftUpPixmap, leftUpPixmap,
			setRightUpPixmap, rightUpPixmap, setRightDownPixmap, rightDownPixmap
		      */
		      void KDListBoxPair::setLeftDownPixmap( const QPixmap& pixmap )
			{
			  d->leftDownButton->setPixmap( pixmap );
			}


		      /*!
			Returns the pixmap that is displayed in the down button for the left
			listbox. The default is an down arrow.

			\return the pixmap displayed
			\sa setLeftDownPixmap, setLeftUpPixmap, leftUpPixmap,
			setRightUpPixmap, rightUpPixmap, setRightDownPixmap, rightDownPixmap
		      */
		      QPixmap KDListBoxPair::leftDownPixmap() const
			{
			  return *(d->leftDownButton->pixmap());
			}


		      /*!
			Assigns a pixmap to be used for the up button for the right
			listbox. The default is an up arrow.

			\param pixmap the new pixmap to be used
			\sa rightUpPixmap, setRightDownPixmap, rightDownPixmap,
			setLeftUpPixmap, leftUpPixmap, setLeftDownPixmap, leftDownPixmap
		      */
		      void KDListBoxPair::setRightUpPixmap( const QPixmap& pixmap )
			{
			  d->rightUpButton->setPixmap( pixmap );
			}


		      /*!
			Returns the pixmap that is displayed in the up button for the right
			listbox. The default is an up arrow.

			\return the pixmap displayed
			\sa setRightUpPixmap, setRightDownPixmap, rightDownPixmap,
			setLeftUpPixmap, leftUpPixmap, setLeftDownPixmap, leftDownPixmap
		      */
		      QPixmap KDListBoxPair::rightUpPixmap() const
			{
			  return *(d->rightUpButton->pixmap());
			}


		      /*!
			Assigns a pixmap to be used for the down button for the right
			listbox. The default is an down arrow.

			\param pixmap the new pixmap to be used
			\sa rightDownPixmap, setRightUpPixmap, rightUpPixmap,
			setLeftUpPixmap, leftUpPixmap, setLeftDownPixmap, leftDownPixmap
		      */
		      void KDListBoxPair::setRightDownPixmap( const QPixmap& pixmap )
			{
			  d->rightDownButton->setPixmap( pixmap );
			}


		      /*!
			Returns the pixmap that is displayed in the down button for the right
			listbox. The default is an down arrow.

			\return the pixmap displayed
			\sa setRightDownPixmap, setRightUpPixmap, rightUpPixmap,
			setLeftUpPixmap, leftUpPixmap, setLeftDownPixmap, leftDownPixmap
		      */
		      QPixmap KDListBoxPair::rightDownPixmap() const
			{
			  return *(d->rightDownButton->pixmap());
			}


		      /*!
			Assigns a label that is used in the button for moving items from the
			left listbox to the right listbox. The default is ">>".

			\param label the label to be used
			\sa leftToRightLabel, setRightToLeftLabel, rightToLeftLabel
		      */
		      void KDListBoxPair::setLeftToRightLabel( const QString& label )
			{
			  d->rightButton->setText( label );
			}


		      /*!
			Returns the label that is used in the button for moving items from
			the left listbox to the right listbox.

			\return the label used
			\sa setLeftToRightLabel, setRightToLeftLabel, rightToLeftLabel
		      */
		      QString KDListBoxPair::leftToRightLabel() const
			{
			  return d->rightButton->text();
			}


		      /*!
			Assigns a label that is used in the button for moving items from the
			right listbox to the left listbox. The default is "<<".

			\param label the label to be used
			\sa rightToLeftLabel, setLeftToRightLabel, leftToRightLabel
		      */
		      void KDListBoxPair::setRightToLeftLabel( const QString& label )
			{
			  d->leftButton->setText( label );
			}


		      /*!
			Returns the label that is used in the button for moving items from
			the right listbox to the left listbox.

			\return the label used
			\sa setRightToLeftLabel, setLeftToRightLabel, leftToRightLabel
		      */
		      QString KDListBoxPair::rightToLeftLabel() const
			{
			  return d->leftButton->text();
			}


		      /*!
			Turns the buttons for moving items from the left to the right
			listbox and vice versa on or off.

			\param enable true to turn the buttons on, false to turn them off
			\sa moveButtonsEnabled, setDragAndDropEnabled, isDragAndDropEnabled
		      */
		      void KDListBoxPair::setMoveButtonsEnabled( bool enable )
			{
			  d->leftButton->setEnabled( enable );
			  d->rightButton->setEnabled( enable );
			}


		      /*!
			Returns whether the buttons for moving items from the left to the right
			listbox and vice versa are on or off.

			\return true if the buttons are on, false otherwise
			\sa setMoveButtonsEnabled, setDragAndDropEnabled,
			isDragAndDropEnabled
		      */
		      bool KDListBoxPair::moveButtonsEnabled() const
			{
			  return d->leftButton->isEnabled() && d->rightButton->isEnabled();
			}




		      /*!
			\fn void KDListBoxPair::itemMovedLeftToRight( QListBoxItem* oldItem,
			QListBoxItem* newItem )

			This signal is emitted when an item is moved from the left
			listbox to the right listbox. Note that oldItem will be deleted
			right after the slots connected to this signal have been executed,
			thus, you should not save the oldItem pointer anywhere.

			A slot connected to this signal can be used for copying any data
			from the old item to the new item that is not covered by
			QListBoxItem::text() and QListBoxItem::pixmap().

			\param oldItem a pointer to the old item in the left listbox
			\param newItem a pointer to the new item in the right listbox
			\sa itemMovedRightToLeft
		      */


		      /*!
			\fn void KDListBoxPair::itemMovedRightToLeft( QListBoxItem* oldItem,
			QListBoxItem* newItem )

			This signal is emitted when an item is moved from the right
			listbox to the left listbox. Note that oldItem will be deleted
			right after the slots connected to this signal have been executed,
			thus, you should not save the oldItem pointer anywhere.

			A slot connected to this signal can be used for copying any data
			from the old item to the new item that is not covered by
			QListBoxItem::text() and QListBoxItem::pixmap().

			\param oldItem a pointer to the old item in the right listbox
			\param newItem a pointer to the new item in the left listbox
			\sa itemMovedLeftToRight
		      */


		      /*!
			\fn void KDListBoxPair::itemReorderedLeft( QListBoxItem* item,
			int oldPos, int newPos )

			This signal is emitted when an item in the left listbox is
			reordered. Since reordering always affects to adjacent items, this
			signal is always emitted in pairs.

			\param item a pointer to the item being reordered
			\param oldPos the old position in the listbox
			\param newPos the new position in the listbox
			\sa itemReorderedRight
		      */


		      /*!
			\fn void KDListBoxPair::itemReorderedRight( QListBoxItem* item,
			int oldPos, int newPos )

			This signal is emitted when an item in the right listbox is
			reordered. Since reordering always affects to adjacent items, this
			signal is always emitted in pairs.

			\param item a pointer to the item being reordered
			\param oldPos the old position in the listbox
			\param newPos the new position in the listbox
			\sa itemReorderedLeft
		      */
#include "KDListBoxPair.moc"
