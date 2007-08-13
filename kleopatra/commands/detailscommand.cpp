#include "detailscommand.h"
#include "command_p.h"

#include "../certificateinfowidgetimpl.h"

#include <QAbstractItemView>

using namespace Kleo;

class DetailsCommand::Private : public Command::Private {
    friend class ::Kleo::DetailsCommand;
public:
    Private( DetailsCommand * qq );
    ~Private();

private:
    QPointer<CertificateInfoWidgetImpl> dialog;
};

DetailsCommand::Private * DetailsCommand::d_func() { return static_cast<Private*>( d.get() ); }
const DetailsCommand::Private * DetailsCommand::d_func() const { return static_cast<const Private*>( d.get() ); }


DetailsCommand::Private::Private( DetailsCommand * qq )
    : Command::Private( qq ), dialog()
{

}

DetailsCommand::Private::~Private() {}



DetailsCommand::DetailsCommand( KeyListController * p )
    : Command( p, new Private( this ) )
{

}

#define d d_func()

DetailsCommand::~DetailsCommand() {
    if ( d->dialog )
	d->dialog->close();
}

void DetailsCommand::doStart() {
    if ( d->indexes().count() != 1 ) {
	qWarning( "DetailsCommand::doStart: can only work with one certificate at a time" );
	return;
    }

    if ( !d->dialog ) {
	d->dialog = new CertificateInfoWidgetImpl( d->key(), false, d->view() );
	d->dialog->setAttribute( Qt::WA_DeleteOnClose );
    }
    if ( d->dialog->isVisible() )
	d->dialog->raise();
    else
	d->dialog->show();

}


void DetailsCommand::doCancel() {
    if ( d->dialog )
	d->dialog->close();
}

#undef d

#include "moc_detailscommand.cpp"
