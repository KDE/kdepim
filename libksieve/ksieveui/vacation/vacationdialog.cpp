/*
    vacationdialog.cpp

    Copyright (c) 2013 Montel Laurent <montel@kde.org>
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/


#include "vacationdialog.h"
#include "vacationeditwidget.h"

#include <kdebug.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmime/kmime_header_parsing.h>
#include <kwindowsystem.h>

#include <QApplication>

using KMime::Types::AddrSpecList;
using KMime::Types::AddressList;
using KMime::Types::MailboxList;
using KMime::HeaderParsing::parseAddressList;

using namespace KSieveUi;

VacationDialog::VacationDialog( const QString &caption, QWidget * parent,
                                bool modal )
    : KDialog( parent )
{
    setCaption( caption );
    setButtons( Ok|Cancel|Default );
    setDefaultButton(  Ok );
    setModal( modal );
    mVacationEditWidget = new VacationEditWidget;
    setMainWidget( mVacationEditWidget );
    KWindowSystem::setIcons( winId(), qApp->windowIcon().pixmap(IconSize(KIconLoader::Desktop),IconSize(KIconLoader::Desktop)), qApp->windowIcon().pixmap(IconSize(KIconLoader::Small),IconSize(KIconLoader::Small)) );
    readConfig();
}

VacationDialog::~VacationDialog()
{
    kDebug() << "~VacationDialog()";
    writeConfig();
}

void VacationDialog::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "VacationDialog" );
    group.writeEntry( "Size", size() );
}

void VacationDialog::readConfig()
{
    KConfigGroup group( KGlobal::config(), "VacationDialog" );
    const QSize size = group.readEntry( "Size", QSize() );
    if ( size.isValid() ) {
        resize( size );
    } else {
        resize( sizeHint().width(), sizeHint().height() );
    }
}

bool VacationDialog::activateVacation() const
{
    return mVacationEditWidget->activateVacation();
}

void VacationDialog::setActivateVacation( bool activate )
{
    mVacationEditWidget->setActivateVacation(activate);
}

QString VacationDialog::messageText() const
{
    return mVacationEditWidget->messageText();
}

void VacationDialog::setMessageText( const QString &text )
{
    mVacationEditWidget->setMessageText(text);
}

int VacationDialog::notificationInterval() const
{
    return mVacationEditWidget->notificationInterval();
}

void VacationDialog::setNotificationInterval( int days )
{
    mVacationEditWidget->setNotificationInterval(days);
}

AddrSpecList VacationDialog::mailAliases() const
{
    return mVacationEditWidget->mailAliases();
}

void VacationDialog::setMailAliases( const AddrSpecList &aliases )
{
    mVacationEditWidget->setMailAliases(aliases);
}

void VacationDialog::setMailAliases( const QString &aliases )
{
    mVacationEditWidget->setMailAliases(aliases);
}

QString VacationDialog::domainName() const
{
    return mVacationEditWidget->domainName();
}

void VacationDialog::setDomainName( const QString &domain )
{
    mVacationEditWidget->setDomainName(domain);
}

bool VacationDialog::domainCheck() const
{
    return mVacationEditWidget->domainCheck();
}

void VacationDialog::setDomainCheck( bool check )
{
    mVacationEditWidget->setDomainCheck(check);
}

bool VacationDialog::sendForSpam() const
{
    return mVacationEditWidget->sendForSpam();
}

void VacationDialog::setSendForSpam( bool enable )
{
    mVacationEditWidget->setSendForSpam(enable);
}

void VacationDialog::enableDomainAndSendForSpam( bool enable )
{
    mVacationEditWidget->enableDomainAndSendForSpam(enable);
}

