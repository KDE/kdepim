#ifndef KINCIDENCENFORMATTER_H
#define KINCIDENCENFORMATTER_H

#include <qstring.h>
#include <qobject.h>

#include "libkcal/incidence.h"
#include "libkcal/event.h"
#include "libkcal/todo.h"
#include "libkcal/journal.h"

using namespace KCal;

class KIncidenceFormatter : public QObject
{
  public:
    static KIncidenceFormatter* instance();
    KIncidenceFormatter();
    ~KIncidenceFormatter();
    QString getFormattedText( Incidence * inc );
 
    void setEvent(Event *event);
    void setTodo(Todo *event );
    void setJournal(Journal*  );

  protected:
    void addLink( const QString &ref, const QString &text,
                  bool newline = true );
    int mColorMode;
    void addTag(const QString & tag,const QString & text);

    void formatCategories(Incidence *event);
    void formatAttendees(Incidence *event);
    void formatReadOnly(Incidence *event);

  private:
    bool mSyncMode;

    QString mText;
    Incidence* mCurrentIncidence; 
    static KIncidenceFormatter* mInstance; 
};

#endif
