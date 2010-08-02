#ifndef KARM_K_TIME_WIDGET_H
#define KARM_K_TIME_WIDGET_H

class TQLineEdit;
class TQWidget;

class KarmLineEdit;

/**
 * Widget used for entering minutes and seconds with validation.
 */

class KArmTimeWidget : public TQWidget 
{
  public:
    KArmTimeWidget( TQWidget* parent = 0, const char* name = 0 );
    void setTime( long minutes );
    long time() const;

  private:
    TQLineEdit *_hourLE;
    KarmLineEdit *_minuteLE;
};

#endif // KARM_K_TIME_WIDGET_H
