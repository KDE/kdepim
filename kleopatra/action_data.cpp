#include "action_data.h"

#include <KToggleAction>
#include <KActionCollection>

using namespace Kleo;

KAction * Kleo::make_action_from_data( const action_data & ad, QObject * parent ) {

    KAction * const a = ad.toggle ? new KToggleAction( parent ) : new KAction( parent ) ;
    a->setObjectName( ad.name );
    a->setText( ad.text );
    if ( !ad.tooltip.isEmpty() )
        a->setToolTip( ad.tooltip );
    if ( ad.icon )
        a->setIcon( KIcon( ad.icon ) );
    if ( ad.receiver && ad.slot )
        if ( ad.toggle )
            QObject::connect( a, SIGNAL(toggled(bool)), ad.receiver, ad.slot );
        else
            QObject::connect( a, SIGNAL(triggered()), ad.receiver, ad.slot );
    if ( !ad.shortcut.isEmpty() )
        a->setShortcuts( KShortcut( ad.shortcut ) );
    a->setEnabled( ad.enabled );
    return a;
}

void Kleo::make_actions_from_data( const action_data * ads, unsigned int size, QObject * parent ) {
    for ( unsigned int i = 0 ; i < size ; ++i )
        make_action_from_data( ads[i], parent );
}

void Kleo::make_actions_from_data( const action_data * ads, unsigned int size, KActionCollection * coll ) {
    for ( unsigned int i = 0 ; i < size ; ++i )
        coll->addAction( ads[i].name, make_action_from_data( ads[i], coll ) );
}
