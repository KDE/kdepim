#include "addressEditor.h"

AddressEditor::AddressEditor(PilotAddress* address, QWidget *parent, const char *name) 
  : QWidget(parent, name), fAddress(address), fDeleteOnCancel(false)
{
  initMetaObject();
  initLayout();
  fillFields();
}

void
AddressEditor::fillFields()
{
  if(fAddress == 0L)
    {
      fAddress = new PilotAddress();
      fDeleteOnCancel = true;
    }
  fLastNameField->setText(fAddress->getField(entryLastname));
  fFirstNameField->setText(fAddress->getField(entryFirstname));
  fCompanyField->setText(fAddress->getField(entryCompany));
  fPhone1Field->setText(fAddress->getField(entryPhone1));
  fPhone2Field->setText(fAddress->getField(entryPhone2));
  fPhone3Field->setText(fAddress->getField(entryPhone3));
  fPhone4Field->setText(fAddress->getField(entryPhone4));
  fPhone5Field->setText(fAddress->getField(entryPhone5));
  fAddressField->setText(fAddress->getField(entryAddress));
  fCityField->setText(fAddress->getField(entryCity));
  fStateField->setText(fAddress->getField(entryState));
  fZipField->setText(fAddress->getField(entryZip));
  fCountryField->setText(fAddress->getField(entryCountry));
  fTitleField->setText(fAddress->getField(entryTitle));
  fCustom1Field->setText(fAddress->getField(entryCustom1));
  fCustom2Field->setText(fAddress->getField(entryCustom2));
  fCustom3Field->setText(fAddress->getField(entryCustom3));
  fCustom4Field->setText(fAddress->getField(entryCustom4));
}

void
AddressEditor::cancel()
{
  hide();
  if(fDeleteOnCancel)
    delete fAddress;
  delete this;
}

void
AddressEditor::commitChanges()
{
  fAddress->setField(entryLastname, fLastNameField->text());
  fAddress->setField(entryFirstname, fFirstNameField->text());
  fAddress->setField(entryCompany, fCompanyField->text());
  fAddress->setField(entryPhone1, fPhone1Field->text());
  fAddress->setField(entryPhone2, fPhone2Field->text());
  fAddress->setField(entryPhone3, fPhone3Field->text());
  fAddress->setField(entryPhone4, fPhone4Field->text());
  fAddress->setField(entryPhone5, fPhone5Field->text());
  fAddress->setField(entryAddress, fAddressField->text());
  fAddress->setField(entryCity, fCityField->text());
  fAddress->setField(entryState, fStateField->text());
  fAddress->setField(entryZip, fZipField->text());
  fAddress->setField(entryCountry, fCountryField->text());
  fAddress->setField(entryTitle, fTitleField->text());
  fAddress->setField(entryCustom1, fCustom1Field->text());
  fAddress->setField(entryCustom2, fCustom2Field->text());
  fAddress->setField(entryCustom3, fCustom3Field->text());
  fAddress->setField(entryCustom4, fCustom4Field->text());
  emit(recordChangeComplete(fAddress));
  hide();
  delete this;
}

AddressEditor::~AddressEditor() {
	;
}

