
#ifndef KitchenSync_KalendarSyncAlgo
#define KitchenSync_KalendarSyncAlgo

#include <libkcal/todo.h>
#include <libkcal/event.h>
#include <kalendarsyncentry.h>

#include <ksync_plugin.h>

namespace KitchenSync {
    class SyncKalendar : public SyncPlugin {
    public:
        SyncKalendar( QObject* obj, const char *name = 0,  const QStringList & = QStringList() );
        virtual ~SyncKalendar();
        SyncReturn sync(int mode,  KSyncEntry *in,
                        KSyncEntry *out );
        void syncAsync( int mode,  KSyncEntry *in,
                        KSyncEntry *out );
    private:
        int m_mode;
        KAlendarSyncEntry *m_entry;
        QStringList blackIds1,  blackIds2;
        void syncMetaEvent( KAlendarSyncEntry* entry1,  KAlendarSyncEntry* entry2 );
        void syncMetaTodo( KAlendarSyncEntry* todo,  KAlendarSyncEntry* todo2 );

        void syncNormal( KAlendarSyncEntry* entry1,  KAlendarSyncEntry* entry2 );
        void syncTodo( KAlendarSyncEntry* todo1,  KAlendarSyncEntry* todo2 );
        void syncAdded(const QPtrList<KCal::Event> &added, const QPtrList<KCal::Event> &otherAdded  );
        void syncAdded(const QPtrList<KCal::Todo> &added,  const QPtrList<KCal::Todo> &otherAdded );
    };
};

#endif
