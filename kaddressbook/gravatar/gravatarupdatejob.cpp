/*
  This file is part of KAddressBook.

  Copyright (c) 2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/


#include "gravatarupdatejob.h"

using namespace KABGravatar;

GravatarUpdateJob::GravatarUpdateJob(QObject *parent)
    : QObject(parent)
{

}

GravatarUpdateJob::~GravatarUpdateJob()
{

}

void GravatarUpdateJob::start()
{

}

bool GravatarUpdateJob::canStart() const
{
    return !mEmail.trimmed().isEmpty() && (mEmail.contains(QLatin1Char('@')));
}

QString GravatarUpdateJob::email() const
{
    return mEmail;
}

void GravatarUpdateJob::setEmail(const QString &email)
{
    mEmail = email;
}

Akonadi::Item GravatarUpdateJob::item() const
{
    return mItem;
}

void GravatarUpdateJob::setItem(const Akonadi::Item &item)
{
    mItem = item;
}

