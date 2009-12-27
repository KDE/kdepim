/*
    This file is part of Kleopatra's test suite.
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
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

#include <config-kleopatra.h>

#include "kleo_test.h"

#include <kleo/cryptobackendfactory.h>
#include <kleo/verifydetachedjob.h>
#include <kleo/keylistjob.h>

#include <gpgme++/error.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/key.h>

#include <QtCore/QObject>

Q_DECLARE_METATYPE( GpgME::VerificationResult )

class VerifyTest : public QObject
{
  Q_OBJECT
  private:
    QByteArray mSignature;
    QByteArray mSignedData;
    QList<Kleo::VerifyDetachedJob*> mParallelJobs;
    QList<Kleo::KeyListJob*> mKeyListJobs;
    const Kleo::CryptoBackend::Protocol * mBackend;
    QEventLoop mEventLoop;
    int mJobsStarted;
    QStringList mKeyListPattern;

  public slots:
    void slotParallelKeyListJobFinished()
    {
      mKeyListJobs.removeAll( static_cast<Kleo::KeyListJob*>( sender() ) );
      if ( mParallelJobs.isEmpty() && mKeyListJobs.isEmpty() )
        mEventLoop.quit();
    }
        
    void slotParallelVerifyJobFinished( GpgME::VerificationResult result )
    {
      QVERIFY( mParallelJobs.contains( static_cast<Kleo::VerifyDetachedJob*>( sender() ) ) );
      QCOMPARE( result.signature( 0 ).validity(), GpgME::Signature::Full );
      mParallelJobs.removeAll( static_cast<Kleo::VerifyDetachedJob*>( sender() ) );

      Kleo::KeyListJob *job = mBackend->keyListJob();
      mKeyListJobs.append( job );
      connect( job, SIGNAL( done() ),
               this, SLOT( slotParallelKeyListJobFinished() ) );
      QVERIFY( !job->start( mKeyListPattern ) );

      if ( mParallelJobs.isEmpty() && mKeyListJobs.isEmpty() )
        mEventLoop.quit();
    }

    void startAnotherJob()
    {
      if ( qrand() % 2 == 0 ) {
        Kleo::VerifyDetachedJob *job = mBackend->verifyDetachedJob();
        mParallelJobs.append( job );
        QVERIFY( !job->start( mSignature, mSignedData ) );
      }
      else {
        Kleo::KeyListJob *job = mBackend->keyListJob();
        QVERIFY( !job->start( mKeyListPattern ) );
      }

      mJobsStarted++;
      if ( mJobsStarted >= 250 ) {
        QTimer::singleShot( 1000, &mEventLoop, SLOT( quit() ) );
      }
      else {
        QTimer::singleShot( qrand() % 100, this, SLOT( startAnotherJob() ) );
      }
    }

  private slots:
    void initTestCase()
    {
      qRegisterMetaType<GpgME::VerificationResult>();

      const QString sigFileName = KLEO_TEST_DATADIR "/test.data.sig";
      const QString dataFileName = KLEO_TEST_DATADIR "/test.data";

      QFile sigFile( sigFileName );
      QVERIFY( sigFile.open(QFile::ReadOnly ) );
      QFile dataFile( dataFileName );
      QVERIFY( dataFile.open(QFile::ReadOnly ) );

      mSignature = sigFile.readAll();
      mSignedData = dataFile.readAll();

      mBackend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );
    }

    void testVerify()
    {
      Kleo::VerifyDetachedJob *job = mBackend->verifyDetachedJob();
      QSignalSpy spy( job, SIGNAL(result(GpgME::VerificationResult)) );
      QVERIFY( spy.isValid() );
      GpgME::Error err = job->start( mSignature, mSignedData );
      QVERIFY( !err );
      QTest::qWait( 1000 ); // ### we need to enter the event loop, can be done nicer though

      QCOMPARE( spy.count(), 1 );
      GpgME::VerificationResult result = spy.takeFirst().at(0).value<GpgME::VerificationResult>();
      QCOMPARE( result.numSignatures(), 1U );

      GpgME::Signature sig = result.signature( 0 );
      QCOMPARE( sig.summary() & GpgME::Signature::KeyMissing, 0 );
      QCOMPARE( (quint64) sig.creationTime(), Q_UINT64_C( 1189650248 ) );
      QCOMPARE( sig.validity(), GpgME::Signature::Full );

      mKeyListPattern = QStringList( QString::fromLatin1( result.signature( 0 ).fingerprint() ) );
    }

    // Run multiple verify jobs in parallel, to exercise bug 208353
    void testParallelVerify()
    {
      for ( int i = 0; i < 100; i++ ) {
        Kleo::VerifyDetachedJob *job = mBackend->verifyDetachedJob();
        mParallelJobs.append( job );
        QVERIFY( !job->start( mSignature, mSignedData ) );
        connect( job, SIGNAL( result( GpgME::VerificationResult ) ),
                 this, SLOT( slotParallelVerifyJobFinished( GpgME::VerificationResult ) ) );
      }

      mEventLoop.exec();
    }

    void testMixedParallelJobs()
    {
      mJobsStarted = 0;
      QTimer::singleShot( 0, this, SLOT( startAnotherJob() ) );
      mEventLoop.exec();
    }
};

QTEST_KLEOMAIN( VerifyTest, NoGUI )

#include "test_verify.moc"
