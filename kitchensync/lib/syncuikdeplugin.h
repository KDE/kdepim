
#ifndef SyncUIKDEPlugin
#define SyncUIKDEPlugin

#include <qwidget.h>

namespace KSync {

    class SyncEntry;
    /**
     * the SyncUiKDEPlugin is similiar to the
     * plugin(s) found in RenameDlg of KDE
     * in KIO::RenameDlg the mimetype get's determined
     * and an approriate Plugin gets loaded.
     * SyncUIKDE differs because it loads for each
     * source and destination a plugin
     */
    class SyncUiKDEPlugin : public QWidget{
    public:
        SyncUiKDEPlugin( QWidget* parent,  const char *name, const QStringList& list )
            : QWidget( parent, name ) {};
        virtual ~SyncUiKDEPlugin() {};
        virtual void setSyncEntry( SyncEntry* entry ) = 0
    };
};
#endif
