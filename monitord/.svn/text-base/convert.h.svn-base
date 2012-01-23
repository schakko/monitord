#ifndef CONVERT_H_
#define CONVERT_H_

 // File: convert.h
 #include <iostream>
 #include <sstream>
 #include <string>
 #include <stdexcept>

#include <string.h> // TODO bis auf strcat/strlen alles C++


/**
 * @brief Fehlerklasse fuer Konvertierungsfehler
 * @author Stephan Effertz (buebchen)
 */

 class BadConversion : public std::runtime_error {
 public:
   BadConversion(const std::string& s)
     : std::runtime_error(s)
     { }
 };

/**
 * @brief double (Fliesskommazahl) in String konvertieren
 * @author Stephan Effertz (buebchen)
 */
inline double convertToDouble(const std::string& s)
 {
   std::istringstream i(s);
   double x;
   if (!(i >> x))
     throw BadConversion("convertToDouble(\"" + s + "\")");
   return x;
 }

/**
 * @brief Integer nach String konvertieren
 * @author Stephan Effertz (buebchen)
 */
inline std::string convertIntToString(const int& x)
 {

  std::ostringstream o;

  if (!(o << x))
  	throw BadConversion("convertIntoToString") ;

  return o.str();
}

/**
 * @brief Integer nach Hexadezimal (string) konvertieren
 * @author Stephan Effertz (buebchen)
 */
inline std::string convertIntToHexString(const int& x)
 {

  std::ostringstream o;

  if (!(o << std::hex << x))
  	throw BadConversion("convertIntoToString") ;

  return o.str();
}

 /**
 * @brief Zeichenkette nach Integer konvertieren (wie atoi)
 * @author Stephan Effertz (buebchen)
 */
inline int convertToInt(const std::string& s)
 {
   std::istringstream i(s);
   int x;
   if (!(i >> x))
     throw BadConversion("convertToInt(\"" + s + "\")");
   return x;
 }

 /**
 * @brief Konvertiert Nibble (0..F) nach Integer (0..15)
 * @author Stephan Effertz (buebchen)
 */
 inline int convertNibbleToInt(const char& c)
 {
 	int x ;
 	if (c >='0' && c<='9')
 	{
 		x=c-'0' ;
 	} else if (c >='A' && c<='F')
 	{
 		x=c-'A'+10 ;
 	} else if (c >='a' && c<='f')
 	{
 		x=c-'a'+10 ;
 	} else {
 		throw BadConversion("convertNibbleToInt");
 	}
 	return x ;
 }

 /**
 * @brief Konvertiert Integer (0..15) nach Nibble (0..F)
 * @author Stephan Effertz (buebchen)
 */
 inline char convertIntToNibble(const unsigned int& intVal)
 {
 	char c ;
 	if ((intVal>16) || (intVal <0))
 	{
 		std::string iString = convertIntToString(intVal) ;
 		throw BadConversion("convertIntToNibble: integer out of range : "+iString);
 	}

 	if (intVal<10)
 	{
 		c='0'+intVal ;
 	}
 	else
 	{
 		c='a'+(intVal-10) ;
 	}
 	return c ;
 }

 /**
 * @brief Konvertiert String in einen HEX-String
 * @author Stephan Effertz (buebchen)
 */
 inline std::string convertStringToHex(const std::string& input)
 {
 	std::string result;
 	result.clear() ;
	int i=0 ;

	while (input.length()-i>0)
	{
		unsigned char temp=input[i] ;

		unsigned char c1,c2 ;

		c2=convertIntToNibble(temp & 0xf) ; // quasi: LSB
		c1=convertIntToNibble(temp >>4) ; // quasi: MSB

		result.push_back(c1) ;
		result.push_back(c2) ;

		i++;
	}

	return result ;
 }

 /**
 * @brief Konvertiert String in einen HEX-String
 * @author Stephan Effertz (buebchen)
 */
 inline bool convertStringToHex(const std::string& input, std::string &result)
{
	result.clear() ;
	int i=0 ;

	while (input.length()-i>0)
	{
		unsigned char temp=input[i] ;

		unsigned char c1,c2 ;

		c2=convertIntToNibble(temp & 0xf) ; // quasi: LSB
		c1=convertIntToNibble(temp >>4) ; // quasi: MSB

		result.push_back(c1) ;
		result.push_back(c2) ;

		i++;
	}

	return true ;
}

/**
 * @brief Konvertiert String in einen HEX-String
 * @author Stephan Effertz (buebchen)
 */
inline bool convertStringToHex(char input[], char* result)
{
	//result.clear() ;
	int i=0 ;

	char byteString[3] ;

	while (strlen(input)-i>0)
	{
		unsigned char temp=input[i] ;

		unsigned char c1,c2 ;

		c2=convertIntToNibble(temp & 0xf) ; // quasi: LSB
		c1=convertIntToNibble(temp >>4) ; // quasi: MSB

		byteString[0]=c1 ;
		byteString[1]=c2 ;
		byteString[2]=0 ;

		strcat (result,byteString) ;
		i++;
	}

	return true ;
}

/**
 * @brief Konvertiert HEX-String in einen String
 * @author Stephan Effertz (buebchen)
 */
inline bool convertHexToString(char input[], std::string &result)
{
	result.clear() ;
	char tempChar ;
	int i=0 ;


	while ((strlen(input)-i) >= 2)
	{
		tempChar = convertNibbleToInt(input[i])*16 + convertNibbleToInt(input[i+1]) ;
		result.push_back(tempChar) ;
		i=i+2 ;
	}
	return true ;
}

/**
 * @brief Konvertiert HEX-String in einen String
 * @author Stephan Effertz (buebchen)
 */
inline bool convertHexToString(char input[], char result[])
{
	char tempChar ;
	int i=0 ;

	while ((strlen(input)-i) >= 2)
	{
		tempChar = convertNibbleToInt(input[i])*16 + convertNibbleToInt(input[i+1]) ;
		result[i/2]=tempChar ;
		i=i+2 ;
	}
	result[i/2]=0 ;
	return true ;
}

/**
 * @brief Konvertiert HEX-String in einen String
 * @author Stephan Effertz (buebchen)
 */
inline bool convertHexToString(std::string input, std::string &result)
{
	result.clear() ;
	char tempChar ;
	int i=0 ;


	while ((input.length()-i) >= 2)
	{
		tempChar = convertNibbleToInt(input[i])*16 + convertNibbleToInt(input[i+1]) ;
		result.push_back(tempChar) ;
		i=i+2 ;
	}
	return true ;
}

inline const char *convertStringBoolText(const std::string &number)
{
	const char *result;
	if (number == "0") {
		result = "false";
	} else if (number == "1") {
		result = "true";
	} else {
		throw BadConversion("convertStringBoolText");
	}
	return result;
}

inline const char *convertStringTextBool(const std::string &text)
{
	const char *result;
	if (text == "false") {
		result = "0";
	} else if (text == "true") {
		result = "1";
	} else {
		throw BadConversion("convertStringTextBool");
	}
	return result;
}

#endif /*CONVERT_H_*/
