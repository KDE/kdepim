#include <qlistview.h>
#include <qpushbutton.h>

#include <klocale.h>
#include <klineeditdlg.h>
#include <kmessagebox.h>

#include <kab2/FieldFormat.h>
#include <kab2/FormatDefinition.h>

#include "DefinitionEditor.h"
#include "DefinitionEntryEditorDialog.h"

DefinitionEditor::Item::Item(KAB::FieldFormat * ff, QListView * parent)
  : QListViewItem(parent),
    fieldFormat_(ff)
{
  update();
}

DefinitionEditor::Item::~Item()
{
}

  KAB::FieldFormat *
DefinitionEditor::Item::fieldFormat()
{
  return fieldFormat_;
}

  void
DefinitionEditor::Item::Item::update()
{
  setText(0, fieldFormat_->name());
  setText(1, fieldFormat_->caption());
  setText
    (
     2,
     fieldFormat_->type() + QString::fromUtf8("/") + fieldFormat_->subType()
    );
}


DefinitionEditor::DefinitionEditor
(
 KAB::FormatDefinition * formatDefinition,
 QWidget * parent,
 const char * name
)
  : DefinitionEditorBase(parent, name),
    formatDefinition_(formatDefinition),
    currentItem_(0)
{
  if (0 == formatDefinition_)
  {
    formatDefinition_ = new KAB::FormatDefinition;
  }

  connect
    (
     lv_definition,
     SIGNAL(currentChanged(QListViewItem *)),
     this,
     SLOT(slotCurrentChanged(QListViewItem *))
    );

  connect(pb_add,     SIGNAL(clicked()),  this, SLOT(slotAdd()));
  connect(pb_remove,  SIGNAL(clicked()),  this, SLOT(slotRemove()));
  connect(pb_edit,    SIGNAL(clicked()),  this, SLOT(slotEdit()));

  populate();

  updateButtons();
}

DefinitionEditor::~DefinitionEditor()
{
}

  void
DefinitionEditor::populate()
{
  lv_definition->clear();

  QValueList<KAB::FieldFormat> fl(formatDefinition_->fieldFormatList());

  QValueList<KAB::FieldFormat>::ConstIterator it;

  for (it = fl.begin(); it != fl.end(); ++it)
  {
    new Item(new KAB::FieldFormat(*it), lv_definition);
  }
}

  void
DefinitionEditor::slotCurrentChanged(QListViewItem * i)
{
  currentItem_ = static_cast<Item *>(i);
  updateButtons();
}

  void
DefinitionEditor::updateButtons()
{
  pb_remove ->setEnabled(0 != currentItem_);
  pb_edit   ->setEnabled(0 != currentItem_);
}

  void
DefinitionEditor::slotAdd()
{
  bool choseName = false;

  QString entryName =
    KLineEditDlg::getText
    (
     i18n("Entry name"),
     QString::fromUtf8("new_entry"),
     &choseName,
     this
    );

  if (!choseName)
    return;

  if (formatDefinition_->contains(entryName))
  {
    KMessageBox::sorry
      (
       this,
       i18n("There is already a field with the name `%1'").arg(entryName)
      );

    return;
  }

  // Use entryName for caption too. It's not the best default, but it's
  // better than nothing.

  KAB::FieldFormat * ff = new KAB::FieldFormat(entryName, entryName);

  DefinitionEntryEditorDialog ee(ff, this);

  int result = ee.exec();

  if (QDialog::Accepted == result)
  {
    qDebug("Accepted");

    bool added = formatDefinition_->add(*ff);

    if (!added)
    {
      qWarning("Couldn't add entry to formatDefinition. Why ?");
    }
    else
    {
      new Item(ff, lv_definition);
    }
  }
  else
  {
    qDebug("Rejected");

    delete ff;
    ff = 0;
  }
}

  void
DefinitionEditor::slotRemove()
{
  if (0 != currentItem_)
  {
    delete currentItem_;
  }
  else
  {
    qDebug("Remove button pressed but it shouldn't have been enabled");
  }
}

  void
DefinitionEditor::slotEdit()
{
  if (0 != currentItem_)
  {
    KAB::FieldFormat * ff = currentItem_->fieldFormat();

    DefinitionEntryEditorDialog ee(ff, this);

    int result = ee.exec();

    if (QDialog::Accepted == result)
    {
      qDebug("Accepted");

      bool replaced = formatDefinition_->replace(*ff);

      if (!replaced)
      {
        qWarning("Couldn't replace entry ! Why ?");
      }
      else
      {
        currentItem_->update();
      }
    }
    else
    {
      qDebug("Rejected");
    }
  }
  else
  {
    qDebug("Edit button pressed but it shouldn't have been enabled");
  }
}


#include "DefinitionEditor.moc"
