/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>                   
                                                                        
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

#ifndef DISTRIBUTIONLISTWIDGET_H
#define DISTRIBUTIONLISTWIDGET_H

#include <kdialogbase.h>
#include <klistview.h>

#include "featurebarwidget.h"

class QButtonGroup;
class QComboBox;
class QListView;

class DistributionListView;

namespace KABC {
class AddressBook;
class DistributionListManager;
}

class DistributionListWidget : public FeatureBarWidget
{
    Q_OBJECT

  public:
    DistributionListWidget( KABC::AddressBook *, ViewManager *, QWidget *parent );
    virtual ~DistributionListWidget();

    void addresseeSelectionChanged();

    QString title() const;
    QString identifier() const;

  public slots:
    void save();
    void dropped( QDropEvent* );

  private slots:
    void createList();
    void editList();
    void removeList();
    void addContact();
    void removeContact();
    void changeEmail();
    void updateNameCombo();
    void updateContactView();
    void selectionContactViewChanged();
    void changed();

  signals:
    void modified( KABC::Addressee::List );

  protected:
    void dropEvent( QDropEvent* );

  private:
    QComboBox *mNameCombo;  
    QLabel *mListLabel;
    DistributionListView *mContactView;

    KABC::DistributionListManager *mManager;
    QPushButton *mCreateListButton;
    QPushButton *mEditListButton;
    QPushButton *mRemoveListButton;
    QPushButton *mChangeEmailButton;
    QPushButton *mAddContactButton;
    QPushButton *mRemoveContactButton;
};

/**
  @short Helper class
*/
class DistributionListView : public KListView
{
  Q_OBJECT

  public:
    DistributionListView( QWidget *parent, const char* name = 0 );

  protected:
    void dragEnterEvent( QDragEnterEvent *e );
    void dropEvent( QDropEvent *e );
    void viewportDragMoveEvent( QDragMoveEvent *e );
    void viewportDropEvent( QDropEvent *e );

  signals:
    void dropped( QDropEvent *e );
};

/**
  @short Helper class
*/
class EmailSelector : public KDialogBase
{
  public:
    EmailSelector( const QStringList &emails, const QString &current,
        QWidget *parent );

    QString selected();

    static QString getEmail( const QStringList &emails, const QString &current,
        QWidget *parent );

  private:
    QButtonGroup *mButtonGroup;
};

#endif
