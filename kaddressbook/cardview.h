#ifndef CARDVIEW_H
#define CARDVIEW_H

#include <qscrollview.h>
#include <qptrlist.h>
#include <qstring.h>
#include <qrect.h>
#include <qpair.h>

class QPainter;
class QResizeEvent;
class QMouseEvent;
class CardView;
class CardViewPrivate;
class CardViewItemPrivate;

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
    CardViewItem(CardView *parent, QString caption = QString::null);
    virtual ~CardViewItem();
    
    /** @return The caption of the card, or QString::null if none was ever
    * set.
    */
    const QString &caption() const;
    
    /** Sets the caption of the card. This is the text that will
    * appear at the top of the card. This is also the string that will
    * be used to sort the cards in the view.
    */
    void setCaption(const QString &caption);
    
    /** Paints the card using the given painter and color group. The
    * card will handle painting itself selected if it is selected.
    */
    virtual void paintCard(QPainter *p, QColorGroup &cg);
    
    /** Repaints the card. This is done by sending a repaint event to the
    * view with the clip rect defined as this card.
    */
    virtual void repaintCard();
    
    /** Adds a field to the card.
    *
    * @param label The label of the field. The field labels must be unique
    * within a card.
    * @param The value of the field. 
    */
    void insertField(const QString &label, const QString &value);
    
    /** Removes the field with label <i>label</i> from the card.
    */
    void removeField(const QString &label);
    
    /** @return The value of the field with label <i>label</i>.
    */
    QString fieldValue(const QString &label);
    
    /** Removes all the fields from this card.
    */
    void clearFields();
    
    /** @return The next card item. The order of the items will be the same
    * as the display order in the view. 0 will be returned if this is the
    * last card.
    */
    CardViewItem *nextItem();
    
    /** @return True if this card is currently selected, false otherwise.
    */
    bool isSelected() const;
    
  protected:
    /** Sets the card as selected. This is usually only called from the
    * card view.
    */
    void setSelected(bool selected);

  private:
    /** Sets the default values.
    */
    void initialize();
    
    /** Calculates the height and width of the bouding rectangle needed
    * by this card. This is based on the number of fields in the card.
    */
    void calcRect();
    
    /** Trims a string to the width <i>width</i> using the font metrics
    * to determine the width of each char. If the string is longer than
    * <i>width</i>, then the string will be trimmed and a '...' will
    * be appended.
    */
    QString trimString(const QString &text, int width, QFontMetrics &fm);
    
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
    CardView(QWidget *parent, const char *name);
    virtual ~CardView();
    
    /** Inserts the item into the card view. This method does not have
    * to be called if you created the item with a proper parent. Once
    * inserted, the CardView takes ownership of the item.
    */
    void insertItem(CardViewItem *item);
    
    /** Takes the item from the view. The item will not be deleted and
    * ownership of the item is returned to the caller.
    */
    void takeItem(CardViewItem *item);
    
    /** Clears the view and deletes all card view items
    */
    void clear();
    
    /** @return The item found at the given point, or 0 if there is no item
    * at that point.
    */
    CardViewItem *itemAt(const QPoint &viewPos);
    
    /** @return The bounding rect of the given item.
    */
    QRect itemRect(const CardViewItem *item);
    
    /** Ensures that the given item is in the viewable area of the widget
    */
    void ensureItemVisible(const CardViewItem *item);
    
    enum SelectionMode { Single, Multi, Extended, NoSelection };
    
    /** Sets the selection mode.
    *
    * @see QListView
    */
    void setSelectionMode(SelectionMode mode);
    
    /** @return The current selection mode.
    */
    SelectionMode selectionMode() const;
    
    /** Selects or deselects the given item. This method honors the current
    * selection mode, so if other items are selected, they may be unselected.
    */
    void setSelected(CardViewItem *item, bool selected);
    
    /** Selects or deselects all items.
    */
    void selectAll(bool state);
    
    /** @return True if the given item is selected, false otherwise.
    */
    bool isSelected(CardViewItem *item) const;
    
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
    CardViewItem *itemAfter(CardViewItem *item);
    
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
    CardViewItem *findItem(const QString &text, const QString &label, 
                           Qt::StringComparisonMode compare = Qt::BeginsWith);
    
    /** Sets if the border around a card should be draw. The border is a thing
    * (1 or 2 pixel) line that bounds the card. When drawn, it shows when
    *  a card is highlighted and when it isn't.
    */
    void setDrawCardBorder(bool enabled);
    
    /** @return True if borders are drawn, false otherwise.
    */
    bool drawCardBorder() const;
    
    /** Sets if the column separator should be drawn. The column separator
    * is a thin verticle line (1 or 2 pixels) that is used to separate the
    * columns in the list view. The separator is just for esthetics and it
    * does not serve a functional purpose.
    */
    void setDrawColSeparators(bool enabled);
    
    /** @return True if column separators are drawn, false otherwise.
    */
    bool drawColSeparators() const;
    
    /** Sets if the field labels should be drawn. The field labels are the
    * unique strings used to identify the fields. Sometimes drawing these
    * labels makes sense as a source of clarity for the user, othertimes they
    * waste too much space and do not assist the user.
    */
    void setDrawFieldLabels(bool enabled);
    
    /** @return True if the field labels are drawn, false otherwise.
    */
    bool drawFieldLabels() const;
    
  signals:
    /** Emitted whenever the selection changes. This means a user highlighted
    *  a new item or unhighlighted a currently selected item.
    */
    void selectionChanged();
    
    /** Same as above method, only it carries the item that was selected. This
    * method will only be emitted in single select mode, since it defineds
    * which item was selected.
    */
    void selectionChanged(CardViewItem *);
    
    /** This method is emitted whenever an item is clicked.
    */
    void clicked(CardViewItem *);
    
    /** Emitted whenever the user 'executes' an item. This is dependant on
    * the KDE global config. This could be a signal click or a doubleclick.
    */
    void executed(CardViewItem *);
    
    /** Emitted whenever the user double clicks on an item.
    */
    void doubleClicked(CardViewItem *);
    
  protected:
    /**
     * Repaints the whole viewport. We use a double buffer to avoid flickering.
     */
    virtual void viewportPaintEvent( QPaintEvent * );
    
    /** Sets the layout to dirty and repaints.
    */
    void resizeEvent(QResizeEvent *e);
    
    /** Sets the layout to dirty and calls for a repaint.
    */
    void setLayoutDirty(bool dirty);
    
    /** Does the math based on the bounding rect of the cards to properly
    * lay the cards out on the screen. This is only done if the layout is
    * marked as dirty.
    */
    void calcLayout();
    
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    
    /** Overload this method to be told when a drag should be started.
    * In most cases you will want to start a drag event with the currently
    * selected item.
    */
    virtual void startDrag();
    
  private:
    CardViewPrivate *d;
};

#endif
