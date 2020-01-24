#pragma once
#include <string>
#include <sstream>
#include <stdexcept>
namespace rows{

// NOTE! should add specific errors and exceptions for 'rows' api
// ...

// throw macro for exceptions
#define THROW_ROWS_RUNTIME(x){\
  std::stringstream strm;\
  strm<<__FILE__":#"<<__LINE__<<": "<<x;\
    std::string s{strm.str()};\
    throw std::runtime_error(s);\
}
}
