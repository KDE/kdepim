#include "idletimedetector.h"

#include <qdatetime.h>
#include <qmessagebox.h>
#include <qtimer.h>

#include <kglobal.h>
#include <klocale.h>    // i18n

IdleTimeDetector::IdleTimeDetector(int maxIdle)
// Trigger a warning after maxIdle minutes
{
  kdDebug(5970) << "Entering IdleTimeDetector::IdleTimeDetector" << endl;
  _maxIdle = maxIdle;

#ifdef HAVE_LIBXSS
  kdDebug(5970) << "IdleTimeDetector: LIBXSS detected @ compile time" << endl;
  int event_base, error_base;
  if(XScreenSaverQueryExtension(qt_xdisplay(), &event_base, &error_base)) 
  {
    _idleDetectionPossible = true;
  }
  else 
  {
    _idleDetectionPossible = false;
  }

  _timer = new QTimer(this);
  connect(_timer, SIGNAL(timeout()), this, SLOT(check()));
#else
  _idleDetectionPossible = false;
#endif // HAVE_LIBXSS

}

bool IdleTimeDetector::isIdleDetectionPossible()
{
  return _idleDetectionPossible;
}

void IdleTimeDetector::check()
{
  kdDebug(5970) << "Entering IdleTimeDetector::check" << endl;
#ifdef HAVE_LIBXSS
  if (_idleDetectionPossible)
  {
    _mit_info = XScreenSaverAllocInfo ();
    XScreenSaverQueryInfo(qt_xdisplay(), qt_xrootwin(), _mit_info);
    int idleSeconds = (_mit_info->idle/1000);
    if (idleSeconds >= _maxIdle)
      informOverrun(idleSeconds);
  }
#endif // HAVE_LIBXSS
}

void IdleTimeDetector::setMaxIdle(int maxIdle)
{
  _maxIdle = maxIdle;
}

#ifdef HAVE_LIBXSS
void IdleTimeDetector::informOverrun(int idleSeconds)
{
  kdDebug(5970) << "Entering IdleTimeDetector::informOverrun" << endl;
  if (!_overAllIdleDetect)
    return; // preferences say the user does not want idle detection.

  _timer->stop();

  QDateTime idleStart = QDateTime::currentDateTime().addSecs(-idleSeconds);
  QString idleStartQString = KGlobal::locale()->formatTime(idleStart.time());

  int id =  QMessageBox::warning( 0, i18n("Idle Detection"),
                                     i18n("Desktop has been idle since %1."
                                          " What should we do?").arg(idleStartQString),
                                     i18n("Revert && Stop"),
                                     i18n("Revert && Continue"),
                                     i18n("Continue Timing"),0,2);
  QDateTime end = QDateTime::currentDateTime();
  int diff = idleStart.secsTo(end)/secsPerMinute;

  if (id == 0) 
  {
    // Revert And Stop
    kdDebug(5970) << "Now it is " << QDateTime::currentDateTime() << endl;
    kdDebug(5970) << "Reverting timer to " << KGlobal::locale()->formatTime(idleStart.time()).ascii() << endl;
    emit(extractTime(idleSeconds/60+diff)); // we need to subtract the time that has been added during idleness.
    emit(stopAllTimersAt(idleStart));
  }
  else if (id == 1) 
  {
    // Revert and Continue
    emit(extractTime(idleSeconds/60+diff));
    _timer->start(testInterval);
  }
  else 
  {
    // Continue
    _timer->start(testInterval);
  }
}
#endif // HAVE_LIBXSS

void IdleTimeDetector::startIdleDetection()
{
  kdDebug(5970) << "Entering IdleTimeDetector::startIdleDetection" << endl; 
#ifdef HAVE_LIBXSS
  kdDebug(5970) << "Starting Timer" << endl;
  if (!_timer->isActive())
    _timer->start(testInterval);
#endif //HAVE_LIBXSS
}

void IdleTimeDetector::stopIdleDetection()
{
#ifdef HAVE_LIBXSS
  if (_timer->isActive())
    _timer->stop();
#endif // HAVE_LIBXSS
}
void IdleTimeDetector::toggleOverAllIdleDetection(bool on)
{
  _overAllIdleDetect = on;
}

#include "idletimedetector.moc"
