#include "attendeecomboboxdelegate.h"

#include "attendeeline.h"

#include <QApplication>

using namespace IncidenceEditorNG;

AttendeeComboBoxDelegate::AttendeeComboBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , standardIndex(0)
{

}

void AttendeeComboBoxDelegate::addItem(const QIcon &icon, const QString &text)
{
    QPair<QIcon, QString> pair;
    pair.first = icon;
    pair.second = text;
    entries << pair;
}

void AttendeeComboBoxDelegate::setStandardIndex(int index)
{
    standardIndex = index;
}


QWidget *AttendeeComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    AttendeeComboBox* editor = new AttendeeComboBox(parent);
    QPair<QIcon, QString> pair;

    foreach(pair, entries) {
        editor->addItem(pair.first, pair.second);
    }

    return editor;
}

void AttendeeComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    AttendeeComboBox *comboBox = static_cast<AttendeeComboBox*>(editor);
    int value = index.model()->data(index, Qt::EditRole).toUInt();
    if (value >= entries.count()) {
        value = standardIndex;
    }
    comboBox->setCurrentIndex(value);
}

void AttendeeComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    AttendeeComboBox *comboBox = static_cast<AttendeeComboBox*>(editor);
    model->setData(index, comboBox->currentIndex(), Qt::EditRole);
}

void AttendeeComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void AttendeeComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton pushButton;
    int value = index.model()->data(index).toUInt();
    pushButton.rect = option.rect;
    pushButton.features |= QStyleOptionButton::HasMenu;
    if (value >= entries.count()) {
        value = standardIndex;
    }
    pushButton.icon = entries[value].first;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &pushButton, painter);
}

AttendeeLineEditDelegate::AttendeeLineEditDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{

}


QWidget *AttendeeLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    KPIM::AddresseeLineEdit* editor = new KPIM::AddresseeLineEdit(parent);
    return editor;
}

void AttendeeLineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    KPIM::AddresseeLineEdit *lineedit = static_cast<KPIM::AddresseeLineEdit*>(editor);
    lineedit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void AttendeeLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    KPIM::AddresseeLineEdit *lineedit = static_cast<KPIM::AddresseeLineEdit*>(editor);
    model->setData(index, lineedit->text(), Qt::EditRole);
}

void AttendeeLineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
