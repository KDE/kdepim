#ifndef KONNECTORPAIRVIEW_H
#define KONNECTORPAIRVIEW_H

#include <klistview.h>

class KonnectorPairManager;

using namespace KSync;

class KonnectorPairItem : public QObject, public QListViewItem
{
  Q_OBJECT

  public:
    KonnectorPairItem( KonnectorPair *pair, KListView *parent );

    QString text( int column ) const;
    QString uid() const;

  private slots:
    void synceesRead( Konnector* );
    void synceeReadError( Konnector* );
    void synceesWritten( Konnector* );
    void synceeWriteError( Konnector* );

  private:
    KonnectorPair *mPair;
    QString mStatusMsg;
};

class KonnectorPairView : public KListView
{
  Q_OBJECT

  public:
    KonnectorPairView( KonnectorPairManager* manager, QWidget *parent );
    ~KonnectorPairView();

    QString selectedPair() const;

    void refresh();

  private slots:
    void refreshView();

  private:
    KonnectorPairManager *mManager;
};

#endif
