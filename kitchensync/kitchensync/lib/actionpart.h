/*
    This file is part of KitchenSync.

    Copyright (c) 2002 Holger Freyther <zecke@handhelds.org>
� � Copyright (c) 2002 Maximilian Rei� <harlekin@handhelds.org>
    Copyright (c) 2004 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef KSYNC_ACTIONPART_H
#define KSYNC_ACTIONPART_H

#include <qpixmap.h>
#include <qstring.h>
#include <kparts/part.h>
#include <qptrlist.h>
#include <qstringlist.h>

#include <kdebug.h>

#include <syncee.h>
#include <synceelist.h>

#include <error.h>
#include <progress.h>

#include "profile.h"

namespace KSync {

class Core;
class Konnector;

enum SyncStatus { SYNC_START=0, SYNC_PROGRESS=1,  SYNC_DONE=2,  SYNC_FAIL };

/**
  A part represents an action, like making a backup, syncing something or
  just showing some data.

  Parts can be put into profiles. When the user triggers the profile the
  Konnectors are read if required, the actions of the parts are executed
  in the sequence configured by the user and then the Konnectors are
  written. If reading or writing the Konnectors is required is determined
  by the parts in the profile.

  A profile can have more than one part of the same type. This allows to
  have profiles with action sequences like: show original data, make a
  backup, sync, show the resulting data.

  The ActionPart is loaded into the KitchenSync Shell. Each ActionPart has to
  provide a QWidget and can provide a config dialog.
*/
class ActionPart : public KParts::Part
{
    Q_OBJECT
  public:
    /**
      The simple constructor

      @param parent parent widget
      @param name Qt name
    */
    ActionPart( QObject *parent = 0, const char *name  = 0 );
    virtual ~ActionPart();

    /**
      @return the type of this part for example like "Addressbook"
    */
    virtual QString type() const = 0;

    /**
      @return the progress made 0-100
    */
    virtual int syncProgress() const;

    /**
      the sync status
    */
    virtual int syncStatus() const;

    /**
      @return a translated string which is used as title for this ActionPart.
     */
    virtual QString title() const = 0;

    /**
      @return a short description
    */
    virtual QString description() const = 0;

    /**
      @return a pixmap for this part
    */
    virtual QPixmap *pixmap() = 0;

    /**
      return a iconName
    */
    virtual QString iconName() const = 0;

    /**
      Return if the part has a GUI.
    */
    virtual bool hasGui() const;

    /**
      if the config part is visible
    */
    virtual bool configIsVisible() const;

    /**
      @return if the part canSync data :)
    */
    virtual bool canSync() const;

    /**
      @return a config widget. Always create a new one
      the ownership will be transferred
    */
    virtual QWidget *configWidget();

    /**
      if you want to sync implement that method
      After successfully syncing you need to call done()
      which will emit a signal
      @param in The Syncee List coming from a KonnectorPlugin
      @param out The Syncee List which will be written to the Konnector
    */
    virtual void sync( const SynceeList &in, SynceeList &out );

    virtual void executeAction() = 0;

    virtual void filterKonnectors( QPtrList<Konnector> & ) {}

    virtual bool needsKonnectorRead() const { return false; }
    
    virtual bool needsKonnectorWrite() const { return false; }

  public slots:
    virtual void slotSynceesRead( Konnector * ) {}

    virtual void slotSynceeReadError( Konnector * ) {}

    virtual void slotSynceesWritten( Konnector * ) {}

    virtual void slotSynceeWriteError( Konnector * ) {}

  protected:

    /**
     * See if the user wants to be asked before writing
     * the Syncees back.
     */
    bool confirmBeforeWriting() const;

    /**
     * @return access to the shell
     */
    Core *core();
    Core *core() const;

    /**
     * call this whenever you make progress
     */
    void progress( int );

  protected slots:
    void progress( const Progress& );
    void error( const Error& );
    void done();

  signals:
    // 0 - 100
    void sig_progress( ActionPart *, int );
    void sig_progress( ActionPart *, const Progress & );
    void sig_error( ActionPart *, const Error & );
    // SYNC_START SYNC_SYNC SYNC_STOP
    void sig_syncStatus( ActionPart *, int );

  protected:
    /**
     * Connect to the PartChange signal
     * @see MainWindow for the slot signature
     */
    /* ActionPart* old,ActionPart* ne */
    void connectPartChange( const char* slot);

    /* ActionPart* part,const Progress& */
    void connectPartProgress( const char* slot );

    /* ActionPart* part, const Error& */
    void connectPartError( const char* slot );

    /* Konnector *,const Progress& */
    void connectKonnectorProgress(const char* slot );

    /* Konnector *, const Error& */
    void connectKonnectorError( const char* slot );

    /* ActionPart*,int status,int prog */
    void connectSyncProgress( const char* slot );

    /* const Profile& */
    void connectProfileChanged( const char* slot );

    /* Konnector *,Syncee::PtrList */
    void connectKonnectorDownloaded( const char* slot );

    /* connectStartSync */
    void connectStartSync(const char* slot);

    /* connectDoneSync */
    void connectDoneSync(const char* slot);

  public slots:
    virtual void slotConfigOk();

  private:
    Core *m_window;
    int m_prog;
    int m_stat;
};

}

#endif
