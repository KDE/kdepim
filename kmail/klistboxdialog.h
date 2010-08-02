#ifndef KLISTBOXDIALOG_H
#define KLISTBOXDIALOG_H

#include <kdialogbase.h>

class TQLabel;
class TQListBox;

class KListBoxDialog : public KDialogBase
{
    Q_OBJECT

public:
    KListBoxDialog( TQString& _selectedString,
                    const TQString& caption,
                    const TQString& labelText,
                    TQWidget*    parent = 0,
                    const char* name   = 0,
                    bool        modal  = true );
    ~KListBoxDialog();

    void setLabelAbove(  const TQString& label  );
    void setCommentBelow(const TQString& comment);

    TQListBox* entriesLB;

private slots:
    void highlighted( const TQString& );

protected:
    TQString& selectedString;
    TQLabel*  labelAboveLA;
    TQLabel*  commentBelowLA;
};

#endif
