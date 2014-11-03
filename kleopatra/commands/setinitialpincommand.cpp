/* -*- mode: c++; c-basic-offset:4 -*-
    commands/setinitialpincommand.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2009 Klarälvdalens Datakonsult AB

    Kleopatra is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Kleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

    In addition, as a special exception, the copyright holders give
    permission to link the code of this program with any edition of
    the Qt library by Trolltech AS, Norway (or with modified versions
    of Qt that use the same license as Qt), and distribute linked
    combinations including the two.  You must obey the GNU General
    Public License in all respects for all of the code used other than
    Qt.  If you modify this file, you may extend this exception to
    your version of the file, but you are not obligated to do so.  If
    you do not wish to do so, delete this exception statement from
    your version.
*/

#include <config-kleopatra.h>

#include "setinitialpincommand.h"
#include "command_p.h"

#include <dialogs/setinitialpindialog.h>

#include <smartcard/readerstatus.h>

#include <KLocalizedString>

#include <cassert>

using namespace Kleo;
using namespace Kleo::Commands;
using namespace Kleo::Dialogs;
using namespace Kleo::SmartCard;
using namespace GpgME;

class SetInitialPinCommand::Private : public Command::Private
{
    friend class ::Kleo::Commands::SetInitialPinCommand;
    SetInitialPinCommand *q_func() const
    {
        return static_cast<SetInitialPinCommand *>(q);
    }
public:
    explicit Private(SetInitialPinCommand *qq);
    ~Private();

private:
    void init()
    {

    }

    void ensureDialogCreated() const
    {
        if (dialog) {
            return;
        }

        SetInitialPinDialog *dlg = new SetInitialPinDialog;
        applyWindowID(dlg);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->setWindowTitle(i18nc("@title", "Set Initial Pin"));

        const std::vector<ReaderStatus::PinState> pinStates = ReaderStatus::instance()->pinStates(0);

        dlg->setNksPinPresent(pinStates.size() >= 1 && pinStates[0] != ReaderStatus::NullPin);
        dlg->setSigGPinPresent(pinStates.size() >= 3 && pinStates[2] != ReaderStatus::NullPin);

        connect(dlg, SIGNAL(nksPinRequested()), q_func(), SLOT(slotNksPinRequested()));
        connect(dlg, SIGNAL(sigGPinRequested()), q_func(), SLOT(slotSigGPinRequested()));
        connect(dlg, SIGNAL(rejected()), q_func(), SLOT(slotDialogRejected()));
        connect(dlg, SIGNAL(accepted()), q_func(), SLOT(slotDialogAccepted()));

        dialog = dlg;
    }

    void ensureDialogVisible()
    {
        ensureDialogCreated();
        if (dialog->isVisible()) {
            dialog->raise();
        } else {
            dialog->show();
        }
    }

private:
    void slotNksPinRequested()
    {
        ReaderStatus::mutableInstance()
        ->startSimpleTransaction("SCD PASSWD --nullpin PW1.CH",
                                 dialog, "setNksPinSettingResult");
    }

    void slotSigGPinRequested()
    {
        ReaderStatus::mutableInstance()
        ->startSimpleTransaction("SCD PASSWD --nullpin PW1.CH.SIG",
                                 dialog, "setSigGPinSettingResult");
    }

    void slotDialogRejected()
    {
        if (dialog->isComplete()) {
            slotDialogAccepted();
        } else {
            canceled();
        }
    }
    void slotDialogAccepted()
    {
        ReaderStatus::mutableInstance()->updateStatus();
        finished();
    }

private:
    mutable QPointer<SetInitialPinDialog> dialog;
};

SetInitialPinCommand::Private *SetInitialPinCommand::d_func()
{
    return static_cast<Private *>(d.get());
}
const SetInitialPinCommand::Private *SetInitialPinCommand::d_func() const
{
    return static_cast<const Private *>(d.get());
}

#define q q_func()
#define d d_func()

SetInitialPinCommand::Private::Private(SetInitialPinCommand *qq)
    : Command::Private(qq, 0),
      dialog()
{

}

SetInitialPinCommand::Private::~Private() {}

SetInitialPinCommand::SetInitialPinCommand()
    : Command(new Private(this))
{
    d->init();
}

SetInitialPinCommand::~SetInitialPinCommand() {}

QDialog *SetInitialPinCommand::dialog() const
{
    d->ensureDialogCreated();
    return d->dialog;
}

void SetInitialPinCommand::doStart()
{
    d->ensureDialogVisible();
}

void SetInitialPinCommand::doCancel()
{
    if (d->dialog) {
        d->dialog->close();
    }
}

#undef q_func
#undef d_func

#include "moc_setinitialpincommand.cpp"
