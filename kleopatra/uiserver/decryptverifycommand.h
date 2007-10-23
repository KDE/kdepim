#ifndef __KLEOPATRA_UISERVER_DECRYPTVERIFYCOMMAND_H__
#define __KLEOPATRA_UISERVER_DECRYPTVERIFYCOMMAND_H__

#include "assuancommand.h"

#include <utils/pimpl_ptr.h>

namespace Kleo {

    class DecryptVerifyCommand : public AssuanCommandMixin<DecryptVerifyCommand> {
    public:
        enum Flags {
            DecryptOff = 0x0,
            DecryptOn = 0x1,
            DecryptImplied = 0x2,

            DecryptMask = 0x3,

            VerifyOff = 0x00,
            //VerifyOn  = 0x10, // non-sensical
            VerifyImplied = 0x20,

            VerifyMask = 0x30,

            DefaultFlags = DecryptImplied|VerifyImplied
        };

        explicit DecryptVerifyCommand();
        ~DecryptVerifyCommand();

    private:
        virtual unsigned int operation() const { return DefaultFlags; }
        virtual Mode mode() const { return checkMode(); }

    private:
        int doStart();
        void doCanceled();
    public:
        static const char * staticName() { return "DECRYPT_VERIFY"; }

        class Private;
    private:
        kdtools::pimpl_ptr<Private> d;
    };

}

#endif // __KLEOPATRA_UISERVER_DECRYPTCOMMAND_H__
