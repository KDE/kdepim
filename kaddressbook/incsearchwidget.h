#ifndef INCSEARCHWIDGET_H
#define INCSEARCHWIDGET_H

#include "incsearchwidget_base.h"

class IncSearchWidget : public IncSearchWidgetBase
{
    Q_OBJECT
public:
    IncSearchWidget(QWidget *parent, const char *name=0);
    ~IncSearchWidget();
public slots:
    /** Set the incremental search fields (in the combo). */
    void setFields(const QStringList& list);
protected slots:
    void incSearchTextChanged(const QString&);
    void incSearchTextReturnPressed();
    void incSearchComboActivated(const QString&);
signals:
    /** Do incremental search, usin the field at index field.
        @see setFields
    */
    void incSearch(const QString& text, int field);
private:
    void announce();
};

#endif
