#ifndef PABWIDGET_H 
#define PABWIDGET_H 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qwidget.h>
#include <qlistview.h>
#include <qstring.h>
#include <qdialog.h>
#include <qpixmap.h>
#include <qtabdialog.h>
#include <qfileinfo.h>
#include <qstringlist.h>
#include <qvaluelist.h>
#include <qtooltip.h>

#include "undo.h"

class QComboBox;
class ContactEntry;
class ContactEntryList;

/**
 * This class is the main view for Pab.  Most non-menu, non-toolbar,
 * and non-status bar GUI code should go here.
 *
 * @short Main view
 * @author Don Sanders <dsanders@kde.org>
 * @version 0.1
 */

class QTabWidget;
class QBoxLayout;
class QHBoxLayout;
class QLineEdit;
class QListViewItem;
class QIconSet;
class QGridLayout;
class PabListView;
class PabWidget;
class QListBox;

class DynamicTip : public QToolTip
{
public:
    DynamicTip( PabListView * parent );

protected:
    void maybeTip( const QPoint & );
};

class PabListViewItem : public QListViewItem
{

public:
  PabListViewItem( QString entryKey, PabListView* parent, QStringList* field );
  QString entryKey();
  ContactEntry *getEntry(); // will change name back to entry some time
  virtual void refresh();
  virtual PabListView* parent();
  virtual QString key ( int, bool ) const;
  virtual void paintCell ( QPainter * p, const QColorGroup & cg, int column, int width, int align );

private:
  QString entryKey_;
  QStringList *field;
  PabListView *parentListView;
};

class PabListView : public QListView
{
  Q_OBJECT

friend class PabListViewItem;
friend class PabWidget;

public:
  PabListView( PabWidget *parent, const char *name = 0L );
  virtual ~PabListView() {}
  void resort();
  bool tooltips();
  PabWidget *getPabWidget();
  PabListViewItem *getItem( QString entryKey );

protected:
  virtual void paintEmptyArea( QPainter * p, const QRect & rect );
  virtual void backgroundColorChange( const QColor &color );
  virtual void contentsMousePressEvent(QMouseEvent*);
  void contentsMouseMoveEvent( QMouseEvent *e );
  void contentsDragEnterEvent( QDragEnterEvent *e );
  void contentsDropEvent( QDropEvent *e );
  void addEmail(const QString& aStr);

public slots:
  void incSearch( const QString &value );
  void setSorting( int column, bool ascending );
  void setSorting( int column );
  void loadBackground();
  void readConfig();
  void saveConfig();

private:
  PabWidget *pabWidget;
  int oldColumn;
  QIconSet *up, *down;
  int column;
  bool ascending;

  bool backPixmapOn;
  QString backPixmap;
  QPixmap background;
  QPixmap iBackground;
  bool underline;
  bool autUnderline;
  QColor cUnderline;
  bool tooltips_;
  QPoint presspos;
};

class PabWidget : public QWidget
{
friend class PabListView;

  Q_OBJECT

public:
  PabWidget( ContactEntryList *cel, QWidget *parent, const char *name = 0L );
  virtual ~PabWidget();
  virtual ContactEntryList* contactEntryList();
  virtual PabListView* pabListView();
  virtual QStringList *fields();
  virtual QString selectedEmails();
  virtual void updateContact( QString addr, QString name );

public slots:
  virtual void showSelectNameDialog();
  virtual void defaultSettings(); 
  virtual void cut();
  virtual void copy();
  virtual void paste();
  virtual void clear();
  virtual void properties();
  virtual void sendMail();
  virtual void selectAll();
  virtual void saveConfig();
  virtual void readConfig();
  virtual void reconstructListView();
  void change( QString entryKey, ContactEntry *ce );
  PabListViewItem* addEntry( QString EntryKey );
  void addNewEntry( ContactEntry *ce );

protected slots:
  void itemSelected( QListViewItem* );
  void selectionChanged();
  void repopulate();
  void viewOptions();
 
protected:
  virtual void selectNames( QStringList fields );
  void setupListView();

  ContactEntryList *cel;
  QStringList field;
  QValueList<int> fieldWidth;
  QIconSet *myIcon;
  QBoxLayout *mainLayout;
  PabListView *listView;
  QLineEdit *iSearch;
  QComboBox *cbField;
};

class PwDeleteCommand : public Command
{
public:
  PwDeleteCommand( PabWidget *pw, QString entryKey, ContactEntry *ce );
  virtual ~PwDeleteCommand();
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  PabWidget* pw;
  QString entryKey;
  ContactEntry *ce;
};

#include <qlist.h>

// all commands need to be changed so that instead of referencing
// a pabListViewItem they reference an entry key
class PwPasteCommand : public Command
{
public:
  PwPasteCommand( PabWidget *pw, QString clipboard );
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  PabWidget *pw;
  QStringList keyList;
  QString clipboard;
};

class PwCutCommand : public Command
{
public:
  PwCutCommand( PabWidget *pw );
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  PabWidget *pw;
  QStringList keyList;
  QString clipText;
  QString oldText;
};

class PwNewCommand : public Command
{
public:
  PwNewCommand( PabWidget *pw, ContactEntry *ce );
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  PabWidget *pw;
  QString entryKey;
  ContactEntry *ce;
};

class PwEditCommand : public Command
{
public:
  PwEditCommand( PabWidget *pw, 
		 QString entryKey,
		 ContactEntry *oldCe, 
		 ContactEntry *newCe );
  virtual ~PwEditCommand();
  virtual QString name();
  virtual void undo();
  virtual void redo();

private:
  PabWidget *pw;
  QString entryKey;
  ContactEntry *oldCe;
  ContactEntry *newCe;
};

#endif // PABWIDGET_H 
