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

#include "kabceditor.h"

#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <Akonadi/Contact/ContactEditor>
#include <AkonadiCore/item.h>


#include <QApplication>
#include <KAboutData>
#include <KLocalizedString>
#include <QCommandLineParser>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QVBoxLayout>

Dialog::Dialog( QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( i18n("Contact Editor") );
  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Close);
  QVBoxLayout *mainLayout = new QVBoxLayout;
  setLayout(mainLayout);
  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));


  QWidget *wdg = new QWidget( this );
  mainLayout->addWidget(wdg);
  mainLayout->addWidget(buttonBox);

  QGridLayout *layout = new QGridLayout( wdg );

  mEditor = new Akonadi::ContactEditor( Akonadi::ContactEditor::EditMode, wdg );
  layout->addWidget( mEditor, 0, 0, 1, 3 );

  QLabel *label = new QLabel( i18n("Item Id:"), wdg );
  layout->addWidget( label, 1, 0 );

  mId = new QLineEdit( wdg );
  layout->addWidget( mId, 1, 1 );

  QPushButton *button = new QPushButton( i18n("Load"), wdg );
  layout->addWidget( button, 1, 2 );

  connect(button, &QPushButton::clicked, this, &Dialog::load);

  button = new QPushButton( i18n("Save"), wdg );
  layout->addWidget( button, 2, 2 );

  connect(button, &QPushButton::clicked, this, &Dialog::save);
}

Dialog::~Dialog()
{
}

void Dialog::load()
{
  mEditor->loadContact( Akonadi::Item( mId->text().toLongLong() ) );
}

void Dialog::save()
{
  mEditor->saveContactInAddressBook();
}

int main( int argc, char **argv )
{
  KAboutData aboutData( QLatin1String("kabceditor"), i18n("KABC Editor"), QLatin1String("1.0" ));
  aboutData.setShortDescription( i18n("A contact editor for Akonadi"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

  Dialog dlg;
  dlg.exec();

  return 0;
}

