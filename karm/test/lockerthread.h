#include <tqthread.h>

class TQString;

/** 
 * Attempt to open and lock a calendar resource in a seperate thread.
 *
 * Result of attempt returned by gotlock().
 */
class LockerThread : public QThread
{
  public:
    LockerThread( const TQString &filename );
    //void setIcsFile( const TQString &filename );
    void run();
    bool gotlock() const { return m_gotlock; };

  private:
    TQString m_icsfile;
    bool    m_gotlock;
};
