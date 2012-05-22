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
#include "evolutionaddressbook.h"
#include "importwizard.h"
#include <KMessageBox>
#include <KLocale>
#include <KFileDialog>

EvolutionAddressBook::EvolutionAddressBook(ImportWizard *parent)
  : AbstractAddressBook( parent )
{
  exportEvolutionAddressBook();
}

EvolutionAddressBook::~EvolutionAddressBook()
{

}

void EvolutionAddressBook::exportEvolutionAddressBook()
{
  KMessageBox::information(mImportWizard,i18n("Export Evolution AddressBook"),i18n("Evolution AddressBook will export as vcard. Import vcard in KAddressBook."));

  const QString directory = KFileDialog::getExistingDirectory( KUrl(), mImportWizard, i18n("Select directory where vcards will stored."));
  if(directory.isEmpty()) {
    return;
  }
  QDir evolutionDir;
  bool found = false;
  for(int i=0;i<9; ++i) {
    evolutionDir.setPath(QString::fromLatin1("/usr/lib/evolution/3.%1/evolution-addressbook-export").arg(i));
    if(evolutionDir.exists()) {
      found = true;
      break;
    }
  }
  if(found) {
    //TODO use QProcess
  }
  //TODO use "/usr/lib/evolution/3.2/evolution-addressbook-export -l" to show list.
  //TODO use "/usr/lib/evolution/3.2/evolution-addressbook-export --format=vcard <addressbook> --output=toto.vcard"
}

  

#include "evolutionaddressbook.moc"
