/*****inline implementation file.  Casual users beware *****/

inline Virtuoso::BaseLog& Virtuoso::GameConsole::output()
{
    return *consoleOutput;
}

inline std::istream& Virtuoso::GameConsole::input()
{
    return *consoleInput;
}


inline void Virtuoso::GameConsole::setConsoleInput(std::istream& in)
{
    consoleInput = &in;
}

inline void Virtuoso::GameConsole::setConsoleOutput (BaseLog& out)
{
    consoleOutput = &out;
}

inline void Virtuoso::GameConsole::commandExecute()
{
    commandExecute(consoleInput);
}





template <class T>
inline T Virtuoso::GameConsole::makeTemp()
{

    T temp;
    return temp;

}


template <typename... Args>
inline void Virtuoso::GameConsole::conditionalExecute(std::function<void(Args...)> f, const Args&... args)
{

    if(input().fail())
    {

        output().error()<<"SYNTAX ERROR IN FUNCTION ARGUMENTS"<<std::endl;
        input().clear();
    }
    else
    {
        f(args...);
    }

}



template <typename FirstType>
inline void Virtuoso::GameConsole::populateTemps( FirstType& in)
{

    std::istream& inst = (*consoleInput);

    inst>>in;

}


//variadic template that recursively parses our function arguments in order
template <typename FirstType, typename... Args>
inline void Virtuoso::GameConsole::populateTemps( FirstType& in, Args&... Temps)
{


    std::istream& inst = (*consoleInput);

    inst>>in;

    populateTemps(Temps...);

}



template <typename... Args>
inline void Virtuoso::GameConsole::goPopulateTemps(Args&... temps)
{
    populateTemps(temps...);
}



template<typename... Args>
inline void Virtuoso::GameConsole::populateAndExecute(std::function<void(Args...)> f,
        typename std::remove_const<typename std::remove_reference<Args>::type >::type...
        temps)
{


    goPopulateTemps< typename std::remove_const<typename std::remove_reference<Args>::type >::type...>(temps...);

    conditionalExecute<Args...>(f, temps...);

}


//function that gets bound as type void to the console with a second function object that may have multiple arguments of various types
//this function gets called when the user enters the function name into the console.
template<typename... Args>
inline void Virtuoso::GameConsole::parse(std::function<void(Args...)> f)
{

    //first we have to create a bunch of temp variables and pass them into the populateAndExecute function
    //the temp variables are needed to store the result.  There's no guarantee in c++ for argument evaluation order, so we can't just
    //skip the intermediate step of passing constructed temps into a second function
    populateAndExecute<Args...>( f, (makeTemp< typename std::remove_const<typename std::remove_reference< Args>::type >::type >  () )... );

}


inline void Virtuoso::GameConsole::bindCommand(const std::string& str,std::function<void(void)> fun )
{

    commandTable[str] = fun;

}


inline void Virtuoso::GameConsole::bindCommand(const std::string& str, void (*fptr)(void) )
{

    commandTable[str] = fptr;

}


template<typename ...Args>
inline void Virtuoso::GameConsole::bindCommand( const std::string& str, void (*fptr)(Args...)  )
{

    commandTable[str] = std::bind(&GameConsole::parse<Args...>, this, fptr);

}

template <typename ...Args>
inline void Virtuoso::GameConsole::bindCommand(const std::string& str,  std::function<void(Args...)> fun)
{

    commandTable[str] = std::bind(&GameConsole::parse<Args...>, this, fun);

}


inline void Virtuoso::GameConsole::bindCommand(const std::string& str,std::function<void(void)> fun , const std::string& help )
{
    setHelpTopic(str, help);
    bindCommand(str, fun);
}

inline void Virtuoso::GameConsole::bindCommand(const std::string& str, void (*fptr)(void) , const std::string& help )
{
    setHelpTopic(str, help);
    bindCommand(str,fptr);
}

template<typename ...Args>
inline void  Virtuoso::GameConsole::bindCommand( const std::string& str, void (*fptr)(Args...) , const std::string& help )
{
    setHelpTopic(str, help);
    bindCommand(str, fptr);
}

template <typename ...Args>
inline void  Virtuoso::GameConsole::bindCommand(const std::string& str,  std::function<void(Args...)> fun, const std::string& help )
{
    setHelpTopic(str, help);
    bindCommand(str, fun);
}


inline void Virtuoso::GameConsole::setHelpTopic(const std::string& str, const std::string& data)
{

    helpTable[str] = data;

}



template <class T>
inline void Virtuoso::GameConsole::bindCVar(const std::string& str, T& var)
{

    cvarReadFTable[str] =
        std::bind(&GameConsole::setCvar<T>, this, &var);

    cvarPrintFTable[str] = std::bind(&GameConsole::printCvar<T>, this, &var);
}




template <class T>
inline void Virtuoso::GameConsole::bindCVar(const std::string& str, T& var, const std::string& help)
{

    bindCVar(str, var);

    setHelpTopic(str, help);

}





template <class T>
void Virtuoso::GameConsole::setCvar(T* var)
{

    T tmp;  ///temp argument is a necessity; without it we risk corruption of our variable value if there is a parse error.  Should be no issue unless someone is using this to parse a ginormous structure or copy construction invokes a state change.

    *consoleInput >> tmp;

    if(input().fail())
    {

        output().error()<<"SYNTAX ERROR IN VARIABLE PARSER"<<std::endl;
        input().clear();
    }
    else
    {
        *var = tmp;
    }
}


template <class T>
inline void Virtuoso::GameConsole::printCvar(T* var)
{

    (*consoleOutput)() << *var <<std::endl;
}




template<class T>
inline void Virtuoso::GameConsole::assignDynamicVariable(std::shared_ptr<T> var)
{
    (*consoleInput)>>(*var);
}

template <class T>
inline void Virtuoso::GameConsole::writeDynamicVariable(std::shared_ptr<T> var)
{

    const T& deref = *(var);
    output()()<<deref;
}


template<class T>
inline void Virtuoso::GameConsole::bindDynamicCVar(const std::string& var, const T& value, const std::string& help)
{

    helpTable[var] = help;

    bindDynamicCVar(var, value);

}


template<class T>
inline void Virtuoso::GameConsole::bindDynamicCVar(const std::string& var, const T& valueIn)
{

    std::shared_ptr<T> ptr(new T(valueIn));

    cvarReadFTable[var] = std::bind( &GameConsole::assignDynamicVariable<T> , this, ptr);

    cvarPrintFTable[var] = std::bind(&GameConsole::writeDynamicVariable<T>, this, ptr);

}

