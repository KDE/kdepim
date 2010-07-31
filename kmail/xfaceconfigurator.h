/*  -*- c++ -*-
    xfaceconfigurator.cpp

    KMail, the KDE mail client.
    Copyright (c) 2004 Jakob Schröter <js@camaya.net>
    Copyright (c) 2002 the KMail authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __KMAIL_XFACECONFIGURATOR_H__
#define __KMAIL_XFACECONFIGURATOR_H__

#include <tqwidget.h>
#include <tqtextedit.h>

class KURL;

class QCheckBox;
class QString;
class QLabel;
class QComboBox;

namespace KMail {

  class XFaceConfigurator : public TQWidget {
    Q_OBJECT
  public:
    XFaceConfigurator( TQWidget * parent=0, const char * name=0 );
    virtual ~XFaceConfigurator();

    bool isXFaceEnabled() const;
    void setXFaceEnabled( bool enable );


    TQString xface() const;
    void setXFace( const TQString & text );

  protected:
    TQCheckBox     * mEnableCheck;
    TQTextEdit     * mTextEdit;
    TQLabel        * mXFaceLabel;
    TQComboBox     * mSourceCombo;


  private:
    void setXfaceFromFile( const KURL &url );

  private slots:
    void slotSelectFile();
    void slotSelectFromAddressbook();
    void slotUpdateXFace();
  };
} // namespace KMail

#endif // __KMAIL_XFACECONFIGURATOR_H__


