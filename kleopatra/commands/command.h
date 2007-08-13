#ifndef __KLEOPATRA_COMMANDS_COMMAND_H__
#define __KLEOPATRA_COMMANDS_COMMAND_H__

#include <QObject>

#include <utils/pimpl_ptr.h>

class QModelIndex;
template <typename T> class QList;
class QAbstractItemView;

namespace Kleo {

    class KeyListController;

    class Command : public QObject {
        Q_OBJECT
    public:
        explicit Command( KeyListController * parent );
        ~Command();

        void setView( QAbstractItemView * view );
        void setIndex( const QModelIndex & idx );
        void setIndexes( const QList<QModelIndex> & idx );

    public Q_SLOTS:
        void start();
	void cancel();

    Q_SIGNALS:
        void finished();
	void canceled();

    private:
        virtual void doStart() = 0;
	virtual void doCancel() = 0;

    protected:
        class Private;
        kdtools::pimpl_ptr<Private> d;
    protected:
        explicit Command( KeyListController * parent, Private * pp );
    };

}

#endif /* __KLEOPATRA_COMMANDS_COMMAND_H__ */
