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

#include "importdialog.h"

#include <klocale.h>
#include <kabc/addressbook.h>
#include <kabc/addressee.h>
#include <kabc/phonenumber.h>
#include <kabc/address.h>

ContactImportDialog::ContactImportDialog(KABC::AddressBook *doc,
                                         QWidget *parent)
        : KImportDialog(parent), mDocument(doc)
{
  // These should be ported to use the KABC::Field labels to be consistant.
  
    mFirstName = new KImportColumn(this,i18n("First Name"),1);
    mLastName = new KImportColumn(this,i18n("Last Name"),1);
//      mFullName = new KImportColumn(this,i18n("Full Name"));
    mEmail = new KImportColumn(this,i18n("Email"));
    mPhoneHome = new KImportColumn(this,i18n("Phone Home"));
    mPhoneBusiness = new KImportColumn(this,i18n("Phone Business"));
    mPhoneMobile = new KImportColumn(this,i18n("Phone Mobile"));
    mFaxHome = new KImportColumn(this,i18n("Fax Home"));
    mFaxBusiness = new KImportColumn(this,i18n("Fax Business"));
    mJobTitle = new KImportColumn(this,i18n("Job Title"));
    mCompany = new KImportColumn(this,i18n("Company"));

    mAddressHomeCity = new KImportColumn(this,i18n("Home Address City"));
    mAddressHomeStreet = new KImportColumn(this,i18n("Home Address Street"));
    mAddressHomeZip = new KImportColumn(this,i18n("Home Address Postal Code"));
    mAddressHomeState = new KImportColumn(this,i18n("Home Address State"));
    mAddressHomeCountry = new KImportColumn(this,i18n("Home Address Country"));

    mAddressBusinessCity = new KImportColumn(this,i18n("Business Address City"));
    mAddressBusinessStreet = new KImportColumn(this,i18n("Business Address Street"));
    mAddressBusinessZip = new KImportColumn(this,i18n("Business Address Postal Code"));
    mAddressBusinessState = new KImportColumn(this,i18n("Business Address State"));
    mAddressBusinessCountry = new KImportColumn(this,i18n("Business Address Country"));

    registerColumns();
}

void ContactImportDialog::convertRow()
{
  KABC::Addressee a;
  a.setGivenName(mFirstName->convert());
  a.setFamilyName(mLastName->convert());
  a.insertEmail(mEmail->convert(), true);
  a.setRole(mJobTitle->convert());
  
  KABC::PhoneNumber p(mPhoneBusiness->convert(), KABC::PhoneNumber::Work);
  a.insertPhoneNumber(p);
  
  p.setNumber(mPhoneHome->convert());
  p.setType(KABC::PhoneNumber::Home);
  a.insertPhoneNumber(p);
  
  p.setNumber(mPhoneMobile->convert());
  p.setType(KABC::PhoneNumber::Cell);
  a.insertPhoneNumber(p);
  
  p.setNumber(mFaxHome->convert());
  p.setType(KABC::PhoneNumber::Home | KABC::PhoneNumber::Fax);
  a.insertPhoneNumber(p);
  
  p.setNumber(mFaxBusiness->convert());
  p.setType(KABC::PhoneNumber::Work | KABC::PhoneNumber::Fax);
  a.insertPhoneNumber(p);
  
  a.setOrganization(mCompany->convert());
  
  KABC::Address addr(KABC::Address::Home);
  addr.setStreet(mAddressHomeStreet->convert());
  addr.setLocality(mAddressHomeCity->convert());
  addr.setRegion(mAddressHomeState->convert());
  addr.setPostalCode(mAddressHomeZip->convert());
  addr.setCountry(mAddressHomeCountry->convert());
  a.insertAddress(addr);
  
  addr.setType(KABC::Address::Work);
  addr.setStreet(mAddressBusinessStreet->convert());
  addr.setLocality(mAddressBusinessCity->convert());
  addr.setRegion(mAddressBusinessState->convert());
  addr.setPostalCode(mAddressBusinessZip->convert());
  addr.setCountry(mAddressBusinessCountry->convert());
  a.insertAddress(addr);
  
  mDocument->insertAddressee(a);
}

