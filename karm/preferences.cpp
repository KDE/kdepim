#undef Unsorted // for --enable-final
#include "preferences.h"
#include <qcheckbox.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qspinbox.h>
#include <klocale.h>
#include <qvbox.h>
#include <qframe.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>

Preferences *Preferences::_instance = 0;

Preferences::Preferences() : KDialogBase(KDialogBase::Tabbed, i18n("Preferences"),
                                         KDialogBase::Ok | KDialogBase::Cancel,
                                         KDialogBase::Ok)
{
    QVBox* tab = addVBoxPage(i18n("General"));

    _doAutoSaveW = new QCheckBox(i18n("Automatically save tasks"), tab, "_doAutoSaveW");
    {
        QHBox* hbox     = new QHBox(tab);
        _saveFileLabelW = new QLabel(i18n("File to save time information to:"), hbox, "save label");
        _saveFileW      = new KURLRequester(hbox, "_saveFileW");
    }
    {
        QHBox* hbox     = new QHBox(tab);
        _autoSaveLabelW = new QLabel(i18n("Minutes between each auto save:"), hbox, "_autoSaveLabelW");
        _autoSaveValueW = new QSpinBox(1, 60*24, 1, hbox, "_autoSaveValueW");
    }

    _doTimeLogingW = new QCheckBox(i18n("Do time logging"), tab, "_doTimeLogingW");
    {
        QHBox* hbox       = new QHBox(tab);
        _timeLogingLabelW = new QLabel(i18n("File to log the times to:"), hbox, "save label");
        _timeLogW         = new KURLRequester(hbox, "_timeLogW");
    }

    {
        QHBox* hbox   = new QHBox(tab);
        _hideOnCloseW = new QCheckBox ( i18n("Hide taskbar icon and application instead of quitting"),
                                        hbox, "_hideOnCloseW");
    }

    _doIdleDetectionW = new QCheckBox(i18n("Try to detect idleness"), tab,"_doIdleDetectionW");
    {
        QHBox* hbox       = new QHBox(tab);
        _idleDetectLabelW = new QLabel(i18n("Minutes before informing about idleness:"), hbox);
        _idleDetectValueW = new QSpinBox(1,60*24, 1, hbox, "_idleDetectValueW");
    }

    {
        QHBox* hbox = new QHBox(tab);
        _promptDeleteW = new QCheckBox( i18n( "Prompt before deleting tasks" ), hbox, "_promptDeleteW" );
    }

    connect(_doAutoSaveW, SIGNAL(clicked()), this, SLOT(autoSaveCheckBoxChanged()));
    connect(_doTimeLogingW, SIGNAL(clicked()), this, SLOT(timeLogingCheckBoxChanged()));
    connect(_hideOnCloseW, SIGNAL(clicked()), this, SLOT(hideOnCloseCheckBoxChanged()));
    connect(_doIdleDetectionW, SIGNAL(clicked()), this, SLOT(idleDetectCheckBoxChanged()));
}

#if 0
Preferences::Preferences() : KDialogBase(KDialogBase::Tabbed, i18n("Preferences"),
                                         KDialogBase::Ok | KDialogBase::Cancel,
                                         KDialogBase::Ok)
{
  //----------------------------------------------------------------------
  // Saving
  //----------------------------------------------------------------------
  QVBox *autoSaveMenu = addVBoxPage(i18n("Saving"));

  _doAutoSaveW = new QCheckBox(i18n("Automatically save tasks"), autoSaveMenu, "_doAutoSaveW");
  QHBox *box3 = new QHBox(autoSaveMenu);
  _saveFileLabelW = new QLabel(i18n("File to save time information to:"), box3, "save label");
  _saveFileW = new KURLRequester(box3, "_saveFileW");
  QHBox *box2 = new QHBox(autoSaveMenu);
  _autoSaveLabelW = new QLabel(i18n("Minutes between each auto save:"), box2, "_autoSaveLabelW");
  _autoSaveValueW = new QSpinBox(1, 60*24, 1, box2, "_autoSaveValueW");
  connect(_doAutoSaveW, SIGNAL(clicked()), this, SLOT(autoSaveCheckBoxChanged()));

   _doTimeLogingW = new QCheckBox(i18n("Do time logging"), autoSaveMenu, "_doTimeLogingW");
   connect(_doTimeLogingW, SIGNAL(clicked()), this, SLOT(timeLogingCheckBoxChanged()));
   QHBox *box4 = new QHBox(autoSaveMenu);
   _timeLogingLabelW = new QLabel(i18n("File to log the times to:"), box4, "save label");
   _timeLogW = new KURLRequester(box4, "_timeLogW");

  QHBox *box5 = new QHBox(autoSaveMenu);
  _hideOnCloseW = new QCheckBox ( i18n("Hide taskbar icon and application instead of quitting"),
                                  box5, "_hideOnCloseW");
  connect(_hideOnCloseW, SIGNAL(clicked()), this, SLOT(hideOnCloseCheckBoxChanged()));

  //----------------------------------------------------------------------
  // Idle Detection Setup
  //----------------------------------------------------------------------
  idleMenu = addVBoxPage(i18n("Idle Detection"));

  _doIdleDetectionW = new QCheckBox(i18n("Try to detect idleness"),
                                   idleMenu,"_doIdleDetectionW");
  connect(_doIdleDetectionW, SIGNAL(clicked()),
          this, SLOT(idleDetectCheckBoxChanged()));

  QHBox *box1 = new QHBox(idleMenu);
  _idleDetectLabelW = new QLabel(i18n("Minutes before informing about idleness:"), box1);
  _idleDetectValueW = new QSpinBox(1,60*24, 1, box1, "_idleDetectValueW");
}
#endif

