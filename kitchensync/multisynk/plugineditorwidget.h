#ifndef PLUGINEDITORWIDGET_H
#define PLUGINEDITORWIDGET_H

#include <qwidget.h>

class KComboBox;

class QLabel;
class QPushButton;

class PluginEditorWidget : public QWidget
{
  Q_OBJECT

  public:
    PluginEditorWidget( QWidget *parent = 0, const char *name = 0 );
    ~PluginEditorWidget();

    void setLabel( const QString &label );

    void setKonnector( KonnectorPair *pair, KSync::Konnector *konnector );
    KSync::Konnector *konnector() const;

  private slots:
    void typeChanged( int );
    void changeOptions();

  private:
    void initGUI();
    void fillTypeBox();
    QString currentType() const;

    KComboBox *mTypeBox;

    QLabel *mInfoLabel;
    QLabel *mLabel;
    QPushButton *mOptionButton;

    KonnectorPair *mPair;
    KSync::Konnector *mKonnector;
    QString mKonnectorUid;
};

#endif
