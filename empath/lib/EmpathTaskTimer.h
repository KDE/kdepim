#ifndef EMPATH_TASK_TIMER_H
#define EMPATH_TASK_TIMER_H

#include <qobject.h>
#include <qtimer.h>
#include "Empath.h"
#include "EmpathTask.h"

class EmpathTaskTimer : public QObject
{
    Q_OBJECT
        
    public:
        
        EmpathTaskTimer(EmpathTask *);
        ~EmpathTaskTimer();
    
    protected slots:
        
        void s_timeout();
        void s_done();
        
    signals:
        
        void newTask(EmpathTask *);
        
    private:
        
        EmpathTask * task_;
        QTimer timer_;
};

#endif
// vim:ts=4:sw=4:tw=78
