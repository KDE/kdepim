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
  fAddress->setField(entryLastname, fLastNameField->text().latin1());
  fAddress->setField(entryFirstname, fFirstNameField->text().latin1());
  fAddress->setField(entryCompany, fCompanyField->text().latin1());
  fAddress->setField(entryPhone1, fPhone1Field->text().latin1());
  fAddress->setField(entryPhone2, fPhone2Field->text().latin1());
  fAddress->setField(entryPhone3, fPhone3Field->text().latin1());
  fAddress->setField(entryPhone4, fPhone4Field->text().latin1());
  fAddress->setField(entryPhone5, fPhone5Field->text().latin1());
  fAddress->setField(entryAddress, fAddressField->text().latin1());
  fAddress->setField(entryCity, fCityField->text().latin1());
  fAddress->setField(entryState, fStateField->text().latin1());
  fAddress->setField(entryZip, fZipField->text().latin1());
  fAddress->setField(entryCountry, fCountryField->text().latin1());
  fAddress->setField(entryTitle, fTitleField->text().latin1());
  fAddress->setField(entryCustom1, fCustom1Field->text().latin1());
  fAddress->setField(entryCustom2, fCustom2Field->text().latin1());
  fAddress->setField(entryCustom3, fCustom3Field->text().latin1());
  fAddress->setField(entryCustom4, fCustom4Field->text().latin1());
  emit(recordChangeComplete(fAddress));
  hide();
  delete this;
}

AddressEditor::~AddressEditor() {
	;
}

