#include <qlayout.h>
#include <qcombobox.h>
#include <kdialog.h>
#include <kdebug.h>
#include <klineedit.h>
#include "incsearchwidget.h"

IncSearchWidget::IncSearchWidget(QWidget *parent, const char *name)
    : IncSearchWidgetBase(parent, name)
{
    layout()->setMargin(KDialog::marginHint());
    layout()->setSpacing(KDialog::spacingHint());
    connect(leIncSearch, SIGNAL(textChanged(const QString&)),
            SLOT(incSearchTextChanged(const QString&)));
    connect(leIncSearch, SIGNAL(returnPressed()),
            SLOT(incSearchTextReturnPressed()));
    connect(cbIncSearch, SIGNAL(activated(const QString&)),
            SLOT(incSearchComboActivated(const QString&)));
}

IncSearchWidget::~IncSearchWidget()
{
}

void IncSearchWidget::incSearchTextChanged(const QString&)
{
    announce();
}

void IncSearchWidget::incSearchTextReturnPressed()
{
    announce();
}

void IncSearchWidget::incSearchComboActivated(const QString&)
{
    announce();
}

void IncSearchWidget::announce()
{
    kdDebug() << "IncSearchWidget::announce: looking up "
              << leIncSearch->text() << " in "
              << cbIncSearch->currentItem() << endl;
    emit(incSearch(leIncSearch->text(), cbIncSearch->currentItem()));
}

void IncSearchWidget::setFields(const QStringList& fields)
{
    cbIncSearch->clear();
    cbIncSearch->insertStringList(fields);
}

#include "incsearchwidget.moc"
