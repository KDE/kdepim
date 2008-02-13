#ifndef __KLEOPATRA_COMMMANDS_CLEARCRLCACHECOMMAND_H__
#define __KLEOPATRA_COMMMANDS_CLEARCRLCACHECOMMAND_H__

#include <commands/command.h>

namespace Kleo {
namespace Commands {

    class ClearCrlCacheCommand : public Command {
        Q_OBJECT
    public:
        explicit ClearCrlCacheCommand( QAbstractItemView * view, KeyListController * parent );
        explicit ClearCrlCacheCommand( KeyListController * parent );
        ~ClearCrlCacheCommand();

    private:
        /* reimp */ void doStart();
        /* reimp */ void doCancel();

    private:
        class Private;
        inline Private * d_func();
        inline const Private * d_func() const;
        Q_PRIVATE_SLOT( d_func(), void slotProcessFinished( int, QProcess::ExitStatus ) )
        Q_PRIVATE_SLOT( d_func(), void slotProcessReadyReadStandardError() )
    };

}
}

#endif // __KLEOPATRA_COMMMANDS_CLEARCRLCACHECOMMAND_H__
