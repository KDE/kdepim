#ifndef KAB2_DEFINITION_ENTRY_EDITOR_H
#define KAB2_DEFINITION_ENTRY_EDITOR_H

#include "DefinitionEntryEditorBase.h"

namespace KAB
{
  class FieldFormat;
}

/**
 * Editor for KAB::FieldFormat.
 */
class DefinitionEntryEditor : public DefinitionEntryEditorBase
{
  Q_OBJECT

  public:

    DefinitionEntryEditor
      (
       KAB::FieldFormat * fieldFormat,
       QWidget * parent = 0,
       const char * name = 0
      );

    virtual ~DefinitionEntryEditor();

  protected slots:

    void slotCaptionChanged(const QString &);
    void slotMimeTypeChanged(const QString &);
    void slotMimeSubTypeChanged(const QString &);
    void slotListChanged(bool);
    void slotUniqueChanged(bool);

  private:

    KAB::FieldFormat * fieldFormat_;
};

#endif
