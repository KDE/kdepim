/****************************************************************************
** EmpathListView meta object code from reading C++ file 'EmpathListView.h'
**
** Created: Fri Mar 17 06:06:17 2000
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#define Q_MOC_EmpathListView
#if !defined(Q_MOC_OUTPUT_REVISION)
#define Q_MOC_OUTPUT_REVISION 7
#elif Q_MOC_OUTPUT_REVISION != 7
#error "Moc format conflict - please regenerate all moc files"
#endif

#include "./EmpathListView.h"
#include <qmetaobject.h>
#include <qapplication.h>

#if defined(Q_SPARCWORKS_FUNCP_BUG)
#define Q_AMPERSAND
#else
#define Q_AMPERSAND &
#endif


const char *EmpathListView::className() const
{
    return "EmpathListView";
}

QMetaObject *EmpathListView::metaObj = 0;

void EmpathListView::initMetaObject()
{
    if ( metaObj )
	return;
    if ( strcmp(KListView::className(), "KListView") != 0 )
	badSuperclassWarning("EmpathListView","KListView");
    (void) staticMetaObject();
}

QString EmpathListView::tr(const char* s)
{
    return ((QNonBaseApplication*)qApp)->translate("EmpathListView",s);
}

QMetaObject* EmpathListView::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    (void) KListView::staticMetaObject();
    typedef void(EmpathListView::*m1_t0)(QListViewItem*);
    typedef void(EmpathListView::*m1_t1)();
    m1_t0 v1_0 = Q_AMPERSAND EmpathListView::s_currentChanged;
    m1_t1 v1_1 = Q_AMPERSAND EmpathListView::s_delayedLinkTimeout;
    QMetaData *slot_tbl = QMetaObject::new_metadata(2);
    slot_tbl[0].name = "s_currentChanged(QListViewItem*)";
    slot_tbl[1].name = "s_delayedLinkTimeout()";
    slot_tbl[0].ptr = *((QMember*)&v1_0);
    slot_tbl[1].ptr = *((QMember*)&v1_1);
    typedef void(EmpathListView::*m2_t0)(QListViewItem*,const QPoint&,int);
    typedef void(EmpathListView::*m2_t1)(QListViewItem*,const QPoint&,int,Area);
    typedef void(EmpathListView::*m2_t2)(QListViewItem*);
    typedef void(EmpathListView::*m2_t3)(const QList<QListViewItem>&);
    m2_t0 v2_0 = Q_AMPERSAND EmpathListView::rightButtonPressed;
    m2_t1 v2_1 = Q_AMPERSAND EmpathListView::rightButtonPressed;
    m2_t2 v2_2 = Q_AMPERSAND EmpathListView::linkChanged;
    m2_t3 v2_3 = Q_AMPERSAND EmpathListView::startDrag;
    QMetaData *signal_tbl = QMetaObject::new_metadata(4);
    signal_tbl[0].name = "rightButtonPressed(QListViewItem*,const QPoint&,int)";
    signal_tbl[1].name = "rightButtonPressed(QListViewItem*,const QPoint&,int,Area)";
    signal_tbl[2].name = "linkChanged(QListViewItem*)";
    signal_tbl[3].name = "startDrag(const QList<QListViewItem>&)";
    signal_tbl[0].ptr = *((QMember*)&v2_0);
    signal_tbl[1].ptr = *((QMember*)&v2_1);
    signal_tbl[2].ptr = *((QMember*)&v2_2);
    signal_tbl[3].ptr = *((QMember*)&v2_3);
    metaObj = QMetaObject::new_metaobject(
	"EmpathListView", "KListView",
	slot_tbl, 2,
	signal_tbl, 4,
	0, 0,
	0, 0,
	0, 0 );
    return metaObj;
}

#include <qobjectdefs.h>
#include <qsignalslotimp.h>

// SIGNAL rightButtonPressed
void EmpathListView::rightButtonPressed( QListViewItem* t0, const QPoint& t1, int t2 )
{
    // No builtin function for signal parameter type QListViewItem*,const QPoint&,int
    QConnectionList *clist = receivers("rightButtonPressed(QListViewItem*,const QPoint&,int)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(QListViewItem*);
    typedef RT1 *PRT1;
    typedef void (QObject::*RT2)(QListViewItem*,const QPoint&);
    typedef RT2 *PRT2;
    typedef void (QObject::*RT3)(QListViewItem*,const QPoint&,int);
    typedef RT3 *PRT3;
    RT0 r0;
    RT1 r1;
    RT2 r2;
    RT3 r3;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	    case 2:
		r2 = *((PRT2)(c->member()));
		(object->*r2)(t0, t1);
		break;
	    case 3:
		r3 = *((PRT3)(c->member()));
		(object->*r3)(t0, t1, t2);
		break;
	}
    }
}

// SIGNAL rightButtonPressed
void EmpathListView::rightButtonPressed( QListViewItem* t0, const QPoint& t1, int t2, Area t3 )
{
    // No builtin function for signal parameter type QListViewItem*,const QPoint&,int,Area
    QConnectionList *clist = receivers("rightButtonPressed(QListViewItem*,const QPoint&,int,Area)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(QListViewItem*);
    typedef RT1 *PRT1;
    typedef void (QObject::*RT2)(QListViewItem*,const QPoint&);
    typedef RT2 *PRT2;
    typedef void (QObject::*RT3)(QListViewItem*,const QPoint&,int);
    typedef RT3 *PRT3;
    typedef void (QObject::*RT4)(QListViewItem*,const QPoint&,int,Area);
    typedef RT4 *PRT4;
    RT0 r0;
    RT1 r1;
    RT2 r2;
    RT3 r3;
    RT4 r4;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	    case 2:
		r2 = *((PRT2)(c->member()));
		(object->*r2)(t0, t1);
		break;
	    case 3:
		r3 = *((PRT3)(c->member()));
		(object->*r3)(t0, t1, t2);
		break;
	    case 4:
		r4 = *((PRT4)(c->member()));
		(object->*r4)(t0, t1, t2, t3);
		break;
	}
    }
}

// SIGNAL linkChanged
void EmpathListView::linkChanged( QListViewItem* t0 )
{
    // No builtin function for signal parameter type QListViewItem*
    QConnectionList *clist = receivers("linkChanged(QListViewItem*)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(QListViewItem*);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}

// SIGNAL startDrag
void EmpathListView::startDrag( const QList<QListViewItem>& t0 )
{
    // No builtin function for signal parameter type const QList<QListViewItem>&
    QConnectionList *clist = receivers("startDrag(const QList<QListViewItem>&)");
    if ( !clist || signalsBlocked() )
	return;
    typedef void (QObject::*RT0)();
    typedef RT0 *PRT0;
    typedef void (QObject::*RT1)(const QList<QListViewItem>&);
    typedef RT1 *PRT1;
    RT0 r0;
    RT1 r1;
    QConnectionListIt it(*clist);
    QConnection   *c;
    QSenderObject *object;
    while ( (c=it.current()) ) {
	++it;
	object = (QSenderObject*)c->object();
	object->setSender( this );
	switch ( c->numArgs() ) {
	    case 0:
		r0 = *((PRT0)(c->member()));
		(object->*r0)();
		break;
	    case 1:
		r1 = *((PRT1)(c->member()));
		(object->*r1)(t0);
		break;
	}
    }
}
