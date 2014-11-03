/*  -*- mode: C++; c-file-style: "gnu" -*-
    qgpgmeprogresstokenmapper.cpp

    This file is part of libkleopatra, the KDE keymanagement library
    Copyright (c) 2004 Klarälvdalens Datakonsult AB

    Libkleopatra is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    Libkleopatra is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

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

#include "qgpgmeprogresstokenmapper.h"

#include <KLocalizedString>
#include <QDebug>
#include "gpgme_backend_debug.h"

#include <QString>

#include <boost/range.hpp>

#include <cassert>

struct Desc {
    int type; // 0 == fallback
    const char *display;  // add %1 for useCur ^ useTot and %1 %2 for useCur == useTot == true
};

static const struct Desc pk_dsa[] = {
    { 0, I18N_NOOP("Generating DSA key...") }
};

static const struct Desc pk_elg[] = {
    { 0, I18N_NOOP("Generating ElGamal key...") }
};

static const struct Desc primegen[] = {
    // FIXME: add all type's?
    { 0, I18N_NOOP("Searching for a large prime number...") }
};

static const struct Desc need_entropy[] = {
    { 0, I18N_NOOP("Waiting for new entropy from random number generator (you might want to exercise the harddisks or move the mouse)...") }
};

static const struct Desc tick[] = {
    { 0, I18N_NOOP("Please wait...") }
};

static const struct Desc starting_agent[] = {
    { 0, I18N_NOOP("Starting gpg-agent (you should consider starting a global instance instead)...") }
};

static const struct _tokens {
    const char *token;
    const Desc *desc;
    unsigned int numDesc;
} tokens[] = {
#define make_token(x) { #x, x, sizeof(x) / sizeof(*x) }
    make_token(pk_dsa),
    make_token(pk_elg),
    make_token(primegen),
    make_token(need_entropy),
    make_token(tick),
    make_token(starting_agent)
#undef make_token
};

QString Kleo::QGpgMEProgressTokenMapper::map(const char *tokenUtf8, int subtoken)
{
    if (!tokenUtf8 || !*tokenUtf8) {
        return QString();
    }

    if (qstrcmp(tokenUtf8, "file:") == 0) {
        return QString();    // gpgme's job
    }

    return map(QString::fromUtf8(tokenUtf8), subtoken);
}

QString Kleo::QGpgMEProgressTokenMapper::map(const QString &token, int subtoken)
{
    if (token.startsWith(QLatin1String("file:"))) {
        return QString();    // gpgme's job
    }

    qCDebug(GPGPME_BACKEND_LOG) << token << subtoken;

    for (const _tokens *it = boost::begin(tokens), *end = boost::end(tokens) ; it != end ; ++it) {
        if (token.compare(QLatin1String(it->token), Qt::CaseInsensitive) == 0) {
            if (it->desc && it->numDesc) {
                for (unsigned int i = 0, e = it->numDesc ; i != e ; ++i)
                    if (it->desc[i].type == subtoken) {
                        return i18n(it->desc[i].display);
                    }
                return i18n(it->desc[0].display);
            } else {
                break;
            }
        }
    }

    return token;
}

