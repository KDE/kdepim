/*
    progressbar.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#ifndef __KLEO_PROGRESSBAR_H__
#define __KLEO_PROGRESSBAR_H__

#include <tqprogressbar.h>
#include <kdepimmacros.h>
class TQTimer;

namespace Kleo {

  /**
     @short A TQProgressBar with self-powered busy indicator
  */
  class KDE_EXPORT ProgressBar : public TQProgressBar {
    Q_OBJECT
  public:
    ProgressBar( TQWidget * parent=0, const char * name=0, WFlags f=0 );

  public slots:
    void slotProgress( const TQString & message, int type, int current, int total );
    void slotProgress( const TQString & message, int current, int total );
    /*! reimplementation to support self-powered busy indicator */
    void setProgress( int progress );
    /*! reimplementation to support self-powered busy indicator */
    void setTotalSteps( int total );
    /*! reimplementation to support self-powered busy indicator */
    void reset();
    /*! reimplementation to preserve visibility */
    void setProgress( int cur, int tot ) { TQProgressBar::setProgress( cur, tot ); }

  private slots:
    void slotBusyTimerTick();

  private:
    void fixup( bool );

  private:
    TQTimer * mBusyTimer;
    int mRealProgress;
  };
}

#endif // __KLEO_PROGRESSBAR_H__
