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
#include "encodingshelper.h"
#include <kdebug.h>
#include <qregexp.h>
//Added by qt3to4:
#include <Q3MemArray>
#include <klocale.h>
#include <stdio.h>
#include "enginedata.h"

class EncodingsHelperPrivate {
};

namespace KMobileTools {

EncodingsHelper::EncodingsHelper()
    : d(new EncodingsHelperPrivate)
{
}


EncodingsHelper::~EncodingsHelper()
{
}

QString EncodingsHelper::fromUCS2(const QString &s)
{
    QString out;
    if(s.length()%4 || s.contains( QRegExp("[^\\dA-F]+") ) != 0 ) return s;
    for(int i=0; i<s.length(); i+=4)
    {
//         kDebug() << s.mid(i, 4) << "<<--->>"<< (s.mid(i, 4).toUInt(0, 16)) << "<<--->>" << QChar(s.mid(i, 4).toUInt(0, 16)) << endl;
        out+= QChar(s.mid(i, 4).toUInt(0, 16));
    }
    return out;
}

QString EncodingsHelper::from8bit(const QString &s)
{
    QString out;
    if(s.length()%2 || s.contains( QRegExp("[^\\dA-F]+") ) != 0 ) return s;
    for(int i=0; i<s.length(); i+=2)
    {
        out+=QChar(s.mid( i,2).toUInt( 0, 16));
    }
    return out;
}

QString EncodingsHelper::toUCS2(const QString &s)
{
    QString out;
    for(int i=0; i<s.length(); i++)
        out+=QString("%1").arg( s.utf16()[i], 4, 16 );
    out=out.replace( ' ', '0' ).toUpper();
    return out;
}

int EncodingsHelper::hasEncoding( const QString &s, bool prefereGSM)
{
    if(s.isEmpty() ) return Null;
    int maxval=0;
    bool isgsm=true;
    QString gsmAlphabet(alphabet7bit, gsmlen);
    for(int i=0; i<s.length(); i++)
    {
        if(s.at( i ).unicode() >= maxval) maxval=s.at( i ).unicode();
        if(!gsmAlphabet.contains( s.at(i) ) ) isgsm=false;
    }
    if(prefereGSM && isgsm) return GSM;
    if(maxval<128) return Ascii;
    if(isgsm) return GSM;
    if(maxval < 256) return Local8Bit;
    return UCS2;
}

QString EncodingsHelper::encodingNameString( int encoding )
{
    switch( encoding ){
        case Ascii:
            return "Ascii";
        case GSM:
            return "GSM";
        case Local8Bit:
            return "8BIT";
        case UCS2:
            return "UCS2 (16BIT)";
        default:
            return i18nc("Unknown encoding", "Unknown");
    }
}


/*!
    \fn KMobileTools::EncodingsHelper::decodeGSM(const QString &s)
 */
QString EncodingsHelper::decodeGSM(const QString &s)
{
    Q3MemArray<QChar> memArray;
    memArray.resize( s.length());
    for(int i=0; i<s.length(); i++)
        memArray[i]=s.at(i);
    return decodeGSM(memArray);
}
/*!
    \fn KMobileTools::EncodingsHelper::decodeGSM(const QMemArray<QChar> &s)
 */
    QString EncodingsHelper::decodeGSM(const Q3MemArray<QChar> &s)
{
    QString out;
    uint i=0;
    QString extChars(extchars, 10);
    while(i<s.size())
    {
        switch( s.at(i).toAscii() ){
            case 0x1b:
                if( (i+1) < s.size() && extChars.contains(s.at(i+1) ) )
                {
                    out+=alphabet7bit[128+extChars.indexOf(s.at(i+1))];
                    i++;
                } else out+=alphabet7bit[s.at(i).toAscii() ];
                break;
            case 0x00:
                /// @TODO handle this a bit better...
                out+=alphabet7bit[s.at(i).toAscii() ];
                break;
            default:
                out+=alphabet7bit[s.at(i).toAscii() ];
        }
        i++;
    }
    return out;
}


/*!
    \fn KMobileTools::EncodingsHelper::encodeGSM(const QString &s)
 */
Q3MemArray<QChar> EncodingsHelper::encodeGSM(const QString &s)
{
    Q3MemArray<QChar> retval/*, retval2*/;
    QString gsmAlphabet(alphabet7bit, gsmlen);
    int found=0, foundlen=0;
//     char *data=new char[s.length()*2];
    for(int i=0; i<s.length(); i++)
    {
        retval.resize( foundlen+1);
        found=gsmAlphabet.indexOf( s.at(i) );
        if(found<128)
        {
            retval[foundlen]=found;
            foundlen++;
        } else
        {
            retval[foundlen]=0x1b;
            foundlen++;
            retval.resize( foundlen+1);
            retval[foundlen]=extchars[found-128].toLatin1();
            foundlen++;
        }
    }
//     retval.setRawData( data, foundlen );
//     kDebug() << retval.nrefs() << endl;
//     retval=retval.copy();
//     retval2=retval.copy();
//     delete [] data;
    return retval;
}

QString EncodingsHelper::getHexString( const QString &s, int fieldLen)
{
    QString out;
    for(int i=0; i<s.length(); i++)
        out+=QString("%1").arg(s.at(i).unicode(), fieldLen, 16);
    return out.replace(' ', '0').toUpper();
}

QString EncodingsHelper::getHexString( const Q3MemArray<QChar> &s, int fieldLen)
{
    QString out;
    for ( uint i=0; i<s.size(); i++)
        out+=QString("%1").arg(s.at(i).unicode(),fieldLen,16);
    return out.replace(' ', '0').toUpper();
}

Q3MemArray<QChar> EncodingsHelper::string2memarray( const QString &s)
{
    Q3MemArray<QChar> out(s.length() );
    for(int i=0; i<s.length(); i++)
        out[i]=s.at(i);
    return out;
}

Q3MemArray<QChar> EncodingsHelper::hexstring2memarray( const QString &s)
{
    Q3MemArray<QChar> out(s.length() / 2 );
    for(int i=0; i<s.length(); i+=2){
        out[i/2]=(uchar) s.mid( i, 2 ).toInt( NULL, 16 );
//         kDebug() << s.mid( i, 2 ) << " <--> " << uint((uchar) out[i/2]) << " (i/2==" << i/2 << ")\n";
    }
    return out;
}

QString EncodingsHelper::memarray2string( const Q3MemArray<QChar> &s)
{
    QString out;
    for(uint i=0; i<s.size(); i++)
        out+=s.at(i);
    return out;
}


}

