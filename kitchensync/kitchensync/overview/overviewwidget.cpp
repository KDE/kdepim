#include <qdatetime.h>
#include <qsplitter.h>
#include <qtextedit.h>
#include <qvbox.h>

#include <kconfig.h>
#include <kdialog.h>
#include <klocale.h>

#include <actionpart.h>
#include <error.h>
#include <progress.h>

#include "overviewwidget.h"

using namespace KSync;
using namespace KSync::OverView;

Widget::Widget( QWidget* parent, const char* name )
  : QWidget( parent, name )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setMargin( KDialog::marginHint() );

  QHBox* info = new QHBox( this );
  info->setSpacing( 10 );
  info->setMargin( 10 );

  QVBox *info2 = new QVBox(info);
  m_device = new QLabel( info2 );
  m_profile= new QLabel( info2 );
  info->setStretchFactor( info2, 5 );

  m_logo = new QLabel( info );

  m_split = new QSplitter( this );
  m_edit = new QTextEdit( m_split );
  m_edit->setReadOnly( true );
  m_edit->setTextFormat( Qt::LogText );
  m_ab = new QWidget( m_split );

  KConfig config( "kitchensyncrc" );
  config.setGroup( "OverviewPart" );

  QValueList<int> sizes = config.readIntListEntry( "SplitterSize" );
  if ( sizes.isEmpty() ) {
    sizes.append( width() / 2 );
    sizes.append( width() / 2 );
  }

  m_split->setSizes( sizes );

  m_layout = new QVBoxLayout( m_ab );
  m_layout->insertStretch( -1, 5 );
  m_layoutFillIndex = 0;

  layout->addWidget( info );
  layout->addWidget( m_split, 100 );

  m_messageList.setAutoDelete( true );
}

Widget::~Widget()
{
  KConfig config( "kitchensyncrc" );
  config.setGroup( "OverviewPart" );

  config.writeEntry( "SplitterSize", m_split->sizes() );
}

void Widget::setProfile( const Profile& prof )
{
  m_profile->setText( "<qt><b>" + i18n( " Profile: " ) + "</b>" + prof.name() + "</qt>" );
  cleanView();
}

void Widget::setProfile( const QString& name, const QPixmap& pix )
{
  m_device->setText( "<qt><b>"+ i18n( " Device: " ) + "</b>" + name + "</qt>" );
  m_logo->setPixmap( pix );
  cleanView();
}

void Widget::addProgress( Konnector *, const Progress& prog )
{
  m_edit->append( "<b>" + QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}

void Widget::addProgress( ActionPart *, const Progress& prog )
{
  m_edit->append( "<b>" + QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}

void Widget::syncProgress( ActionPart * part, int status, int )
{
  OverViewProgressEntry* it;
  for ( it = m_messageList.first(); it; it = m_messageList.next() ) {
    if ( QString::compare( it->name(), part->name() ) == 0 ) {
      it->setProgress( status );
      return;
    }
  }

  OverViewProgressEntry* test = new OverViewProgressEntry( m_ab, "test" );
  m_messageList.append( test );

  if ( !part->title().isEmpty() )  {
    test->setText( part->title() );
  }

  if ( part->pixmap() ) {
    test->setPixmap( *(part->pixmap()) );
  }

  test->setProgress( status );
  m_layout->insertWidget( m_layoutFillIndex , test, 0, AlignTop );
  m_layoutFillIndex++;
  test->show();
}

void Widget::addError( Konnector *, const Error& prog )
{
  m_edit->append( "<b>"+ QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}

void Widget::addError( ActionPart *, const Error& prog )
{
  m_edit->append( "<b>"+ QDateTime::currentDateTime().toString() + "</b> " + prog.text() );
}

void Widget::startSync()
{
  m_edit->append( "Starting to sync now" );
}

void Widget::cleanView()
{
  m_messageList.clear();
}

#include "overviewwidget.moc"
