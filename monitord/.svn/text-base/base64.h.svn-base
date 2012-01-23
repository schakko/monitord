#ifndef BASE64_H_
#define BASE64_H_

#include <string>

/**
 * @brief kodiert den �bergebenen string ins base64 Format
 * @author Stephan Effertz (buebchen)
 * @param bytes_to_encode Ausgangszeichenkette
 * @param len Laenge der Ausgangszeichenkette
 */
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int len);

/**
 * @brief kodiert den �bergebenen string ins base64 Format
 * @author Stephan Effertz (buebchen)
 * @param s Ausgangszeichenkette
 *
 * Achtung diese Funktion kann keine Nullbytes umwandeln, da Nullbytes
 * das Ende der Eingabe markieren
 */
std::string base64_encode(std::string const& s);

/**
 * @brief  wandelt einen base64 kodierten String in den Originaltext
 * @author Stephan Effertz (buebchen)
 * @param  s Base64 kodierter String
 * @return dekodierter String
 *
 * Achtung kann zur Zeit keine Nullbytes verarbeiten !
 */
std::string base64_decode(std::string const& s);

#endif /*BASE64_H_*/
