#include "EmpathTaskTimer.h"

EmpathTaskTimer::EmpathTaskTimer(EmpathTask * t)
	:	QObject(),
		task_(t)
{
	QObject::connect(
		task_,	SIGNAL(finished()),
		this,	SLOT(s_done()));
	
	QObject::connect(
		&timer_,SIGNAL(timeout()),
		this,	SLOT(s_timeout()));
	
	QObject::connect(
		this,	SIGNAL(newTask(EmpathTask *)),
		empath,	SLOT(s_newTask(EmpathTask *)));
	
	timer_.start(100, true);
}

EmpathTaskTimer::~EmpathTaskTimer()
{
//	delete task_;
//	task_ = 0;
}

	void
EmpathTaskTimer::s_done()
{
	empathDebug("s_done() called");
	QObject::disconnect(
		task_,	SIGNAL(finished()),
		this,	SLOT(s_done()));
	timer_.stop();
	delete this;
}

	void
EmpathTaskTimer::s_timeout()
{
	emit(newTask(task_));
}

