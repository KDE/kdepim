#ifndef IMPORTMAILSWIDGET_H
#define IMPORTMAILSWIDGET_H

#include <QWidget>
#include "mailimporter_export.h"

namespace Ui {
class ImportMailsWidget;
}

class MAILIMPORTER_EXPORT ImportMailsWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit ImportMailsWidget(QWidget *parent = 0);
    ~ImportMailsWidget();

    void setStatusMessage( const QString& status );
    void setFrom( const QString& from );
    void setTo( const QString& to );
    void setCurrent( const QString& current );
    void setCurrent( int percent );
    void setOverall( int percent );

private:
    Ui::ImportMailsWidget *ui;
};

#endif // IMPORTMAILSWIDGET_H
