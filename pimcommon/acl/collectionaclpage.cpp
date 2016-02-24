// -*- mode: C++; c-file-style: "gnu" -*-
/**
 *
 * Copyright (c) 2004 David Faure <faure@kde.org>
 *
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

#include "collectionaclpage.h"
#include "aclmanager.h"
#include "imapaclattribute.h"
#include "collectionaclwidget.h"
#include <KLocalizedString>
#include <akonadi/collection.h>

#include <QHBoxLayout>

using namespace PimCommon;

CollectionAclPage::CollectionAclPage( QWidget *parent )
    : CollectionPropertiesPage( parent )
{
    setObjectName( QLatin1String( "PimCommon::CollectionAclPage" ) );

    setPageTitle( i18n( "Access Control" ) );
    init();
}

void CollectionAclPage::init()
{
    QHBoxLayout *layout = new QHBoxLayout( this );
    layout->setMargin(0);
    mCollectionAclWidget = new CollectionAclWidget(this);
    layout->addWidget(mCollectionAclWidget);
}

bool CollectionAclPage::canHandle( const Akonadi::Collection &collection ) const
{
    return collection.hasAttribute<PimCommon::ImapAclAttribute>();
}

void CollectionAclPage::load( const Akonadi::Collection &collection )
{
    mCollectionAclWidget->aclManager()->setCollection( collection );
}

void CollectionAclPage::save( Akonadi::Collection &collection )
{
    mCollectionAclWidget->aclManager()->save(mCollectionAclWidget->recursive());

    // The collection dialog expects the changed collection to run
    // its own ItemModifyJob, so make him happy...
    PimCommon::ImapAclAttribute *attribute = mCollectionAclWidget->aclManager()->collection().attribute<PimCommon::ImapAclAttribute>();
    collection.addAttribute( attribute->clone() );
}

