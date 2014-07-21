#ifndef CHIASMUSKEYSELECTOR_H
#define CHIASMUSKEYSELECTOR_H

#include <kdialog.h>
#include "messageviewer_export.h"

class QListWidget;
class KLineEdit;
class QLabel;

namespace MessageViewer {

class MESSAGEVIEWER_EXPORT ChiasmusKeySelector : public KDialog
{
    Q_OBJECT

public:
    ChiasmusKeySelector( QWidget* parent, const QString& caption,
                         const QStringList& keys, const QString& currentKey,
                         const QString& lastOptions );

    QString key() const;
    QString options() const;

private Q_SLOTS:
    void slotItemSelectionChanged();

private:
    QLabel* mLabel;
    QListWidget* mListBox;
    KLineEdit* mOptions;
};

}

#endif