Preferences *Preferences::instance()
{
  if (_instance == 0) {
    _instance = new Preferences();
  }
  return _instance;
}


void Preferences::disableIdleDetection()
{
  // idleMenu->setEnabled(false);
  _doIdleDetectionW->setEnabled(false);
  _idleDetectLabelW->setEnabled(false);
}


//--------------------------------------------------------------------------------
//                            SLOTS
//--------------------------------------------------------------------------------

void Preferences::showDialog()
{

  // set all widgets
  _saveFileW->lineEdit()->setText(_saveFileV);

  _doTimeLogingW->setChecked(_doTimeLogingV);
  _timeLogW->lineEdit()->setText(_timeLogV);

  _doIdleDetectionW->setChecked(_doIdleDetectionV);
  _idleDetectValueW->setValue(_idleDetectValueV);

  _doAutoSaveW->setChecked(_doAutoSaveV);
  _autoSaveValueW->setValue(_autoSaveValueV);

  _hideOnCloseW->setChecked(_hideOnCloseV);
  _promptDeleteW->setChecked(_promptDeleteV);

  idleDetectCheckBoxChanged();
  timeLogingCheckBoxChanged();
  hideOnCloseCheckBoxChanged();

  show();
}

void Preferences::slotOk()
{
  _saveFileV = _saveFileW->lineEdit()->text();
  _timeLogV = _timeLogW->lineEdit()->text();
  _doTimeLogingV    = _doTimeLogingW->isChecked();
  _doIdleDetectionV = _doIdleDetectionW->isChecked();
  _idleDetectValueV = _idleDetectValueW->value();
  _doAutoSaveV    = _doAutoSaveW->isChecked();
  _autoSaveValueV = _autoSaveValueW->value();
  _hideOnCloseV = _hideOnCloseW->isChecked();
  _promptDeleteV = _promptDeleteW->isChecked();

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

void Preferences::timeLogingCheckBoxChanged()
{
  bool enabled = _doTimeLogingW->isChecked();
  _timeLogingLabelW->setEnabled(enabled);
  _timeLogW->setEnabled(enabled);
}

void Preferences::hideOnCloseCheckBoxChanged()
{
}

void Preferences::emitSignals()
{
  emit(saveFile(_saveFileV));
  emit(timeLoging(_doTimeLogingV));
  emit(timeLog(_timeLogV));
  emit(detectIdleness(_doIdleDetectionV));
  emit(idlenessTimeout(_idleDetectValueV));
  emit(autoSave(_doAutoSaveV));
  emit(autoSavePeriod(_autoSaveValueV));
  emit(setupChanged());
  emit(hideOnClose(_hideOnCloseV));
}

QString Preferences::saveFile()
{
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

bool Preferences::timeLoging() 
{
  return _doTimeLogingV;
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

//--------------------------------------------------------------------------------
//                                  Load and Save
//--------------------------------------------------------------------------------
void Preferences::load()
{
  KConfig &config = *kapp->config();

  config.setGroup( QString::fromLatin1("Idle detection") );
  _doIdleDetectionV = config.readBoolEntry(QString::fromLatin1("enabled"), true );
  _idleDetectValueV = config.readNumEntry(QString::fromLatin1("period"), 15);

  config.setGroup( QString::fromLatin1("Saving") );
  _saveFileV      = config.readEntry( QString::fromLatin1("file"),
                                      locateLocal( "appdata", QString::fromLatin1("karmdata.txt")));
  _doTimeLogingV  = config.readBoolEntry( QString::fromLatin1("time loging"), false);
  _timeLogV       = config.readEntry( QString::fromLatin1("time log file"),
                                      locateLocal("appdata", QString::fromLatin1("karmlog.txt")));
  _doAutoSaveV    = config.readBoolEntry( QString::fromLatin1("auto save"), true);
  _autoSaveValueV = config.readNumEntry( QString::fromLatin1("auto save period"), 5);
  _hideOnCloseV   = config.readBoolEntry( QString::fromLatin1("hide on close"), true);
  _promptDeleteV  = config.readBoolEntry( QString::fromLatin1("prompt delete"), true);

  emitSignals();
}

void Preferences::save()
{
  KConfig &config = *KGlobal::config();

  config.setGroup( QString::fromLatin1("Idle detection"));
  config.writeEntry( QString::fromLatin1("enabled"), _doIdleDetectionV);
  config.writeEntry( QString::fromLatin1("period"), _idleDetectValueV);

  config.setGroup( QString::fromLatin1("Saving"));
  config.writeEntry( QString::fromLatin1("file"), _saveFileV);
  config.writeEntry( QString::fromLatin1("time loging"), _doTimeLogingV);
  config.writeEntry( QString::fromLatin1("time log file"), _timeLogV);
  config.writeEntry( QString::fromLatin1("auto save"), _doAutoSaveV);
  config.writeEntry( QString::fromLatin1("auto save period"), _autoSaveValueV);
  config.writeEntry( QString::fromLatin1("hide on close"), _hideOnCloseV);
  config.writeEntry( QString::fromLatin1("prompt delete"), _promptDeleteV);

  config.sync();

}

#include "preferences.moc"
