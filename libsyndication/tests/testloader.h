#ifndef TEST_TESTLIBSYNDICATION_H
#define TEST_TESTLIBSYNDICATION_H

#include "loader.h"

#include <QObject>

class TestLibSyndication : public QObject
{
    Q_OBJECT
            
    public:
        
        TestLibSyndication(const QString& url);
    
    public slots:
        
        void slotLoadingComplete(Syndication::Loader* loader,
                            Syndication::FeedPtr feed,
                            Syndication::ErrorCode error);
};

#endif // TEST_TESTLIBSYNDICATION_H
