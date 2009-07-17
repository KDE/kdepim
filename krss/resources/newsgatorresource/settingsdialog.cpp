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

#include "settingsdialog.h"
#include "newsgatorjobs.h"

#include <KListWidget>
#include <KLocale>
#include <KDebug>

SettingsDialog::SettingsDialog( QWidget *parent )
     : KAssistantDialog( parent )
{
    showButton( KAssistantDialog::Help, false );
    enableButton( KAssistantDialog::User1, false );

    QWidget *widget = new QWidget( this );
    m_credentialsUi.setupUi( widget );
    addPage( widget, i18n( "NewsGator Credentials" ) );
    m_locationsWidget = new KListWidget( this );
    addPage( m_locationsWidget, i18n( "Choose a NewsGator Location to sync" ) );

    setInitialSize( QSize( 350, 250 ) );
}

void SettingsDialog::next()
{
    enableButton( KAssistantDialog::User2, false );

    NewsgatorLocationsRetrieveJob * const job = new NewsgatorLocationsRetrieveJob();
    job->setUserName( m_credentialsUi.userNameEdit->text() );
    job->setPassword( m_credentialsUi.passwordEdit->text() );
    connect( job, SIGNAL( result( KJob* ) ), this, SLOT( slotLocationsRetrieved( KJob* ) ) );
    job->start();
}

void SettingsDialog::setUserName( const QString& userName )
{
    m_credentialsUi.userNameEdit->setText( userName );
}

QString SettingsDialog::userName() const
{
    return m_credentialsUi.userNameEdit->text();
}

void SettingsDialog::setPassword( const QString& password )
{
    m_credentialsUi.passwordEdit->setText( password );
}

QString SettingsDialog::password() const
{
    return m_credentialsUi.passwordEdit->text();
}

void SettingsDialog::setLocation( const Location& location )
{
    m_location = location;
}

Location SettingsDialog::location() const
{
    Location location;
    if ( m_locationsWidget->count() == 0 || m_locationsWidget->currentRow() < 0 )
        return location;

    location.id = m_locationsWidget->currentItem()->data( Qt::UserRole ).toInt();
    location.name = m_locationsWidget->currentItem()->text();
    return location;
}

void SettingsDialog::slotLocationsRetrieved( KJob *job )
{
    if ( job->error() ) {
        kWarning() << job->errorString();
    }

    const QList<Location> locs = static_cast<const NewsgatorLocationsRetrieveJob*>( job )->locations();
    Q_FOREACH( const Location& loc, locs ) {
        QListWidgetItem *item = new QListWidgetItem( loc.name );
        item->setData( Qt::UserRole, loc.id );
        m_locationsWidget->addItem( item );
    }

    m_locationsWidget->setCurrentRow( 0 );
    enableButton( KAssistantDialog::User1, true );
    enableButton( KAssistantDialog::User2, true );
    KAssistantDialog::next();
}

#include "settingsdialog.moc"
