#include <qdatetime.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtextedit.h>
#include <qsplitter.h>

#include <kiconloader.h>
#include <klocale.h>

#include <error.h>
#include <progress.h>
#include <konnectorinfo.h>

#include <ksync_profile.h>
#include <konnectorprofile.h>
#include <manipulatorpart.h>
#include <ksync_profile.h>

#include "overviewwidget.h"

using namespace KSync;
using namespace KSync::OverView;

Widget::Widget( QWidget* parent, const char* name )
    : QWidget( parent, name ) {
    m_lay = new QVBoxLayout(this);
    QLabel* label = new QLabel(this);
    label->setText( i18n("<qt><h1>Overview</h2></qt>") );

    QFrame* fra = new QFrame(this);
    fra->setFrameStyle( QFrame::HLine | QFrame::Sunken);

    //
    QHBox* info = new QHBox(this);
    m_logo = new QLabel(info);
    m_logo->setText("Logo");

    QVBox *info2 = new QVBox(info);
    m_device = new QLabel(info2);
    m_device->setText("Device");
    m_profile= new QLabel(info2);
    m_profile->setText("Profile");

    QSplitter *split = new QSplitter(this);
    m_edit = new QTextEdit( split );
    QLabel* ab = new QLabel(split);
    ab->setText("SplitLabel");

    m_lay->addWidget(label);
    m_lay->addWidget(fra  );
    m_lay->addWidget(info,  10 );
    m_lay->addWidget(split, 100);
}
Widget::~Widget() {

}
void Widget::setProfile( const Profile& prof) {
    m_profile->setText("<qt>"+i18n("<b>Profile: </b>") +prof.name() + "</qt>");
}
void Widget::setProfile( const QString& name, const QPixmap& pix) {
    m_device->setText("<qt>"+ i18n("<b>Device: </b>") + name + "</qt>");
    m_logo->setPixmap( pix );
}
void Widget::addProgress( const UDI&, const Progress& prog) {
    m_edit->append( "<b>"+QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::addProgress( ManipulatorPart*, const Progress& prog) {
    m_edit->append( "<b>"+QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::addError( const UDI&, const Error& prog) {
    m_edit->append( "<b>"+QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::addError( ManipulatorPart*, const Error& prog) {
    m_edit->append( "<b>"+QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::startSync() {
    m_edit->append("Starting to sync now");
}

#include "overviewwidget.moc"
