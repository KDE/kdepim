/*
    utilities.h

    KNode, the KDE newsreader
    Copyright (c) 1999-2001 the KNode authors.
    See file AUTHORS for details

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, US
*/

#ifndef UTIL
#define UTIL

#include <qvector.h>
#include <qlist.h>
#include <qfile.h>

#include <kurl.h>

class QWidget;
class QString;
class QChar;
class QStringList;
class QSize;

class KTempFile;


//*****************************************************************************
// utility classes
//*****************************************************************************

// clone of QSortedList...
template<class type> class Q_EXPORT QSortedVector : public QVector<type>
{
public:
    QSortedVector() {}
    QSortedVector ( uint size ) : QVector<type>(size) {}
    QSortedVector( const QSortedVector<type> &l ) : QVector<type>(l) {}
    ~QSortedVector() { clear(); }
    QSortedVector<type> &operator=(const QSortedVector<type> &l)
      { return (QSortedVector<type>&)QList<type>::operator=(l); }

    virtual int compareItems( QCollection::Item s1, QCollection::Item s2 )
      { if ( *((type*)s1) == *((type*)s2) ) return 0; return ( *((type*)s1) < *((type*)s2) ? -1 : 1 ); }
};


//==============================================================================


class KNFile : public QFile {

  public:
    KNFile(const QString& fname=QString::null);
    ~KNFile();
    const QCString& readLine();
    const QCString& readLineWnewLine();

   protected:
    bool increaseBuffer();

    QCString buffer;
    char *dataPtr;
    int filePos, readBytes;
};


//========================================================================================


class KNSaveHelper {

public:

  KNSaveHelper(QString saveName, QWidget *parent);
  ~KNSaveHelper();

  // returns a file open for writing
  QFile* getFile(QString dialogTitle);

private:

  QWidget *p_arent;
  QString s_aveName;
  KURL url;
  QFile* file;
  KTempFile* tmpFile;
  static QString lastPath;

};


//========================================================================================


class KNLoadHelper {

public:

  KNLoadHelper(QWidget *parent);
  ~KNLoadHelper();

  // returns a file open for reading
  QFile* getFile(QString dialogTitle);
  // returns the file after getFile(QString) was called
  QFile* getFile() { return file; };
  KURL getURL()    { return url; };

private:

  QWidget *p_arent;
  KURL url;
  QFile *file;
  QString tempName;
  static QString lastPath;

};


//========================================================================================


class KNHelper {

public:

  // list selection dialog, used instead of a popup menu
  // when a select action is called via the keyboard.
  // returns -1 when the user canceled the dialog.
  static int selectDialog(QWidget *parent, const QString &caption, const QStringList &options, int initialValue);

  static void saveWindowSize(const QString &name, const QSize &s);
  static void restoreWindowSize(const QString &name, QWidget *d, const QSize &defaultSize);

  static const QString encryptStr(const QString& aStr);
  static const QString decryptStr(const QString& aStr);
  static QString rot13(const QString &s);

  // checks whether s contains any non-us-ascii characters
  static bool isUsAscii(const QString &s);

  // used for rewarping a text when replying to a message or inserting a file into a box
  static QString rewrapStringList(QStringList text, int wrapAt, QChar quoteChar, bool stopAtSig, bool alwaysSpace);

  static void displayInternalFileError(QWidget *w=0);   // use this for all internal files
  static void displayExternalFileError(QWidget *w=0);   // use this for all external files
  static void displayRemoteFileError(QWidget *w=0);     // use this for remote files
  static void displayTempFileError(QWidget *w=0);       // use this for error on temporary files

};

#endif
