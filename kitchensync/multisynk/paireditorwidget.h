#ifndef PAIREDITORWIDGET_H
#define PAIREDITORWIDGET_H

#include <qwidget.h>

class KLineEdit;

class KonnectorPair;
class PluginEditorWidget;
class QRadioButton;

class PairEditorWidget : public QWidget
{
  Q_OBJECT

  public:
    PairEditorWidget( QWidget *parent = 0, const char *name = 0 );
    ~PairEditorWidget();

    void setPair( KonnectorPair *pair );
    KonnectorPair *pair() const;

  private:
    void initGUI();
    QWidget* createPluginTab();
    QWidget* createSyncOptionTab();
    QWidget* createFilterTab();

    KLineEdit *mPairNameEdit;

    QRadioButton *mResolveManually;
    QRadioButton *mResolveFirst;
    QRadioButton *mResolveSecond;
    QRadioButton *mResolveBoth;

    QValueList<PluginEditorWidget*> mEditorWidgets;

    KonnectorPair *mPair;
};

#endif
