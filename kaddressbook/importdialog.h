/*                                                                      
    This file is part of KAddressBook.                                  
    Copyright (c) 2002 Mike Pilone <mpilone@slac.com>                   
                                                                        
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

#ifndef IMPORTDIALOG_H
#define IMPORTDIALOG_H

#include <libkdepim/kimportdialog.h>

class QWidget;

namespace KABC { class AddressBook; }

/**
  Dialog used for importing CSV style contact lists.
  
  @author Cornelius Schumacher
*/
class ContactImportDialog : public KImportDialog 
{
  public:
    ContactImportDialog( KABC::AddressBook *ab, QWidget *parent );

    void convertRow();
    
  private:
    KImportColumn *mFirstName;
    KImportColumn *mLastName;
    KImportColumn *mAdditionalName;
    KImportColumn *mNamePrefix;
    KImportColumn *mNameSuffix;
    KImportColumn *mFormattedName;
    KImportColumn *mNickName;
    KImportColumn *mBirthday;
    KImportColumn *mEmail;
    KImportColumn *mJobTitle;
    KImportColumn *mRole;
    KImportColumn *mPhoneBusiness;
    KImportColumn *mPhoneHome;
    KImportColumn *mPhoneMobile;
    KImportColumn *mFaxHome;
    KImportColumn *mFaxBusiness;
    KImportColumn *mCarPhone;
    KImportColumn *mIsdn;
    KImportColumn *mPager;
    KImportColumn *mMailClient;
    KImportColumn *mCompany;
    KImportColumn *mNote;
    KImportColumn *mUrl;

    KImportColumn *mAddressHomeStreet;
    KImportColumn *mAddressHomeCity;
    KImportColumn *mAddressHomeState;
    KImportColumn *mAddressHomeZip;
    KImportColumn *mAddressHomeCountry;
    KImportColumn *mAddressHomeLabel;

    KImportColumn *mAddressBusinessStreet;
    KImportColumn *mAddressBusinessCity;
    KImportColumn *mAddressBusinessState;
    KImportColumn *mAddressBusinessZip;
    KImportColumn *mAddressBusinessCountry;
    KImportColumn *mAddressBusinessLabel;

    QPtrList<KImportColumn> mCustomList;

    KABC::AddressBook *mAddressBook;
};

#endif
