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

#ifndef CARDVIEW_H
#define CARDVIEW_H

#include <qpair.h>
#include <qpoint.h>
#include <qptrlist.h>
#include <qrect.h>
#include <qscrollview.h>
#include <qstring.h>

class QLabel;
class QMouseEvent;
class QPainter;
class QResizeEvent;

class CardView;
class CardViewItemPrivate;
class CardViewPrivate;
class CardViewTip;

/** Represents a single card (item) in the card view. A card has a caption
* and a list of fields. A Field is a label<->value pair. The labels in a
* card should be unique, since they will be used to index the values.
*/
class CardViewItem
{
  friend class CardView;

  public:
    /** A single field in the card view. The first item is the label
    * and the second item is the value.
    */
    typedef QPair<QString, QString> Field;

    /** Constructor.
    *
    * @param parent The CardView that this card should be displayed on.
    * @param caption The caption of the card. This is the text that will
    * appear at the top of the card. This is also the string that will
    * be used to sort the cards in the view.
    */
    CardViewItem( CardView *parent, const QString &caption = QString() );
    virtual ~CardViewItem();

    /** @return The caption of the card, or QString::null if none was ever
    * set.
    */
    const QString &caption() const;

    /** Sets the caption of the card. This is the text that will
    * appear at the top of the card. This is also the string that will
    * be used to sort the cards in the view.
    */
    void setCaption( const QString &caption );

    /** Paints the card using the given painter and color group. The
    * card will handle painting itself selected if it is selected.
    */
    virtual void paintCard( QPainter *p, QColorGroup &cg );

    /** Repaints the card. This is done by sending a repaint event to the
    * view with the clip rect defined as this card.
    */
    virtual void repaintCard();

    /** Adds a field to the card.
    *
    * @param label The label of the field. The field labels must be unique
    * within a card.
    * @param value The value of the field.
    */
    void insertField( const QString &label, const QString &value );

    /** Removes the field with label <i>label</i> from the card.
    */
    void removeField( const QString &label );

    /** @return The value of the field with label <i>label</i>.
    */
    QString fieldValue( const QString &label ) const;

    /** Removes all the fields from this card.
    */
    void clearFields();

    /** @return The next card item. The order of the items will be the same
    * as the display order in the view. 0 will be returned if this is the
    * last card.
    */
    CardViewItem *nextItem() const;

    /** @return True if this card is currently selected, false otherwise.
    */
    bool isSelected() const;

    /** Called by the parent card view when the mouse has been resting for
    * a certain amount of time. If the label or value at pos is obscured
    * (trimmed) make the label display the full text.
    */
    void showFullString( const QPoint &pos, CardViewTip *tip );

    /** @return a pointer to the Field at the position itempos
    * in this item. 0 is returned if itempos is in the caption.
    * @param itempos the position in item coordinates
    */
    Field *fieldAt( const QPoint &itempos ) const;

    CardView *cardView() const { return mView; };

    /** @return The height of this item as rendered, in pixels.

        if @p allowCache is true, the item may use an internally
        cached value rather than recalculating from scratch. The
        argument is mainly to allow the cardView to change global settings (like
        maxFieldLines) that might influence the items heights
    */
    int height( bool allowCache = true ) const;

  protected:
    /** Sets the card as selected. This is usually only called from the
    * card view.
    */
    void setSelected( bool selected );

  private:
    /** Sets the default values.
    */
    void initialize();

    /** Trims a string to the width <i>width</i> using the font metrics
    * to determine the width of each char. If the string is longer than
    * <i>width</i>, then the string will be trimmed and a '...' will
    * be appended.
    */
    QString trimString( const QString &text, int width, QFontMetrics &fm ) const;

    CardViewItemPrivate *d;
    CardView *mView;
};

/** The CardView is a method of displaying data in cards. This idea is
* similar to the idea of a rolodex or business cards. Each card has a
* caption and a list of fields, which are label<->value pairs. The CardView
* displays multiple cards in a grid. The Cards are sorted based on their
* caption.
*
* The CardView class is designed to mirror the API of the QListView or
* QIconView. The CardView is also completely independant of KAddressBook and
* can be used elsewhere. With the exception of a few simple config checks,
* the CardView is also 100% independant of KDE.
*/
class CardView : public QScrollView
{
  friend class CardViewItem;

  Q_OBJECT

  public:
    /** Constructor.
    */
    CardView( QWidget *parent, const char *name );
    virtual ~CardView();

    /** Inserts the item into the card view. This method does not have
    * to be called if you created the item with a proper parent. Once
    * inserted, the CardView takes ownership of the item.
    */
    void insertItem( CardViewItem *item );

