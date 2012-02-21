/*
  Copyright (c) 2012 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "importwizard.h"

#include <kaboutapplicationdialog.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <ktoolinvocation.h>

ImportWizard::ImportWizard(QWidget *parent)
  : KAssistantDialog(parent)
{
  setModal(true);
  setWindowTitle( i18n( "KMailCVT Import Tool" ) );
  connect(this,SIGNAL(helpClicked()),this,SLOT(help()));
}

ImportWizard::~ImportWizard()
{
}

void ImportWizard::help()
{
  KAboutApplicationDialog a( KGlobal::mainComponent().aboutData(), this );
  a.exec();
}

#include "importwizard.moc"
