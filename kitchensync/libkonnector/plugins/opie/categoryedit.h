
#ifndef CategoryEdit_H
#define CategoryEdit_H

#include <qmap.h>
#include <qstring.h>
#include <qvaluelist.h>

#include "opiecategories.h"

namespace OpieHelper {

    class CategoryEdit {
    public:
        CategoryEdit();
        CategoryEdit(const QString &fileName);
        ~CategoryEdit();
        // returns a
        QByteArray file() const;
        // global
        int addCategory( const QString &name, int id = 0 );
        // id = 0 means generate a new id
        int addCategory(const QString &appName,  const QString &name,  int id = 0);
        void parse( const QString &fileName );
        QString categoryById(const QString &id, const QString &app )const;
        void clear();
        QValueList<OpieCategories> categories()const {  return m_categories; };
    private:
        QMap<int, bool> ids; // from tt Qtopia::UidGen
        QValueList<OpieCategories> m_categories;
    };
};


#endif
