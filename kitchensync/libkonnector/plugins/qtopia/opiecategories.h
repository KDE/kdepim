#ifndef opiecategories_h
#define opiecategories_h

#include <qstring.h>

namespace KSync {
    class OpieSocket;
};

class OpieCategories {
 public:
    friend class KSync::OpieSocket;
    friend bool operator== ( const OpieCategories &a, const OpieCategories &b );
    OpieCategories();
    OpieCategories(const QString &id, const QString &name, const QString &app );
    OpieCategories(const OpieCategories & );
    ~OpieCategories() {};
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







