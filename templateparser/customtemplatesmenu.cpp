/*
 * Copyright (C) 2006 Dmitry Morozhnikov <dmiceman@ubiz.ru>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "customtemplatesmenu.h"
#include "customtemplates.h"
#include "customtemplates_kfg.h"
#include "globalsettings_base.h"

#include <KActionCollection>
#include <KActionMenu>
#include <KIcon>
#include <KLocalizedString>
#include <KMenu>

#include <QSignalMapper>

using namespace TemplateParser;

CustomTemplatesMenu::CustomTemplatesMenu( QWidget *owner, KActionCollection *ac )
{
    mOwnerActionCollection = ac;

    mCustomForwardActionMenu = new KActionMenu( KIcon( QLatin1String("mail-forward-custom") ),
                                                i18n( "With Custom Template" ), owner );
    mOwnerActionCollection->addAction( QLatin1String("custom_forward"), mCustomForwardActionMenu );

    mCustomReplyActionMenu = new KActionMenu( KIcon( QLatin1String("mail-reply-custom") ),
                                              i18n( "Reply With Custom Template" ), owner );
    mOwnerActionCollection->addAction( QLatin1String("custom_reply"), mCustomReplyActionMenu );

    mCustomReplyAllActionMenu = new KActionMenu( KIcon( QLatin1String("mail-reply-all-custom") ),
                                                 i18n( "Reply to All With Custom Template" ), owner );
    mOwnerActionCollection->addAction( QLatin1String("custom_reply_all"), mCustomReplyAllActionMenu );

    mCustomForwardMapper = new QSignalMapper( this );
    connect( mCustomForwardMapper, SIGNAL(mapped(int)),
             this, SLOT(slotForwardSelected(int)) );

    mCustomReplyMapper = new QSignalMapper( this );
    connect( mCustomReplyMapper, SIGNAL(mapped(int)),
             this, SLOT(slotReplySelected(int)) );

    mCustomReplyAllMapper = new QSignalMapper( this );
    connect( mCustomReplyAllMapper, SIGNAL(mapped(int)),
             this, SLOT(slotReplyAllSelected(int)) );

    update();
}

CustomTemplatesMenu::~CustomTemplatesMenu()
{
    clear();

    delete mCustomReplyActionMenu;
    delete mCustomReplyAllActionMenu;
    delete mCustomForwardActionMenu;

    delete mCustomReplyMapper;
    delete mCustomReplyAllMapper;
    delete mCustomForwardMapper;
}

void CustomTemplatesMenu::clear()
{
    QListIterator<QAction *> ait( mCustomTemplateActions );
    while ( ait.hasNext() ) {
        QAction *action = ait.next();
        mCustomReplyMapper->removeMappings( action );
        mCustomReplyAllMapper->removeMappings( action );
        mCustomForwardMapper->removeMappings( action );
    }
    qDeleteAll( mCustomTemplateActions );
    mCustomTemplateActions.clear();

    mCustomReplyActionMenu->menu()->clear();
    mCustomReplyAllActionMenu->menu()->clear();
    mCustomForwardActionMenu->menu()->clear();
    mCustomTemplates.clear();
}

void CustomTemplatesMenu::update()
{
    clear();

    const QStringList list = GlobalSettings::self()->customTemplates();
    QStringList::const_iterator it = list.constBegin();
    QStringList::const_iterator end = list.constEnd();
    int idx = 0;
    int replyc = 0;
    int replyallc = 0;
    int forwardc = 0;
    for ( ; it != end; ++it ) {
        CTemplates t( *it );
        mCustomTemplates.append( *it );
        QString nameAction( *it );
        nameAction.replace( QLatin1Char('&'), QLatin1String("&&") );

        const QString nameActionName = nameAction.replace(QLatin1Char(' '), QLatin1Char('_'));

        QAction *action;
        switch ( t.type() ) {
        case CustomTemplates::TReply:
            action = new QAction( nameAction, mOwnerActionCollection ); //krazy:exclude=tipsandthis
            action->setShortcut( t.shortcut() );
            mOwnerActionCollection->addAction(nameActionName,action);
            connect( action, SIGNAL(triggered(bool)), mCustomReplyMapper, SLOT(map()) );
            mCustomReplyMapper->setMapping( action, idx );
            mCustomReplyActionMenu->addAction( action );
            mCustomTemplateActions.append( action );
            ++replyc;
            break;

        case CustomTemplates::TReplyAll:
            action = new QAction( nameAction, mOwnerActionCollection ); //krazy:exclude=tipsandthis
            action->setShortcut( t.shortcut() );
            mOwnerActionCollection->addAction(nameActionName,action);
            connect( action, SIGNAL(triggered(bool)), mCustomReplyAllMapper, SLOT(map()) );
            mCustomReplyAllMapper->setMapping( action, idx );
            mCustomReplyAllActionMenu->addAction( action );
            mCustomTemplateActions.append( action );
            ++replyallc;
            break;

        case CustomTemplates::TForward:
            action = new QAction( nameAction, mOwnerActionCollection ); //krazy:exclude=tipsandthis
            mOwnerActionCollection->addAction(nameActionName,action);
            action->setShortcut( t.shortcut() );
            connect( action, SIGNAL(triggered(bool)), mCustomForwardMapper, SLOT(map()) );
            mCustomForwardMapper->setMapping( action, idx );
            mCustomForwardActionMenu->addAction( action );
            mCustomTemplateActions.append( action );
            ++forwardc;
            break;

        case CustomTemplates::TUniversal:
            action = new QAction( nameAction, mOwnerActionCollection ); //krazy:exclude=tipsandthis
            mOwnerActionCollection->addAction(nameActionName,action);

            connect( action, SIGNAL(triggered(bool)), mCustomReplyMapper, SLOT(map()) );
            mCustomReplyMapper->setMapping( action, idx );
            mCustomReplyActionMenu->addAction( action );
            mCustomTemplateActions.append( action );
            ++replyc;
            action = new QAction( nameAction, mOwnerActionCollection ); //krazy:exclude=tipsandthis
            connect( action, SIGNAL(triggered(bool)), mCustomReplyAllMapper, SLOT(map()) );
            mCustomReplyAllMapper->setMapping( action, idx );
            mCustomReplyAllActionMenu->addAction( action );
            mCustomTemplateActions.append( action );
            ++replyallc;
            action = new QAction( nameAction, mOwnerActionCollection ); //krazy:exclude=tipsandthis
            connect( action, SIGNAL(triggered(bool)), mCustomForwardMapper, SLOT(map()) );
            mCustomForwardMapper->setMapping( action, idx );
            mCustomForwardActionMenu->addAction( action );
            mCustomTemplateActions.append( action );
            ++forwardc;
            break;
        }

        ++idx;
    }

    if ( !replyc ) {
        QAction *noAction =
                mCustomReplyActionMenu->menu()->addAction( i18n( "(no custom templates)" ) );
        noAction->setEnabled( false );
        mCustomReplyActionMenu->setEnabled( false );
    }
    if ( !replyallc ) {
        QAction *noAction =
                mCustomReplyAllActionMenu->menu()->addAction( i18n( "(no custom templates)" ) );
        noAction->setEnabled( false );
        mCustomReplyAllActionMenu->setEnabled( false );
    }
    if ( !forwardc ) {
        QAction *noAction =
                mCustomForwardActionMenu->menu()->addAction( i18n( "(no custom templates)" ) );
        noAction->setEnabled( false );
        mCustomForwardActionMenu->setEnabled( false );
    }
}

void CustomTemplatesMenu::slotReplySelected( int idx )
{
    emit replyTemplateSelected( mCustomTemplates.at( idx ) );
}

void CustomTemplatesMenu::slotReplyAllSelected( int idx )
{
    emit replyAllTemplateSelected( mCustomTemplates.at( idx ) );
}

void CustomTemplatesMenu::slotForwardSelected( int idx )
{
    emit forwardTemplateSelected( mCustomTemplates.at( idx ) );
}

