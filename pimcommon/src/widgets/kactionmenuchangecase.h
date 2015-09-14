/*
  Copyright (c) 2015 Montel Laurent <montel@kde.org>

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

#ifndef KACTIONMENUCHANGECASE_H
#define KACTIONMENUCHANGECASE_H

#include <KActionMenu>
#include "pimcommon_export.h"
class QAction;
class KActionCollection;
namespace PimCommon
{
class KActionMenuChangeCasePrivate;
class PIMCOMMON_EXPORT KActionMenuChangeCase : public KActionMenu
{
    Q_OBJECT
public:
    explicit KActionMenuChangeCase(QObject *parent = Q_NULLPTR);
    ~KActionMenuChangeCase();

    QAction *upperCaseAction() const;

    QAction *sentenceCaseAction() const;

    QAction *lowerCaseAction() const;

    QAction *reverseCaseAction() const;

    void appendInActionCollection(KActionCollection *ac);

Q_SIGNALS:
    void upperCase();
    void sentenceCase();
    void lowerCase();
    void reverseCase();

private:
    KActionMenuChangeCasePrivate *const d;
};
}

#endif // KACTIONMENUCHANGECASE_H
