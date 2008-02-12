#ifndef __KLEOPATRA_COMMMANDS_SIGNENCRYPTFILESCOMMAND_H__
#define __KLEOPATRA_COMMMANDS_SIGNENCRYPTFILESCOMMAND_H__

#include <commands/command.h>

#include <crypto/controller.h>

namespace Kleo {
namespace Commands {

    class SignEncryptFilesCommand : public Command, public Crypto::ExecutionContext {
        Q_OBJECT
    public:
        explicit SignEncryptFilesCommand( QAbstractItemView * view, KeyListController * parent );
        explicit SignEncryptFilesCommand( KeyListController * parent );
        ~SignEncryptFilesCommand();

    private:
        /* reimp */ void doStart();
        /* reimp */ void doCancel();

        /* reimp */ void applyWindowID( QDialog * dlg ) const;

    private:
        class Private;
        inline Private * d_func();
        inline const Private * d_func() const;
        Q_PRIVATE_SLOT( d_func(), void slotControllerDone() )
        Q_PRIVATE_SLOT( d_func(), void slotControllerError(int,QString) )
    };

}
}

#endif // __KLEOPATRA_COMMMANDS_SIGNENCRYPTFILESCOMMAND_H__
