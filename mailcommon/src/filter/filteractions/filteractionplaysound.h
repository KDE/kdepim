/*
 * Copyright (c) 1996-1998 Stefan Taferner <taferner@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef MAILCOMMON_FILTERACTIONPLAYSOUND_H
#define MAILCOMMON_FILTERACTIONPLAYSOUND_H

#include "filteractionwithtest.h"

namespace Phonon
{
class MediaObject;
}

namespace MailCommon
{

//=============================================================================
// FilterActionPlaySound - execute command
// Execute a sound
//=============================================================================
class FilterActionPlaySound : public FilterActionWithTest
{
    Q_OBJECT
public:
    FilterActionPlaySound();
    ~FilterActionPlaySound();
    ReturnCode process(ItemContext &context, bool applyOnOutbound) const Q_DECL_OVERRIDE;
    SearchRule::RequiredPart requiredPart() const Q_DECL_OVERRIDE;
    static FilterAction *newAction();
    bool argsFromStringInteractive(const QString &argsStr, const QString &filterName) Q_DECL_OVERRIDE;

    bool isEmpty() const Q_DECL_OVERRIDE;
    QString informationAboutNotValidAction() const Q_DECL_OVERRIDE;
private:
    mutable Phonon::MediaObject *mPlayer;
};

}

#endif
