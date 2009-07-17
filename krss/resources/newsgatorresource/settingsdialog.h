/*
    Copyright (C) 2009    Dmitry Ivanov <vonami@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KRSS_NEWSGATORRESOURCE_SETTINGSDIALOG_H
#define KRSS_NEWSGATORRESOURCE_SETTINGSDIALOG_H

#include "location.h"
#include "ui_credentialswidget.h"

#include <KAssistantDialog>
#include <KListWidget>

class KJob;

class SettingsDialog : public KAssistantDialog
{
    Q_OBJECT
public:
    explicit SettingsDialog( QWidget *parent = 0 );
    void next();

    void setUserName( const QString& userName );
    QString userName() const;

    void setPassword( const QString& password );
    QString password() const;

    void setLocation( const Location& location );
    Location location() const;

private Q_SLOTS:
    void slotLocationsRetrieved( KJob *job );

private:
    Ui::CredentialsWidget m_credentialsUi;
    KListWidget *m_locationsWidget;
    Location m_location;
};

#endif // KRSS_NEWSGATORBACKEND_SETTINGSDIALOG_H
