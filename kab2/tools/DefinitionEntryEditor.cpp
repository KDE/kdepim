#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>

#include <kab2/FieldFormat.h>
#include "DefinitionEntryEditor.h"

DefinitionEntryEditor::DefinitionEntryEditor
(
 KAB::FieldFormat * ff,
 QWidget * parent,
 const char * name
)
  : DefinitionEntryEditorBase(parent, name),
    fieldFormat_(ff)
{
  l_name          ->setText(fieldFormat_->name());
  le_caption      ->setText(fieldFormat_->caption());

  le_mimeType     ->setText(fieldFormat_->type());
  le_mimeSubType  ->setText(fieldFormat_->subType());

  cb_list         ->setChecked(fieldFormat_->list());
  cb_unique       ->setChecked(fieldFormat_->unique());
}

DefinitionEntryEditor::~DefinitionEntryEditor()
{
}

  void
DefinitionEntryEditor::slotCaptionChanged(const QString & s)
{
  fieldFormat_->setCaption(s);
}

  void
DefinitionEntryEditor::slotMimeTypeChanged(const QString & s)
{
  fieldFormat_->setType(s);
}

  void
DefinitionEntryEditor::slotMimeSubTypeChanged(const QString & s)
{
  fieldFormat_->setSubType(s);
}

  void
DefinitionEntryEditor::slotListChanged(bool b)
{
  fieldFormat_->setList(b);
}

  void
DefinitionEntryEditor::slotUniqueChanged(bool b)
{
  fieldFormat_->setUnique(b);
}



#include "DefinitionEntryEditor.moc"
