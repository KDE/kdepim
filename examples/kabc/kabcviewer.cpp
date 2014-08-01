/*
    This file is part of Akonadi.

    Copyright (c) 2007 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include "kabcviewer.h"

#include <QVBoxLayout>



#include <klocale.h>

#include <Akonadi/Contact/ContactViewer>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <QCommandLineOption>

Dialog::Dialog( QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( i18n("Contact Viewer") );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

  mBrowser = new Akonadi::ContactViewer( this );
  mainLayout->addWidget(mBrowser);
  mainLayout->addWidget(buttonBox);
  resize( 520, 580 );
}

Dialog::~Dialog()
{
}

void Dialog::loadUid( Akonadi::Item::Id uid )
{
  mBrowser->setContact( Akonadi::Item( uid ) );
}

int main( int argc, char **argv )
{
  KAboutData aboutData( QLatin1String("kabcviewer"), i18n("KABC Viewer"), QLatin1String("1.0" ));
  aboutData.setShortDescription( i18n("A contact viewer for Akonadi"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

  parser.addOption(QCommandLineOption(QStringList() << QLatin1String("uid"), i18n( "Uid of the Akonadi contact" ), QLatin1String("uid")));


  Dialog dlg;
  if ( !parser.isSet( QLatin1String("uid") ) ) {
    parser.showHelp();
    return 1;
  }

  dlg.loadUid( parser.value( QLatin1String("uid") ).toLongLong() );
  dlg.exec();

  return 0;
}

