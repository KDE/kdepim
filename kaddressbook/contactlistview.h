#ifndef CONTACTLISTVIEW_H
#define CONTACTLISTVIEW_H

#include <qcolor.h>
#include <qpixmap.h>
#include <qtooltip.h>
#include <qstring.h>

#include <klistview.h>

#include <kabc/field.h>

class QDropEvent;
class KAddressBookTableView;
class ContactListView;

/** The whole tooltip design needs a lot of work. Currently it is
* hacked together to function.
*/
class DynamicTip : public QToolTip
{
  public:
    DynamicTip( ContactListView * parent );

  protected:
    void maybeTip( const QPoint & );
    
  private:
};

class ContactListViewItem : public KListViewItem
{

public:
  ContactListViewItem(const KABC::Addressee &a, ContactListView* parent, 
                      KABC::AddressBook *doc, const KABC::Field::List &fields );
  const KABC::Addressee &addressee() const { return mAddressee; }
  virtual void refresh();
  virtual ContactListView* parent();
  virtual QString key ( int, bool ) const;
  
  /** Adds the border around the cell if the user wants it.
  * This is how the single line config option is implemented.
  */
  virtual void paintCell(QPainter * p, const QColorGroup & cg, 
                         int column, int width, int align );

private:
  KABC::Addressee mAddressee;
  KABC::Field::List mFields;
  ContactListView *parentListView;
  KABC::AddressBook *mDocument;
};


/////////////////////////////////////////////
// ContactListView

class ContactListView : public KListView
{
  Q_OBJECT

public:
  ContactListView(KAddressBookTableView *view, 
                  KABC::AddressBook *doc,
                  QWidget *parent, 
                  const char *name = 0L );
  virtual ~ContactListView() {}
  //void resort();
  
  /** Returns true if tooltips should be displayed, false otherwise
  */
  bool tooltips() const { return mToolTips; }
  void setToolTipsEnabled(bool enabled) { mToolTips = enabled; }
  
  bool alternateBackground() const { return mABackground; }
  void setAlternateBackgroundEnabled(bool enabled);
  
  bool singleLine() const { return mSingleLine; }
  void setSingleLineEnabled(bool enabled) { mSingleLine = enabled; }
  
  const QColor &alternateColor() const { return mAlternateColor; }
  
  /** Sets the background pixmap to <i>filename</i>. If the
  * QString is empty (QString::isEmpty()), then the background
  * pixmap will be disabled.
  */
  void setBackgroundPixmap(const QString &filename);

protected:
  /** Paints the background pixmap in the empty area. This method is needed
  * since Qt::FixedPixmap will not scroll with the list view.
  */
  virtual void paintEmptyArea( QPainter * p, const QRect & rect );
  virtual void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent( QMouseEvent *e );
  void contentsDropEvent( QDropEvent *e );
  virtual bool acceptDrag(QDropEvent *e) const;

protected slots:
  void itemDropped(QDropEvent *e);
  
public slots:
  //void incSearch( const QString &value );
  //void setSorting( int column, bool ascending );
  //void setSorting( int column );
  //void loadBackground();
  //void readConfig();
  //void saveConfig();

signals:
  void startAddresseeDrag();
  void addresseeDropped(QDropEvent *);
  
private:
  KAddressBookTableView *pabWidget;
  int oldColumn;
  int column;
  bool ascending;

  bool mABackground;
  bool mSingleLine;
  bool mToolTips;

  QColor mAlternateColor;
  
//  bool backPixmapOn;
//  QString backPixmap;
//  QPixmap background;
//  QPixmap iBackground;
//  bool underline;
//  bool autUnderline;
//  bool tooltips_;
  QPoint presspos;
};


#endif
