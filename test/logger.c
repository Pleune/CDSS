#include "cdss/plog.h"
#include <unistd.h>

int
test_logger_basic(void)
{
    size_t i;
    for(i=0; i<100; i++)
        plog(L_DEBUG, "DEBUG CRAP NOT SHOWN BY DEFAULT %i", i);

    for(i=0; i<100; i++)
        plog(L_INFO, "SOME FAIRLY USELESS INFO %i", i);

    for(i=0; i<100; i++)
        plog(L_WARN, "SOME WARNINGS %i", i);

    for(i=0; i<100; i++)
        plog(L_ERROR, "SOME ERRORS %i", i);

    //for(i=0; i<100; i++)
        //plog(L_FATAL, "FATAL ERRORS SHOULD DIE %i", i);

    plog_flush();

    return 0;
}

int
main()
{
    return test_logger_basic();
}
