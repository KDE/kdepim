
#ifndef OpieHelper_MetaTodo
#define OpieHelper_MetaTodo

#include <qptrlist.h>

#include <libkcal/todo.h>

namespace OpieHelper {
    class MetaTodo;
    class MetaTodoReturn {
        friend class MetaTodo;
    public:
        MetaTodoReturn();
        MetaTodoReturn( const MetaTodoReturn& );
        ~MetaTodoReturn();
        QPtrList<KCal::Todo> added();
        QPtrList<KCal::Todo> removed();
        QPtrList<KCal::Todo> modified();
        MetaTodoReturn &operator=( const MetaTodoReturn& );
    private:
        QPtrList<KCal::Todo> m_add;
        QPtrList<KCal::Todo> m_del;
        QPtrList<KCal::Todo> m_mod;
    };

    class MetaTodo {
    public:
        MetaTodo();
        ~MetaTodo();
        /**
         * doMeta searches for differences between a new and
         * old set of Todo Events
         */
        MetaTodoReturn doMeta(QPtrList<KCal::Todo> &New,
                              QPtrList<KCal::Todo> &old);
    };
};

#endif
