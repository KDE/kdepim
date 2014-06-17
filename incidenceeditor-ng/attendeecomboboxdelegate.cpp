#include "attendeecomboboxdelegate.h"

#include "attendeeline.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QMenu>
#include <QHelpEvent>
#include <QToolTip>
#include <QWhatsThis>

#include <KDebug>
#include <KLocalizedString>

using namespace IncidenceEditorNG;

AttendeeComboBoxDelegate::AttendeeComboBoxDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
    , standardIndex(0)
{
    connect(this, SIGNAL(closeEditor(QWidget*)), SLOT(doCloseEditor(QWidget*)));
}

void AttendeeComboBoxDelegate::addItem(const QIcon &icon, const QString &text)
{
    QPair<QIcon, QString> pair;
    pair.first = icon;
    pair.second = text;
    entries << pair;
}

void AttendeeComboBoxDelegate::clear()
{
    entries.clear();
}


void AttendeeComboBoxDelegate::setToolTip(const QString& tT)
{
    toolTip = tT;
}

void AttendeeComboBoxDelegate::setWhatsThis(const QString& wT)
{
    whatsThis = wT;
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

    connect(editor,SIGNAL(leftPressed()),SLOT(leftPressed()));
    connect(editor,SIGNAL(rightPressed()),SLOT(rightPressed()));

    editor->setPopupMode( QToolButton::MenuButtonPopup);
    editor->setToolTip(toolTip);
    editor->setWhatsThis(whatsThis);
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
    comboBox->menu()->close();
}

void AttendeeComboBoxDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void AttendeeComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton myOption;;

    int value = index.model()->data(index).toUInt();
    if (value >= entries.count()) {
        value = standardIndex;
    }

    myOption.rect = option.rect;
    myOption.state = option.state;
    myOption.icon = entries[value].first;

    QApplication::style()->drawControl(QStyle::CE_PushButton, &myOption, painter);
}

bool AttendeeComboBoxDelegate::eventFilter ( QObject * editor, QEvent * event )
{
  if (event->type() == QEvent::Enter) {
    AttendeeComboBox *comboBox = static_cast<AttendeeComboBox*>(editor);
    comboBox->showMenu();
    return editor->eventFilter(editor, event);
  }

  return QStyledItemDelegate::eventFilter(editor, event);
}

void AttendeeComboBoxDelegate::doCloseEditor(QWidget* editor)
{
    AttendeeComboBox *comboBox = static_cast<AttendeeComboBox*>(editor);
    comboBox->menu()->close();
}

void AttendeeComboBoxDelegate::leftPressed()
{
    emit closeEditor(static_cast<QWidget*>(QObject::sender()),QAbstractItemDelegate::EditPreviousItem);
}

void AttendeeComboBoxDelegate::rightPressed()
{
    emit closeEditor(static_cast<QWidget*>(QObject::sender()),QAbstractItemDelegate::EditNextItem);
}


bool AttendeeComboBoxDelegate::helpEvent( QHelpEvent* event, QAbstractItemView* view,
    const QStyleOptionViewItem& option, const QModelIndex& index )
{
    if (!event || !view) {
        return false;
    }
    switch (event->type()) {
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
          QToolTip::showText(he->globalPos(), toolTip, view);
        return true;
        break;}
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis:
        return true;
        break;
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
          QWhatsThis::showText(he->globalPos(), whatsThis, view);
        return true;
        break ; }
#endif
    default:
        break;
    }
    return QStyledItemDelegate::helpEvent( event, view, option, index );
}

AttendeeLineEditDelegate::AttendeeLineEditDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
    toolTip = i18nc( "@info:tooltip",
                     "Enter the name or email address of the attendee." );
    whatsThis = i18nc( "@info:whatsthis",
                       "The email address or name of the attendee. An invitation "
                       "can be sent to the user if an email address is provided." );
}

QWidget *AttendeeLineEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &/* option */, const QModelIndex &/* index */) const
{
    AttendeeLineEdit* editor = new AttendeeLineEdit(parent);
    connect(editor,SIGNAL(leftPressed()),SLOT(leftPressed()));
    connect(editor,SIGNAL(rightPressed()),SLOT(rightPressed()));
    editor->setToolTip(toolTip);
    editor->setWhatsThis(whatsThis);
    editor->setCompletionMode(completionMode);
    editor->setClearButtonShown( true );

    return editor;
}

void AttendeeLineEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    AttendeeLineEdit *lineedit = static_cast<AttendeeLineEdit*>(editor);
    lineedit->setText(index.model()->data(index, Qt::EditRole).toString());
}

void AttendeeLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    AttendeeLineEdit *lineedit = static_cast<AttendeeLineEdit*>(editor);
    model->setData(index, lineedit->text(), Qt::EditRole);
}

void AttendeeLineEditDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

void AttendeeLineEditDelegate::leftPressed()
{
    emit closeEditor(static_cast<QWidget*>(QObject::sender()),QAbstractItemDelegate::EditPreviousItem);
}

void AttendeeLineEditDelegate::rightPressed()
{
    emit closeEditor(static_cast<QWidget*>(QObject::sender()),QAbstractItemDelegate::EditNextItem);
}

void AttendeeLineEditDelegate::setCompletionMode(KGlobalSettings::Completion mode)
{
    completionMode = mode;
}

bool AttendeeLineEditDelegate::helpEvent( QHelpEvent* event, QAbstractItemView* view,
    const QStyleOptionViewItem& option, const QModelIndex& index )
{
    if (!event || !view) {
        return false;
    }
    switch (event->type()) {
#ifndef QT_NO_TOOLTIP
    case QEvent::ToolTip: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
          QToolTip::showText(he->globalPos(), toolTip, view);
        return true;
        break;}
#endif
#ifndef QT_NO_WHATSTHIS
    case QEvent::QueryWhatsThis:
        return true;
        break;
    case QEvent::WhatsThis: {
        QHelpEvent *he = static_cast<QHelpEvent*>(event);
          QWhatsThis::showText(he->globalPos(), whatsThis, view);
        return true;
        break ; }
#endif
    default:
        break;
    }
    return QStyledItemDelegate::helpEvent( event, view, option, index );
}
