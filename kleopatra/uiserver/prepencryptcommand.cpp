#include "prepencryptcommand.h"
#include "encryptemailcontroller.h"
#include "kleo-assuan.h"

#include <KLocale>

using namespace Kleo;
using namespace boost;

class PrepEncryptCommand::Private : public QObject {
    Q_OBJECT
private:
    friend class ::Kleo::PrepEncryptCommand;
    PrepEncryptCommand * const q;
public:
    explicit Private( PrepEncryptCommand * qq )
        : q( qq ), controller() {}

private:
    void checkForErrors() const;

public Q_SLOTS:
    void slotRecipientsResolved();
    void slotError( int, const QString & );

private:
    shared_ptr<EncryptEMailController> controller;
};

PrepEncryptCommand::PrepEncryptCommand()
    : AssuanCommandMixin<PrepEncryptCommand>(), d( new Private( this ) )
{

}

PrepEncryptCommand::~PrepEncryptCommand() {}

void PrepEncryptCommand::Private::checkForErrors() const {

    if ( q->numBulkInputDevices() || q->numBulkOutputDevices() || q->numBulkMessageDevices() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "INPUT/OUTPUT/MESSAGE may only be given after PREP_ENCRYPT" ) );
    
    if ( q->numFiles() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "PREP_ENCRYPT is an email mode command, connection seems to be in filemanager mode" ) );

    if ( !q->senders().empty() )
        throw assuan_exception( makeError( GPG_ERR_CONFLICT ),
                                i18n( "SENDER may not be given prior to PREP_ENCRYPT" ) );

    if ( q->recipients().empty() )
        throw assuan_exception( makeError( GPG_ERR_MISSING_VALUE ),
                                i18n( "No recipients given" ) );

}

int PrepEncryptCommand::doStart() {

    removeMemento( EncryptEMailController::mementoName() );

    d->checkForErrors();

    d->controller.reset( new EncryptEMailController );

    d->controller->setCommand( shared_from_this() );

    if ( hasOption( "protocol" ) )
        // --protocol is optional for PREP_ENCRYPT
        d->controller->setProtocol( checkProtocol( EMail ) );

    QObject::connect( d->controller.get(), SIGNAL(recipientsResolved()), d.get(), SLOT(slotRecipientsResolved()) );
    QObject::connect( d->controller.get(), SIGNAL(error(int,QString)), d.get(), SLOT(slotError(int,QString)) );

    d->controller->startResolveRecipients( recipients() );

    return 0;
}

void PrepEncryptCommand::Private::slotRecipientsResolved() {
    if ( const int err = q->sendStatus( "PROTOCOL", controller->protocolAsString() ) )
        q->done( err, i18n( "Failed to send PROTOCOL status" ) );
    q->registerMemento( EncryptEMailController::mementoName(),
                        make_typed_memento( controller ) );
    q->done();
}

void PrepEncryptCommand::Private::slotError( int err, const QString & details ) {
    q->done( err, details );
}

void PrepEncryptCommand::doCanceled() {
    d->controller->cancel();
}

#include "prepencryptcommand.moc"
