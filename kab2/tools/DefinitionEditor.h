#ifndef KAB2_DEFINITION_EDITOR_H
#define KAB2_DEFINITION_EDITOR_H

#include "DefinitionEditorBase.h"

class DefinitionEditor : public DefinitionEditorBase
{
  Q_OBJECT

  public:

    DefinitionEditor(QWidget * parent = 0, const char * name = 0);
    virtual ~DefinitionEditor();
};

#endif
