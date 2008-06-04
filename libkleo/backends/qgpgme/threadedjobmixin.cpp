#include "threadedjobmixin.h"

#include <qgpgme/dataprovider.h>

#include <gpgme++/data.h>

#include <QString>
#include <QStringList>
#include <QByteArray>

#include <boost/mem_fn.hpp>

#include <algorithm>

using namespace Kleo;
using namespace GpgME;
using namespace boost;

static const unsigned int GetAuditLogFlags = Context::AuditLogWithHelp|Context::HtmlAuditLog;

QString _detail::audit_log_as_html( Context * ctx ) {
  if ( !ctx )
    return QString();
  QGpgME::QByteArrayDataProvider dp;
  Data data( &dp );
  assert( !data.isNull() );
  if ( const Error err = ctx->getAuditLog( data, GetAuditLogFlags ) )
    return QString::fromLocal8Bit( err.asString() );
  else
    return QString::fromUtf8( dp.data().data() );
}

static QList<QByteArray> from_sl( const QStringList & sl ) {
  QList<QByteArray> result;
  std::transform( sl.begin(), sl.end(), std::back_inserter( result ),
                  mem_fn( &QString::toUtf8 ) );
  return result;
}

static QList<QByteArray> single( const QByteArray & ba ) {
  QList<QByteArray> result;
  result.push_back( ba );
  return result;
}

_detail::PatternConverter::PatternConverter( const QByteArray & ba )
  : m_list( single( ba ) ), m_patterns( 0 ) {}
_detail::PatternConverter::PatternConverter( const QString & s )
  : m_list( single( s.toUtf8() ) ), m_patterns( 0 ) {}
_detail::PatternConverter::PatternConverter( const QList<QByteArray> & lba )
  : m_list( lba ), m_patterns( 0 ) {}
_detail::PatternConverter::PatternConverter( const QStringList & sl )
  :  m_list( from_sl( sl ) ), m_patterns( 0 ) {}

const char ** _detail::PatternConverter::patterns() const {
  if ( !m_patterns ) {
    m_patterns = new const char*[ m_list.size() + 1 ];
    const char ** end = std::transform( m_list.begin(), m_list.end(), m_patterns,
                                        mem_fn( &QByteArray::constData ) );
    *end = 0;
  }
  return m_patterns;
}

_detail::PatternConverter::~PatternConverter() {
  delete [] m_patterns;
}