    /** Takes the item from the view. The item will not be deleted and
    * ownership of the item is returned to the caller.
    */
    void takeItem( CardViewItem *item );

    /** Clears the view and deletes all card view items
    */
    void clear();

    /** @return The current item, the item that has the focus.
    * Whenever the view has focus, this item has a focus rectangle painted
    * at it's border.
    * @sa setCurrentItem()
    */
    CardViewItem *currentItem() const;

    /** Sets the CardViewItem @p item to the current item in the view.
    */
    void setCurrentItem( CardViewItem *item );

    /** @return The item found at the given point, or 0 if there is no item
    * at that point.
    */
    CardViewItem *itemAt( const QPoint &viewPos ) const;

    /** @return The bounding rect of the given item.
    */
    QRect itemRect( const CardViewItem *item ) const;

    /** Ensures that the given item is in the viewable area of the widget
    */
    void ensureItemVisible( const CardViewItem *item );

    /** Repaints the given item.
    */
    void repaintItem( const CardViewItem *item );

    enum SelectionMode { Single, Multi, Extended, NoSelection };

    /** Sets the selection mode.
    *
    * @see QListView
    */
    void setSelectionMode( SelectionMode mode );

    /** @return The current selection mode.
    */
    SelectionMode selectionMode() const;

    /** Selects or deselects the given item. This method honors the current
    * selection mode, so if other items are selected, they may be unselected.
    */
    void setSelected( CardViewItem *item, bool selected );

    /** Selects or deselects all items.
    */
    void selectAll(bool state);

    /** @return True if the given item is selected, false otherwise.
    */
    bool isSelected( CardViewItem *item ) const;

    /** @return The first selected item. In single select mode, this will be
    * the only selected item, in other modes this will be the first selected
    * item, but others may exist. 0 if no item is selected.
    */
    CardViewItem *selectedItem() const;

    /** @return The first item in the view. This may be 0 if no items have
    * been inserted. This method combined with CardViewItem::nextItem()
    * can be used to iterator through the list of items.
    */
    CardViewItem *firstItem() const;

    /** @return The item after the given item or 0 if the item is the last
    * item.
    */
    CardViewItem *itemAfter( const CardViewItem *item ) const;

    /** @return The number of items in the view.
    */
    int childCount() const;

    /** Attempts to find the first item matching the params.
    *
    * @param text The text to match.
    * @param label The label of the field to match against.
    * @param compare The compare method to use in doing the search.
    *
    * @return The first matching item, or 0 if no items match.
    */
    CardViewItem *findItem( const QString &text, const QString &label,
                            Qt::StringComparisonMode compare = Qt::BeginsWith ) const;

    /** Returns the amounts of pixels required for one column.
    * This depends on wheather drawSeparators is enabled:
    * If so, it is itemWidth + 2*itemSpacing + separatorWidth
    * If not, it is itemWidth + itemSpacing
    * @see itemWidth(), setItemWidth(), itemSpacing() and setItemSpacing()
    */
    uint columnWidth() const;

    /** Sets if the border around a card should be draw. The border is a thing
    * (1 or 2 pixel) line that bounds the card. When drawn, it shows when
    *  a card is highlighted and when it isn't.
    */
    void setDrawCardBorder( bool enabled );

    /** @return True if borders are drawn, false otherwise.
    */
    bool drawCardBorder() const;

    /** Sets if the column separator should be drawn. The column separator
    * is a thin verticle line (1 or 2 pixels) that is used to separate the
    * columns in the list view. The separator is just for esthetics and it
    * does not serve a functional purpose.
    */
    void setDrawColSeparators( bool enabled );

    /** @return True if column separators are drawn, false otherwise.
    */
    bool drawColSeparators() const;

    /** Sets if the field labels should be drawn. The field labels are the
    * unique strings used to identify the fields. Sometimes drawing these
    * labels makes sense as a source of clarity for the user, othertimes they
    * waste too much space and do not assist the user.
    */
    void setDrawFieldLabels( bool enabled );

    /** @return True if the field labels are drawn, false otherwise.
    */
    bool drawFieldLabels() const;

    /** Sets if fields with no value should be drawn (of cause the label only,
    * but it allows for embedded editing sometimes...)
    */
    void setShowEmptyFields( bool show );

    /** @return Wheather empty fields should be shown
    */
    bool showEmptyFields() const;

