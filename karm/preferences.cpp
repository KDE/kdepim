#undef Unsorted // for --enable-final
#include <qcheckbox.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qvbox.h>

#include <kapplication.h>       // kapp
#include <kconfig.h>
#include <kglobal.h>
#include <klineedit.h>          // lineEdit()
#include <klocale.h>            // i18n
#include <kstandarddirs.h>
#include <kurlrequester.h>

#include "preferences.h"

Preferences *Preferences::_instance = 0;

Preferences::Preferences()
  : KDialogBase( KDialogBase::Tabbed, i18n("Preferences"),
                 KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok )
{
  QVBox* tab = addVBoxPage( i18n("General") );

  _doAutoSaveW      = new QCheckBox( i18n("Automatically save tasks"),
                                     tab, "_doAutoSaveW" );
  {
    QHBox* hbox     = new QHBox( tab );
    _saveFileLabelW = new QLabel( i18n("File to save time information to:"),
                                  hbox, "save label" );
    _saveFileW      = new KURLRequester( hbox, "_saveFileW" );
  }
  {
    QHBox* hbox     = new QHBox(tab);
    _autoSaveLabelW = new QLabel( i18n("Minutes between each auto save:" ),
                                  hbox, "_autoSaveLabelW");
    _autoSaveValueW = new QSpinBox( 1, 60*24, 1, hbox, "_autoSaveValueW" );
  }

  _doTimeLoggingW = new QCheckBox( i18n("Do time logging"),
                                   tab, "_doTimeLoggingW" );
  {
    QHBox* hbox       = new QHBox(tab);
    _timeLoggingLabelW = new QLabel( i18n("File to log the times to:"),
                                     hbox, "save label");
    _timeLogW         = new KURLRequester( hbox, "_timeLogW" );
  }
  {
    QHBox* hbox   = new QHBox(tab);
    _hideOnCloseW = new QCheckBox ( i18n("Hide taskbar icon and "
                                         "application instead of quitting"),
                                    hbox, "_hideOnCloseW");
  }

  _doIdleDetectionW = new QCheckBox( i18n("Try to detect idleness"),
                                     tab,"_doIdleDetectionW");
  {
    QHBox* hbox       = new QHBox(tab);
    _idleDetectLabelW = new QLabel( i18n("Minutes before informing about "
                                         "idleness:"), hbox);
    _idleDetectValueW = new QSpinBox(1,60*24, 1, hbox, "_idleDetectValueW");
  }
  {
    QHBox* hbox = new QHBox(tab);
    _promptDeleteW = new QCheckBox( i18n( "Prompt before deleting tasks" ),
                                    hbox, "_promptDeleteW" );
    }
  {
    QVBox* vbox                =  new QVBox(tab);
    _displayColumnsLabelW      = new QLabel( i18n("The following columns should"
                                                  " be displayed:"), vbox);
    _displaySessionW           = new QCheckBox ( i18n("Session time"),
                                                 vbox, "_displaySessionW");
    _displayTimeW              = new QCheckBox ( i18n("Cumulated task time"),
                                               vbox, "_displayTimeW");
    _displayTotalSessionW      = new QCheckBox( i18n("Total session time"),
                                                vbox, "_displayTotalSessionW");
    _displayTotalTimeW         = new QCheckBox ( i18n("Total task time"),
                                                 vbox, "_displayTotalTimeW");
  }

    connect( _doAutoSaveW, SIGNAL( clicked() ), this,
             SLOT( autoSaveCheckBoxChanged() ));
    connect( _doTimeLoggingW, SIGNAL( clicked() ), this,
             SLOT( timeLoggingCheckBoxChanged() ));
    connect( _hideOnCloseW, SIGNAL( clicked() ), this,
             SLOT(hideOnCloseCheckBoxChanged() ));
    connect( _doIdleDetectionW, SIGNAL( clicked() ), this,
             SLOT( idleDetectCheckBoxChanged() ));
}

Preferences *Preferences::instance()
{
  if (_instance == 0) {
    _instance = new Preferences();
  }
  return _instance;
}


void Preferences::disableIdleDetection()
{
  _doIdleDetectionW->setEnabled(false);
  _idleDetectLabelW->setEnabled(false);
}


//---------------------------------------------------------------------------
//                            SLOTS
//---------------------------------------------------------------------------

