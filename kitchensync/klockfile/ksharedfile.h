/* This file is part of the KDE libraries
    Copyright (C) 2001 Holger Freyther <freyher@kde.org>
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
		  
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef ksharedfile_h
#define ksharedfile_h

#include <sys/types.h>

#include <qstring.h>
#include <qobject.h>

class QFile;
class QTimer; 

/** The point of KSharedFile is to make it possible for more than one 
    program to work with a file. It take's care that only one program
    writes to the file at a time. There's no really way of pretending a
program to write to the file.
    The program creates a instance of KSharedFile and sets the FileName.
    If it want's to write to the file it calls tryLockFile( ) and either get's
    the lock or not.
    Then there are also signal which signalize a change.
    @short KSharedFile to lock a file
    @author Holger Freyther <freyther@kde.org>
    @version 0.5 
 */

class KSharedFile : public QObject 
{
  Q_OBJECT;
 public:
  /** Instantiate the class.
    @param filename The resource to be shared with others
   */
  KSharedFile( const QString &filename );
  /** this function is for convience 
      it does the same a above but takes a file as paramter
      @param file The file to be shared.
  */
  KSharedFile( const QFile &file );
  ~KSharedFile( );
  
  class Ticket
  {
      friend class KSharedFile;
      Ticket ( const QString &filename ) : m_fileName( filename ) {}
    private:
      QString m_fileName;

  };
  /** sets the Filename
      @param filename The name of the resource to be shared 
   */
  void setFileName( const QString &filename );
  /** This method is for convience.It sets the File
      @param file The file to be shared
   */
  void setFile( const QFile &file );
  /** @return the fileName of the shared file
   */
  QString fileName( ) const;

  /** This tries to lock the file. It returns right after trying either successful or not
 @return if Ticket is not equal to 0l the file was successfully locked
  */
  Ticket* requestSaveTicket( );
  /** This loads the file specified
     @see setFileName(const QString & )
   */
  QString load( );
  /** This writes to the file if the ticket is valid
 @param 
   */
  bool save( Ticket *ticket, const QString &string );
  /** after locking this unlocks the file
   */
  bool unlockFile( Ticket *ticket );
  /** check whether or not the file is locked
   @return the state of the lock
   */
  bool isLocked( );
  /**
     @returns whether or not I locked the file
   */
  bool didILock( );
  /** In future this will tell you who to blame for the file is locked.

   */
  QString whoHoldsLock( ) const;


 protected:
  QString m_uniqueName;

 private:
  QString m_fileName;
  bool m_locked:1;
  bool m_lockedOther:1;
  QTimer *m_FileCheckTimer;
  time_t m_ChangeTime;

 private slots:
  void checkFile( );
 signals:
  /**
    This signal get emitted when the file get unlocked
    @param filename The name of the file which gets locked
   */
  void fileUnlocked( const QString &filename);
  /** The file got locked
      @param filename filename got locked
  */  
  void fileLocked( const QString &filename );
  /**
      The file changed during a lock and unlock session
      @param filename The file with the name filenam changed
   */
  void fileChanged( const QString &filename );
};

#endif
