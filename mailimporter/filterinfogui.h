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

#ifndef FILTERINFOGUI_H
#define FILTERINFOGUI_H

#include "filters.h"

namespace MailImporter
{
class MAILIMPORTER_EXPORT FilterInfoGui
{
public:
    explicit FilterInfoGui();
    virtual ~FilterInfoGui();
    virtual void setStatusMessage(const QString &status);
    virtual void setFrom(const QString &from);
    virtual void setTo(const QString &to);
    virtual void setCurrent(const QString &current);
    virtual void setCurrent(int percent = 0);
    virtual void setOverall(int percent = 0);

    virtual void addInfoLogEntry(const QString &log);
    virtual void addErrorLogEntry(const QString &log);

    virtual void clear();
    virtual void alert(const QString &message);
    virtual QWidget *parent();
};
}

#endif /* FILTERINFOGUI_H */

