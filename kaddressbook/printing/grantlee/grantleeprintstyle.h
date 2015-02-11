/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

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

#ifndef GRANTLEEPRINTSTYLE_H
#define GRANTLEEPRINTSTYLE_H

#include "printstyle.h"

namespace KAddressBookGrantlee
{
class GrantleePrint;
}

namespace KABPrinting
{

class PrintProgress;

class GrantleePrintStyle : public PrintStyle
{
    Q_OBJECT

public:
    explicit GrantleePrintStyle(const QString &themePath, PrintingWizard *parent);
    ~GrantleePrintStyle();

    void print(const KContacts::Addressee::List &, PrintProgress *) Q_DECL_OVERRIDE;
private:
    KAddressBookGrantlee::GrantleePrint *mGrantleePrint;
};

class GrantleeStyleFactory : public PrintStyleFactory
{
public:
    explicit GrantleeStyleFactory(const QString &name, const QString &themePath, PrintingWizard *parent);

    PrintStyle *create() const Q_DECL_OVERRIDE;
    QString description() const Q_DECL_OVERRIDE;
private:
    QString mThemePath;
    QString mName;
};

}

#endif // GRANTLEEPRINTSTYLE_H
