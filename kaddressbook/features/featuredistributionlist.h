/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mirko Boehm <mirko@kde.org>
                                                                        
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
                                                                        
    As a special exception, permission is given to link this program    
    with any edition of Qt, and distribute the resulting executable,    
    without including the source code for Qt in the source distribution.
*/                                                                      

#ifndef FEATUREDISTRIBUTIONLIST_H
#define FEATUREDISTRIBUTIONLIST_H

#include <qcombobox.h>
#include <qpushbutton.h>

#include <kabc/distributionlist.h>

#include "featuredistributionlistview.h"

class FeatureDistributionList : public QWidget
{
  Q_OBJECT

  public:
    FeatureDistributionList( KABC::AddressBook*, QWidget *parent = 0, const char* name = 0 );
    virtual ~FeatureDistributionList();

    /** Store changes in resource. */
    virtual void commit();

  protected:
    /** Set up the displayed information (list of lists etc). */
    void update();

    /** Enable or disable buttons, adjust list member display to
        selection in mCbListSelect. */
    void updateGUI();

    /** The addressbook. */
    KABC::AddressBook *mDoc;

    /** The list manager. */
    KABC::DistributionListManager *mManager;

    /** Catch the show event. */
    void showEvent(QShowEvent *);

    /** Catch the drop event. */
    void dropEvent(QDropEvent*);

  public slots:
    /** The selection in the addressee list changed. */
    void slotAddresseeSelectionChanged();

    /** Notification of drop events by the widgets. */
    void slotDropped(QDropEvent*);

    void slotListNew();
    void slotListRename();
    void slotListRemove();
    void slotEntryChangeEmail();
    void slotEntryRemove();
    void slotListSelected(int);

  signals:
    void modified();

  private:
    void initGUI();

    QComboBox* mCbListSelect;
    QPushButton* mPbListRename;
    QPushButton* mPbListRemove;
    QPushButton* mPbChangeEmail;
    QPushButton* mPbEntryRemove;
    QPushButton* mPbListNew;

    FeatureDistributionListView* mLvAddressees;
};

#endif
