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

#ifndef ABSTRACTADDRESSBOOK_H
#define ABSTRACTADDRESSBOOK_H

#include <QObject>


class ImportWizard;

namespace KABC {
  class Addressee;
}
class KJob;

class AbstractAddressBook: public QObject
{
  Q_OBJECT
public:
  explicit AbstractAddressBook(ImportWizard *parent);
  ~AbstractAddressBook();
protected:
  void createGroup();
  void createContact( const KABC::Addressee& address );
  
  void addAddressBookImportInfo( const QString& log );
  void addAddressBookImportError( const QString& log );

private Q_SLOTS:
  void slotStoreDone(KJob*job);

private:
  ImportWizard *mImportWizard;
};

#endif // ABSTRACTADDRESSBOOK_H
