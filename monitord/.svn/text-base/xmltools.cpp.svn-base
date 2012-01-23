#include "xmltools.h"

#include "convert.h"
#include <iostream>

using namespace std ;

std::string getNodeText(XMLNode parent,std::string childName,std::string defaultValue, bool trim)
{
	XMLNode xNode ;
	std::string returnValue ;
	if (!((xNode=parent.getChildNode(childName.c_str())).isEmpty()))
	{
		returnValue=xNode.getText() ;
	} else {
		returnValue=defaultValue ;
	}

	return returnValue;
}

bool getNodeBool(XMLNode parent,std::string childName,bool defaultValue)
{
	int iDefault ;
	iDefault = (defaultValue==true ? 1:0) ;
	int temp=getNodeInt(parent,childName,iDefault) ;
	return (temp!=0);
}


int getNodeInt(XMLNode parent,std::string childName,int defaultValue)
{
	int returnValue ;
	std::string nodeText=getNodeText(parent,childName,"") ;

	try
	{
			returnValue=convertToInt(nodeText) ;
	}
	catch (BadConversion e)
	{
		returnValue=defaultValue ;
	}
	return returnValue;
}
