
#include <stdlib.h>
#include <stdio.h>

#include <time.h>

int main ( int argc,  char* argv[] ) {
    if ( argc < 2 ) {
        printf("You need to specify a long\n");
        return 0;
    }
    time_t myTime = (time_t) atol( argv[1] );
    tzset();
    struct tm* myTM;
    myTM = localtime( &myTime );
    printf("localtime: year: %d\nday: %d\n month: %d\n", myTM->tm_year, myTM->tm_mday, myTM->tm_mon  );
    myTM = gmtime( &myTime );
    printf("gmtime: year: %d\nday: %d\nmonth: %d\n", myTM->tm_year, myTM->tm_mday, myTM->tm_mon  );
    return 0;
};
