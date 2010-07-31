/*
    KNode, the KDE newsreader
    Copyright (c) 1999-2005 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef UTILITIES_H
#define UTILITIES_H

#include <kurl.h>

#include <tqfile.h>

#include <tqglobal.h>
#include <tqptrvector.h>
#include <tqptrlist.h>

class QWidget;
class QString;
class QChar;
class QStringList;
class QSize;

class KTempFile;


//*****************************************************************************
// utility classes
//*****************************************************************************

/** clone of TQSortedList... */
template<class type> class Q_EXPORT QSortedVector : public TQPtrVector<type>
{
public:
    QSortedVector() {}
    QSortedVector ( uint size ) : TQPtrVector<type>(size) {}
    QSortedVector( const QSortedVector<type> &l ) : TQPtrVector<type>(l) {}
    ~QSortedVector() { TQPtrVector<type>::clear(); }
    QSortedVector<type> &operator=(const QSortedVector<type> &l)
      { return (QSortedVector<type>&)TQPtrList<type>::operator=(l); }

    virtual int compareItems( TQPtrCollection::Item s1, TQPtrCollection::Item s2 )
      { if ( *((type*)s1) == *((type*)s2) ) return 0; return ( *((type*)s1) < *((type*)s2) ? -1 : 1 ); }
};


//==============================================================================


class KNFile : public TQFile {

  public:
    KNFile(const TQString& fname=TQString::null);
    ~KNFile();
    const TQCString& readLine();
    const TQCString& readLineWnewLine();
    /** searches for the string from the current file position
	returns -1 when the string wasn't found. */
    int findString(const char *s);

   protected:
    bool increaseBuffer();

    TQCString buffer;
    char *dataPtr;
    int filePos, readBytes;
};


//========================================================================================


class KNSaveHelper {

public:

  KNSaveHelper(TQString saveName, TQWidget *parent);
  ~KNSaveHelper();

  /** returns a file open for writing */
  TQFile* getFile(const TQString &dialogTitle);

private:

  TQWidget *p_arent;
  TQString s_aveName;
  KURL url;
  TQFile* file;
  KTempFile* tmpFile;
  static TQString lastPath;

};


//========================================================================================


class KNLoadHelper {

public:

  KNLoadHelper(TQWidget *parent);
  ~KNLoadHelper();

  /** opens a file dialog and returns a file open for reading */
  KNFile* getFile( const TQString &dialogTitle );
  /** tries to access the file specified by the url and returns
      a file open for reading */
  KNFile* setURL(KURL url);
  /** returns the file after getFile(TQString) of setURL(url) was called */
  KNFile* getFile()const { return f_ile; };
  KURL getURL() const    { return u_rl; };

private:

  TQWidget *p_arent;
  KURL u_rl;
  KNFile *f_ile;
  TQString t_empName;
  static TQString l_astPath;

};


//========================================================================================


class KNHelper {

public:

  /** list selection dialog, used instead of a popup menu
      when a select action is called via the keyboard.
      returns -1 when the user canceled the dialog. */
  static int selectDialog(TQWidget *parent, const TQString &caption, const TQStringList &options, int initialValue);

  static void saveWindowSize(const TQString &name, const TQSize &s);
  static void restoreWindowSize(const TQString &name, TQWidget *d, const TQSize &defaultSize);

  static const TQString encryptStr(const TQString& aStr);
  static const TQString decryptStr(const TQString& aStr);
  static TQString rot13(const TQString &s);

  /** used for rewarping a text when replying to a message or inserting a file into a box */
  static TQString rewrapStringList(TQStringList text, int wrapAt, TQChar quoteChar, bool stopAtSig, bool alwaysSpace);

  /** use this for all internal files */
  static void displayInternalFileError(TQWidget *w=0);
  /** use this for all external files */
  static void displayExternalFileError(TQWidget *w=0);
  /** use this for remote files */
  static void displayRemoteFileError(TQWidget *w=0);
  /** use this for error on temporary files */
  static void displayTempFileError(TQWidget *w=0);

};

#endif
