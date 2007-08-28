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

#ifndef KTNEF_ATTACHPROPERTYDIALOG_H
#define KTNEF_ATTACHPROPERTYDIALOG_H

#include "attachpropertydialogbase.h"
#include <QMap>
#include <QPixmap>

#include <ktnef/ktnefattach.h>
#include <ktnef/ktnefproperty.h>
#include <ktnef/ktnefpropertyset.h>

class Q3ListView;
class Q3ListViewItem;

class AttachPropertyDialog : public AttachPropertyDialogBase
{
public:
	AttachPropertyDialog(QWidget *parent = 0);
	~AttachPropertyDialog();

        void setAttachment(KTnef::KTNEFAttach *attach);

protected slots:
	void saveClicked();

private:
        KTnef::KTNEFAttach *m_attach;
};

void formatProperties( const QMap<int,KTnef::KTNEFProperty*>&, Q3ListView*, Q3ListViewItem*, const QString& = "prop" );
void formatPropertySet( KTnef::KTNEFPropertySet*, Q3ListView* );
void saveProperty( Q3ListView*, KTnef::KTNEFPropertySet*, QWidget* );
QPixmap loadRenderingPixmap( KTnef::KTNEFPropertySet*, const QColor& );

#endif // KTNEF_ATTACHPROPERTYDIALOG_H
