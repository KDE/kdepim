#ifndef OpieHelper_MetaEvent
#define OpieHelper_MetaEvent

#include <qptrlist.h>
#include <libkcal/event.h>

namespace OpieHelper {
    class MetaEvent;
    class MetaEventReturn {
        friend class MetaEvent;
    public:
        MetaEventReturn();
        MetaEventReturn( const MetaEventReturn& );
        ~MetaEventReturn();
        QPtrList<KCal::Event> added();
        QPtrList<KCal::Event> modified();
        QPtrList<KCal::Event> removed();
        MetaEventReturn &operator=(const MetaEventReturn& );
    private:
        QPtrList<KCal::Event> m_add;
        QPtrList<KCal::Event> m_del;
        QPtrList<KCal::Event> m_mod;

    };

    class MetaEvent {
    public:
        MetaEvent();
        ~MetaEvent();
        MetaEventReturn doMeta( QPtrList<KCal::Event> &newE,
                                QPtrList<KCal::Event> &mod );

    };

};

#endif
