/* qeventloopinteractor.h
   Copyright (C) 2003 Klarälvdalens Datakonsult AB

   This file is part of QGPGME.
 
   QGPGME is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
 
   QGPGME is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with QGPGME; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307 USA.  */

// -*- c++ -*-
#ifndef __QGPGME_EVENTLOOPINTERACTOR_H__
#define __QGPGME_EVENTLOOPINTERACTOR_H__

#include <gpgmepp/eventloopinteractor.h>

#include <qobject.h>
#include <kdepimmacros.h>

#include <kdemacros.h>

namespace GpgME {
  class Context;
  class Error;
  class TrustItem;
  class Key;
} // namespace GpgME
  
namespace QGpgME {

  class KDE_EXPORT EventLoopInteractor : public QObject, public GpgME::EventLoopInteractor {
    Q_OBJECT
  protected:
    EventLoopInteractor( QObject * parent, const char * name=0 );
  public:
    virtual ~EventLoopInteractor();

    static EventLoopInteractor * instance();

  signals:
    void nextTrustItemEventSignal( GpgME::Context * context, const GpgME::TrustItem & item  );
    void nextKeyEventSignal( GpgME::Context * context, const GpgME::Key & key );
    void operationDoneEventSignal( GpgME::Context * context, const GpgME::Error & e );

    void aboutToDestroy();

  protected slots:
    void slotWriteActivity( int socket );
    void slotReadActivity( int socket );
    
  protected:
    //
    // IO Notification Interface
    //

    /*! \reimp */
    void * registerWatcher( int fd, Direction dir, bool & ok );
    /*! \reimp */
    void unregisterWatcher( void * tag );

    //
    // Event Handler Interface
    //

    /*! \reimp */
    void nextTrustItemEvent( GpgME::Context * context, const GpgME::TrustItem & item );
    /*! \reimp */
    void nextKeyEvent( GpgME::Context * context, const GpgME::Key & key );
    /*! \reimp */
    void operationDoneEvent( GpgME::Context * context, const GpgME::Error & e );

  private:
    class Private;
    Private * d;
    static EventLoopInteractor * mSelf;
  };

} // namespace QGpgME

#endif // __QGPGME_EVENTLOOPINTERACTOR_H__


