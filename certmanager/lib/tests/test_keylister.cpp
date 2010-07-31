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

#include <tqmessagebox.h>
#include <tqstringlist.h>
#include <tqtimer.h>

#include <assert.h>

namespace {
  class TestColumnStrategy : public Kleo::KeyListView::ColumnStrategy {
  public:
    ~TestColumnStrategy() {}
    TQString title( int col ) const;
    TQString toolTip( const GpgME::Key & key, int col ) const;
    TQString text( const GpgME::Key & key, int col ) const;
  };

  TQString TestColumnStrategy::title( int col ) const {
    switch ( col ) {
    case 0: return "Subject";
    case 1: return "EMail";
    case 2: return "Issuer";
    case 3: return "Serial";
    case 4: return "Protocol";
    case 5: return "Validity";
    default: return TQString::null;
    }
  }

  TQString TestColumnStrategy::toolTip( const GpgME::Key & key, int ) const {
    return "Fingerprint: " + TQString::fromUtf8( key.primaryFingerprint() );
  }

  TQString TestColumnStrategy::text( const GpgME::Key & key, int col ) const {
    if ( key.isNull() )
      return "<null>";
    switch ( col ) {
    case 0: return TQString::fromUtf8( key.userID(0).id() );
    case 1: return TQString::fromUtf8( key.userID(0).email() );
    case 2: return TQString::fromUtf8( key.issuerName() );
    case 3: return key.issuerSerial();
    case 4: return key.protocolAsString();
    case 5: return TQChar( key.userID(0).validityAsString() );
    default: return TQString::null;
    }
  }
}

CertListView::CertListView( TQWidget * parent, const char * name, WFlags f )
  : Kleo::KeyListView( new TestColumnStrategy(), 0, parent, name, f )
{
  setHierarchical( true );
  setRootIsDecorated( true );
}

void CertListView::slotResult( const GpgME::KeyListResult & result ) {
  kdDebug() << "CertListView::slotResult()" << endl;
  if ( result.isNull() )
    TQMessageBox::information( this, "Key Listing Result", "KeyListResult is null!" );
  else if ( result.error() )
    TQMessageBox::critical( this, "Key Listing Result",
			   TQString("KeyListResult Error: %1").arg( result.error().asString() ) );
  else if ( result.isTruncated() )
    TQMessageBox::information( this, "Key Listing Result", "KeyListResult is truncated!" );
  else
    TQMessageBox::information( this, "Key Listing Result", "Key listing successful" );
}

void CertListView::slotStart() {
  kdDebug() << "CertListView::slotStart()" << endl;
  Kleo::KeyListJob * job = Kleo::CryptoBackendFactory::instance()->smime()->keyListJob( false );
  assert( job );
  TQObject::connect( job, TQT_SIGNAL(nextKey(const GpgME::Key&)),
		    this, TQT_SLOT(slotAddKey(const GpgME::Key&)) );
  TQObject::connect( job, TQT_SIGNAL(result(const GpgME::KeyListResult&)),
		    this, TQT_SLOT(slotResult(const GpgME::KeyListResult&)) );
#if 0
  TQStringList l;
  l << "Marc";
  job->start( l, false );
#else
  job->start( TQStringList(), false );
#endif
}

int main( int argc, char** argv ) {

  KAboutData aboutData( "test_keylister", "KeyLister Test", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app;

  CertListView * clv = new CertListView( 0, "CertListView top-level" );
  app.setMainWidget( clv );
  clv->show();

  TQTimer::singleShot( 5000, clv, TQT_SLOT(slotStart()) );

  return app.exec();
}

#include "test_keylister.moc"
