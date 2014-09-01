/*
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

#include "libkleo/ui/keyselectiondialog.h"
#include <gpgme++/key.h>


#include <KAboutData>
#include <kglobal.h>
#include <kiconloader.h>
#include <QDebug>

#include <vector>
#include <QApplication>
#include <KLocalizedString>
#include <QCommandLineParser>

int main( int argc, char ** argv ) {
  KAboutData aboutData( QLatin1String("test_keyselectiondialog"), i18n("KeySelectionDialog Test"), QLatin1String("0.1") );
  QApplication app(argc, argv);
  QCommandLineParser parser;
  KAboutData::setApplicationData(aboutData);
  parser.addVersionOption();
  parser.addHelpOption();
  aboutData.setupCommandLine(&parser);
  parser.process(app);
  aboutData.processCommandLine(&parser);

  KIconLoader::global()->addAppDir( "libkleopatra" );
  KIconLoader::global()->addAppDir( "kdepim" );

  Kleo::KeySelectionDialog dlg( "Kleo::KeySelectionDialog Test",
                                "Please select a key:",
                                std::vector<GpgME::Key>(),
                                Kleo::KeySelectionDialog::AllKeys, true, true );

  if ( dlg.exec() == QDialog::Accepted ) {
    qDebug() <<"accepted; selected key:" << (dlg.selectedKey().userID(0).id() ? dlg.selectedKey().userID(0).id() :"<null>") <<"\nselected _keys_:";
    for ( std::vector<GpgME::Key>::const_iterator it = dlg.selectedKeys().begin() ; it != dlg.selectedKeys().end() ; ++it )
      qDebug() << (it->userID(0).id() ? it->userID(0).id() :"<null>");
  } else {
    qDebug() <<"rejected";
  }

  return 0;
}
