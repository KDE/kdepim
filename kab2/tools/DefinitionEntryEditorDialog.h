#ifndef DEFINITION_ENTRY_EDITOR_DIALOG_H
#define DEFINITION_ENTRY_EDITOR_DIALOG_H

#include <kdialogbase.h>

class DefinitionEntryEditor;

namespace KAB
{
  class FieldFormat;
}

/**
 * Dialog used to wrap a DefinitionEntryEditor.
 */
class DefinitionEntryEditorDialog : public KDialogBase
{
  Q_OBJECT

  public:

    DefinitionEntryEditorDialog
      (
       KAB::FieldFormat * ff,
       QWidget * parent = 0,
       const char * name = 0
      );

    virtual ~DefinitionEntryEditorDialog();

  private:

    DefinitionEntryEditor * widget_;
};

#endif
