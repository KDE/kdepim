#ifndef __KLEOPATRA_COMMANDS_COMMAND_P_H__
#define __KLEOPATRA_COMMANDS_COMMAND_P_H__

#include "command.h"
#include "controllers/keylistcontroller.h"
#include "models/keylistmodel.h"

#include <QPointer>
#include <QList>
#include <QModelIndex>

#include <gpgme++/key.h>

class Kleo::Command::Private {
    friend class ::Kleo::Command;
    Command * const q;
public:
    explicit Private( Command * qq );
    ~Private();

    QAbstractItemView * view() const { return view_; }
    AbstractKeyListModel * model() const { return controller_ ? controller_->model() : 0 ; }
    const QList<QModelIndex> & indexes() const { return indexes_; }
    GpgME::Key key() const { return model() && !indexes_.empty() ? model()->key( indexes_.front() ) : GpgME::Key::null ; }
    std::vector<GpgME::Key> keys() const { return model() ? model()->keys( indexes_ ) : std::vector<GpgME::Key>() ; }

private:
    QList<QModelIndex> indexes_;
    QPointer<QAbstractItemView> view_;
    QPointer<KeyListController> controller_;
};

#endif /* __KLEOPATRA_COMMANDS_COMMAND_P_H__ */
