#ifndef opiecategories_h
#define opiecategories_h

#include <qstring.h>

class OpieCategories {
 public:
    friend class OpieSocket;
    OpieCategories();
    OpieCategories(const QString &id, const QString &name, const QString &app );
    OpieCategories(const OpieCategories & );
    OpieCategories &operator=(const OpieCategories & );
    QString id()const;
    QString name()const;
    QString app()const;

 private:
    class OpieCategoriesPrivate;
    OpieCategoriesPrivate *d;
    QString m_name;
    QString m_app;
    QString m_id;
};

#endif







