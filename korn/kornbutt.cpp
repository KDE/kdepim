
#include<assert.h>

#include<qdatetime.h>
#include<qfile.h>
#include<qpixmap.h>
#include<qptrlist.h>
#include <qstring.h>
#include<qtooltip.h>

#include<kaudioplayer.h>
#include<kdebug.h>
#include<kpassivepopup.h>
#include<kprocess.h>
#include<klocale.h>

#include"kornbutt.h"
#include"mailsubject.h"
#include"kornshell.h"
#include"maildrop.h"
#include"btnstyle.h"

KornButton::KornButton( QWidget *parent, KMailDrop *box, KornShell *shell )
	: QToolButton(parent, "kornbutton"),
	_box(box),
	_shell(shell),
	_lastNum(0),
	_style( 0 )
{
	assert(box);

	connect(_box, SIGNAL(changed(int)),       this, SLOT(setNumber(int)));
	connect(_box, SIGNAL(notifyDisconnect()), this, SLOT(disconnectMonitor()));
	connect(_box, SIGNAL(configChanged()),    this, SLOT(monitorUpdated()));
	connect(_box, SIGNAL(showPassivePopup(QPtrList<KornMailSubject>*,int)),
	        this, SLOT  (showPassivePopup(QPtrList<KornMailSubject>*,int)));

	switch( _box->displayStyle() ){
		case KMailDrop::Colour: 
			_style = new KornBtnColourTextStyle( this, _box );
			break;
		case KMailDrop::Icon: 
			_style = new KornBtnIconStyle( this, _box );
			break;
		case KMailDrop::Plain: 
		default:
			_style = new KornBtnColourTextStyle( this, _box );
			break;
	}
	_style->enable();
	setAutoRaise(true);

	monitorUpdated();

	_box->startMonitor();
}

KornButton::~KornButton()
{
	if( _style)
		delete _style; _style = 0;
}

void KornButton::setNumber( int )
{
	if (0 == _box)
		return;

	if (_box->count() != _lastNum)
		setText(QString::number(_box->count()));
		
	bool newMessages = _box->count() > _lastNum;
	_lastNum = _box->count();

	_style->update( _box->count(), newMessages );

	if (newMessages && !_box->soundFile().isEmpty() && QFile::exists(_box->soundFile()))
		KAudioPlayer::play(_box->soundFile());

	if (newMessages && !_box->newMailCmd().isEmpty())
		executeCmd(_box->newMailCmd());
}

void KornButton::runCommand(bool onlyIfUnread)
{
  kdDebug() << "KornButton::runCommand(" << (onlyIfUnread ? "true" : "false") << ")" << endl;
  if (0 == _box) {
	  kdDebug() << "KornButton::runCommand: button not connected to a box?"
		  	<< endl;
    return;
  }

  if (_box->resetCounter()>=0)
  	_box->setResetCounter(_box->count()+_box->resetCounter()); //Set counter to 0.

  if (onlyIfUnread) {

	  kdDebug( ) << "calling box check" << endl;
    _box->recheck();

    if (0 == _box->count())
      return;
  }

  kdDebug() << "KornButton::runCommand: clickCmd == `" << _box->clickCmd() << "'" << endl;

	if (!_box->clickCmd().isEmpty())
		executeCmd(_box->clickCmd());
}

void KornButton::executeCmd(QString cmd) {
	KProcess* proc = new KProcess;
	proc->setUseShell(true);
	*proc << cmd;
	connect(proc, SIGNAL(processExited(KProcess*)),
	        this, SLOT(commandRun(KProcess*)));
	proc->start();
}

void KornButton::commandRun(KProcess* proc) {
	delete proc;
}

void KornButton::disconnectMonitor()
{
	delete _style; _style = 0;
  _box->stopMonitor();
  _box = 0;

  emit dying(this);
}

void KornButton::monitorUpdated()
{
  kdDebug() << "KornButton::monitorUpdated()" << endl;
  kdDebug() << "count = " << _box->count() << endl;

  if (0 == _box)
    return;

  bool newMessages = _box->count() > _lastNum;
  _lastNum = _box->count();

  QToolTip::remove(this);
  QToolTip::add(this, _box->caption());

  delete _style; _style = 0;

  switch( _box->displayStyle() ){
	  case KMailDrop::Colour: 
		  _style = new KornBtnColourTextStyle( this, _box );
		  break;
	  case KMailDrop::Icon: 
		  _style = new KornBtnIconStyle( this, _box );
		  break;
	  case KMailDrop::Plain: 
	  default:
		  _style = new KornBtnColourTextStyle( this, _box );
		  break;
  }
  _style->enable();
  _style->update( _box->count(), newMessages );
}

  void
KornButton::mousePressEvent(QMouseEvent * e)
{
  if (e->button() == RightButton) {
    emit(rightClick());
  }
}
  void
KornButton::mouseReleaseEvent(QMouseEvent * e)
{
  switch (e->button()) {
    case LeftButton:  runCommand( true );	break;
    case MidButton:   runCommand( false );	break;
    default:	break;
  }
}

  void
KornButton::popupMenu()
{
  // call shell (which created this) tho handle event.
  _shell->popup(this);
}

void KornButton::showPassivePopup( QPtrList< KornMailSubject > *new_subjects, int total_new_emails )
{
  QDateTime date;
  QString message;
  if( _box->passiveDate() )
    message = i18n( "From  Subject  Data" ); //Information above a passive popup
  else	
    message = i18n( "From  Subject" );
  message+= "\n----------------------";
  for( QPtrList<KornMailSubject>::ConstIterator it = new_subjects->begin(); it != new_subjects->end(); ++it )
  {
    date.setTime_t( (*it)->getDate() );
    message += "\n";
    message += (*it)->getSender();
    message += "  ";
    message += (*it)->getSubject();
    if( _box->passiveDate() )
    {
      message += "  ";
      message += date.toString( );
    }
  }
  if( total_new_emails )
    message += i18n( "\n\n[There are %1 new messages]" ).arg( QString::number( total_new_emails ) );
  
  KPassivePopup::message( QString::QString( "korn: %1" ).arg( _box->caption() ), message, QPixmap(), this, "passive_popup", 5000 );
}

HeadButton::HeadButton( QWidget *parent )
  :
  QToolButton(parent, "headbutton")
{
  setText("Korn");
}

void
HeadButton::mousePressEvent(QMouseEvent * e)
{
  if (e->button() == RightButton) {
    emit(rightClick());
  }
}


#include "kornbutt.moc"
