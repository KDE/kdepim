/*
    test_keylister.cpp

    This file is part of libkleopatra's test suite.
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2, as published by the Free Software Foundation.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

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

#include <config.h>

#include "test_keylister.h"

#include <kleo/keylistjob.h>
#include <cryptplugwrapper.h>
#include <kleo/cryptobackendfactory.h>

#include <gpgmepp/keylistresult.h>
#include <gpgmepp/key.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kdebug.h>

#include <qmessagebox.h>
#include <qstringlist.h>
#include <qtimer.h>

#include <assert.h>

namespace {
  class TestColumnStrategy : public Kleo::KeyListView::ColumnStrategy {
  public:
    ~TestColumnStrategy() {}
    QString title( int col ) const;
    QString toolTip( const GpgME::Key & key, int col ) const;
    QString text( const GpgME::Key & key, int col ) const;
  };

  QString TestColumnStrategy::title( int col ) const {
    switch ( col ) {
    case 0: return "Subject";
    case 1: return "EMail";
    case 2: return "Issuer";
    case 3: return "Serial";
    case 4: return "Protocol";
    case 5: return "Validity";
    default: return QString::null;
    }
  }

  QString TestColumnStrategy::toolTip( const GpgME::Key & key, int ) const {
    return "Fingerprint: " + QString::fromUtf8( key.primaryFingerprint() );
  }

  QString TestColumnStrategy::text( const GpgME::Key & key, int col ) const {
    if ( key.isNull() )
      return "<null>";
    switch ( col ) {
    case 0: return QString::fromUtf8( key.userID(0).id() );
    case 1: return QString::fromUtf8( key.userID(0).email() );
    case 2: return QString::fromUtf8( key.issuerName() );
    case 3: return key.issuerSerial();
    case 4: return key.protocolAsString();
    case 5: return QChar( key.userID(0).validityAsString() );
    default: return QString::null;
    }
  }
}

CertListView::CertListView( QWidget * parent, const char * name, WFlags f )
  : Kleo::KeyListView( new TestColumnStrategy(), 0, parent, name, f )
{
  setHierarchical( true );
  setRootIsDecorated( true );
}

void CertListView::slotResult( const GpgME::KeyListResult & result ) {
  kdDebug() << "CertListView::slotResult()" << endl;
  if ( result.isNull() )
    QMessageBox::information( this, "Key Listing Result", "KeyListResult is null!" );
  else if ( result.error() )
    QMessageBox::critical( this, "Key Listing Result",
			   QString("KeyListResult Error: %1").arg( result.error().asString() ) );
  else if ( result.isTruncated() )
    QMessageBox::information( this, "Key Listing Result", "KeyListResult is truncated!" );
  else
    QMessageBox::information( this, "Key Listing Result", "Key listing successful" );
}

void CertListView::slotStart() {
  kdDebug() << "CertListView::slotStart()" << endl;
  Kleo::KeyListJob * job = Kleo::CryptoBackendFactory::instance()->smime()->keyListJob( false );
  assert( job );
  QObject::connect( job, SIGNAL(nextKey(const GpgME::Key&)),
		    this, SLOT(slotAddKey(const GpgME::Key&)) );
  QObject::connect( job, SIGNAL(result(const GpgME::KeyListResult&)),
		    this, SLOT(slotResult(const GpgME::KeyListResult&)) );
#if 0
  QStringList l;
  l << "Marc";
  job->start( l, false );
#else
  job->start( QStringList(), false );
#endif
}

int main( int argc, char** argv ) {

  KAboutData aboutData( "test_keylister", "KeyLister Test", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app;

  CertListView * clv = new CertListView( 0, "CertListView top-level" );
  app.setMainWidget( clv );
  clv->show();

  QTimer::singleShot( 5000, clv, SLOT(slotStart()) );

  return app.exec();
}

#include "test_keylister.moc"
