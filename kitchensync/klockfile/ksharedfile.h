/* This file is part of the KDE libraries
    Copyright (C) 2001, 2002 Holger Freyther <freyher@kde.org>
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
#include <dcopobject.h>

class QFile;
class QTimer; 

/** 
 *   The point of KSharedFile is to make it possible for more than one 
 *   program to work with a file. It take's care that only one program
 *   writes to the file at a time. It'll not create any locks on the
 *   file though.
 *   The program creates a instance of KSharedFile and sets the FileName.
 *   If it want's to write to the file it calls tryLockFile( ) and either get's
 *   the lock or not.
 *   Then there are also signal which signalize a change.
 *   @short KSharedFile to lock a file
 *   @author Holger Freyther <freyther@kde.org>
 *   @version 0.51 
 */

class KSharedFile : public QObject, public DCOPObject 
{
  Q_OBJECT
  K_DCOP
 public:
 
  /** 
   * Instantiate the class.
   * @param resource The resource to be shared with others
   */
  KSharedFile( const QString &resource );
  
  /** 
   * this function is for convience 
   * it does the same a above but takes a file as paramter
   * @param file The file to be shared.
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
  void setFileName (const QString &filename );
  
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
  Ticket* requestWriteTicket( );

  /** This tries to to lock the file for reading

   */
  Ticket* requestReadTicket();
  
  /** This writes to the file if the ticket is valid
   * @param ticket The ticket.
   * @param string The string to write
   * @return failure
   */
  bool save( Ticket *ticket, const QString &string );
  
  /**
   * This writes to the file if the ticket is valid
   * @param ticket The ticket received by locking
   * @param array The array to write
   * @return failure
   * @see save
   */
  bool save( Ticket *ticket, const QByteArray & array);
  
  
  QFile* save( Ticket *ticket );

  QString readAsString( bool &ok, Ticket *ticket );
  QByteArray readAsByteArray( bool &ok, Ticket *ticket );
  QFile* readAsFile( Ticket *ticket );
  
  /** after locking this unlocks the file
   */
  bool unlockReadFile( Ticket *ticket );
  bool unlockWriteFile( Ticket *ticket );
  /** check whether or not the file is locked
   @return the state of the lock
   */
  bool canReadLock( );
  bool canWriteLock();
  /**
     @returns whether or not I locked the file
   */
  bool didIReadLock( );
  bool didIWriteLock();
  /** In future this will tell you who to blame for the file is locked.

   */
  //QString whoHoldsLock( ) const;
 k_dcop:
   ASYNC slotFileChanged(QString );

 private:
  QFile *m_file;
  QString m_fileName;
  time_t m_ChangeTime;
  bool readLock;
  bool writeLock;
  void updateLocks();

 signals:
  /**
    This signal get emitted when the file get unlocked
    @param filename The name of the file which gets locked
   */
  void fileWriteUnlocked( const QString &filename);
  /** The file got locked
      @param filename filename got locked
  */  
  void fileWriteLocked( const QString &filename );
  /**
      The file changed during a lock and unlock session
      @param filename The file with the name filenam changed
   */
  void fileChanged( const QString &filename );


};

#endif










