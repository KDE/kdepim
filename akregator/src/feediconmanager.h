/*
    This file is part of Akregator.

    Copyright (C) 2004 Sashmit Bhaduri <smt@vfemail.net>
                  2005 Frank Osterfeld <frank.osterfeld at kdemail.net>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef FEEDICONMGR_H
#define FEEDICONMGR_H

#include <dcopobject.h>

#include <tqobject.h>

class QPixmap;
class QString;

class DCOPClient;
class KURL;


namespace Akregator
{
    class Feed;
    class TreeNode;
    
    class FeedIconManager:public TQObject, public DCOPObject
    {
        Q_OBJECT
        K_DCOP
        
        public:

            FeedIconManager(TQObject * = 0L, const char * = 0L);
            ~FeedIconManager();
            
            static FeedIconManager *self();

            void fetchIcon(Feed* feed);
            
            TQString iconLocation(const KURL &) const;
            
        k_dcop:
            void slotIconChanged(bool, const TQString&, const TQString&);

        signals:
            void signalIconChanged(const TQString &, const TQPixmap &);

        public slots:
            void slotFeedDestroyed(TreeNode* node);

      protected:

            /** returns the url used to access the icon, e.g.
                http://dot.kde.org/ for "dot.kde.org/1113317400/" */
            TQString getIconURL(const KURL& url);

            void loadIcon(const TQString &);
      
      private:
            static FeedIconManager *m_instance;

            class FeedIconManagerPrivate;
            FeedIconManagerPrivate* d;
    };
}

#endif
