// kmfawidgets.h - KMFilterAction parameter widgets
// Copyright: (c) 2001 Marc Mutz <Marc@Mutz.com>
// License: GPL

#ifndef _kmfawidgets_h_
#define _kmfawidgets_h_

#include <klineedit.h>
#include <tqstring.h>

/** The param widget for KMFilterActionWithAddress..
    @author Marc Mutz <mutz@kde.org>
*/

class TQPushButton;
class KURLRequester;

class KMFilterActionWithAddressWidget : public QWidget
{
  Q_OBJECT
public:
  KMFilterActionWithAddressWidget( TQWidget* parent=0, const char* name=0 );

  void clear() { mLineEdit->clear(); }
  TQString text() const { return mLineEdit->text(); }
  void setText( const TQString & aString ) { mLineEdit->setText( aString ); }

signals:
  // Forwarded from the internal text edit
  void textChanged(const TQString&);

protected slots:
  void slotAddrBook();

private:
  TQPushButton* mBtn;
  TQLineEdit*   mLineEdit;
};

class KMSoundTestWidget : public QWidget
{
  Q_OBJECT
public:
  KMSoundTestWidget( TQWidget * parent, const char * name=0 );
  ~KMSoundTestWidget();
  TQString url() const;
  void setUrl( const TQString & url );
  void clear();
signals:
  void testPressed();
protected slots:
  void playSound();
  void openSoundDialog( KURLRequester * );
  void slotUrlChanged( const TQString & );

private:
  KURLRequester *m_urlRequester;
  TQPushButton *m_playButton;
};

#endif /*_kmfawidget_h_*/
