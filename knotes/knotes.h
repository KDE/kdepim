/*

 KNotes -- Notes for the KDE project

 Copyright (C) Bernd Johannes Wuebben
               wuebben@math.cornell.edu
	       wuebben@kde.org

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

 */

#ifndef __KNOTES__
#define __KNOTES__

#include <qmultilinedit.h>
#include <qcolor.h>
#include <qpopupmenu.h>
#include <qtimer.h>
#include <qmultilinedit.h> 
#include <qfont.h>
#include <qmessagebox.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qfileinfo.h> 
#include <qdatetime.h> 
#include <qkeycode.h>
#include <qbutton.h>
#include <qdir.h>
#include <qlineedit.h>
#include <qtabdialog.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

#include <kurl.h> // must go before kapp.h
#include <kapp.h>
#include <kfm.h>
#include <kcolordlg.h>
#include <kfontdialog.h>

typedef struct _DefStruct{
  QColor forecolor;
  QColor backcolor;
  int 	width;
  int 	height;
  bool 	frame3d;
  bool 	autoindent;
  QFont   font;
  QString mailcommand;
  QString printcommand;
}DefStruct;

typedef struct _AlarmEntry{
  QString name;
  QDateTime dt;
}AlarmEntry;


class myPushButton: public QPushButton
{
  Q_OBJECT
public:
  myPushButton ( QWidget *parent=0, const char* name=0 );
  ~myPushButton () {}
  bool flat;
  int last_button;
protected:
  void enterEvent( QEvent * );
  void leaveEvent( QEvent * );
  void mousePressEvent( QMouseEvent *e);
  void mouseReleaseEvent( QMouseEvent *e);
  void mouseMoveEvent( QMouseEvent *e);
  void paint( QPainter *_painter );
  void drawButton( QPainter *p ){paint(p);}
  void drawButtonLabel( QPainter *p ){paint(p);}
};


class KPostitMultilineEdit: public QMultiLineEdit{
  Q_OBJECT

public:
  
  KPostitMultilineEdit(QWidget *parent=0, const char *wname=0);
  bool autoIndentMode;
  int  autoIndentID;

protected:
  void  mouseDoubleClickEvent ( QMouseEvent * e );
  void  keyPressEvent(QKeyEvent *e);
  void  mynewLine();

private:
  QString prefixString(QString string);

};

class ConfigDlg;
class FontDlg;
class QTabDialog;

class KPostit :public QFrame{

  Q_OBJECT

public:
  
  KPostit(QWidget *parent=0, const char *wname=0,int number=0,QString pname="");

  // one instance for all kpostits
  static QList<KPostit>   PostitList;     
  static QStrList         PostitFilesList; 
  static QList<AlarmEntry> AlarmList;
  static  bool dock;

  /*  static ConfigDlg *configdlg =0L;
  static FontDlg* fontdlg =0L;
  static QTabDialog* tabdialog =0L;
  */
  int number;
  bool hidden;
  QString name;
  QString propertystring;
  QLabel* label;
  myPushButton* mybutton;
  KPostitMultilineEdit* edit;
  
  QPopupMenu *right_mouse_button;

protected:

  bool  eventFilter( QObject *, QEvent * );
  void resizeEvent( QResizeEvent * );
  void  closeEvent( QCloseEvent * );


public slots:

  void  set3DFrame();
  void  setNoFrame();
  void  toggleFrame();
  void  toggleDock();
  void  set_colors();
  void  set_background_color();
  void  set_foreground_color();
  void  clear_text();
  bool  loadnotes();
  bool  savenotes();
  void  quit();
  void  insertDate();
  void  insertNetFile( const char *_url);
  bool  insertFile(const char* filename);
  void  slotDropEvent( KDNDDropZone * _dropZone );
  void  slotKFMFinished();
  void  RMBActivated(int);
  void  close();
  void  selectFont();
  void  findKPostit(int );
  void  newKPostit();
  void  renameKPostit();
  void  deleteKPostit();
  void  hideKPostit();
  void  insertCalendar();
  void  mail();
  void  print();
  void save_all();
  void  defaults();
  void  setAlarm();
  void  help();
  void  toggleIndentMode();
  void toDesktop(int);
  void toggleSticky();

private:
  void  setAutoIndent();
  void  setNoAutoIndent();


  QFont font;
  QPopupMenu *colors;
  QPopupMenu *options;
  QPopupMenu *operations;
  QPopupMenu *desktops;
  KFM *kfm;
  int  frame3dID;
  int  dockID;
  QColor forecolor;
  QColor backcolor;

  QTimer *timer;
  bool frame3d;

  QPoint pointerOffset;
  bool dragging;
  int sticky_id;

};

class sessionWidget : public QWidget {
  Q_OBJECT
public:
  sessionWidget();
  ~sessionWidget() {};
public slots:
  void wm_saveyourself();
};

#endif


