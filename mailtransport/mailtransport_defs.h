/*
    Copyright (c) 2006 Volker Krause <vkrause@kde.org>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/

#ifndef KPIM_MAILTRANSPORT_DEFS_H
#define KPIM_MAILTRANSPORT_DEFS_H

/**
  @file mailtransport_defs.h
  Internal file containing constant definitions etc.
*/

#define WALLET_FOLDER "mailtransports"

#define DBUS_INTERFACE_NAME "org.kde.pim.TransportManager"
#define DBUS_OBJECT_PATH "/TransportManager"

#define SMTP_PROTOCOL "smtp"
#define SMTPS_PROTOCOL "smtps"

#define SMTP_PORT 25
#define SMTPS_PORT 465

#endif
