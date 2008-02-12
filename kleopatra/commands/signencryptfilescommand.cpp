#include <config-kleopatra.h>

#include "signencryptfilescommand.h"

#include "command_p.h"

#include <crypto/signencryptfilescontroller.h>

#include <KLocale>
#include <KMessageBox>

#include <QFileDialog>
#include <QStringList>

#include <exception>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Crypto;
using namespace boost;

namespace {
    struct nodelete {
        template <typename T>
        void operator()( const T * ) const {}
    };
}

class SignEncryptFilesCommand::Private : public Command::Private {
    friend class ::Kleo::Commands::SignEncryptFilesCommand;
    SignEncryptFilesCommand * q_func() const { return static_cast<SignEncryptFilesCommand*>( q ); }
public:
    explicit Private( SignEncryptFilesCommand * qq, KeyListController * c );
    ~Private();

    QStringList selectFiles() const;

    void init();

private:
    void slotControllerDone() {
        finished();
    }
    void slotControllerError( int, const QString & ) {
        finished();
    }

private:
    shared_ptr<const ExecutionContext> shared_qq;
    SignEncryptFilesController controller;
};


SignEncryptFilesCommand::Private * SignEncryptFilesCommand::d_func() { return static_cast<Private*>( d.get() ); }
const SignEncryptFilesCommand::Private * SignEncryptFilesCommand::d_func() const { return static_cast<const Private*>( d.get() ); }

#define d d_func()
#define q q_func()

SignEncryptFilesCommand::Private::Private( SignEncryptFilesCommand * qq, KeyListController * c )
    : Command::Private( qq, c ),
      shared_qq( qq, nodelete() ),
      controller()
{
    controller.setOperationMode( SignEncryptFilesController::SignAllowed | SignEncryptFilesController::EncryptAllowed );
}

SignEncryptFilesCommand::Private::~Private() {}

SignEncryptFilesCommand::SignEncryptFilesCommand( KeyListController * c )
    : Command( new Private( this, c ) )
{
    d->init();
}

SignEncryptFilesCommand::SignEncryptFilesCommand( QAbstractItemView * v, KeyListController * c )
    : Command( v, new Private( this, c ) )
{
    d->init();
}

void SignEncryptFilesCommand::Private::init() {
    controller.setExecutionContext( shared_qq );
    connect( &controller, SIGNAL(done()), q, SLOT(slotControllerDone()) );
    connect( &controller, SIGNAL(error(int,QString)), q, SLOT(slotControllerError(int,QString)) );
}

SignEncryptFilesCommand::~SignEncryptFilesCommand() {}

void SignEncryptFilesCommand::doStart() {

    try {

        const QStringList files = d->selectFiles();
        if ( files.empty() ) {
            d->finished();
            return;
        }

        d->controller.setFiles( files );
        d->controller.start();

    } catch ( const std::exception & e ) {
        KMessageBox::information( d->view(),
                                  i18n("An error occurred: %1",
                                       QString::fromLocal8Bit( e.what() ) ),
                                  i18n("Sign/Encrypt Files Error") );
        d->finished();
    }
}

void SignEncryptFilesCommand::doCancel() {
    d->controller.cancel();
}

void SignEncryptFilesCommand::applyWindowID( QDialog * dlg ) const {
    if ( dlg )
        dlg->setParent( d->view(), dlg->windowFlags() );
}

QStringList SignEncryptFilesCommand::Private::selectFiles() const {
    return QFileDialog::getOpenFileNames( view(), i18n( "Select one of more files to sign and/or encrypt" ) );
}

#undef d
#undef q

#include "moc_signencryptfilescommand.cpp"
