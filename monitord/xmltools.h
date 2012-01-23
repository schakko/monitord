#ifndef XMLTOOLS_H_
#define XMLTOOLS_H_

#include "../xmlParser/xmlParser.h"
#include <string>

int getNodeInt(XMLNode parent,std::string childName,int defaultValue) ;
std::string getNodeText(XMLNode parent,std::string childName,std::string defaultValue, bool trim=true) ;
bool getNodeBool(XMLNode parent,std::string childName,bool defaultValue);


#endif /*XMLTOOLS_H_*/
