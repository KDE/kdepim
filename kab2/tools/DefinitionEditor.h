#ifndef KAB2_DEFINITION_EDITOR_H
#define KAB2_DEFINITION_EDITOR_H

#include <qlistview.h>

#include "DefinitionEditorBase.h"

namespace KAB
{
  class FieldFormat;
  class FormatDefinition;
}

class DefinitionEditor : public DefinitionEditorBase
{
  Q_OBJECT

  public:

    class Item : public QListViewItem
    {
      public:

        Item(KAB::FieldFormat * ff, QListView * parent);
        virtual ~Item();

        KAB::FieldFormat * fieldFormat();
        void update();

    private:

        KAB::FieldFormat * fieldFormat_;
    };

    DefinitionEditor
      (
       KAB::FormatDefinition * = 0,
       QWidget * parent = 0,
       const char * name = 0
      );

    virtual ~DefinitionEditor();

  protected slots:

    void slotCurrentChanged(QListViewItem *);
    void slotAdd();
    void slotRemove();
    void slotEdit();

  protected:

    virtual void updateButtons();

    virtual void populate();

  private:

    KAB::FormatDefinition * formatDefinition_;

    Item * currentItem_;
};

#endif
