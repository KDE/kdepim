/*
† † † † † † † †=.            This file is part of the OPIE Project
† † † † † † †.=l.            Copyright (c)  2002 Holger Freyther <zecke@handhelds.org>
† † † † † †.>+-=                            2002 Maximilian Reiﬂ <harlekin@handhelds.org> 
†_;:, † † .> † †:=|.         This library is free software; you can
.> <`_, † > †. † <=          redistribute it and/or  modify it under
:`=1 )Y*s>-.-- † :           the terms of the GNU Library General Public
.="- .-=="i, † † .._         License as published by the Free Software
†- . † .-<_> † † .<>         Foundation; either version 2 of the License,
† † †._= =} † † † :          or (at your option) any later version.
† † .%`+i> † † † _;_.
† † .i_,=:_. † † †-<s.       This library is distributed in the hope that
† † †+ †. †-:. † † † =       it will be useful,  but WITHOUT ANY WARRANTY;
† † : .. † †.:, † † . . .    without even the implied warranty of
† † =_ † † † †+ † † =;=|`    MERCHANTABILITY or FITNESS FOR A
† _.=:. † † † : † †:=>`:     PARTICULAR PURPOSE. See the GNU
..}^=.= † † † = † † † ;      Library General Public License for more
++= † -. † † .` † † .:       details.
†: † † = †...= . :.=-
†-. † .:....=;==+<;          You should have received a copy of the GNU
† -_. . . † )=. †=           Library General Public License along with
† † -- † † † †:-=`           this library; see the file COPYING.LIB.
                             If not, write to the Free Software Foundation,
                             Inc., 59 Temple Place - Suite 330,
                             Boston, MA 02111-1307, USA.
			     
*/


#include <qptrlist.h>
#include <kparts/mainwindow.h>

#include <manipulatorpart.h>

class PartBar;
class QHBox;
class QWidgetStack;

namespace KitchenSync {
  // no idea why we have this window
  class KSyncMainWindow : public KParts::MainWindow /*, public KSyncInterface */
    {
      Q_OBJECT
    public:
      KSyncMainWindow(QWidget *widget =0l, const char *name = 0l, WFlags f = WType_TopLevel );
      ~KSyncMainWindow();

    private:
      virtual void initActions();
      void addModPart( ManipulatorPart * );
      PartBar *m_bar;
      QHBox *m_lay;
      QWidgetStack *m_stack;
      QPtrList<ManipulatorPart> m_parts;
      
      private slots:
      void initPlugins();
      void slotSync();
      void slotBackup();
      void slotRestore();
      void slotConfigure();
      void slotActivated(ManipulatorPart *);
      void slotQuit();
    
  };

  
};
