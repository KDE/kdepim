#ifndef ABBROWSERAPP_H
#define ABBROWSERAPP_H
// $Id$
//
// AbBrowser Application
//

#include <kuniqueapp.h>

class Pab;

class AbBrowserApp : public KUniqueApplication {
    Q_OBJECT
  public:
    AbBrowserApp();
    virtual ~AbBrowserApp();

    int newInstance();
    
  private:
    Pab *mAbBrowser;
};

#endif
