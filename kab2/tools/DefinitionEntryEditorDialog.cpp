#include <kab2/FieldFormat.h>

#include "DefinitionEntryEditor.h"
#include "DefinitionEntryEditorDialog.h"

DefinitionEntryEditorDialog::DefinitionEntryEditorDialog
(
 KAB::FieldFormat * ff,
 QWidget * parent,
 const char * name
)
  : KDialogBase(parent, name)
{
  widget_ = new DefinitionEntryEditor(ff, this);

  setMainWidget(widget_);
}

DefinitionEntryEditorDialog::~DefinitionEntryEditorDialog()
{
}

#include "DefinitionEntryEditorDialog.moc"
