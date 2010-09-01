/*  -*- c++ -*-
    sievejob.h

    KMail, the KDE mail client.
    Copyright (c) 2002 Marc Mutz <mutz@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License,
    version 2.0, as published by the Free Software Foundation.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, US
*/

#ifndef __KMAIL_SIEVE_JOB_H__
#define __KMAIL_SIEVE_JOB_H__

#include <tqobject.h>
#include <tqvaluestack.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqcstring.h>

#include <kurl.h>
#include <kio/global.h>

class TQTextDecoder;
namespace KIO {
  class Job;
}

namespace KMail {

  class SieveJob : public TQObject {
    Q_OBJECT
  protected:
    enum Command { Get, Put, Activate, Deactivate, SearchActive, List, Delete };
    SieveJob( const KURL & url, const TQString & script,
	      const TQValueStack<Command> & commands,
	      TQObject * parent=0, const char * name=0 );
    SieveJob( const KURL & url, const TQString & script,
	      const TQValueStack<Command> & commands,
	      bool showProgressInfo,
	      TQObject * parent=0, const char * name=0 );
    virtual ~SieveJob();

  public:
    enum Existence { DontKnow, Yes, No };

    /**
     * Store a Sieve script. If @param makeActive is set, also mark the
     * script active
     */
    static SieveJob * put( const KURL & dest, const TQString & script,
			   bool makeActive, bool wasActive );

    /**
     * Get a specific Sieve script
     */
    static SieveJob * get( const KURL & src, bool showProgressInfo=true );

    /**
     * List all available scripts
     */
    static SieveJob * list( const KURL & url );

    static SieveJob * del( const KURL & url );

    static SieveJob * activate( const KURL & url );

    static SieveJob * desactivate( const KURL & url );

    void kill( bool quiet=true );

    const TQStringList & sieveCapabilities() const {
      return mSieveCapabilities;
    }

    bool fileExists() const {
      return mFileExists;
    }

  signals:
    void gotScript( KMail::SieveJob * job, bool success,
		    const TQString & script, bool active );

    /**
     * We got the list of available scripts
     *
     * @param scriptList is the list of script filenames
     * @param activeScript lists the filename of the active script, or an
     *        empty string if no script is active.
     */
    void gotList( KMail::SieveJob *job, bool success,
                  const TQStringList &scriptList, const TQString &activeScript );

    void result(  KMail::SieveJob * job, bool success,
                  const TQString & script, bool active );

    void item( KMail::SieveJob * job, const TQString & filename, bool active );

  protected:
    void schedule( Command command, bool showProgressInfo );

  protected slots:
    void slotData( KIO::Job *, const TQByteArray & ); // for get
    void slotDataReq( KIO::Job *, TQByteArray & ); // for put
    void slotEntries( KIO::Job *, const KIO::UDSEntryList & ); // for listDir
    void slotResult( KIO::Job * ); // for all commands

  protected:
    KURL mUrl;
    KIO::Job * mJob;
    TQTextDecoder * mDec;
    TQString mScript;
    TQString mActiveScriptName;
    Existence mFileExists;
    TQStringList mSieveCapabilities;
    TQValueStack<Command> mCommands;
    bool mShowProgressInfo;

    // List of Sieve scripts on the server, used by @ref list()
    TQStringList mAvailableScripts;
  };

} // namespace KMail

#endif // __KMAIL_SIEVE_JOB_H__

// vim: set noet sts=2 ts=8 sw=2:

