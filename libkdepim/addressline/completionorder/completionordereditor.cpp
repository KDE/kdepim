/** -*- c++ -*-
 * completionordereditor.cpp
 *
 *  Copyright (c) 2004 David Faure <faure@kde.org>
 *                2010 Tobias Koenig <tokoe@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *  In addition, as a special exception, the copyright holders give
 *  permission to link the code of this program with any edition of
 *  the Qt library by Trolltech AS, Norway (or with modified versions
 *  of Qt that use the same license as Qt), and distribute linked
 *  combinations including the two.  You must obey the GNU General
 *  Public License in all respects for all of the code used other than
 *  Qt.  If you modify this file, you may extend this exception to
 *  your version of the file, but you are not obligated to do so.  If
 *  you do not wish to do so, delete this exception statement from
 *  your version.
 */

#include "completionordereditor.h"
#include "completionorderwidget.h"
#include <kdescendantsproxymodel.h>
#include "ldap/ldapclient.h"
#include "ldap/ldapclientsearch.h"
#include "ldap/ldapclientsearchconfig.h"

#include <kabc/addressee.h>
#include <kabc/contactgroup.h>
#include <kldap/ldapserver.h>

#include <KDE/KConfigGroup>
#include <KDE/KLocale>

#include <QtDBus/QDBusConnection>
#include <QToolButton>
#include <QTreeWidget>

using namespace KPIM;

CompletionOrderEditor::CompletionOrderEditor( KLDAP::LdapClientSearch* ldapSearch,
                                              QWidget* parent )
    : KDialog( parent )
{
    setCaption( i18n( "Edit Completion Order" ) );
    setButtons( Ok|Cancel );
    setDefaultButton( Ok );
    setModal( true );
    showButtonSeparator( true );

    mCompletionOrderWidget = new CompletionOrderWidget(this);
    mCompletionOrderWidget->setObjectName(QLatin1String("completionorderwidget"));
    setMainWidget( mCompletionOrderWidget );

    mCompletionOrderWidget->setLdapClientSearch(ldapSearch);
    connect( this, SIGNAL(okClicked()), this, SLOT(slotOk()));

    mCompletionOrderWidget->loadCompletionItems();
    readConfig();
}

CompletionOrderEditor::~CompletionOrderEditor()
{
    writeConfig();
}

void CompletionOrderEditor::readConfig()
{
    KConfigGroup group( KGlobal::config(), "CompletionOrderEditor" );
    const QSize size = group.readEntry( "Size", QSize(600, 400) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void CompletionOrderEditor::writeConfig()
{
    KConfigGroup group( KGlobal::config(), "CompletionOrderEditor" );
    group.writeEntry( "Size", size() );
    group.sync();
}

void CompletionOrderEditor::slotOk()
{
    mCompletionOrderWidget->save();
    KDialog::accept();
}

