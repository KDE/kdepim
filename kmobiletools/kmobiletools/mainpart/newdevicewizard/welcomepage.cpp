/***************************************************************************
   Copyright (C) 2007 by Matthias Lechner <matthias@lmme.de>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/

#include "welcomepage.h"

#include <KDE/KLocale>
#include <KDE/KStandardDirs>

#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>

WelcomePage::WelcomePage( QWidget* parent )
 : QWizardPage( parent )
{
    setTitle( i18n( "Welcome!" ) );

    QLabel* welcomeLabel = new QLabel( this );
    welcomeLabel->setText( i18n( "This wizard will assist you in adding a new device to KMobileTools.\n\n"
                                 "Before you proceed with the device setup, please connect your\n"
                                 "mobile phone to your computer." ) );

    QLabel* phoneConnection = new QLabel( this );
    phoneConnection->setPixmap( KGlobal::dirs()->findResource( "data", "kmobiletools/phone_connection.png" ) );

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget( welcomeLabel, 0, Qt::AlignTop );
    layout->addWidget( phoneConnection, 1, Qt::AlignJustify | Qt::AlignVCenter );
    setLayout( layout );
}


WelcomePage::~WelcomePage()
{
}


