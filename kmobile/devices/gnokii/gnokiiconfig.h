#ifndef GNOKIICONFIG_H
#define GNOKIICONFIG_H

#include <qobject.h>

#include <gnokiiconfigui.h>

class GnokiiConfig : public GnokiiConfigUI
{
Q_OBJECT
public:
    GnokiiConfig( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~GnokiiConfig();

    void setValues(const QString &model, const QString &connection, const QString &port, const QString &baud);
    void getValues(QString &model, QString &connection, QString &port, QString &baud) const;

private slots:
    void slotCheckValues();
    void slotCheckValues(const QString &);
};

#endif
