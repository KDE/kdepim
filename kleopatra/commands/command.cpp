#include "command.h"
#include "command_p.h"

#include <QAbstractItemView>

using namespace Kleo;

Command::Private::Private( Command * qq )
    : q( qq ),
      indexes_(),
      view_(),
      controller_( qobject_cast<KeyListController*>( qq->parent() ) )
{

}

Command::Private::~Private() {}

Command::Command( KeyListController * p )
    : QObject( p ), d( new Private( this ) )
{

}

Command::Command( KeyListController * p, Private * pp )
    : QObject( p ), d( pp )
{

}

Command::~Command() {}



void Command::setView( QAbstractItemView * view ) {
    d->view_ = view;
}

void Command::setIndex( const QModelIndex & idx ) {
    d->indexes_.clear();
    d->indexes_.push_back( idx );
}

void Command::setIndexes( const QList<QModelIndex> & idx ) {
    d->indexes_ = idx;
}

void Command::start() {
    doStart();
}

void Command::cancel() {
    doCancel();
    emit canceled();
}

#include "moc_command.cpp"

