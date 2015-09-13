/*
  Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef CALENDARSUPPORT_PRINTING_JOURNALPRINT_H
#define CALENDARSUPPORT_PRINTING_JOURNALPRINT_H

#include "calendarsupport_export.h"
#include "calprintpluginbase.h"
#include "ui_calprintjournalconfig_base.h"

namespace CalendarSupport
{

class CalPrintJournal : public CalPrintPluginBase
{
public:
    CalPrintJournal(): CalPrintPluginBase() {}
    ~CalPrintJournal() {}

    QString groupName() const Q_DECL_OVERRIDE
    {
        return QStringLiteral("Print journal");
    }
    QString description() const Q_DECL_OVERRIDE
    {
        return i18n("Print &journal");
    }
    QString info() const Q_DECL_OVERRIDE
    {
        return i18n("Prints all journals for a given date range");
    }
    QWidget *createConfigWidget(QWidget *) Q_DECL_OVERRIDE;
    int sortID() const Q_DECL_OVERRIDE
    {
        return CalPrinterBase::Journallist;
    }
    bool enabled() const Q_DECL_OVERRIDE
    {
        return true;
    }

public:
    void print(QPainter &p, int width, int height) Q_DECL_OVERRIDE;
    void readSettingsWidget() Q_DECL_OVERRIDE;
    void setSettingsWidget() Q_DECL_OVERRIDE;
    void loadConfig() Q_DECL_OVERRIDE;
    void saveConfig() Q_DECL_OVERRIDE;
    void setDateRange(const QDate &from, const QDate &to) Q_DECL_OVERRIDE;

protected:
    bool mUseDateRange;
};

class CalPrintJournalConfig
    : public QWidget, public Ui::CalPrintJournalConfig_Base
{
public:
    explicit CalPrintJournalConfig(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

}

#endif
