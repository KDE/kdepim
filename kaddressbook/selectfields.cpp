#include "selectfields.h"

#include <qlayout.h>
#include <klocale.h>

#include "selectfieldswidget.h"

////////////////////////
// SelectFields Methods

SelectFields::SelectFields(QStringList oldFields,
			               QWidget * parent,
			               const char * name,
			               bool modal)
  : KDialogBase(KDialogBase::Plain, i18n("Select Fields to Display"),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, modal)
{
    QWidget *page = plainPage();
    
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    mWidget = new SelectFieldsWidget(oldFields, page, "mWidget");
    layout->addWidget(mWidget);
}

SelectFields::SelectFields(QWidget * parent,
			               const char * name,
			               bool modal)
  : KDialogBase(KDialogBase::Plain, i18n("Select Fields to Display"),
                KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok,
                parent, name, modal)
{
    QWidget *page = plainPage();
    
    QVBoxLayout *layout = new QVBoxLayout(page);
    
    mWidget = new SelectFieldsWidget(page, "mWidget");
    layout->addWidget(mWidget);
}

QStringList SelectFields::chosenFields()
{
    return mWidget->chosenFields();
}

void SelectFields::setOldFields(const QStringList &fields)
{
  mWidget->setOldFields(fields);
}

#include "selectfields.moc"
