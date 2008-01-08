/* -*- mode: c++; c-basic-offset:4 -*-
    test_verificationresultdialog.cpp

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

#include <vector>
#include <string>
#include <cassert>

#include <QFile>
#include <QDebug>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <qgpgme/eventloopinteractor.h>

#include <gpgme++/error.h>
#include <gpgme++/verificationresult.h>
#include <gpgme++/key.h>

#include <kleo/verifydetachedjob.h>
#include <kleo/cryptobackendfactory.h>

#include "../uiserver/signaturedisplaywidget.h"

class Relay : public QObject {
    Q_OBJECT
public:
    explicit Relay( QObject * p=0 ) : QObject( p ) {}

public Q_SLOTS:
    void slotVerifyDetachedResult( const GpgME::VerificationResult & result, const std::vector<GpgME::Key> & keys ) {
        mKeys = keys;
        mSignatures = result.signatures();
        Kleo::SignatureDisplayWidget * d = new Kleo::SignatureDisplayWidget();
        d->setSignature( mSignatures[0], mKeys.size()?mKeys[0]:GpgME::Key() );
        d->show();
    }

private:
    std::vector<GpgME::Key> mKeys;
    std::vector<GpgME::Signature> mSignatures;
};


int main( int argc, char * argv[] ) {

    KAboutData aboutData( "test_verificationresultdialog", 0, ki18n("VerificationResultDialog Test"), "0.1" );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add( "sig <sig>",         ki18n("Signature file") );
    options.add( "data <data>",         ki18n("Data file") );

    KCmdLineArgs::addCmdLineOptions( options );
    KApplication app;
    KCmdLineArgs * args = KCmdLineArgs::parsedArgs();

    const QString sigFileName = args->getOption("sig");
    const QString dataFileName = args->getOption("data");

    const Kleo::CryptoBackend::Protocol * const backend = Kleo::CryptoBackendFactory::instance()->protocol( "openpgp" );


    Relay relay;
    Kleo::VerifyDetachedJob *job = backend->verifyDetachedJob();
    QObject::connect( job, SIGNAL(result(GpgME::VerificationResult, std::vector<GpgME::Key>)),
             &relay, SLOT(slotVerifyDetachedResult(GpgME::VerificationResult, std::vector<GpgME::Key>)) );

    QFile sigFile( sigFileName );
    if ( !sigFile.open(QFile::ReadOnly ) ) return -1;

    QFile dataFile( dataFileName );
    if ( !dataFile.open(QFile::ReadOnly ) ) return -1;

    const QByteArray signature = sigFile.readAll();
    const QByteArray data = dataFile.readAll();
    job->start( signature, data );

    return app.exec();
}

#include "test_verificationresultdialog.moc"
