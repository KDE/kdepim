/* -*- mode: C++; c-file-style: "gnu" -*-
  Copyright (C) 2009 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.net
  Copyright (c) 2009 Andras Mantia <andras@kdab.net>

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

#include "global.h"
MailViewer::Global * MailViewer::Global::mSelf = 0;

using namespace MailViewer;

Global * MailViewer::Global::instance()
{
  if ( !mSelf )
    mSelf = new Global();
  return mSelf;
}

Global::Global()
{
  mSelf = this;
  mConfig = 0;
}

Global::~Global()
{
}

KSharedConfigPtr Global::config()
{
  if ( !mConfig )
    mConfig = KSharedConfig::openConfig( "mailviewerrc" );
  return mConfig;
}



