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
    bool detectIdleness();
    int idlenessTimeout();
    QString loadFile();
    QString saveFile();
    QString activeCalendarFile();
    bool timeLogging();
    QString timeLog();
    bool autoSave();
    int autoSavePeriod();
    bool hideOnClose();
    bool promptDelete();
    QString fileFormat();
    bool useLegacyFileFormat();
    bool displayColumn(int n);
    void emitSignals();

  public slots:
    void showDialog();
    void load();
    void save();

  signals:
    void detectIdleness(bool on);
    void idlenessTimeout(int minutes);
    void saveFile(QString);
    void timeLogging(bool on);
    void timeLog(QString);
    void autoSave(bool on);
    void autoSavePeriod(int minutes);
    void setupChanged();
    void hideOnClose(bool on);
  
  protected slots:
    virtual void slotOk();
    virtual void slotCancel();
    void idleDetectCheckBoxChanged();
    void autoSaveCheckBoxChanged();
    void timeLoggingCheckBoxChanged();
    void hideOnCloseCheckBoxChanged();

  private:
    Preferences();
    static Preferences *_instance;
    bool _unsavedChanges;

    // Widgets in the dialog
    // (All variables ends in W to indicate that they are Widgets)
    QCheckBox *_doIdleDetectionW, *_doAutoSaveW, *_doTimeLoggingW,
              *_hideOnCloseW, *_promptDeleteW;
    QCheckBox *_displayTimeW, *_displaySessionW,
              *_displayTotalTimeW, *_displayTotalSessionW;
    QLabel    *_idleDetectLabelW, *_autoSaveLabelW, *_saveFileLabelW,
              *_timeLoggingLabelW, *_displayColumnsLabelW;
    QSpinBox  *_idleDetectValueW, *_autoSaveValueW;
    KURLRequester *_saveFileW, *_timeLogW;
  
    // Values for the preferences.
    // (All variables in in V to indicate they are Values)
    bool _doIdleDetectionV, _doAutoSaveV, _doTimeLoggingV,
         _hideOnCloseV, _promptDeleteV;
    /// should columns be displayed
    bool _displayColumnV[4];
    int  _idleDetectValueV, _autoSaveValueV;
    QString _saveFileV, _legacySaveFileV, _timeLogV;
    QString _fileFormat;
};

#endif // KARM_PREFERENCES_H

