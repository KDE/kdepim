#ifndef __KLEOPATRA_COMMANDS_DETAILSCOMMAND_H__
#define __KLEOPATRA_COMMANDS_DETAILSCOMMAND_H__

#include <commands/command.h>

namespace Kleo {

    class DetailsCommand : public Command {
        Q_OBJECT
    public:
	explicit DetailsCommand( KeyListController * parent );
	~DetailsCommand();

    private:
	/* reimp */ void doStart();
	/* reimp */ void doCancel();
	
    private:
	class Private;
	inline Private * d_func();
	inline const Private * d_func() const;
    };

}

#endif /* __KLEOPATRA_COMMANDS_DETAILSCOMMAND_H__ */
