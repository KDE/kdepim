/*
    kngrouppropdlg.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef KNGROUPPROPDLG_H
#define KNGROUPPROPDLG_H

#include <kdialogbase.h>

class QCheckBox;
class QComboBox;

class KLineEdit;

class KNGroup;

namespace KNConfig {
class IdentityWidget;
}


class KNGroupPropDlg : public KDialogBase  {

  public:
    KNGroupPropDlg(KNGroup *group, QWidget *parent=0, const char *name=0);
    ~KNGroupPropDlg();

    bool nickHasChanged()const { return n_ickChanged; }

  protected:
    KNGroup *g_rp;
    bool n_ickChanged;
    KNConfig::IdentityWidget* i_dWidget;
    KLineEdit *n_ick;
    QCheckBox *u_seCharset;
    QComboBox *c_harset;

  protected slots:
    void slotOk();

};

#endif
