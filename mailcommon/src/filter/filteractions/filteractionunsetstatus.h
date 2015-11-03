/*

  Copyright (c) 2012-2015 Montel Laurent <montel@kde.org>

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

#ifndef FILTERACTIONUNSETSTATUS_H
#define FILTERACTIONUNSETSTATUS_H

#include "filteractionstatus.h"
namespace MailCommon
{

class FilterActionUnsetStatus: public FilterActionStatus
{
    Q_OBJECT
public:
    explicit FilterActionUnsetStatus(QObject *parent = Q_NULLPTR);
    ReturnCode process(ItemContext &context, bool applyOnOutbound) const Q_DECL_OVERRIDE;
    SearchRule::RequiredPart requiredPart() const Q_DECL_OVERRIDE;

    static FilterAction *newAction();
    QString sieveCode() const Q_DECL_OVERRIDE;
    QStringList sieveRequires() const Q_DECL_OVERRIDE;
    bool isEmpty() const Q_DECL_OVERRIDE;
    QString informationAboutNotValidAction() const Q_DECL_OVERRIDE;
private:
    bool checkIsValid(int &index) const;
};
}

#endif // FILTERACTIONUNSETSTATUS_H
