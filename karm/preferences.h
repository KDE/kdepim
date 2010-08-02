#ifndef KARM_PREFERENCES_H
#define KARM_PREFERENCES_H

#include <kdialogbase.h>

class TQCheckBox;
class TQLabel;
class TQSpinBox;
class TQString;
class KURLRequester;

/**
 * Provide an interface to the configuration options for the program.
 */

class Preferences :public KDialogBase
{
  Q_OBJECT

  public:
    static Preferences *instance( const TQString& icsfile = "" );
    void disableIdleDetection();

    // Retrive information about settings
    bool detectIdleness() const;
    int idlenessTimeout() const;
    TQString iCalFile() const;
    TQString activeCalendarFile() const;
    bool autoSave() const;
    bool logging() const;
    int autoSavePeriod() const;
    bool promptDelete() const;
    TQString setPromptDelete( bool prompt );
    bool displayColumn(int n) const;
    TQString userRealName() const;

    void emitSignals();
    bool readBoolEntry( const TQString& uid );
    void writeEntry( const TQString &key, bool value );
    void deleteEntry( const TQString &key );

  public slots:
    void showDialog();
    void load();
    void save();

  signals:
    void detectIdleness(bool on);
    void idlenessTimeout(int minutes);
    void iCalFile(TQString);
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

    Preferences( const TQString& icsfile = "" );
    static Preferences *_instance;
    bool _unsavedChanges;

    // Widgets
    TQCheckBox *_doIdleDetectionW, *_doAutoSaveW, *_promptDeleteW;
    TQCheckBox *_displayTimeW, *_displaySessionW,
              *_displayTotalTimeW, *_displayTotalSessionW;
    TQCheckBox *_loggingW;
    TQLabel    *_idleDetectLabelW, *_displayColumnsLabelW;
    TQSpinBox  *_idleDetectValueW, *_autoSaveValueW;
    KURLRequester *_iCalFileW ;

    // Values
    bool _doIdleDetectionV, _doAutoSaveV, _promptDeleteV, _loggingV;
    bool _displayColumnV[4];
    int  _idleDetectValueV, _autoSaveValueV;
    TQString _iCalFileV;

    /** real name of the user, used during ICAL saving */
    TQString _userRealName;
};

#endif // KARM_PREFERENCES_H

