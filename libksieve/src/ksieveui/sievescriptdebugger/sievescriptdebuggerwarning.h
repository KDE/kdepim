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

#ifndef SIEVESCRIPTDEBUGGERWARNING_H
#define SIEVESCRIPTDEBUGGERWARNING_H

#include <KMessageWidget>
#include "ksieveui_export.h"
namespace KSieveUi
{
class KSIEVEUI_EXPORT SieveScriptDebuggerWarning : public KMessageWidget
{
    Q_OBJECT
public:
    explicit SieveScriptDebuggerWarning(QWidget *parent = Q_NULLPTR);
    ~SieveScriptDebuggerWarning();

    void setErrorMessage(const QString &msg);
    void setWarningMessage(const QString &msg);
};
}
#endif // SIEVESCRIPTDEBUGGERWARNING_H
