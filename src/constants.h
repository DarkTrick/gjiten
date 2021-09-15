#ifndef __CONSTANTS_H__
#define __CONSTANTS_H__

#define DEFWORDFONTNAME "-*-fixed-medium-r-normal-*-14-*-*-*-*-*-jisx0208.1983-0"
#define DEFBIGFONTNAME "-*-fixed-medium-r-normal-*-24-*-*-*-*-*-jisx0208.1983-0";
#define DEFMAXWORDMATCHES 100
#define MAXDICFILES 100

#define APPLICATION_NAME "GJiten"

#define RESOURCE_PATH "/org/gjiten/data/"

#define VINFL_FILENAME GJITEN_DATADIR"/" "vconj.utf8"
#define VINFL_RESOURCE RESOURCE_PATH     "vconj.utf8"

#define RADKFILE_NAME     GJITEN_DATADIR"/" "radkfile.utf8"
#define RADKFILE_RESOURCE RESOURCE_PATH     "radkfile.utf8"

#define GJITEN_DICDIR GJITEN_DATADIR"/dics"


#define EXACT_MATCH 1      //jp en
#define START_WITH_MATCH 2 //jp
#define END_WITH_MATCH 3   //jp
#define ANY_MATCH 4        //jp en
#define WORD_MATCH 5       //en


#define SRCH_OK    0
#define SRCH_FAIL  1
#define SRCH_START  2
#define SRCH_CONT  3


#endif