    /** @return the advisory internal margin in items. Setting a value above 1 means
    * a space between the item contents and the focus recttangle drawn around
    * the current item. The default value is 0.
    * The value should be used by CardViewItem and derived classes.
    * Note that this should not be greater than half of the minimal item width,
    * which is 80. It is currently not checked, so setting a value greater than 40
    * will probably mean a crash in the items painting routine.
    */
    // Note: I looked for a value in QStyle::PixelMetric to use, but I could
    // not see a useful one. One may turn up in a future version of Qt.
    uint itemMargin() const;

    /** Sets the internal item margin. @see itemMargin().
    */
    void setItemMargin( uint margin );

    /** @return the item spacing.
    * The item spacing is the space (in pixels) between each item in a
    * column, between the items and column separators if drawn, and between
    * the items and the borders of the widget. The default value is set to 10.
    */
    // Note: There is no useful QStyle::PixelMetric to use for this atm.
    // An option would be using KDialog::spacingHint().
    uint itemSpacing() const;

    /** Sets the item spacing.
    * @see itemSpacing()
    */
    void setItemSpacing( uint spacing );

    /** @return the width made available to the card items. */
    int itemWidth() const;

    /** Sets the width made available to card items. */
    void setItemWidth( int width );

    /** Sets the header font */
    void setHeaderFont( const QFont &fnt );

    /** @return the header font */
    QFont headerFont() const;

    /** Reimplementation from QWidget */
    void setFont( const QFont &fnt );

    /** Sets the column separator width */
    void setSeparatorWidth( int width );

    /** @return the column separator width */
    int separatorWidth() const;

    /** Sets the maximum number of lines to display pr field.
        If set to 0 (the default) all lines will be displayed.
    */
    void setMaxFieldLines( int howmany );

    /** @return the maximum number of lines pr field */
    int maxFieldLines() const;

  signals:
    /** Emitted whenever the selection changes. This means a user highlighted
    *  a new item or unhighlighted a currently selected item.
    */
    void selectionChanged();

    /** Same as above method, only it carries the item that was selected. This
    * method will only be emitted in single select mode, since it defineds
    * which item was selected.
    */
    void selectionChanged( CardViewItem* );

    /** This method is emitted whenever an item is clicked.
    */
    void clicked( CardViewItem* );

    /** Emitted whenever the user 'executes' an item. This is dependant on
    * the KDE global config. This could be a single click or a doubleclick.
    * Also emitted when the return key is pressed on an item.
    */
    void executed( CardViewItem* );

    /** Emitted whenever the user double clicks on an item.
    */
    void doubleClicked( CardViewItem* );

    /** Emitted when the current item changes
    */
    void currentChanged( CardViewItem* );

    /** Emitted when the return key is pressed in an item.
    */
    void returnPressed( CardViewItem* );

    /** Emitted when the context menu is requested in some way.
    */
    void contextMenuRequested( CardViewItem*, const QPoint& );

  protected:
    /** Determines which cards intersect that region and tells them to paint
    * themselves.
    */
    void drawContents( QPainter *p, int clipx, int clipy, int clipw, int cliph );

    /** Sets the layout to dirty and repaints.
    */
    void resizeEvent( QResizeEvent* );

    /** Changes the direction the canvas scolls.
    */
    void contentsWheelEvent( QWheelEvent* );

    /** Sets the layout to dirty and calls for a repaint.
    */
    void setLayoutDirty( bool dirty );

    /** Does the math based on the bounding rect of the cards to properly
    * lay the cards out on the screen. This is only done if the layout is
    * marked as dirty.
    */
    void calcLayout();

    virtual void contentsMousePressEvent( QMouseEvent* );
    virtual void contentsMouseMoveEvent( QMouseEvent* );
    virtual void contentsMouseReleaseEvent( QMouseEvent* );
    virtual void contentsMouseDoubleClickEvent( QMouseEvent* );

    virtual void enterEvent( QEvent* );
    virtual void leaveEvent( QEvent* );

    virtual void focusInEvent( QFocusEvent* );
    virtual void focusOutEvent( QFocusEvent* );

    virtual void keyPressEvent( QKeyEvent* );

    /** Overload this method to be told when a drag should be started.
    * In most cases you will want to start a drag event with the currently
    * selected item.
    */
    virtual void startDrag();

  private slots:
    /** Called by a timer to display a label with truncated text.
    * Pop up a label, if there is a field with obscured text or
    * label at the cursor position.
    */
    void tryShowFullText();

  private:
    /** draws and erases the rubber bands while columns are resized.
    * @p pos is the horizontal position inside the viewport to use as
    * the anchor.
    * If pos is 0, only erase is done.
    */
    void drawRubberBands( int pos );

    CardViewPrivate *d;
};

#endif
