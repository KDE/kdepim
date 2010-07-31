/*
    attachpropertydialog.h

    Copyright (C) 2002 Michael Goffioul <kdeprint@swing.be>

    This file is part of KTNEF, the KDE TNEF support library/program.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef ATTACHPROPERTYDLG_H
#define ATTACHPROPERTYDLG_H

#include "attachpropertydialogbase.h"
#include <tqmap.h>
#include <tqpixmap.h>

class KTNEFAttach;
class KTNEFProperty;
class KTNEFPropertySet;
class QListView;
class QListViewItem;

class AttachPropertyDialog : public AttachPropertyDialogBase
{
public:
	AttachPropertyDialog(TQWidget *parent = 0, const char *name = 0);
	~AttachPropertyDialog();

	void setAttachment(KTNEFAttach *attach);

protected slots:
	void saveClicked();

private:
	KTNEFAttach *m_attach;
};

void formatProperties( const TQMap<int,KTNEFProperty*>&, TQListView*, TQListViewItem*, const TQString& = "prop" );
void formatPropertySet( KTNEFPropertySet*, TQListView* );
void saveProperty( TQListView*, KTNEFPropertySet*, TQWidget* );
TQPixmap loadRenderingPixmap( KTNEFPropertySet*, const TQColor& );

#endif
