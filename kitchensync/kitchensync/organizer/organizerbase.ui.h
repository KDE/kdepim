/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/
#include <kconfig.h>
#include <kfiledialog.h>
#include <kurl.h>

#include "organizerbase.h"

void OrganizerDialogBase::slotDefault()
{
KConfig conf( "korganizerrc" );
conf.setGroup("General" );
urlReq->setURL(conf.readEntry("Active Calendar" ) );
}

