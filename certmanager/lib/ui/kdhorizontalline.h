#ifndef __KDTOOLS__KDHORIZONTALLINE_H__
#define __KDTOOLS__KDHORIZONTALLINE_H__

#include <qframe.h>
#include <qstring.h>

class KDHorizontalLine : public QFrame {
  Q_OBJECT
  Q_PROPERTY( QString title READ title WRITE setTitle )
public:
  KDHorizontalLine( QWidget * parent=0, const char * name=0,  WFlags f=0 );
  KDHorizontalLine( const QString & title, QWidget * parent=0, const char * name=0,  WFlags f=0 );
  ~KDHorizontalLine();

  QString title() const { return mTitle; }

  /*! \reimp to hard-code the frame shape */
  void setFrameStyle( int style );

  QSize sizeHint() const;
  QSize minimumSizeHint() const;
  QSizePolicy sizePolicy() const;

public slots:
  virtual void setTitle( const QString & title );

protected:
  void paintEvent( QPaintEvent * );

private:
  void calculateFrame();

private:
  QString mTitle;
  Qt::AlignmentFlags mAlign;
  int mLenVisible;
};

#endif /* __KDTOOLS__KDHORIZONTALLINE_H__ */

