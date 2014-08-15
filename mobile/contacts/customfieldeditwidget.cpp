
#include "customfieldeditwidget.h"

#include "customfieldeditordialog.h"

#include <KLocalizedString>

#include <QCheckBox>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QTimeEdit>
#include <QToolButton>

CustomFieldEditWidget::CustomFieldEditWidget( QWidget *parent )
  : QWidget( parent ), mEditor( 0 )
{
  mLayout = new QGridLayout( this );

  mName = new QLabel;
  mLayout->addWidget( mName, 0, 0 );

  QToolButton *editButton = new QToolButton;
  editButton->setText( i18n( "Edit" ) );
  mLayout->addWidget( editButton, 0, 2 );

  QToolButton *deleteButton = new QToolButton;
  deleteButton->setText( i18n( "Delete" ) );
  mLayout->addWidget( deleteButton, 0, 3 );

  connect( editButton, SIGNAL(clicked()), SLOT(edit()) );
  connect( deleteButton, SIGNAL(clicked()), SLOT(remove()) );
}

void CustomFieldEditWidget::setCustomField( const CustomField &field )
{
  mCustomField = field;

  mName->setText( mCustomField.title() + QLatin1Char(':') );

  delete mEditor;
  switch ( mCustomField.type() ) {
    case CustomField::TextType:
      mTextEditor = new QLineEdit;
      mEditor = mTextEditor;
      break;
    case CustomField::NumericType:
      mNumericEditor = new QSpinBox;
      mEditor = mNumericEditor;
      break;
    case CustomField::BooleanType:
      mBooleanEditor = new QCheckBox;
      mEditor = mBooleanEditor;
      break;
    case CustomField::DateType:
      mDateEditor = new QDateEdit;
      mEditor = mDateEditor;
      break;
    case CustomField::TimeType:
      mTimeEditor = new QTimeEdit;
      mEditor = mTimeEditor;
      break;
    case CustomField::DateTimeType:
      mDateTimeEditor = new QDateTimeEdit;
      mEditor = mDateTimeEditor;
      break;
  }

  mLayout->addWidget( mEditor, 1, 0, 1, 4 );

  setValue( mCustomField.value() );
}

CustomField CustomFieldEditWidget::customField() const
{
  mCustomField.setValue( value() );

  return mCustomField;
}

void CustomFieldEditWidget::setValue( const QString &value )
{
  switch ( mCustomField.type() ) {
    case CustomField::TextType:
      mTextEditor->setText( value );
      break;
    case CustomField::NumericType:
      mNumericEditor->setValue( value.toInt() );
      break;
    case CustomField::BooleanType:
      mBooleanEditor->setChecked( value == QLatin1String( "true" ) );
      break;
    case CustomField::DateType:
      mDateEditor->setDate( QDate::fromString( value, Qt::ISODate ) );
      break;
    case CustomField::TimeType:
      mTimeEditor->setTime( QTime::fromString( value, Qt::ISODate ) );
      break;
    case CustomField::DateTimeType:
      mDateTimeEditor->setDateTime( QDateTime::fromString( value, Qt::ISODate ) );
      break;
  }
}

QString CustomFieldEditWidget::value() const
{
  switch ( mCustomField.type() ) {
    case CustomField::TextType:
      return mTextEditor->text();
      break;
    case CustomField::NumericType:
      return QString::number( mNumericEditor->value() );
      break;
    case CustomField::BooleanType:
      return (mBooleanEditor->isChecked() ? QLatin1String( "true" ) : QLatin1String( "false" ) );
      break;
    case CustomField::DateType:
      return mDateEditor->date().toString( Qt::ISODate );
      break;
    case CustomField::TimeType:
      return mTimeEditor->time().toString( Qt::ISODate );
      break;
    case CustomField::DateTimeType:
      return mDateTimeEditor->dateTime().toString( Qt::ISODate );
      break;
    default:
      return QString();
      break;
  }
}

void CustomFieldEditWidget::remove()
{
  emit remove( this );
}

void CustomFieldEditWidget::edit()
{
  CustomFieldEditorDialog dlg;
  dlg.setCustomField( mCustomField );
  if ( dlg.exec() )
    setCustomField( dlg.customField() );
}

