#ifndef DYNAMIC_VARIABLE_H_INCLUDED
#define DYNAMIC_VARIABLE_H_INCLUDED

#include <iostream>

namespace Virtuoso
{

///Dynamic variables are string-based variables that can be created, assigned, and dereferenced from the console itself rather than C++ code
struct DynamicVariable:public std::string
{
};

//here we just overload the iostream operators.  The only real difference is that on input a dynamic variable takes a full line from input

inline std::ostream& operator<<(std::ostream& instream, const DynamicVariable& var)
{
    const std::string& str = var;
    return instream<<str;
}

///dynamic variable definitions take a full line from the input stream
inline std::istream& operator>>(std::istream& istream, DynamicVariable& var)
{
    return  getline(istream, var);
}

}

#endif
