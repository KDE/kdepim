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

#ifndef UTILITIES_H
#define UTILITIES_H

#include <kurl.h>

#include <qfile.h>

#include <qglobal.h>
#include <qptrvector.h>
#include <qptrlist.h>

class QWidget;
class QString;
class QChar;
class QStringList;
class QSize;

class KTempFile;


//*****************************************************************************
// utility classes
//*****************************************************************************

/** clone of QSortedList... */
template<class type> class Q_EXPORT QSortedVector : public QPtrVector<type>
{
public:
    QSortedVector() {}
    QSortedVector ( uint size ) : QPtrVector<type>(size) {}
    QSortedVector( const QSortedVector<type> &l ) : QPtrVector<type>(l) {}
    ~QSortedVector() { QPtrVector<type>::clear(); }
    QSortedVector<type> &operator=(const QSortedVector<type> &l)
      { return (QSortedVector<type>&)QPtrList<type>::operator=(l); }

    virtual int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
      { if ( *((type*)s1) == *((type*)s2) ) return 0; return ( *((type*)s1) < *((type*)s2) ? -1 : 1 ); }
};


//==============================================================================


class KNFile : public QFile {

  public:
    KNFile(const QString& fname=QString::null);
    ~KNFile();
    const QCString& readLine();
    const QCString& readLineWnewLine();
    /** searches for the string from the current file position
	returns -1 when the string wasn't found. */
    int findString(const char *s);

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

  /** returns a file open for writing */
  QFile* getFile(const QString &dialogTitle);

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

  /** opens a file dialog and returns a file open for reading */
  KNFile* getFile(QString dialogTitle);
  /** tries to access the file specified by the url and returns
      a file open for reading */
  KNFile* setURL(KURL url);
  /** returns the file after getFile(QString) of setURL(url) was called */
  KNFile* getFile()const { return f_ile; };
  KURL getURL() const    { return u_rl; };

private:

  QWidget *p_arent;
  KURL u_rl;
  KNFile *f_ile;
  QString t_empName;
  static QString l_astPath;

};


//========================================================================================


class KNHelper {

public:

  /** list selection dialog, used instead of a popup menu
      when a select action is called via the keyboard.
      returns -1 when the user canceled the dialog. */
  static int selectDialog(QWidget *parent, const QString &caption, const QStringList &options, int initialValue);

  static void saveWindowSize(const QString &name, const QSize &s);
  static void restoreWindowSize(const QString &name, QWidget *d, const QSize &defaultSize);

  static const QString encryptStr(const QString& aStr);
  static const QString decryptStr(const QString& aStr);
  static QString rot13(const QString &s);

  /** used for rewarping a text when replying to a message or inserting a file into a box */
  static QString rewrapStringList(QStringList text, int wrapAt, QChar quoteChar, bool stopAtSig, bool alwaysSpace);

  /** use this for all internal files */
  static void displayInternalFileError(QWidget *w=0);
  /** use this for all external files */
  static void displayExternalFileError(QWidget *w=0);
  /** use this for remote files */
  static void displayRemoteFileError(QWidget *w=0);
  /** use this for error on temporary files */
  static void displayTempFileError(QWidget *w=0);

};

#endif
