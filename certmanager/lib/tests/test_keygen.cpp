/*  -*- mode: C++; c-file-style: "gnu" -*-
    test_keygen.cpp

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "test_keygen.h"

#include <kleo/keylister.h>
#include <kleo/keygenerationjob.h>

#include <cryptplugwrapper.h>
#include <cryptplugwrapperlist.h>
#include <cryptplugfactory.h>

#include <ui/progressdialog.h>

#include <gpgmepp/keygenerationresult.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include <qmessagebox.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlabel.h>

#include <assert.h>

static const char * keyParams[] = {
  "Key-Type", "Key-Length",
  "Subkey-Type", "Subkey-Length",
  "Name-Real", "Name-Comment", "Name-Email", "Name-DN",
  "Expire-Date",
  "Passphrase"
};
static const int numKeyParams = sizeof keyParams / sizeof *keyParams;

static const char * protocol = 0;

KeyGenerator::KeyGenerator( QWidget * parent, const char * name, WFlags )
  : KDialogBase( parent, name, true, "KeyGenerationJob test",
		 Close|User1, User1, true, KGuiItem( "Create" ) )
{
  QWidget * w = new QWidget( this );
  setMainWidget( w );

  QGridLayout * glay = new QGridLayout( w, numKeyParams+3, 2, marginHint(), spacingHint() );

  int row = -1;

  ++row;
  glay->addMultiCellWidget( new QLabel( "<GnupgKeyParms format=\"internal\">", w ),
			    row, row, 0, 1 );
  for ( int i = 0 ; i < numKeyParams ; ++i ) {
    ++row;
    glay->addWidget( new QLabel( keyParams[i], w ), row, 0 );
    glay->addWidget( mLineEdits[i] = new QLineEdit( w ), row, 1 );
  }

  ++row;
  glay->addMultiCellWidget( new QLabel( "</GnupgKeyParms>", w ),
			    row, row, 0, 1 );
  ++row;
  glay->setRowStretch( row, 1 );
  glay->setColStretch( 1, 1 );

  connect( this, SIGNAL(user1Clicked()), SLOT(slotStartKeyGeneration()) );
}

KeyGenerator::~KeyGenerator() {}

void KeyGenerator::slotStartKeyGeneration() {
  QString params = "<GnupgKeyParms format=\"internal\">\n";
  for ( int i = 0 ; i < numKeyParams ; ++i )
    if ( mLineEdits[i] && !mLineEdits[i]->text().stripWhiteSpace().isEmpty() )
      params += keyParams[i] + ( ": " + mLineEdits[i]->text().stripWhiteSpace() ) + '\n';
  params += "</GnupgKeyParms>\n";

  const CryptPlugWrapper * wrapper = Kleo::CryptPlugFactory::instance()->list().findForLibName( protocol );
  if ( !wrapper )
    wrapper = Kleo::CryptPlugFactory::instance()->smime();
  assert( wrapper );

  kdDebug() << "Using protocol " << wrapper->protocol() << endl;

  Kleo::KeyGenerationJob * job = wrapper->keyGenerationJob();
  assert( job );

  connect( job, SIGNAL(result(const GpgME::KeyGenerationResult&,const QByteArray&)),
	   SLOT(slotResult(const GpgME::KeyGenerationResult&,const QByteArray&)) );

  const GpgME::Error err = job->start( params );
  if ( err )
    showError( err );
  else
    (void)new Kleo::ProgressDialog( job, "Generating key", this );
}

void KeyGenerator::showError( const GpgME::Error & err ) {
  KMessageBox::error( this, "Could not start key generation: " + QString::fromLocal8Bit( err.asString() ),
		      "Key Generation Error" );
}

void KeyGenerator::slotResult( const GpgME::KeyGenerationResult & res, const QByteArray & keyData ) {
  if ( res.error() )
    showError( res.error() );
  else
    KMessageBox::information( this, QString("Key generated successfully, %1 bytes long").arg( keyData.size() ),
			      "Key Generation Finished" );
}
  
int main( int argc, char** argv ) {
  if ( argc == 2 ) {
    protocol = argv[1];
    argc = 1; // hide from KDE
  }
  KAboutData aboutData( "test_keygen", "KeyGenerationJob Test", "0.1" );
  KCmdLineArgs::init( argc, argv, &aboutData );
  KApplication app;

  KeyGenerator * keygen = new KeyGenerator( 0, "KeyGenerator top-level" );
  app.setMainWidget( keygen );
  keygen->show();

  return app.exec();
}

#include "test_keygen.moc"
