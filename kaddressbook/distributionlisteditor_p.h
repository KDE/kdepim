#ifndef KPIM_DISTRIBUTIONLISTEDITOR_P_H
#define KPIM_DISTRIBUTIONLISTEDITOR_P_H

#include <libkdepim/addresseelineedit.h>
#include <libkdepim/distributionlist.h>

#include <qstring.h>

namespace KABC {
    class Addressee;
    class AddressBook;
}

namespace KPIM {
namespace DistributionListEditor {

class LineEdit : public KPIM::AddresseeLineEdit
{
    Q_OBJECT
public:
    explicit LineEdit( QWidget* parent = 0 );
};


class Line : public QWidget
{
    Q_OBJECT
public:
    explicit Line( KABC::AddressBook* book, QWidget* parent = 0 );

    void setEntry( const KPIM::DistributionList::Entry& entry );
    KPIM::DistributionList::Entry entry() const; 

signals:
    void cleared();
    void textChanged();

private:
    KABC::Addressee findAddressee( const QString& name, const QString& email ) const; 

private slots:
    void textChanged( const QString& );

private:
    QString m_uid;
    QString m_initialText;
    LineEdit* m_lineEdit;
    KABC::AddressBook* m_addressBook;
};

} // namespace DisributionListEditor
} // namespace KPIM

#endif // KPIM_DISTRIBUTIONLISTEDITOR_P_H


