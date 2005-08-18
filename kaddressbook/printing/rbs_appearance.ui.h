/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename slots use Qt Designer which will
** update this file, preserving your code. Create an init() slot in place of
** a constructor, and a destroy() slot in place of a destructor.
*****************************************************************************/


void RingBinderStyleAppearanceForm::groupLetter()
{
  if ( letterListBox->currentItem() > 0 ) {
      int id = letterListBox->currentItem();
      letterListBox->changeItem(
                letterListBox->text(id-1) + letterListBox->text(id).at(0)
              , id - 1);
      if ( letterListBox->text(id).length() > 1 ) {
          letterListBox->changeItem(
                    letterListBox->text(id).right(letterListBox->text(id).length()-1)
                  , id
                  );
          letterListBox->setCurrentItem(id);
      } else {
          letterListBox->removeItem(id);
      }
  }
}

void RingBinderStyleAppearanceForm::ungroupLetter()
{
  if ( letterListBox->text(letterListBox->currentItem()).length() > 1 ) {
      int id = letterListBox->currentItem();
      letterListBox->insertItem( QString(letterListBox->text(id).at(letterListBox->text(id).length()-1)), id+1 );
      letterListBox->changeItem( letterListBox->text(id).left(letterListBox->text(id).length()-1), id );
  }
}
