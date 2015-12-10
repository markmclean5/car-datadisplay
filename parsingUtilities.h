#ifndef PARSINGUTILITIES_H
#define PARSINGUTILITIES_H
#include <config4cpp/Configuration.h>
#include <string>
using namespace config4cpp;
using namespace std;

/* Parse integer */
int parseInt(Configuration*, string, string);
/* Parse integer inside subscope */
int parseInt(Configuration*, string, string, string);
/* Parse string */
string parseString(Configuration*, string, string);
/* Parse string inside subscope */
string parseString(Configuration*, string, string, string);
/* Parse float */
float parseFloat(Configuration*, string, string);
/* Parse float inside subscope */
float parseFloat(Configuration*, string, string, string);
/* Parse color */
void parseColor(Configuration*, string, float*, string);
/* Parse color inside subscope */
void parseColor(Configuration*, string, string, float*, string);

#endif
