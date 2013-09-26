/*
  This file is part of KOrganizer.

  Copyright (c) 2008 Thomas Thrainer <tom_t@gmx.at>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "tododelegates.h"
#include "todomodel.h"
#include "todoviewview.h"

#include <Akonadi/Calendar/ETMCalendar>
#include <calendarsupport/categoryconfig.h>
#include <calendarsupport/kcalprefs.h>
#include <calendarsupport/categoryhierarchyreader.h>

#include <libkdepim/widgets/kcheckcombobox.h>

#include <KCalCore/CalFilter>

#include <KComboBox>
#include <KDateComboBox>

#include <QApplication>
#include <QPainter>
#include <QTextDocument>
#include <QToolTip>

// ---------------- COMPLETION DELEGATE --------------------------
// ---------------------------------------------------------------

TodoCompleteDelegate::TodoCompleteDelegate( QObject *parent )
 : QStyledItemDelegate( parent )
{
}

TodoCompleteDelegate::~TodoCompleteDelegate()
{
}

void TodoCompleteDelegate::paint( QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index ) const
{
  QStyle *style;

  QStyleOptionViewItemV4 opt = option;
  initStyleOption( &opt, index );

  style = opt.widget ? opt.widget->style() : QApplication::style();
  style->drawPrimitive( QStyle::PE_PanelItemViewItem, &opt, painter );

  if ( index.data( Qt::EditRole ).toInt() > 0 ) {
    bool isEditing = false;
    TodoViewView *view = qobject_cast<TodoViewView*>( parent() );
    if ( view ) {
      isEditing = view->isEditing( index );
    }

    // TODO QTreeView does not set State_Editing. Qt task id 205051
    // should be fixed with Qt 4.5, but wasn't. According to the
    // task tracker the fix arrives in "Some future release".
    if ( !( opt.state & QStyle::State_Editing ) && !isEditing ) {
      QStyleOptionProgressBar pbOption;
      pbOption.QStyleOption::operator=( option );
      initStyleOptionProgressBar( &pbOption, index );

      style->drawControl( QStyle::CE_ProgressBar, &pbOption, painter );
    }
  }
}

QSize TodoCompleteDelegate::sizeHint( const QStyleOptionViewItem &option,
                                        const QModelIndex &index ) const
{
  QStyleOptionViewItemV4 opt = option;
  initStyleOption( &opt, index );

  QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

  QStyleOptionProgressBar pbOption;
  pbOption.QStyleOption::operator=( option );
  initStyleOptionProgressBar( &pbOption, index );

  return style->sizeFromContents( QStyle::CT_ProgressBar, &pbOption,
                                  QSize(), opt.widget );
}

void TodoCompleteDelegate::initStyleOptionProgressBar(
                                QStyleOptionProgressBar *option,
                                const QModelIndex &index ) const
{
  option->rect.adjust( 0, 1, 0, -1 );
  option->maximum = 100;
  option->minimum = 0;
  option->progress = index.data().toInt();
  option->text = index.data().toString() + QChar::fromLatin1( '%' );
  option->textAlignment = Qt::AlignCenter;
  option->textVisible = true;
}

QWidget *TodoCompleteDelegate::createEditor( QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  TodoCompleteSlider *slider = new TodoCompleteSlider( parent );

  slider->setRange( 0, 100 );
  slider->setOrientation( Qt::Horizontal );

  return slider;
}

void TodoCompleteDelegate::setEditorData( QWidget *editor,
                                            const QModelIndex &index ) const
{
  QSlider *slider = static_cast<QSlider *>( editor );

  slider->setValue( index.data( Qt::EditRole ).toInt() );
}

void TodoCompleteDelegate::setModelData( QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index ) const
{
  QSlider *slider = static_cast<QSlider *>( editor );

  model->setData( index, slider->value() );
}

void TodoCompleteDelegate::updateEditorGeometry( QWidget *editor,
                                                   const QStyleOptionViewItem &option,
                                                   const QModelIndex &index ) const
{
  Q_UNUSED( index );

  editor->setGeometry( option.rect );
}

TodoCompleteSlider::TodoCompleteSlider( QWidget *parent )
  : QSlider( parent )
{
  connect( this, SIGNAL(valueChanged(int)),
           this, SLOT(updateTip(int)) );
}

void TodoCompleteSlider::updateTip( int value )
{
  QPoint p;
  p.setY( height() / 2 );
  p.setX( style()->sliderPositionFromValue ( minimum(), maximum(),
                                             value, width() ) );

  QString text = QString::fromLatin1( "%1%" ).arg( value );
  QToolTip::showText( mapToGlobal( p ), text, this );
}

// ---------------- PRIORITY DELEGATE ----------------------------
// ---------------------------------------------------------------

TodoPriorityDelegate::TodoPriorityDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

TodoPriorityDelegate::~TodoPriorityDelegate()
{
}

QWidget *TodoPriorityDelegate::createEditor( QWidget *parent,
                                               const QStyleOptionViewItem &option,
                                               const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  KComboBox *combo = new KComboBox( parent );

  combo->addItem( i18nc( "@action:inmenu Unspecified priority", "unspecified" ) );
  combo->addItem( i18nc( "@action:inmenu highest priority", "1 (highest)" ) );
  combo->addItem( i18nc( "@action:inmenu", "2" ) );
  combo->addItem( i18nc( "@action:inmenu", "3" ) );
  combo->addItem( i18nc( "@action:inmenu", "4" ) );
  combo->addItem( i18nc( "@action:inmenu medium priority", "5 (medium)" ) );
  combo->addItem( i18nc( "@action:inmenu", "6" ) );
  combo->addItem( i18nc( "@action:inmenu", "7" ) );
  combo->addItem( i18nc( "@action:inmenu", "8" ) );
  combo->addItem( i18nc( "@action:inmenu lowest priority", "9 (lowest)" ) );

  return combo;
}

void TodoPriorityDelegate::setEditorData( QWidget *editor,
                                            const QModelIndex &index ) const
{
  KComboBox *combo = static_cast<KComboBox *>( editor );

  combo->setCurrentIndex( index.data( Qt::EditRole ).toInt() );
}

void TodoPriorityDelegate::setModelData( QWidget *editor,
                                           QAbstractItemModel *model,
                                           const QModelIndex &index ) const
{
  KComboBox *combo = static_cast<KComboBox *>( editor );

  model->setData( index, combo->currentIndex() );
}

void TodoPriorityDelegate::updateEditorGeometry( QWidget *editor,
                                                   const QStyleOptionViewItem &option,
                                                   const QModelIndex &index ) const
{
  Q_UNUSED( index );

  editor->setGeometry( option.rect );
}

// ---------------- DUE DATE DELEGATE ----------------------------
// ---------------------------------------------------------------

TodoDueDateDelegate::TodoDueDateDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

TodoDueDateDelegate::~TodoDueDateDelegate()
{
}

QWidget *TodoDueDateDelegate::createEditor( QWidget *parent,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  KDateComboBox *dateEdit = new KDateComboBox( parent );

  return dateEdit;
}

void TodoDueDateDelegate::setEditorData( QWidget *editor,
                                           const QModelIndex &index ) const
{
  KDateComboBox *dateEdit = static_cast<KDateComboBox *>( editor );

  dateEdit->setDate( index.data( Qt::EditRole ).toDate() );
}

void TodoDueDateDelegate::setModelData( QWidget *editor,
                                          QAbstractItemModel *model,
                                          const QModelIndex &index ) const
{
  KDateComboBox *dateEdit = static_cast<KDateComboBox *>( editor );

  model->setData( index, dateEdit->date() );
}

void TodoDueDateDelegate::updateEditorGeometry( QWidget *editor,
                                                  const QStyleOptionViewItem &option,
                                                  const QModelIndex &index ) const
{
  Q_UNUSED( index );
  editor->setGeometry( QStyle::alignedRect( QApplication::layoutDirection(), Qt::AlignCenter,
                                            editor->size(), option.rect ) );
}

// ---------------- CATEGORIES DELEGATE --------------------------
// ---------------------------------------------------------------

TodoCategoriesDelegate::TodoCategoriesDelegate( QObject *parent )
  : QStyledItemDelegate( parent ), mCalendar(0)
{
}

TodoCategoriesDelegate::~TodoCategoriesDelegate()
{
}

QWidget *TodoCategoriesDelegate::createEditor( QWidget *parent,
                                                 const QStyleOptionViewItem &option,
                                                 const QModelIndex &index ) const
{
  Q_UNUSED( option );
  Q_UNUSED( index );

  KPIM::KCheckComboBox *combo = new KPIM::KCheckComboBox( parent );
  QStringList categories;

  if ( mCalendar ) {
    KCalCore::CalFilter *filter = mCalendar->filter();
    if ( filter->criteria() & KCalCore::CalFilter::ShowCategories ) {
      categories = filter->categoryList();
      categories.sort();
    } else {
      CalendarSupport::CategoryConfig cc( CalendarSupport::KCalPrefs::instance() );
      categories = cc.customCategories();
      QStringList filterCategories = filter->categoryList();
      categories.sort();
      filterCategories.sort();

      QStringList::Iterator it = categories.begin();
      QStringList::Iterator jt = filterCategories.begin();
      while ( it != categories.end() && jt != filterCategories.end() ) {
        if ( *it == *jt ) {
          it = categories.erase( it );
          jt++;
        } else if ( *it < *jt ) {
          it++;
        } else if ( *it > *jt ) {
          jt++;
        }
      }
    }
  }

  CalendarSupport::CategoryHierarchyReaderQComboBox( combo ).read( categories );
  // TODO test again with newer version of Qt, if it manages then to move
  // the popup together with the combobox.
  //combo->showPopup();
  return combo;
}

void TodoCategoriesDelegate::setEditorData( QWidget *editor,
                                              const QModelIndex &index ) const
{
  KPIM::KCheckComboBox *combo = static_cast<KPIM::KCheckComboBox *>( editor );

  combo->setCheckedItems( index.data( Qt::EditRole ).toStringList(), Qt::UserRole );
}

void TodoCategoriesDelegate::setModelData( QWidget *editor,
                                             QAbstractItemModel *model,
                                             const QModelIndex &index ) const
{
  KPIM::KCheckComboBox *combo = static_cast<KPIM::KCheckComboBox *>( editor );

  model->setData( index, combo->checkedItems( Qt::UserRole ) );
}

void TodoCategoriesDelegate::updateEditorGeometry( QWidget *editor,
                                                     const QStyleOptionViewItem &option,
                                                     const QModelIndex &index ) const
{
  Q_UNUSED( index );

  editor->setGeometry( option.rect );
}

void TodoCategoriesDelegate::setCalendar( const Akonadi::ETMCalendar::Ptr &cal )
{
  mCalendar = cal;
}

// ---------------- RICH TEXT DELEGATE ---------------------------
// ---------------------------------------------------------------

TodoRichTextDelegate::TodoRichTextDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
  m_textDoc = new QTextDocument( this );
}

TodoRichTextDelegate::~TodoRichTextDelegate()
{
}

void TodoRichTextDelegate::paint( QPainter *painter,
                                    const QStyleOptionViewItem &option,
                                    const QModelIndex &index ) const
{
  if ( index.data( TodoModel::IsRichTextRole ).toBool() ) {
    QStyleOptionViewItemV4 opt = option;
    initStyleOption( &opt, index );

    const QWidget *widget = opt.widget;
    QStyle *style = widget ? widget->style() : QApplication::style();

    QRect textRect = style->subElementRect( QStyle::SE_ItemViewItemText,
                                            &opt, widget );

    // draw the item without text
    opt.text.clear();
    style->drawControl( QStyle::CE_ItemViewItem, &opt, painter, widget );

    // draw the text (rich text)
    QPalette::ColorGroup cg = opt.state & QStyle::State_Enabled ?
                                QPalette::Normal : QPalette::Disabled;
    if ( cg == QPalette::Normal && !( opt.state & QStyle::State_Active ) ) {
      cg = QPalette::Inactive;
    }

    if ( opt.state & QStyle::State_Selected ) {
      painter->setPen(
        QPen( opt.palette.brush( cg, QPalette::HighlightedText ), 0 ) );
    } else {
      painter->setPen(
        QPen( opt.palette.brush( cg, QPalette::Text ), 0 ) );
    }
    if ( opt.state & QStyle::State_Editing ) {
      painter->setPen( QPen( opt.palette.brush( cg, QPalette::Text ), 0 ) );
      painter->drawRect( textRect.adjusted( 0, 0, -1, -1 ) );
    }

    m_textDoc->setHtml( index.data().toString() );

    painter->save();
    painter->translate( textRect.topLeft() );

    QRect tmpRect = textRect;
    tmpRect.moveTo( 0, 0 );
    m_textDoc->setTextWidth( tmpRect.width() );
    m_textDoc->drawContents( painter, tmpRect );

    painter->restore();
  } else {
    // align the text at top so that when it has more than two lines
    // it will just cut the extra lines out instead of aligning centered vertically
    QStyleOptionViewItem copy = option;
    copy.displayAlignment = Qt::AlignLeft | Qt::AlignTop;
    QStyledItemDelegate::paint( painter, copy, index );
  }
}

QSize TodoRichTextDelegate::sizeHint( const QStyleOptionViewItem &option,
                                      const QModelIndex &index ) const
{
  QSize ret = QStyledItemDelegate::sizeHint( option, index );
  if ( index.data( TodoModel::IsRichTextRole ).toBool() ) {
    m_textDoc->setHtml( index.data().toString() );
    ret = ret.expandedTo( m_textDoc->size().toSize() );
  }
  // limit height to max. 2 lines
  // TODO add graphical hint when truncating! make configurable height?
  if ( ret.height() > option.fontMetrics.height() * 2 ) {
    ret.setHeight( option.fontMetrics.height() * 2 );
  }

  // This row might not have a checkbox, so give it more height so it appears the same size as other rows.
  const int checkboxHeight = QApplication::style()->sizeFromContents( QStyle::CT_CheckBox, &option, QSize() ).height();
  return QSize( ret.width(), qMax( ret.height(), checkboxHeight ) );
}

#include "tododelegates.moc"
