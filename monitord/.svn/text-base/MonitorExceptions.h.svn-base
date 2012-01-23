#ifndef MONITOREXCEPTIONS_H_
#define MONITOREXCEPTIONS_H_

#include <stdexcept>
#include "convert.h"

class MonitorException : public std::runtime_error {
 public:
   MonitorException(const std::string& s)
     : std::runtime_error(s)
     { }
 };
 #define ThrowMonitorException(err) throw( MonitorException(std::string(__FILE__)+ std::string(" Line ") + convertIntToString(__LINE__) + std::string(": ") + std::string(err) ))

#endif /*MONITOREXCEPTIONS_H_*/
