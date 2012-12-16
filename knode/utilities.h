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

class KTemporaryFile;
class QFile;

//*****************************************************************************
// utility classes
//*****************************************************************************


/** File save helper (includes file save dialog and network upload). */
class KNSaveHelper {

public:

  KNSaveHelper(QString saveName, QWidget *parent);
  ~KNSaveHelper();

  /** returns a file open for writing */
  QFile* getFile(const QString &dialogTitle);

private:

  QWidget *p_arent;
  QString s_aveName;
  KUrl url;
  QFile* file;
  KTemporaryFile* tmpFile;
  static QString lastPath;

};


//========================================================================================


/** File open helper (includes file open dialog and network download). */
class KNLoadHelper {

public:

  explicit KNLoadHelper(QWidget *parent);
  ~KNLoadHelper();

  /** opens a file dialog and returns a file open for reading */
  QFile* getFile( const QString &dialogTitle );
  /** tries to access the file specified by the url and returns
      a file open for reading */
  QFile* setURL(const KUrl& url);
  /** returns the file after getFile(QString) of setURL(url) was called */
  QFile* getFile()const { return f_ile; }
  KUrl getURL() const    { return u_rl; }

private:

  QWidget *p_arent;
  KUrl u_rl;
  QFile *f_ile;
  QString t_empName;
  static KUrl l_astPath;

};


//========================================================================================


/** Some static helper methods. */
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

  /** used for rewarping a text when replying to a message or inserting a file into a box */
  static QString rewrapStringList(const QStringList &text, int wrapAt, QChar quoteChar, bool stopAtSig, bool alwaysSpace);

  /** use this for all internal files */
  static void displayInternalFileError(QWidget *w=0);
  /** use this for all external files */
  static void displayExternalFileError(QWidget *w=0);
  /** use this for remote files */
  static void displayRemoteFileError(QWidget *w=0);
  /** use this for error on temporary files */
  static void displayTempFileError(QWidget *w=0);

  /** Searches for the string from the current file position. Returns -1 if
   * the string wasn't found.
   */
  static int findStringInFile( QFile *file, const char *str );

};

#endif
