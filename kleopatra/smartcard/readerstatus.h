/* -*- mode: c++; c-basic-offset:4 -*-
    smartcard/readerstatus.h

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

#ifndef __KLEOPATRA__SMARTCARD__READERSTATUS_H___
#define __KLEOPATRA__SMARTCARD__READERSTATUS_H___

#include <QObject>
#include <QMetaType>

#include <utils/pimpl_ptr.h>

#include <vector>

namespace Kleo
{
namespace SmartCard
{

class ReaderStatus : public QObject
{
    Q_OBJECT
public:
    explicit ReaderStatus(QObject *parent = 0);
    ~ReaderStatus();

    static const ReaderStatus *instance();
    static ReaderStatus *mutableInstance();

    void startSimpleTransaction(const QByteArray &cmd, QObject *receiver, const char *slot);

    enum AppType {
        UnknownApplication,
        OpenPGPApplication,
        NksApplication,
        P15Application,
        DinSigApplication,
        GeldkarteApplication,

        NumAppTypes
    };

    enum PinState {
        UnknownPinState,
        NullPin,
        PinBlocked,
        NoPin,
        PinOk,

        NumPinStates
    };

    enum Status {
        NoCard,
        CardPresent,
        CardActive,
        CardUsable,

        _NumScdStates,

        CardCanLearnKeys = _NumScdStates,
        CardHasNullPin,

        CardError,

        NumStates
    };

    Status cardStatus(unsigned int slot) const;
    bool anyCardHasNullPin() const;
    bool anyCardCanLearnKeys() const;

    std::vector<PinState> pinStates(unsigned int slot) const;

public Q_SLOTS:
    void updateStatus();
    void startMonitoring();

Q_SIGNALS:
    void anyCardHasNullPinChanged(bool);
    void anyCardCanLearnKeysChanged(bool);
    void cardStatusChanged(unsigned int slot, Kleo::SmartCard::ReaderStatus::Status status);

private:
    class Private;
    kdtools::pimpl_ptr<Private> d;
};

} // namespace SmartCard
} // namespace Kleo

Q_DECLARE_METATYPE(Kleo::SmartCard::ReaderStatus::Status)

#endif /* __KLEOPATRA__SMARTCARD__READERSTATUS_H___ */
