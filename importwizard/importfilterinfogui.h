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

#ifndef IMPORTFILTERINFOGUI_H
#define IMPORTFILTERINFOGUI_H

#include "filterinfogui.h"
class ImportMailPage;

class ImportFilterInfoGui : public MailImporter::FilterInfoGui
{
public:
    explicit ImportFilterInfoGui(ImportMailPage *parent);
    ~ImportFilterInfoGui();

    void setStatusMessage(const QString &status);
    void setFrom(const QString &from);
    void setTo(const QString &to);
    void setCurrent(const QString &current);
    void setCurrent(int percent = 0);
    void setOverall(int percent = 0);
    void addErrorLogEntry(const QString &log);
    void addInfoLogEntry(const QString &log);
    void clear();
    void alert(const QString &message);
    QWidget *parent();

private:
    ImportMailPage *m_parent;
};

#endif /* IMPORTFILTERINFOGUI_H */

