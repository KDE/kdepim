#include <qdatetime.h>
#include <qvbox.h>
#include <qtextedit.h>
#include <qsplitter.h>

#include <klocale.h>

#include <error.h>
#include <progress.h>

#include <manipulatorpart.h>

#include "overviewwidget.h"

using namespace KSync;
using namespace KSync::OverView;

Widget::Widget( QWidget* parent, const char* name )
    : QWidget( parent, name ) {
    m_lay = new QVBoxLayout(this);
    QLabel* label = new QLabel(this);
    label->setText( "<qt><h1>" + i18n("Overview") + "</h1></qt>" );

    QFrame* line = new QFrame( this );
    line->setFrameStyle( QFrame::HLine | QFrame::Sunken );

    QHBox* info = new QHBox(this);
    info->setSpacing( 10 );
    info->setMargin( 10 );

    QVBox *info2 = new QVBox(info);
    m_device = new QLabel(info2);
    m_profile= new QLabel(info2);
    info->setStretchFactor( info2, 5 );

    m_logo = new QLabel( info );

    m_split = new QSplitter(this);
    m_edit = new QTextEdit( m_split );
    m_edit->setReadOnly(true);
    m_edit->setTextFormat(Qt::LogText);
    m_ab = new QWidget( m_split );

    QValueList<int> list;
    list.append( 0 );
    list.append( 200 );
    m_split->setSizes( list );
    m_split->setResizeMode( m_edit, QSplitter::KeepSize );

    m_layout = new QVBoxLayout( m_ab );
    m_layout->insertStretch( -1, 5 );
    m_layoutFillIndex = 0;

    m_lay->addWidget( label );
    m_lay->addWidget( line );
    m_lay->addWidget( info );
    m_lay->addWidget( m_split, 100 );

    m_messageList.setAutoDelete( true );
}
Widget::~Widget() {

}
void Widget::setProfile( const Profile& prof) {
    m_profile->setText("<qt><b>" + i18n(" Profile: ") + "</b>" + prof.name() + "</qt>");
    cleanView();
}
void Widget::setProfile( const QString& name, const QPixmap& pix) {
    m_device->setText("<qt><b>"+ i18n(" Device: ") + "</b>" + name + "</qt>");
    m_logo->setPixmap( pix );
    cleanView();
}
void Widget::addProgress( const UDI&, const Progress& prog) {
    m_edit->append( "<b>"+QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::addProgress( ManipulatorPart* part, const Progress& prog) {
    m_edit->append( "<b>" + QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::syncProgress( ManipulatorPart* part, int status, int percent )  {

    OverViewProgressEntry* it;
    for ( it = m_messageList.first(); it; it = m_messageList.next() )  {
        if ( QString::compare( it->name(), part->name() ) == 0 ) {
            it->setProgress( status );
            return;
        }
    }

    OverViewProgressEntry* test = new OverViewProgressEntry( m_ab, "test" );
    m_messageList.append( test );

    if ( !part->name().isEmpty() )  {
        test->setText( part->name() );
    }
    if ( part->pixmap() ) {
        test->setPixmap( *(part->pixmap()) );
    }
    test->setProgress( status );
    m_layout->insertWidget(  m_layoutFillIndex , test, 0, AlignTop );
    m_layoutFillIndex++;
    test->show();
}

void Widget::addError( const UDI&, const Error& prog) {
    m_edit->append( "<b>"+ QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::addError( ManipulatorPart*, const Error& prog) {
    m_edit->append( "<b>"+ QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}
void Widget::startSync() {
    m_edit->append("Starting to sync now");
}
void Widget::cleanView() {
    m_messageList.clear();
}

#include "overviewwidget.moc"
