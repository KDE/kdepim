/*
   Copyright (C) 2012-2016 Montel Laurent <montel@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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

    void setStatusMessage(const QString &status) Q_DECL_OVERRIDE;
    void setFrom(const QString &from) Q_DECL_OVERRIDE;
    void setTo(const QString &to) Q_DECL_OVERRIDE;
    void setCurrent(const QString &current) Q_DECL_OVERRIDE;
    void setCurrent(int percent = 0) Q_DECL_OVERRIDE;
    void setOverall(int percent = 0) Q_DECL_OVERRIDE;
    void addErrorLogEntry(const QString &log) Q_DECL_OVERRIDE;
    void addInfoLogEntry(const QString &log) Q_DECL_OVERRIDE;
    void clear() Q_DECL_OVERRIDE;
    void alert(const QString &message) Q_DECL_OVERRIDE;
    QWidget *parent() const  Q_DECL_OVERRIDE;

private:
    ImportMailPage *m_parent;
};

#endif /* IMPORTFILTERINFOGUI_H */

