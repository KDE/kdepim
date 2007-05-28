/***************************************************************************
   Copyright (C) 2007
   by Marco Gulino <marco@kmobiletools.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
 ***************************************************************************/
#ifndef KMOBILETOOLSENCODINGSHELPER_H
#define KMOBILETOOLSENCODINGSHELPER_H

#include <libkmobiletools/kmobiletools_export.h>

#include <QtCore/QString>
//Added by qt3to4:
#include <Qt3Support/Q3MemArray>
#include "gsmcodec.h"

class EncodingsHelperPrivate;

namespace KMobileTools {

/**
	@author Marco Gulino <marco@kmobiletools.org>
*/
class KMOBILETOOLS_EXPORT EncodingsHelper{
public:
    EncodingsHelper();

    ~EncodingsHelper();
    static QString fromUCS2(const QString &);
    static QString from8bit(const QString &);
    static QString toUCS2(const QString &);
    enum Codecs { Null=0x0, Ascii=0x1, GSM=0x2, Local8Bit=0x3, UCS2=0x4 };
    static int hasEncoding(const QString &, bool prefereGSM=false);
    static QString encodingNameString(int encoding);
            /** Decodes a string from 7 bit GSM default alphabet as described in GSM03.38.
             */
    static QString decodeGSM(const Q3MemArray<QChar> &s);
    static QString decodeGSM(const QString &s);
    static Q3MemArray<QChar> encodeGSM(const QString &s);
    static QString getHexString( const QString &s, int fieldLen=2);
    static QString getHexString( const Q3MemArray<QChar> &s, int fieldLen=2);
    static QString memarray2string( const Q3MemArray<QChar> &s );
    static Q3MemArray<QChar> string2memarray( const QString &s );
    static Q3MemArray<QChar> hexstring2memarray ( const QString &s );

private:
    EncodingsHelperPrivate * const d;
protected:
};
}

#endif
