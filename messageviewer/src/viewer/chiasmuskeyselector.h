#ifndef CHIASMUSKEYSELECTOR_H
#define CHIASMUSKEYSELECTOR_H

#include <QDialog>
#include <KConfigGroup>

class QListWidget;
class QLineEdit;
class QLabel;
class QPushButton;

namespace MessageViewer
{

class ChiasmusKeySelector : public QDialog
{
    Q_OBJECT

public:
    ChiasmusKeySelector(QWidget *parent, const QString &caption,
                        const QStringList &keys, const QString &currentKey,
                        const QString &lastOptions);

    QString key() const;
    QString options() const;

private Q_SLOTS:
    void slotItemSelectionChanged();

private:
    QLabel *mLabel;
    QListWidget *mListBox;
    QLineEdit *mOptions;
    QPushButton *mOkButton;
};

}

#endif
