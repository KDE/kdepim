/*
    Empath - Mailer for KDE
    
    Copyright 1999, 2000
        Rik Hemsley <rik@kde.org>
        Wilco Greven <j.w.greven@student.utwente.nl>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifdef __GNUG__
# pragma interface "EmpathMaildir.h"
#endif

#ifndef EMPATH_MAILDIR_H
#define EMPATH_MAILDIR_H 

// Qt includes
#include <qdir.h>
#include <qdict.h>
#include <qstring.h>
#include <qdir.h>
#include <qlist.h>
#include <qtimer.h>
#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>

// Local includes
#include "EmpathURL.h"
#include "EmpathDefines.h"
#include "EmpathIndexRecord.h"
#include "RMM_Message.h"

class EmpathFolder;

/**
 * @internal
 * @author Rikkus
 */
class EmpathMaildir : public QObject
{
    Q_OBJECT
        
    public:
        
        EmpathMaildir(const QString & basePath, const EmpathURL & url);

        bool createdOK() const { return createdOK_; }

        virtual ~EmpathMaildir();
        
        void init();
        
        const QString & basePath() const    { return basePath_; }
        const EmpathURL & url() const       { return url_; }
        const QString & path() const        { return path_; }
        
        EmpathSuccessMap mark(const QStringList &, EmpathIndexRecord::Status);
        
        QString writeMessage(RMM::RMessage);
        
        RMM::RMessage message(const QString &);
        
        EmpathSuccessMap removeMessage (const QStringList &);
        
        void sync(bool force = false);
        
    protected slots:
        
        void s_timerBeeped();
        
    private:
        
        EmpathMaildir();

        bool        _removeMessage(const QString & id);
        bool        _mark(const QString & id, EmpathIndexRecord::Status);
        QString     _write(RMM::RMessage);
        QCString    _messageData(const QString &, bool isFullName = false);
        void        _markNewMailAsSeen();
        void        _markAsSeen(const QString &);
        void        _clearTmp();
        bool        _checkDirs();
        QString     _generateFlagsString(EmpathIndexRecord::Status);
        bool        _touched(EmpathFolder *);
        void        _tagOrAdd(EmpathFolder *);
        void        _removeUntagged(EmpathFolder *);
        void        _recalculateCounters(EmpathFolder *);
       
        QDateTime    mtime_;
        QDict<bool>  tagList_;
        
        QTimer       timer_;

        QStringList cachedEntryList_;
        QStringList & _entryList();

        bool createdOK_;

        // Order dependency
        QString      path_;
        EmpathURL    url_;
        QString      basePath_;
        // End order dependency
};

typedef QList<EmpathMaildir> EmpathMaildirList;
typedef QListIterator<EmpathMaildir> EmpathMaildirListIterator;

#endif

// vim:ts=4:sw=4:tw=78
