//=============================================================================
// File:       binhex.h
// Contents:   Declarations for DwBinhex
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

#ifndef DW_BINHEX_H
#define DW_BINHEX_H

#ifndef DW_CONFIG_H
#include <mimelib/config.h>
#endif

#ifndef DW_STRING_H
#include <mimelib/string.h>
#endif

//=============================================================================
//+ Name DwBinhex -- Class for converting files to or from Binhex 4.0 format
//+ Description
//. {\tt DwBinhex} converts data to or from Binhex 4.0 format.  Binhex is a
//. format used almost exclusively on Macintosh computers for encoding
//. files into text characters for transmission through the mail transport
//. system or for archiving on non-Macintosh systems.  The format includes
//. the file name, file type, file creator, Macintosh Finder flags, data fork,
//. resource fork, and checksums. In MIME, the use of Binhex is deprecated;
//. applesingle and appledouble are the preferred format for encoding
//. Macintosh files.  The Binhex 4.0 format is described in RFC-1741.
//. Binhex is a widely used, {\it de facto} standard, but it is not an
//. official Internet standard.
//.
//. To use {\tt DwBinhex} for converting a Macintosh file to Binex format,
//. call the member functions {\tt SetFileName()}, {\tt SetFileType()},
//. {\tt SetFileCreator()}, {\tt SetFlag1()}, {\tt SetFlag2()},
//. {\tt SetDataFork()}, and {\tt SetResourceFork()} to set the elements
//. to be encoded.  Any elements that are not set by calling one of the
//. member functions are assigned reasonable defaults.  Then call the
//. {\tt Encode()} member function to actually perform the conversion to
//. Binhex.  Finally, call {\tt BinhexChars()} to retrieve the Binhex
//. characters.
//.
//. To use {\tt DwBinhex} for converting a Macintosh file from Binhex format,
//. call the member function {\tt SetBinhexChars()} to assign the Binhex
//. characters to be converted.  Then call {\tt Decode()} to actually
//. perform the conversion.  Finally, call {\tt FileName()}, {\tt FileType()},
//. {\tt FileCreator()}, {\tt Flag1()}, {\tt Flag2()}, {\tt DataFork()},
//. and {\tt ResourceFork()} to extract the decoded elements.
//.
//. Note: {\tt DwBinhex} does not change the file name in any way. When you
//. you are dealing with file names, you should be aware of the fact that
//. some filenames that are valid on a Macintosh may cause problems or
//. unexpected results on a non-Macintosh system, and vice versa.  Such
//. problem characters include slash ('/'), colon (':'), space and possibly
//. other characters.
//=============================================================================
// Last modified 1997-08-25
//+ Noentry ~DwBinhex


class DW_EXPORT DwBinhex {

public:

    DwBinhex();
    //. This is the default constructor.

    virtual ~DwBinhex();

    void Initialize();
    //. Resets the object's internal state to its initial state.  Call
    //. this member function to reuse the object for more than one encode
    //. or decode operation.

    const char* FileName() const;
    void SetFileName(const char* aName);
    //. Gets or sets the file name.  The file name is restricted
    //. to a maximum length of 63 characters.

    void FileType(char* aBuf) const;
    void SetFileType(const char* aType);
    //. Gets or sets the file type.  All Macintosh files have a file type,
    //. which is represented by four bytes.  Some examples include "TEXT"
    //. for a text file, or "APPL" for an application.  {\tt aBuf} should
    //. point to an array of at least four characters.

    void FileCreator(char* aBuf) const;
    void SetFileCreator(const char* aType);
    //. Gets or sets the file creator.  Most Macintosh files have a creator,
    //. which is represented by a signature of four bytes.  The creator
    //. specifies which application to launch when a file's icon is double
    //. clicked.  {\tt aBuf} should point to an array of at least four
    //. characters.

    DwUint8 Flag1() const;
    void SetFlag1(DwUint8 aFlag);
    //. Gets or sets the first byte of the Macintosh Finder flags.  For
    //. files that originate on non-Macintosh systems, this byte should
    //. be set to zero (the default).

    DwUint8 Flag2() const;
    void SetFlag2(DwUint8 aFlag);
    //. Gets or sets the second byte of the Macintosh Finder flags.  For
    //. files that originate on non-Macintosh systems, this byte should
    //. be set to zero (the default).

    const DwString& DataFork() const;
    void SetDataFork(const DwString& aStr);
    //. Gets or sets the data fork for the file.  For files that originate
    //. on non-Macintosh systems, such as a GIF or JPEG file, the file data
    //. should be set as the data fork.

    const DwString& ResourceFork() const;
    void SetResourceFork(const DwString& aStr);
    //. Gets or sets the resource fork for the file.  For files that originate
    //. on non-Macintosh systems, such as a GIF or JPEG file, the resource
    //. should be normally be empty.

    const DwString& BinhexChars() const;
    void SetBinhexChars(const DwString& aStr);
    //. Gets or sets the characters of the Binhex encoded file.

    void Encode();
    //. Converts the Macintosh file information to Binhex format.

    int Decode();
    //. Converts the Macintosh file information from Binhex format.  Returns
    //. zero if the decode operation completes successufully; otherwise,
    //. the function returns -1.
   
private:

    char mFileName[64];
    char mFileType[8];
    char mFileCreator[8];
    DwUint8 mFlag1;
    DwUint8 mFlag2;
    DwString mDataFork;
    DwString mResourceFork;
    DwString mBinhexChars;

};

#endif
