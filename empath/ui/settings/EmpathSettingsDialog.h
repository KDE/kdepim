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

#ifndef EMPATH_SETTINGS_DIALOG_H
#define EMPATH_SETTINGS_DIALOG_H

// KDE includes
#include <kdialogbase.h>

class IdentitySettingsForm;
class DisplaySettingsForm;
class ComposeSettingsForm;
class SendingSettingsForm;
class AccountsSettingsForm;

class EmpathSettingsDialog : public KDialogBase
{
    Q_OBJECT

    public:

        static void run()
        {
            if (0 == instance_)
                instance_ = new EmpathSettingsDialog;

            instance_->show();
        }

        virtual ~EmpathSettingsDialog();

    protected:

        static EmpathSettingsDialog * instance_;
        EmpathSettingsDialog();
        EmpathSettingsDialog(const EmpathSettingsDialog &);
        EmpathSettingsDialog & operator = (const EmpathSettingsDialog &);

    protected slots:

        void slotApply();
        void slotDefault();
        void slotCancel();

        void slotPreviewDepth1(const QColor &);
        void slotPreviewDepth2(const QColor &);

    private:

        void _load();

        void _identityDefaults();
        void _composeDefaults();
        void _displayDefaults();
        void _sendingDefaults();
        void _accountsDefaults();

        IdentitySettingsForm    * identitySettingsForm;
        DisplaySettingsForm     * displaySettingsForm;
        ComposeSettingsForm     * composeSettingsForm;
        SendingSettingsForm     * sendingSettingsForm;
        AccountsSettingsForm    * accountsSettingsForm;
};

#endif
// vim:ts=4:sw=4:tw=78
