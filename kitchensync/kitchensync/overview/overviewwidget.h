#ifndef KSYNC_OVERVIEW_WIDGET_H
#define KSYNC_OVERVIEW_WIDGET_H

#include <qlabel.h>
#include <qlayout.h>
#include <qptrlist.h>
#include <qwidget.h>

#include "overviewprogressentry.h"

class QSplitter;
class QTextEdit;

namespace KSync {

class Error;
class KonnectorProfile;
class Profile;
class Progress;

namespace OverView {

/**
  This is the MainWidget of the OverView and the only interface to the part...
 */
class Widget : public QWidget
{
  Q_OBJECT

  public:
    Widget( QWidget* parent, const char* name );
    ~Widget();

    void setProfile( const Profile& );
    void setProfile( const QString&,const QPixmap& pix );
    void addProgress( Konnector *, const Progress& );
    void addProgress( ActionPart*, const Progress& );
    void addError( Konnector *, const Error& );
    void addError( ActionPart*, const Error& );
    void syncProgress( ActionPart*, int, int);
    void startSync();
    void cleanView();

  private:
    int m_layoutFillIndex;
    QLabel* m_device;
    QLabel* m_profile;
    QLabel* m_logo;
    QVBoxLayout* m_layout;
    QPtrList<OverViewProgressEntry> m_messageList;
    QSplitter *m_split;
    QWidget* m_ab;
    QTextEdit* m_edit;
};

}

}

#endif
