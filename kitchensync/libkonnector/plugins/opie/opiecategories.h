
#include <qstring.h>

class OpieCategories {
 public:
    friend class OpieSocket;
    OpieCategories();
    OpieCategories(int id, const QString &name, const QString &app );
    OpieCategories(const OpieCategories & );
    OpieCategories &operator=(const OpieCategories & );
    int id();
    QString name();
    QString app();

 private:
    class OpieCategoriesPrivate;
    OpieCategoriesPrivate *d;
    QString m_name;
    QString m_app;
    int m_id;
};
