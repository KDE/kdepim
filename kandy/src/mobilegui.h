/*
    This file is part of Kandy.

    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#ifndef MOBILEGUI_H
#define MOBILEGUI_H

#include <kandyiface.h>

#include "mobilegui_base.h"

class CommandScheduler;
class ATCommand;
class AddressSyncer;

class MobileGui : public MobileGui_base, virtual public KandyIface
{ 
    Q_OBJECT
  public:
    MobileGui(CommandScheduler *,QWidget* parent=0,const char* name=0,
              WFlags fl=0);
    ~MobileGui();

    void exit();

  signals:
    void sendCommand(const QString &);
    void phonebookRead();

    void statusMessage(const QString &);
    void transientStatusMessage(const QString &);

  public slots:
    void readModelInformation();
    void readPhonebook();
    void savePhonebook();
    void refreshStatus();
    void writePhonebook();
    void readKabc();
    void writeKabc();
    void mergePhonebooks();
    void syncPhonebooks();

  protected slots:
    void processResult(ATCommand *);

  private:
    void fillPhonebook(ATCommand *);
    QString quote(const QString &);
    QString dequote(const QString &);
  
    void updateKabBook();
    void updateMobileBook();
    void updateCommonBook();
  
    CommandScheduler *mScheduler;
    
    AddressSyncer *mSyncer;
    
    QString mLastWriteId;
    QString mSyncReadId;
    QString mSyncWriteId;
    
    bool mSyncing;
};

#endif // MOBILEGUI_H
