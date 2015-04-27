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

#ifndef KMAILCVTFILTERINFOGUI_H
#define KMAILCVTFILTERINFOGUI_H

#include "filterinfogui.h"
#include "kimportpage.h"

class KMailCvtFilterInfoGui : public MailImporter::FilterInfoGui
{
public:
    explicit KMailCvtFilterInfoGui(KImportPage *dlg, QWidget *parent);
    ~KMailCvtFilterInfoGui();

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
    QWidget *parent() const Q_DECL_OVERRIDE;

private:
    QWidget *m_parent;
    KImportPage *m_dlg;
};

#endif /* KMAILCVTFILTERINFOGUI_H */

