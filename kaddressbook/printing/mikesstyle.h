#ifndef MIKESSTYLE_H
#define MIKESSTYLE_H

#include <qfont.h>

#include "printstyle.h"

namespace KABPrinting {

    class MikesStyle : public PrintStyle
    {
        Q_OBJECT
    public:
        MikesStyle(PrintingWizard* parent, const char* name);
        ~MikesStyle();
        void print(QStringList);
    protected:
        void doPaint(QPainter &painter, const KABC::Addressee &a,
                     int maxHeight,
                     const QFont& font, const QFont& bFont);
        int calcHeight(const KABC::Addressee &a,
                       const QFont& font, const QFont& bFont);
        void paintTagLine(QPainter &p, const QFont& font);
        QString trimString(const QString &text, int width,
                           QFontMetrics &fm);
    };

    class MikesStyleFactory : public PrintStyleFactory
    {
    public:
        MikesStyleFactory(PrintingWizard* parent_,
                                   const char* name_=0);
        PrintStyle *create();
        QString description();
    };

}

#endif
