// $Id$
// (C) 2001 by Cornelius Schumacher

#ifndef _KANDYPREFSDIALOG_H
#define _KANDYPREFSDIALOG_H

#include <qframe.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>

#include <kdialogbase.h>

#include "kprefsdialog.h"

/**
  Dialog to change the kandy configuration.
*/
class KandyPrefsDialog : public KPrefsDialog
{
    Q_OBJECT
  public:
    /** Initialize dialog and pages */
    KandyPrefsDialog(QWidget *parent=0,char *name=0,bool modal=false);
    ~KandyPrefsDialog();

  protected:
    void setupSerialTab();
};

#endif
