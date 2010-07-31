/*******************************************************************************
**
** Filename   : util
** Created on : 03 April, 2005
** Copyright  : (c) 2005 Till Adam
** Email      : <adam@kde.org>
**
*******************************************************************************/

/*******************************************************************************
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   It is distributed in the hope that it will be useful, but
**   WITHOUT ANY WARRANTY; without even the implied warranty of
**   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
**   General Public License for more details.
**
**   You should have received a copy of the GNU General Public License
**   along with this program; if not, write to the Free Software
**   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
**   In addition, as a special exception, the copyright holders give
**   permission to link the code of this program with any edition of
**   the Qt library by Trolltech AS, Norway (or with modified versions
**   of Qt that use the same license as Qt), and distribute linked
**   combinations including the two.  You must obey the GNU General
**   Public License in all respects for all of the code used other than
**   Qt.  If you modify this file, you may extend this exception to
**   your version of the file, but you are not obligated to do so.  If
**   you do not wish to do so, delete this exception statement from
**   your version.
**
*******************************************************************************/
#ifndef KMAILUTIL_H
#define KMAILUTIL_H

#include <stdlib.h>

#include <tqobject.h>
#include <tqcstring.h>

#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <klocale.h>

class DwString;
class KURL;
class QWidget;

namespace KMail
{
    /**
     * The Util namespace contains a collection of helper functions use in
     * various places.
     */
namespace Util {
    /**
     * Convert all sequences of "\r\n" (carriage return followed by a line
     * feed) to a single "\n" (line feed). The conversion happens in place.
     * Returns the length of the resulting string.
     * @param str The string to convert.
     * @param strLen The length of the string to convert.
     * @return The new length of the converted string.
     */
    size_t crlf2lf( char* str, const size_t strLen );


    /**
     * Convert "\n" line endings to "\r\n".
     * @param src The source string to convert.
     * @return The result string.
     */
    TQCString lf2crlf( const TQCString & src );
    /**
     * Convert "\n" line endings to "\r\n".
     * @param src The source string to convert. NOT null-terminated.
     * @return The result string. NOT null-terminated.
     */
    TQByteArray lf2crlf( const TQByteArray & src );

    /**
     * Construct a TQCString from a DwString
     */
    TQCString CString( const DwString& str );

    /**
     * Construct a TQByteArray from a DwString
     */
    TQByteArray ByteArray( const DwString& str );

    /**
     * Construct a DwString from a QCString
     */
    DwString dwString( const TQCString& str );

    /**
     * Construct a DwString from a QByteArray
     */
    DwString dwString( const TQByteArray& str );

    /**
     * Fills a TQByteArray from a TQCString - removing the trailing null.
     */
    void setFromQCString( TQByteArray& arr, const TQCString& cstr );

    inline void setFromQCString( TQByteArray& arr, const TQCString& cstr )
    {
      if ( cstr.size() )
        arr.duplicate( cstr.data(), cstr.size()-1 );
      else
        arr.resize(0);
    }

    /**
     * Creates a TQByteArray from a TQCString without detaching (duplicating the data).
     * Fast, but be careful, the TQCString gets modified by this; this is only good for
     * the case where the TQCString is going to be thrown away afterwards anyway.
     */
    TQByteArray byteArrayFromQCStringNoDetach( TQCString& cstr );
    inline TQByteArray byteArrayFromQCStringNoDetach( TQCString& cstr )
    {
      TQByteArray arr = cstr;
      if ( arr.size() )
        arr.resize( arr.size() - 1 );
      return arr;
    }

    /**
     * Restore the TQCString after byteArrayFromQCStringNoDetach modified it
     */
    void restoreQCString( TQCString& str );
    inline void restoreQCString( TQCString& str )
    {
      if ( str.data() )
        str.resize( str.size() + 1 );
    }

    /**
     * Fills a TQCString from a TQByteArray - adding the trailing null.
     */
    void setFromByteArray( TQCString& cstr, const TQByteArray& arr );

    inline void setFromByteArray( TQCString& result, const TQByteArray& arr )
    {
      const int len = arr.size();
      result.resize( len + 1 /* trailing NUL */ );
      memcpy(result.data(), arr.data(), len);
      result[len] = 0;
    }

    /**
     * Append a bytearray to a bytearray. No trailing nuls anywhere.
     */
    void append( TQByteArray& that, const TQByteArray& str );

    /**
     * Append a char* to a bytearray. Trailing nul not copied.
     */
    void append( TQByteArray& that, const char* str );

    /**
     * Append a TQCString to a bytearray. Trailing nul not copied.
     */
    void append( TQByteArray& that, const TQCString& str );

    void insert( TQByteArray& that, uint index, const char* s );

   /**
     * A LaterDeleter is intended to be used with the RAII ( Resource
     * Acquisition is Initialization ) paradigm. When an instance of it
     * goes out of scope it deletes the associated object  It can be
     * disabled, in case the deletion needs to be avoided for some
     * reason, since going out-of-scope cannot be avoided.
     */
    class LaterDeleter
    {
      public:
      LaterDeleter( TQObject *o)
        :m_object( o ), m_disabled( false )
      {
      }
      virtual ~LaterDeleter()
      {
        if ( !m_disabled ) {
          m_object->deleteLater();
        }
      }
      void setDisabled( bool v )
      {
        m_disabled = v;
      }
      protected:
      TQObject *m_object;
      bool m_disabled;
    };

    // return true if we should proceed, false if we should abort
    inline bool checkOverwrite( const KURL& url, TQWidget* w )
    {
        if ( KIO::NetAccess::exists( url, false /*dest*/, w ) ) {
            if ( KMessageBox::Cancel ==
                    KMessageBox::warningContinueCancel(
                        w,
                        i18n( "A file named \"%1\" already exists. "
                            "Are you sure you want to overwrite it?" ).arg( url.prettyURL() ),
                        i18n( "Overwrite File?" ),
                        i18n( "&Overwrite" ) ) )
                return false;
        }
        return true;
    }



}
}

#endif
