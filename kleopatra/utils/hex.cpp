/* -*- mode: c++; c-basic-offset:4 -*-
    utils/hex.cpp

    This file is part of Kleopatra, the KDE keymanager
    Copyright (c) 2007 Klarälvdalens Datakonsult AB

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

#include "hex.h"

#include <Libkleo/Exception>

#include <KLocalizedString>

#include <QString>
#include <QByteArray>

using namespace Kleo;

static unsigned char unhex(unsigned char ch)
{
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'A' && ch <= 'F') {
        return ch - 'A' + 10;
    }
    if (ch >= 'a' && ch <= 'f') {
        return ch - 'a' + 10;
    }
    const char cch = ch;
    throw Exception(gpg_error(GPG_ERR_ASS_SYNTAX),
                    i18n("Invalid hex char '%1' in input stream.",
                         QString::fromLatin1(&cch, 1)));
}

std::string Kleo::hexdecode(const std::string &in)
{
    std::string result;
    result.reserve(in.size());
    for (std::string::const_iterator it = in.begin(), end = in.end(); it != end; ++it)
        if (*it == '%') {
            ++it;
            unsigned char ch = '\0';
            if (it == end)
                throw Exception(gpg_error(GPG_ERR_ASS_SYNTAX),
                                i18n("Premature end of hex-encoded char in input stream"));
            ch |= unhex(*it) << 4;
            ++it;
            if (it == end)
                throw Exception(gpg_error(GPG_ERR_ASS_SYNTAX),
                                i18n("Premature end of hex-encoded char in input stream"));
            ch |= unhex(*it);
            result.push_back(ch);
        } else if (*it == '+') {
            result += ' ';
        } else  {
            result.push_back(*it);
        }
    return result;
}

std::string Kleo::hexencode(const std::string &in)
{
    std::string result;
    result.reserve(3 * in.size());

    static const char hex[] = "0123456789ABCDEF";

    for (std::string::const_iterator it = in.begin(), end = in.end(); it != end; ++it)
        switch (const unsigned char ch = *it) {
        default:
            if ((ch >= '!' && ch <= '~') || ch > 0xA0) {
                result += ch;
                break;
            }
        // else fall through
        case ' ':
            result += '+';
            break;
        case '"':
        case '#':
        case '$':
        case '%':
        case '\'':
        case '+':
        case '=':
            result += '%';
            result += hex[(ch & 0xF0) >> 4 ];
            result += hex[(ch & 0x0F)      ];
            break;
        }

    return result;
}

std::string Kleo::hexdecode(const char *in)
{
    if (!in) {
        return std::string();
    }
    return hexdecode(std::string(in));
}

std::string Kleo::hexencode(const char *in)
{
    if (!in) {
        return std::string();
    }
    return hexencode(std::string(in));
}

QByteArray Kleo::hexdecode(const QByteArray &in)
{
    if (in.isNull()) {
        return QByteArray();
    }
    const std::string result = hexdecode(std::string(in.constData()));
    return QByteArray(result.data(), result.size());
}

QByteArray Kleo::hexencode(const QByteArray &in)
{
    if (in.isNull()) {
        return QByteArray();
    }
    const std::string result = hexencode(std::string(in.constData()));
    return QByteArray(result.data(), result.size());
}
