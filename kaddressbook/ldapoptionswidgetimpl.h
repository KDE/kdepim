#ifndef LDAPOPTIONSWIDGETIMPL_H
#define LDAPOPTIONSWIDGETIMPL_H
#include "ldapoptionswidget.h"
#include <qlistbox.h>

class Servers
    {
    public:
        Servers(): pt(389) {}
        Servers( int port, const QString& baseDN )
            : pt(port), be(baseDN)
        { }

        int port() const { return pt; }
        QString baseDN() const { return be; }
        void setport( int port ) { pt = port; }
        void setbase( QString base) {  be = base; }

    private:
        int pt;
        QString be;
    };
typedef QMap<QString, Servers> ServersMap;
class LDAPOptionsWidgetImpl : public LDAPOptionsWidget
{ 
    Q_OBJECT

public:
    LDAPOptionsWidgetImpl( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~LDAPOptionsWidgetImpl();

    ServersMap _ldapservers;
    Servers server;

 signals:
     void selectionChanged();  

public slots:
    void addHost();
    void editHost();
    void removeHost();
    void checkSelected();

   

};

#endif // LDAPOPTIONSWIDGETIMPL_H
