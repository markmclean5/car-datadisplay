#include "parsingUtilities.h"
#include <config4cpp/Configuration.h>
#include <iostream>
using namespace std;
using namespace config4cpp;


/* Parsing Functions - these need to go in their own file */
/* Parse integer */
int parseInt(Configuration * cfg, string scope, string attr){
	try {
		return cfg->lookupInt(scope.c_str(), attr.c_str());
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}
/* Parse integer inside subscope */
int parseInt(Configuration * cfg, string scope, string subscope, string attr){
	try {
		return cfg->lookupInt(scope.c_str(), subscope.append(attr).c_str());
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}
/* Parse string */
string parseString(Configuration * cfg, string scope, string attr){
	try {
		return cfg->lookupString(scope.c_str(), attr.c_str());
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}
/* Parse string inside subscope */
string parseString(Configuration * cfg, string scope, string subscope, string attr){
	try {
		return cfg->lookupString(scope.c_str(), subscope.append(attr).c_str());
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}
/* Parse float */
float parseFloat(Configuration * cfg, string scope, string attr){
	try {
		return cfg->lookupFloat(scope.c_str(), attr.c_str());
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}
/* Parse float inside subscope */
float parseFloat(Configuration * cfg, string scope, string subscope, string attr){
	try {
		return cfg->lookupFloat(scope.c_str(), subscope.append(attr).c_str());
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}

/* Parse color */
void parseColor(Configuration * cfg, string scope, float* output, string attr)
{
	try {
		StringVector color;
		int idx = 0;
		cfg->lookupList(scope.c_str(), attr.c_str(), color);
		for(;idx<4;idx++) {
			output[idx] = stof(color[idx]);
		}
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}

/* Parse color inside subscope */
void parseColor(Configuration * cfg, string scope, string subscope, float* output, string attr)
{
	try {
		StringVector color;
		int idx = 0;
		cfg->lookupList(scope.c_str(), subscope.append(attr).c_str(), color);
		for(;idx<4;idx++) {
			output[idx] = stof(color[idx]);
		}
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}

/* Parse boolean */
bool parseBool(Configuration * cfg, string scope, string attr){
	bool result = false;
	try {
		string str;
		str = cfg->lookupString(scope.c_str(), attr.c_str());
		cout << "Lookup result: " << str << endl;

		if(str.compare("true") == 0 || str.compare("TRUE") == 0)
			result = true;
		cout << "Result: "<< result << endl;
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
	return result;
}
/* Parse boolean inside subscope */
bool parseBool(Configuration * cfg, string scope, string subscope, string attr){
	try {
		string str = cfg->lookupString(scope.c_str(), subscope.append(attr).c_str());
		bool result = false;
		if(str.compare("true") == 0 || str.compare("TRUE") == 0)
			result = true;
		return result;
	}
	catch(const ConfigurationException & ex) {
		cout << ex.c_str() << endl;
	}
}
