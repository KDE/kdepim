/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Qt includes
#include <qvbox.h>

// KDE includes
#include <klocale.h>

// Local includes
#include "EmpathSettingsDialog.h"
#include "IdentitySettingsForm.h"
#include "ComposeSettingsForm.h"
#include "DisplaySettingsForm.h"
#include "SendingSettingsForm.h"
#include "AccountsSettingsForm.h"

EmpathSettingsDialog * EmpathSettingsDialog::instance_ = 0L;

EmpathSettingsDialog::EmpathSettingsDialog()
    :   KDialogBase(
            IconList,
            i18n("Settings"),
            Help | Default | Apply | Ok | Cancel,
            Cancel,
            (QWidget *)0L,
            "EmpathSettingsDialog",
            false,
            true
       )
{
    identitySettingsForm =
        new IdentitySettingsForm(addVBoxPage(i18n("Identity")));

    displaySettingsForm  =
        new DisplaySettingsForm(addVBoxPage(i18n("Display")));

    composeSettingsForm  =
        new ComposeSettingsForm(addVBoxPage(i18n("Composing")));

    sendingSettingsForm  =
        new SendingSettingsForm(addVBoxPage(i18n("Sending")));

    accountsSettingsForm =
        new AccountsSettingsForm(addVBoxPage(i18n("Accounts")));
}

EmpathSettingsDialog::~EmpathSettingsDialog()
{
}

    void
EmpathSettingsDialog::slotApply()
{
}

    void
EmpathSettingsDialog::slotDefault()
{
}

    void
EmpathSettingsDialog::slotCancel()
{
}

// vim:ts=4:sw=4:tw=78
