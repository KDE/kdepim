#ifndef __KLEOPATRA_UISERVER_ECHOCOMMAND_H__
#define __KLEOPATRA_UISERVER_ECHOCOMMAND_H__

#include "assuancommand.h"
#include <QObject>

namespace Kleo {

    /*!
      \brief GnuPG UI Server command for testing

      <h3>Usage</h3>

      \code
      ECHO [--inquire <keyword>] [--nohup]
      \endcode

      <h3>Description</h3>

      The ECHO command is a simple tool for testing. If a bulk input
      channel has been set up by the client, ECHO will read data from
      it, and pipe it right back into the bulk output channel. It is
      an error for an input channel to exist without an output
      channel.

      ECHO will also send back any non-option command line arguments
      in a status message. If the --inquire command line option has
      been given, ECHO will inquire with that keyword, and send the
      received data back on the status channel.
    */
    class EchoCommand : public QObject, public AssuanCommandMixin<EchoCommand> {
        Q_OBJECT
    public:
        EchoCommand();
        ~EchoCommand();

        static const char * staticName() { return "ECHO"; }

    private:
        int start( const std::string & line );
        void canceled();

    private Q_SLOTS:
        void slotInquireData( int, const QByteArray & );
        void slotInputReadyRead();
        void slotOutputBytesWritten();

    private:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    };

}

#endif /* __KLEOPATRA_UISERVER_ECHOCOMMAND_H__ */
