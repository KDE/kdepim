#include <qlayout.h>
#include <qtextbrowser.h>
#include <qprogressbar.h>
#include <kapplication.h>
#include <kdialog.h>
#include <kdebug.h>
#include "printprogress.h"

namespace KABPrinting {

    PrintProgress::PrintProgress(QWidget *parent)
        : PrintProgressBase(parent)
    {
        // <HACK reason="Designers fixed layout spacings">
        layout()->setMargin(KDialog::marginHint());
        layout()->setSpacing(KDialog::spacingHint());
        // </HACK>
    }

    PrintProgress::~PrintProgress()
    {
    }

    void PrintProgress::addMessage(const QString &msg)
    {
        messages.append(msg);
        QString head=QString("<qt><b>Progress:</b><ul>");
        QString foot=QString("</ul></qt>");
        QString body;
        QStringList::Iterator it;
        for(it=messages.begin(); it!=messages.end(); ++it)
        {
            body.append(QString("<li>")+(*it)+QString("</li>"));
        }
        tbLog->setText(head+body+foot);
        kapp->processEvents();
    }

    void PrintProgress::setProgress(int step)
    {
        pbProgress->setProgress(step);
        kapp->processEvents();
    }
}
