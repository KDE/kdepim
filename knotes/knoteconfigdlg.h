/*******************************************************************
 KNotes -- Notes for the KDE project

 Copyright (c) 1997-2004, The KNotes Developers

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*******************************************************************/

#ifndef KNOTECONFIGDLG_H
#define KNOTECONFIGDLG_H

#include <kconfigdialog.h>

class QString;
class KNoteConfig;


class KNoteConfigDlg : public KConfigDialog
{
    Q_OBJECT
public:
    KNoteConfigDlg( KNoteConfig *config, const QString &title,
                    bool defaults, QWidget *parent=0, const char *name=0 );
    ~KNoteConfigDlg();

public slots:
    void slotUpdateCaption();

protected:
    void makeDisplayPage( bool defaults );
    void makeEditorPage();
    void makeActionsPage();
};

#endif
