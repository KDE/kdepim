#ifndef KARM_PREFERENCES_H
#define KARM_PREFERENCES_H

#include <kdialogbase.h>

class QCheckBox;
class QLabel;
class QSpinBox;
class KURLRequester;

/**
 * Provide an interface to the configuration options for the program.
 */

class Preferences :public KDialogBase 
{
  Q_OBJECT

  public:
    static Preferences *instance();
    void disableIdleDetection();

    // Retrive information about settings
    bool detectIdleness() const;
    int idlenessTimeout() const;
    QString flatFile() const;
    QString iCalFile() const;
    QString activeCalendarFile() const;
    bool timeLogging() const;
    bool usingiCalFile() const;
    bool usingFlatFile() const;
    QString timeLog() const;
    bool autoSave() const;
    int autoSavePeriod() const;
    bool promptDelete() const;
    QString fileFormat() const;
    bool displayColumn(int n) const;

    void emitSignals();

  public slots:
    void showDialog();
    void load();
    void save();

  signals:
    void detectIdleness(bool on);
    void idlenessTimeout(int minutes);
    void flatFile(QString);
    void iCalFile(QString);
    void timeLogging(bool on);
    void usingiCalFile(bool on);
    void usingFlatFile(bool on);
    void timeLog(QString);
    void autoSave(bool on);
    void autoSavePeriod(int minutes);
    void setupChanged();
  
  protected slots:
    virtual void slotOk();
    virtual void slotCancel();
    void idleDetectCheckBoxChanged();
    void autoSaveCheckBoxChanged();
    void iCalFileCheckBoxChanged();
    void flatFileCheckBoxChanged();
    void timeLoggingCheckBoxChanged();

  private:
    void makeDisplayPage();
    void makeBehaviorPage();
    void makeStoragePage();
  
    Preferences();
    static Preferences *_instance;
    bool _unsavedChanges;

    // Widgets
    QCheckBox *_doIdleDetectionW, *_doAutoSaveW, *_doTimeLoggingW,
              *_promptDeleteW, *_useFlatFileW, *_useiCalFileW;
    QCheckBox *_displayTimeW, *_displaySessionW,
              *_displayTotalTimeW, *_displayTotalSessionW;
    QLabel    *_idleDetectLabelW, *_displayColumnsLabelW;
    QSpinBox  *_idleDetectValueW, *_autoSaveValueW;
    KURLRequester *_flatFileW, *_logFileW, *_iCalFileW ;
  
    // Values
    bool _doIdleDetectionV, _doAutoSaveV, _doTimeLoggingV,
         _promptDeleteV, _useiCalFileV, _useFlatFileV;
    bool _displayColumnV[4];
    int  _idleDetectValueV, _autoSaveValueV;
    QString _flatFileV, _logFileV, _iCalFileV;
};

#endif // KARM_PREFERENCES_H

