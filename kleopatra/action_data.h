#ifndef __KLEOPATRA_ACTIONDATA_H__
#define __KLEOPATRA_ACTIONDATA_H__

#include <QString>

class QObject;
class KAction;
class KActionCollection;

namespace Kleo {

    struct action_data {
        const char * name;
        QString text;
        QString tooltip;
        const char * icon;
        const QObject * receiver;
        const char * slot;
        QString shortcut;
        bool toggle;
        bool enabled;
    };

    void make_actions_from_data( const action_data * data, unsigned int numData, KActionCollection * collection );
    void make_actions_from_data( const action_data * data, unsigned int numData, QObject * parent );

    KAction * make_action_from_data( const action_data & data, QObject * parent );

}

#endif /* __KLEOPATRA_ACTIONDATA_H__ */
