/* -*- mode: c++; c-basic-offset:4 -*-
    core/decryptverifyfilescommand.cpp

    This file is part of KleopatraClient, the Kleopatra interface library
    Copyright (c) 2008 Klarälvdalens Datakonsult AB

    KleopatraClient is free software; you can redistribute it and/or modify
    it under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    KleopatraClient is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "decryptverifyfilescommand.h"

using namespace KLEOPATRACLIENT_NAMESPACE;

DecryptVerifyFilesCommand::DecryptVerifyFilesCommand( QObject * p )
    : Command( p )
{
    setCommand( "DECRYPT_VERIFY_FILES" );
    setOption( "nohup" );
}

DecryptVerifyFilesCommand::~DecryptVerifyFilesCommand() {}

#include "moc_decryptverifyfilescommand.cpp"
