
#ifndef CUSTOMFIELDEDITWIDGET_H
#define CUSTOMFIELDEDITWIDGET_H

#include <QWidget>

#include "customfields_p.h"

class QCheckBox;
class QDateEdit;
class QDateTimeEdit;
class QGridLayout;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTimeEdit;

class CustomFieldEditWidget : public QWidget
{
  Q_OBJECT

  public:
    explicit CustomFieldEditWidget( QWidget *parent = 0 );

    void setCustomField( const CustomField &field );
    CustomField customField() const;

    void setValue( const QString &value );
    QString value() const;

  Q_SIGNALS:
    void remove( QWidget *widget );

  private Q_SLOTS:
    void remove();
    void edit();

  private:
    mutable CustomField mCustomField;

    QGridLayout *mLayout;
    QLabel *mName;
    QWidget *mEditor;
    QLineEdit *mTextEditor;
    QSpinBox *mNumericEditor;
    QCheckBox *mBooleanEditor;
    QDateEdit *mDateEditor;
    QTimeEdit *mTimeEditor;
    QDateTimeEdit *mDateTimeEditor;
};

#endif
