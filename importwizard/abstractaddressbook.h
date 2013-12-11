/*
  Copyright (c) 2012-2013 Montel Laurent <montel@kde.org>

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

#include <Akonadi/Collection>
#include "abstractbase.h"

class ImportWizard;

namespace KABC {
class Addressee;
class ContactGroup;
}

class KJob;

class AbstractAddressBook: public AbstractBase
{
    Q_OBJECT
public:
    explicit AbstractAddressBook(ImportWizard *parent);
    ~AbstractAddressBook();

protected:
    void createGroup(const KABC::ContactGroup& group);
    void createContact( const KABC::Addressee& address );

    void addImportInfo( const QString& log );
    void addImportError( const QString& log );

    void addAddressBookImportInfo( const QString& log );
    void addAddressBookImportError( const QString& log );
    void cleanUp();

private Q_SLOTS:
    void slotStoreDone(KJob*job);

protected:
    ImportWizard *mImportWizard;

private:
    bool selectAddressBook();
    Akonadi::Collection mCollection;
};

#endif // ABSTRACTADDRESSBOOK_H
