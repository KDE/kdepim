// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/**
 * attachment.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#include "attachment.h"

using namespace Komposer;

class Attachment::Private
{
public:
  TQString name;
  TQCString cte;
  TQByteArray data;
  TQCString type;
  TQCString subType;
  TQCString paramAttr;
  TQString paramValue;
  TQCString contDisp;
};

Attachment::Attachment( const TQString &name,
                        const TQCString &cte,
                        const TQByteArray &data,
                        const TQCString &type,
                        const TQCString &subType,
                        const TQCString &paramAttr,
                        const TQString &paramValue,
                        const TQCString &contDisp )
  : d( new Private )
{
  d->name = name;
  d->cte = cte;
  d->data = data;
  d->type = type;
  d->subType = subType;
  d->paramAttr = paramAttr;
  d->paramValue = paramValue;
  d->contDisp = contDisp;
}

Attachment::~Attachment()
{
  delete d; d = 0;
}

QString
Attachment::name() const
{
  return d->name;
}

QCString
Attachment::cte() const
{
  return d->cte;
}

QByteArray
Attachment::data() const
{
  return d->data;
}

QCString
Attachment::type() const
{
  return d->type;
}


QCString
Attachment::subType() const
{
  return d->subType;
}

QCString
Attachment::paramAttr() const
{
  return d->paramAttr;
}

QString
Attachment::paramValue() const
{
  return d->paramValue;
}

QCString
Attachment::contentDisposition() const
{
  return d->contDisp;
}

