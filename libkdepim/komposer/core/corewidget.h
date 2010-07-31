// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
/**
 * corewidget.h
 *
 * Copyright (C)  2003-2004  Zack Rusin <zack@kde.org>
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
#ifndef COREWIDGET_H
#define COREWIDGET_H

#include "attachment.h"

#include <tqwidget.h>

namespace Komposer
{

  class CoreWidget : public QWidget
  {
    Q_OBJECT
  public:
    CoreWidget( TQWidget *parent, const char *name=0 );

    virtual TQString subject() const =0;
    virtual TQStringList to()  const =0;
    virtual TQStringList cc()  const =0;
    virtual TQStringList bcc() const =0;
    virtual TQString from() const =0;
    virtual TQString replyTo() const =0;
    virtual AttachmentList attachments() const =0;
  };
}

#endif