void Preferences::showDialog()
{

  // set all widgets
  _saveFileW->lineEdit()->setText(_saveFileV);

  _doTimeLoggingW->setChecked(_doTimeLoggingV);
  _timeLogW->lineEdit()->setText(_timeLogV);

  _doIdleDetectionW->setChecked(_doIdleDetectionV);
  _idleDetectValueW->setValue(_idleDetectValueV);

  _doAutoSaveW->setChecked(_doAutoSaveV);
  _autoSaveValueW->setValue(_autoSaveValueV);

  _hideOnCloseW->setChecked(_hideOnCloseV);
  _promptDeleteW->setChecked(_promptDeleteV);

  _displaySessionW->setChecked(_displayColumnV[0]);
  _displayTimeW->setChecked(_displayColumnV[1]);
  _displayTotalSessionW->setChecked(_displayColumnV[2]);
  _displayTotalTimeW->setChecked(_displayColumnV[3]);

  // adapt visibility of preference items according
  // to settings
  idleDetectCheckBoxChanged();
  timeLoggingCheckBoxChanged();
  hideOnCloseCheckBoxChanged();

  show();
}

void Preferences::slotOk()
{
  _saveFileV = _saveFileW->lineEdit()->text();
  _timeLogV = _timeLogW->lineEdit()->text();
  _doTimeLoggingV    = _doTimeLoggingW->isChecked();
  _doIdleDetectionV = _doIdleDetectionW->isChecked();
  _idleDetectValueV = _idleDetectValueW->value();
  _doAutoSaveV    = _doAutoSaveW->isChecked();
  _autoSaveValueV = _autoSaveValueW->value();
  _hideOnCloseV = _hideOnCloseW->isChecked();
  _promptDeleteV = _promptDeleteW->isChecked();

  _displayColumnV[0] = _displaySessionW->isChecked();
  _displayColumnV[1]=_displayTimeW->isChecked();
  _displayColumnV[2]=_displayTotalSessionW->isChecked();
  _displayColumnV[3]=_displayTotalTimeW->isChecked();

  emitSignals();
  save();
  KDialogBase::slotOk();
}

void Preferences::slotCancel()
{
  KDialogBase::slotCancel();
}

void Preferences::idleDetectCheckBoxChanged()
{
  bool enabled = _doIdleDetectionW->isChecked();
  _idleDetectLabelW->setEnabled(enabled);
  _idleDetectValueW->setEnabled(enabled);
}

void Preferences::autoSaveCheckBoxChanged()
{
  bool enabled = _doAutoSaveW->isChecked();
  _autoSaveLabelW->setEnabled(enabled);
  _autoSaveValueW->setEnabled(enabled);
  _saveFileLabelW->setEnabled(enabled);
  _saveFileW->setEnabled(enabled);
}

void Preferences::timeLoggingCheckBoxChanged()
{
  bool enabled = _doTimeLoggingW->isChecked();
  _timeLoggingLabelW->setEnabled(enabled);
  _timeLogW->setEnabled(enabled);
}

void Preferences::hideOnCloseCheckBoxChanged()
{
}

void Preferences::emitSignals()
{
  emit saveFile( _saveFileV );
  emit timeLogging( _doTimeLoggingV );
  emit timeLog( _timeLogV );
  emit detectIdleness( _doIdleDetectionV );
  emit idlenessTimeout( _idleDetectValueV );
  emit autoSave( _doAutoSaveV );
  emit autoSavePeriod( _autoSaveValueV );
  emit setupChanged(  );
  emit hideOnClose( _hideOnCloseV );
}

QString Preferences::saveFile()
{
  return _saveFileV;
}

QString Preferences::activeCalendarFile()
{
  KStandardDirs dirs;
  QString korganizerrc = locateLocal( "config",
                                      QString::fromLatin1("korganizerrc") );
  KConfig korgconfig( korganizerrc, true );
  korgconfig.setGroup( "General" );

  return korgconfig.readEntry( "Active Calendar" ).section( ':', 1 );
}

QString Preferences::loadFile()
{
  if ( useLegacyFileFormat() )
    return _legacySaveFileV;
  else
    return _saveFileV;
}

QString Preferences::timeLog()
{
  return _timeLogV;
}

bool Preferences::detectIdleness()
{
  return _doIdleDetectionV;
}

int Preferences::idlenessTimeout()
{
  return _idleDetectValueV;
}

