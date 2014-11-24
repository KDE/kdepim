/*
  Copyright (c) 2000, 2001, 2002 Cornelius Schumacher <schumacher@kde.org>
  Copyright (C) 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  Copyright (c) 2010 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
    Author: Sérgio Martins <sergio.martins@kdab.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "categorydialog.h"
#include "categoryhierarchyreader.h"
#include "ui_categorydialog_base.h"

#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/categoryconfig.h>

#include <QIcon>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace IncidenceEditorNG;
using namespace CalendarSupport;

class CategoryWidgetBase : public QWidget, public Ui::CategoryDialog_base
{
public:
    CategoryWidgetBase(QWidget *parent) : QWidget(parent)
    {
        setupUi(this);
    }
};

CategoryWidget::CategoryWidget(CategoryConfig *cc, QWidget *parent)
    : QWidget(parent), mCategoryConfig(cc)
{
    QHBoxLayout *topL = new QHBoxLayout(this);
    topL->setMargin(0);
    mWidgets = new CategoryWidgetBase(this);
    topL->addWidget(mWidgets);

    mWidgets->mButtonAdd->setIcon(QIcon::fromTheme("list-add"));
    mWidgets->mButtonRemove->setIcon(QIcon::fromTheme("list-remove"));
    mWidgets->mLineEdit->setPlaceholderText(i18n("Click to add a new category"));

    connect(mWidgets->mLineEdit, SIGNAL(textChanged(QString)),
            SLOT(handleTextChanged(QString)));

    mWidgets->mButtonAdd->setEnabled(false);
    mWidgets->mButtonRemove->setEnabled(false);
    mWidgets->mColorCombo->setEnabled(false);

    connect(mWidgets->mCategories, SIGNAL(itemSelectionChanged()),
            SLOT(handleSelectionChanged()));

    connect(mWidgets->mButtonAdd, SIGNAL(clicked()),
            SLOT(addCategory()));

    connect(mWidgets->mButtonRemove, SIGNAL(clicked()),
            SLOT(removeCategory()));

    connect(mWidgets->mColorCombo, SIGNAL(activated(QColor)),
            SLOT(handleColorChanged(QColor)));

}

CategoryWidget::~CategoryWidget()
{
}

AutoCheckTreeWidget *CategoryWidget::listView() const
{
    return mWidgets->mCategories;
}

void CategoryWidget::hideButton()
{
}

void CategoryWidget::setCategories(const QStringList &categoryList)
{
    mWidgets->mCategories->clear();
    mCategoryList.clear();

    QStringList::ConstIterator it;
    QStringList cats = mCategoryConfig->customCategories();
    for (it = categoryList.begin(); it != categoryList.end(); ++it) {
        if (!cats.contains(*it)) {
            cats.append(*it);
        }
    }
    mCategoryConfig->setCustomCategories(cats);
    CategoryHierarchyReaderQTreeWidget(mWidgets->mCategories).read(cats);
}

void CategoryWidget::setSelected(const QStringList &selList)
{
    clear();
    QStringList::ConstIterator it;

    const bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
    mWidgets->mCategories->setAutoCheckChildren(false);
    for (it = selList.begin(); it != selList.end(); ++it) {
        QStringList path = CategoryHierarchyReader::path(*it);
        QTreeWidgetItem *item = mWidgets->mCategories->itemByPath(path);
        if (item) {
            item->setCheckState(0, Qt::Checked);
        }
    }
    mWidgets->mCategories->setAutoCheckChildren(remAutoCheckChildren);
}

static QStringList getSelectedCategories(AutoCheckTreeWidget *categoriesView)
{
    QStringList categories;

    QTreeWidgetItemIterator it(categoriesView, QTreeWidgetItemIterator::Checked);
    while (*it) {
        QStringList path = categoriesView->pathByItem(*it++);
        if (path.count()) {
            path.replaceInStrings(CategoryConfig::categorySeparator, QString("\\") +
                                  CategoryConfig::categorySeparator);
            categories.append(path.join(CategoryConfig::categorySeparator));
        }
    }

    return categories;
}

void CategoryWidget::clear()
{
    const bool remAutoCheckChildren = mWidgets->mCategories->autoCheckChildren();
    mWidgets->mCategories->setAutoCheckChildren(false);

    QTreeWidgetItemIterator it(mWidgets->mCategories);
    while (*it) {
        (*it++)->setCheckState(0, Qt::Unchecked);
    }

    mWidgets->mCategories->setAutoCheckChildren(remAutoCheckChildren);
}

void CategoryWidget::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->mCategories->setAutoCheckChildren(autoselectChildren);
}

void CategoryWidget::hideHeader()
{
    mWidgets->mCategories->header()->hide();
}

QStringList CategoryWidget::selectedCategories(QString &categoriesStr)
{
    mCategoryList = getSelectedCategories(listView());
    categoriesStr = mCategoryList.join(", ");
    return mCategoryList;
}

QStringList CategoryWidget::selectedCategories() const
{
    return mCategoryList;
}

void CategoryWidget::setCategoryList(const QStringList &categories)
{
    mCategoryList = categories;
}

void CategoryWidget::addCategory()
{
    QTreeWidgetItem *newItem = new QTreeWidgetItem(listView(),
            QStringList(mWidgets->mLineEdit->text()));
    listView()->scrollToItem(newItem);
    listView()->clearSelection();
    newItem->setSelected(true);
}

void CategoryWidget::removeCategory()
{
    // Multi-select not supported, only one selected
    QTreeWidgetItem *itemToDelete = listView()->selectedItems().first();
    delete itemToDelete;
}

void CategoryWidget::handleTextChanged(const QString &newText)
{
    mWidgets->mButtonAdd->setEnabled(!newText.isEmpty());
}

void CategoryWidget::handleSelectionChanged()
{
    const bool hasSelection = !listView()->selectedItems().isEmpty();
    mWidgets->mButtonRemove->setEnabled(hasSelection);
    mWidgets->mColorCombo->setEnabled(hasSelection);

    if (hasSelection) {
        const QTreeWidgetItem *item = listView()->selectedItems().first();
        const QColor &color = KCalPrefs::instance()->categoryColor(item->text(0));
        if (color.isValid()) {
            mWidgets->mColorCombo->setColor(color);
            // update is needed. bug in KColorCombo?
            mWidgets->mColorCombo->update();
        }
    }
}

void CategoryWidget::handleColorChanged(const QColor &newColor)
{
    if (!listView()->selectedItems().isEmpty()) {
        const QTreeWidgetItem *item = listView()->selectedItems().first();
        const QString category = item->text(0);
        if (newColor.isValid()) {
            KCalPrefs::instance()->setCategoryColor(category, newColor);
        }
    }
}

CategoryDialog::CategoryDialog(CategoryConfig *cc, QWidget *parent)
    : QDialog(parent), d(0)
{
    setWindowTitle(i18n("Select Categories"));
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QDialogButtonBox *buttonBox = 0;

#ifdef KDEPIM_MOBILE_UI
    // HACK: This is for maemo, which hides the button if there is only a cancel
    //       button.
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
#else
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Apply);
#endif

    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CategoryDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CategoryDialog::reject);

    QWidget *page = new QWidget;
    mainLayout->addWidget(page);
    mainLayout->addWidget(buttonBox);
    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setMargin(0);

    mWidgets = new CategoryWidget(cc, this);
    mCategoryConfig = cc;
    mWidgets->setObjectName("CategorySelection");
    mWidgets->hideHeader();
    lay->addWidget(mWidgets);

    mWidgets->setCategories();
    mWidgets->listView()->setFocus();

    connect(okButton, &QPushButton::clicked, this, &CategoryDialog::slotOk);
    connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &CategoryDialog::slotApply);
}

CategoryDialog::~CategoryDialog()
{
    delete mWidgets;
}

QStringList CategoryDialog::selectedCategories() const
{
    return mWidgets->selectedCategories();
}

void CategoryDialog::slotApply()
{
    QStringList l;

    QStringList path;
    QTreeWidgetItemIterator it(mWidgets->listView());
    while (*it) {
        path = mWidgets->listView()->pathByItem(*it++);
        path.replaceInStrings(
            CategoryConfig::categorySeparator,
            QString("\\") + CategoryConfig::categorySeparator);
        l.append(path.join(CategoryConfig::categorySeparator));
    }
    mCategoryConfig->setCustomCategories(l);
    mCategoryConfig->writeConfig();

    QString categoriesStr;
    QStringList categories = mWidgets->selectedCategories(categoriesStr);
    emit categoriesSelected(categories);
    emit categoriesSelected(categoriesStr);
}

void CategoryDialog::slotOk()
{
    slotApply();
    accept();
}

void CategoryDialog::updateCategoryConfig()
{
    QString tmp;
    QStringList selected = mWidgets->selectedCategories(tmp);
    mWidgets->setCategories();
    mWidgets->setSelected(selected);
}

void CategoryDialog::setAutoselectChildren(bool autoselectChildren)
{
    mWidgets->setAutoselectChildren(autoselectChildren);
}

void CategoryDialog::setCategoryList(const QStringList &categories)
{
    mWidgets->setCategoryList(categories);
}

void CategoryDialog::setSelected(const QStringList &selList)
{
    mWidgets->setSelected(selList);
}

