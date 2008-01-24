//=============================================================================
// File:       uuencode.h
// Contents:   Declarations for DwUuencode
// Maintainer: Doug Sauder <dwsauder@fwb.gulf.net>
// WWW:        http://www.fwb.gulf.net/~dwsauder/mimepp.html
//
// Copyright (c) 1996, 1997 Douglas W. Sauder
// All rights reserved.
// 
// IN NO EVENT SHALL DOUGLAS W. SAUDER BE LIABLE TO ANY PARTY FOR DIRECT,
// INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF
// THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF DOUGLAS W. SAUDER
// HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// DOUGLAS W. SAUDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT
// NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS"
// BASIS, AND DOUGLAS W. SAUDER HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
// SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
//
//=============================================================================

#ifndef DW_UUENCODE_H
#define DW_UUENCODE_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

//=============================================================================
//+ Name DwUuencode -- Class for performing uuencode or uudecode operations
//+ Description
//. {\tt DwUuencode} performs uuencode or uudecode operations.  Uuencode
//. is a format for encoding binary data into text characters for transmission
//. through the mail system.  The format also includes the file name and the
//. file mode.  (Note: The file mode is significant only in UNIX.)  In MIME,
//. the use of uuencode is deprecated; base64 is the preferred encoding
//. for sending binary data.
//.
//. To use {\tt DwUuencode} for encoding binary data into uuencode format,
//. set the file name, file mode, and binary data string using the member
//. functions {\tt SetFileName()}, {\tt SetFileMode()}, and
//. {\tt SetBinaryChars()}.  Then call the member function {\tt Encode()}.
//. Finally, retrieve the uuencoded text characters by calling
//. {\tt AsciiChars()}.
//.
//. To use {\tt DwUuencode} to decode uuencoded data, set the ASCII characters
//. using the member function {\tt SetAsciiChars()}, then call {\tt Decode()}.
//. Finally, retrieve the file name, file mode, and binary characters by
//. calling {\tt FileName()}, {\tt FileMode()}, and {\tt BinaryChars()}.
//=============================================================================
// Last modified 1997-07-29
//+ Noentry ~DwUuencode mFileName mMode mBinaryChars mAsciiChars


class DW_EXPORT DwUuencode {

public:

    DwUuencode();
    //. This is the default constructor.

    virtual ~DwUuencode();

    void Initialize();
    //. Resets the object's internal state to its initial state.  Call
    //. this member function to reuse the object for more than one encode
    //. or decode operation.

    void SetFileName(const char* aName);
    //. Sets the file name to be included in the uuencoded output.  The
    //. implementation limits the file name to 255 characters.

    const char* FileName() const;
    //. Returns the file name extracted while uudecoding. The implementation
    //. limits the file name to 255 characters.

    void SetFileMode(DwUint16 aMode);
    //. Sets the file mode to be included in the uuencoded output.  If
    //. the file mode is not explicitly set using this member function,
    //. a default value of 0644 (octal) is assumed.

    DwUint16 FileMode() const;
    //. Returns the file mode extracted while uudecoding.

    void SetBinaryChars(const DwString& aStr);
    //. Sets the string of binary data to be used in the uuencode operation.

    const DwString& BinaryChars() const;
    //. Returns the string of binary data extracted during a uudecode
    //. operation.

    void SetAsciiChars(const DwString& aStr);
    //. Sets the string of ASCII characters to used in the decode operation.

    const DwString& AsciiChars() const;
    //. Returns the string of ASCII characters created during a uuencode
    //. operation.

    void Encode();
    //. Creates an ASCII string of characters by uuencoding the file name,
    //. file mode, and binary data.

    int Decode();
    //. Extracts the file name, file mode, and binary data from the ASCII
    //. characters via a uudecode operation.

private:

    char     mFileName[256];
    DwUint16 mMode;

    DwString mBinaryChars;
    DwString mAsciiChars;
   
};

#endif