bool Preferences::autoSave()
{
  return _doAutoSaveV;
}

bool Preferences::timeLogging() 
{
  return _doTimeLoggingV;
}

int Preferences::autoSavePeriod()
{
  return _autoSaveValueV;
}

bool Preferences::hideOnClose()
{
    return _hideOnCloseV;
}

bool Preferences::promptDelete()
{
    return _promptDeleteV;
}

bool Preferences::useLegacyFileFormat()
{
  return fileFormat() == QString::fromLatin1( "karmdata" );
}

bool Preferences::displayColumn(int n)  { return _displayColumnV[n]; }

QString Preferences::fileFormat()
{
  return _fileFormat;
}

//---------------------------------------------------------------------------
//                                  Load and Save
//---------------------------------------------------------------------------
void Preferences::load()
{
  KConfig &config = *kapp->config();

  config.setGroup( QString::fromLatin1("Idle detection") );
  _doIdleDetectionV = config.readBoolEntry( QString::fromLatin1("enabled"),
                                            true );
  _idleDetectValueV = config.readNumEntry(QString::fromLatin1("period"), 15);

  config.setGroup( QString::fromLatin1("Saving") );
  _fileFormat     = config.readEntry( QString::fromLatin1("file format"),
                                      QString::fromLatin1("karmdata"));
  _saveFileV      = config.readEntry( QString::fromLatin1("kcal file"),
                                      locateLocal( "appdata",
                                                   QString::fromLatin1( "karmdata.ics")));
  _legacySaveFileV = config.readEntry( QString::fromLatin1("file"),
                                      locateLocal( "appdata",
                                                   QString::fromLatin1("karmdata.txt")));
  _doTimeLoggingV  = config.readBoolEntry( QString::fromLatin1("time logging"), false);
  _timeLogV       = config.readEntry( QString::fromLatin1("time log file"),
                                      locateLocal( "appdata",
                                                   QString::fromLatin1("karmlog.txt")));
  _doAutoSaveV    = config.readBoolEntry( QString::fromLatin1("auto save"), true);
  _autoSaveValueV = config.readNumEntry( QString::fromLatin1("auto save period"), 5);
  _hideOnCloseV   = config.readBoolEntry( QString::fromLatin1("hide on close"), true);
  _promptDeleteV  = config.readBoolEntry( QString::fromLatin1("prompt delete"), true);

  _displayColumnV[0] = config.readBoolEntry( QString::fromLatin1("display session time"), true);
  _displayColumnV[1] = config.readBoolEntry( QString::fromLatin1("display time"), true);
  _displayColumnV[2] = config.readBoolEntry( QString::fromLatin1("display total session time"), true);
  _displayColumnV[3] = config.readBoolEntry( QString::fromLatin1("display total time"), true);

  emitSignals();
}

void Preferences::save()
{
  KConfig &config = *KGlobal::config();

  config.setGroup( QString::fromLatin1("Idle detection"));
  config.writeEntry( QString::fromLatin1("enabled"), _doIdleDetectionV);
  config.writeEntry( QString::fromLatin1("period"), _idleDetectValueV);

  config.setGroup( QString::fromLatin1("Saving"));
  config.writeEntry( QString::fromLatin1("file format"), QString::fromLatin1("karm_kcal_2"));
  config.writeEntry( QString::fromLatin1("file"), _legacySaveFileV);
  config.writeEntry( QString::fromLatin1("kcal file"), _saveFileV);
  config.writeEntry( QString::fromLatin1("time logging"), _doTimeLoggingV);
  config.writeEntry( QString::fromLatin1("time log file"), _timeLogV);
  config.writeEntry( QString::fromLatin1("auto save"), _doAutoSaveV);
  config.writeEntry( QString::fromLatin1("auto save period"), _autoSaveValueV);
  config.writeEntry( QString::fromLatin1("hide on close"), _hideOnCloseV);
  config.writeEntry( QString::fromLatin1("prompt delete"), _promptDeleteV);

  config.writeEntry( QString::fromLatin1("display session time"), _displayColumnV[0]);
  config.writeEntry( QString::fromLatin1("display time"), _displayColumnV[1]);
  config.writeEntry( QString::fromLatin1("display total session time"), _displayColumnV[2]);
  config.writeEntry( QString::fromLatin1("display total time"), _displayColumnV[3]);

  config.sync();

}

#include "preferences.moc"
