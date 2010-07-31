/* -*- Mode: C++ -*-
   KD Tools - a set of useful widgets for Qt
*/

/****************************************************************************
** Copyright (C) 2005 Klarälvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Tools library.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid commercial KD Tools licenses may use this file in
** accordance with the KD Tools Commercial License Agreement provided with
** the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.klaralvdalens-datakonsult.se/?page=products for
**   information about KD Tools Commercial License Agreements.
**
** Contact info@klaralvdalens-datakonsult.se if any conditions of this
** licensing are not clear to you.
**
** In addition, as a special exception, the copyright holders give
** permission to link the code of this program with any edition of the
** Qt library by Trolltech AS, Norway (or with modified versions of Qt
** that use the same license as Qt), and distribute linked
** combinations including the two.  You must obey the GNU General
** Public License in all respects for all of the code used other than
** Qt.  If you modify this file, you may extend this exception to your
** version of the file, but you are not obligated to do so.  If you do
** not wish to do so, delete this exception statement from your
** version.
**
**********************************************************************/

#ifndef __KDTOOLS__KDHORIZONTALLINE_H__
#define __KDTOOLS__KDHORIZONTALLINE_H__

#include <tqframe.h>
#include <tqstring.h>

class KDHorizontalLine : public TQFrame {
  Q_OBJECT
  Q_PROPERTY( TQString title READ title WRITE setTitle )
public:
  KDHorizontalLine( TQWidget * parent=0, const char * name=0,  WFlags f=0 );
  KDHorizontalLine( const TQString & title, TQWidget * parent=0, const char * name=0,  WFlags f=0 );
  ~KDHorizontalLine();

  TQString title() const { return mTitle; }

  /*! \reimp to hard-code the frame shape */
  void setFrameStyle( int style );

  TQSize sizeHint() const;
  TQSize minimumSizeHint() const;
  TQSizePolicy sizePolicy() const;

  static int indentHint();

public slots:
  virtual void setTitle( const TQString & title );

protected:
  void paintEvent( TQPaintEvent * );

private:
  void calculateFrame();

private:
  TQString mTitle;
  Qt::AlignmentFlags mAlign;
  int mLenVisible;
};

#endif /* __KDTOOLS__KDHORIZONTALLINE_H__ */

