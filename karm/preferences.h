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
    QString iCalFile() const;
    QString activeCalendarFile() const;
    bool autoSave() const;
    int autoSavePeriod() const;
    bool promptDelete() const;
    bool displayColumn(int n) const;

    void emitSignals();

  public slots:
    void showDialog();
    void load();
    void save();

  signals:
    void detectIdleness(bool on);
    void idlenessTimeout(int minutes);
    void iCalFile(QString);
    void autoSave(bool on);
    void autoSavePeriod(int minutes);
    void setupChanged();
  
  protected slots:
    virtual void slotOk();
    virtual void slotCancel();
    void idleDetectCheckBoxChanged();
    void autoSaveCheckBoxChanged();

  private:
    void makeDisplayPage();
    void makeBehaviorPage();
    void makeStoragePage();
  
    Preferences();
    static Preferences *_instance;
    bool _unsavedChanges;

    // Widgets
    QCheckBox *_doIdleDetectionW, *_doAutoSaveW, *_promptDeleteW; 
    QCheckBox *_displayTimeW, *_displaySessionW,
              *_displayTotalTimeW, *_displayTotalSessionW;
    QLabel    *_idleDetectLabelW, *_displayColumnsLabelW;
    QSpinBox  *_idleDetectValueW, *_autoSaveValueW;
    KURLRequester *_iCalFileW ;
  
    // Values
    bool _doIdleDetectionV, _doAutoSaveV, _promptDeleteV;
    bool _displayColumnV[4];
    int  _idleDetectValueV, _autoSaveValueV;
    QString _iCalFileV;
};

#endif // KARM_PREFERENCES_H

