/* setupDialog.h			KPilot
**
** Copyright (C) 2000-2001 by Gregory Stern
**
** This file is part of the Abbrowser conduit, a conduit for KPilot that
** synchronises the Pilot's memo pad application with Abbrowser and kab.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, 
** MA 02139, USA.
*/

/*
** Bug reports and questions can be sent to adridg@cs.kun.nl
*/

#ifndef __ABBROWSER_CONDUIT_SETUP_H
#define __ABBROWSER_CONDUIT_SETUP_H

class AbbrowserConduitConfig;

#include "gsetupDialog.h"

class AbbrowserConduitGeneralPage : public setupDialogPage
    {
      Q_OBJECT
      
    public:
      AbbrowserConduitGeneralPage(setupDialog *,KConfig& );
      
      virtual int commitChanges(KConfig&);
      
    protected:
      AbbrowserConduitConfig *fConfigWidget;
    } ;

class AbbrowserConduitOptions : public setupDialog
    {
      Q_OBJECT
      
    friend class AbbrowserConduit;
    friend class AbbrowserConduitGeneralPage;
    public:
      AbbrowserConduitOptions(QWidget *parent);
      
    protected:
      static const QString Group;
    };

#endif


// $Log$
// Revision 1.3  2001/02/07 15:46:31  stern
// Updated copyright headers for source release. Added CVS log. No code change.
//
