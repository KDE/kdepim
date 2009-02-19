/*
    threadedjobmixin.h

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

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

#ifndef __KLEO_THREADEDJOBMIXING_H__
#define __KLEO_THREADEDJOBMIXING_H__

#include "qgpgmeprogresstokenmapper.h"

#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrentRun>
#include <QString>

#include <gpgme++/context.h>
#include <gpgme++/interfaces/progressprovider.h>

#include <boost/bind.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_same.hpp>

#include <cassert>

namespace Kleo {
namespace _detail {

  QString audit_log_as_html( GpgME::Context * ctx, GpgME::Error & err );

  class PatternConverter {
    const QList<QByteArray> m_list;
    mutable const char ** m_patterns;
  public:
    explicit PatternConverter( const QByteArray & ba );
    explicit PatternConverter( const QString & s );
    explicit PatternConverter( const QList<QByteArray> & lba );
    explicit PatternConverter( const QStringList & sl );
    ~PatternConverter();

    const char ** patterns() const;
  };

  template <typename T_base, typename T_result=boost::tuple<GpgME::Error,QString,GpgME::Error> >
  class ThreadedJobMixin : public T_base, public GpgME::ProgressProvider {
  public:
    typedef ThreadedJobMixin<T_base, T_result> mixin_type;
    typedef T_result result_type;

  protected:
    BOOST_STATIC_ASSERT(( boost::tuples::length<T_result>::value > 2 ));
    BOOST_STATIC_ASSERT((
      boost::is_same<
        typename boost::tuples::element<
          boost::tuples::length<T_result>::value - 2,
          T_result
        >::type,
        QString
      >::value
    ));
    BOOST_STATIC_ASSERT((
      boost::is_same<
        typename boost::tuples::element<
          boost::tuples::length<T_result>::value - 1,
          T_result
        >::type,
        GpgME::Error
      >::value
    ));

    explicit ThreadedJobMixin( GpgME::Context * ctx )
      : T_base( 0 ), m_ctx( ctx ), m_watcher(), m_auditLog(), m_auditLogError()
    {

    }

    void lateInitialization() {
      assert( m_ctx );
      connect( &m_watcher, SIGNAL(finished()), this, SLOT(slotFinished()) );
      m_ctx->setProgressProvider( this );
    }

    template <typename T_binder>
    void run( const T_binder & func ) {
      m_watcher.setFuture( QtConcurrent::run( boost::bind( func, m_ctx ) ) );
    }
    GpgME::Context * context() const { return m_ctx; }

    virtual void resultHook( const result_type & ) {}

    void slotFinished() {
      const T_result r = m_watcher.result();
      m_auditLog = boost::get<boost::tuples::length<T_result>::value-2>( r );
      m_auditLogError = boost::get<boost::tuples::length<T_result>::value-1>( r );
      resultHook( r );
      emit this->done();
      doEmitResult( r );
      this->deleteLater();
    }
    /* reimp */ void slotCancel() {
      if ( m_ctx ) m_ctx->cancelPendingOperation();
    }
    /* reimp */ QString auditLogAsHtml() const { return m_auditLog; }
    /* reimp */ GpgME::Error auditLogError() const { return m_auditLogError; }
    /* reimp */ void showProgress( const char * what, int type, int current, int total ) {
        // will be called from the thread exec'ing the operation, so
        // just bounce everything to the owning thread:
        // ### hope this is thread-safe (meta obj is const, and
        // ### portEvent is thread-safe, so should be ok)
        QMetaObject::invokeMethod( this, "progress", Qt::QueuedConnection,
                                   Q_ARG( QString, QGpgMEProgressTokenMapper::map( what, type ) ),
                                   Q_ARG( int, current ),
                                   Q_ARG( int, total ) );
    }
  private:
    template <typename T1, typename T2>
    void doEmitResult( const boost::tuple<T1,T2> & tuple ) {
      emit this->result( boost::get<0>( tuple ), boost::get<1>( tuple ) );
    }

    template <typename T1, typename T2, typename T3>
    void doEmitResult( const boost::tuple<T1,T2,T3> & tuple ) {
      emit this->result( boost::get<0>( tuple ), boost::get<1>( tuple ), boost::get<2>( tuple ) );
    }

    template <typename T1, typename T2, typename T3, typename T4>
    void doEmitResult( const boost::tuple<T1,T2,T3,T4> & tuple ) {
      emit this->result( boost::get<0>( tuple ), boost::get<1>( tuple ), boost::get<2>( tuple ), boost::get<3>( tuple ) );
    }

    template <typename T1, typename T2, typename T3, typename T4, typename T5>
    void doEmitResult( const boost::tuple<T1,T2,T3,T4,T5> & tuple ) {
      emit this->result( boost::get<0>( tuple ), boost::get<1>( tuple ), boost::get<2>( tuple ), boost::get<3>( tuple ), boost::get<4>( tuple ) );
    }

  private:
    GpgME::Context * m_ctx;
    QFutureWatcher<T_result> m_watcher;
    QString m_auditLog;
    GpgME::Error m_auditLogError;
  };

}
}

#endif /* __KLEO_THREADEDJOBMIXING_H__ */

