#ifndef VIRTUOSO_GAME_CONSOLE_H_INCLUDED
#define VIRTUOSO_GAME_CONSOLE_H_INCLUDED

#include "DynamicVariable.h"

#include <iostream>
#include <map>
#include <functional>

#include <type_traits>

#include <fstream>
#include <string>

#include "BaseLog.h"
#include "SimpleLog.h"

#include <memory>

#include <sstream>

#include "WindowedQueue.h"

///This is the backend of a console interface for games, to help with debugging, cheats, etc
///This is intended to be lightweight and efficient, rather than a general purpose scripting solution.
///It is intended to perform two main functions: printing and setting variables as the game executes, and executing compiled C++ code from within the game.
///If more power is needed, this class should be deprecated and replaced with an integrated scripting language (eg, python) terminal.
///
///This class also has help functionality and the ability to process large batches of commands, eg from a configuration file.  This makes saving user preferences
///and automated testing easy.
///Comments with # at the beginning of a line are supported also for further help with ini files and such
///
///\todo more detailed documentation

namespace Virtuoso{

class GameConsole
{

public:

    static const unsigned int defaultHistorySize; ///size of the history file

    typedef WindowedQueue<std::string> ConsoleHistoryBuffer;

    ConsoleHistoryBuffer history_buffer; ///history buffer of previous commands

    BaseLog& output(); ///log getter

    std::istream& input(); ///input stream getter

    void setConsoleInput(std::istream& in); ///setter for console istream

    void setConsoleOutput(BaseLog& out); ///setter for console output log

    void commandExecute(); ///get a command from the input stream and execute it.

    ///reads a string from the input stream given as the argument, and executes the command associated with it if there is one.  if not, reports an error.
    void commandExecute(std::istream* input);

    ///constructor.  Requires an input stream to receive commands from and an output stream to put results of commands to.
    ///binds the default commands to the command table.
    GameConsole(std::istream& input, BaseLog& ostream);

    ///default constructor that initializes the i/o pointers to null, then binds the default commands to the command table.
    GameConsole();

    ///function which takes in a string and a variable we want to associate with it.  Places the appropriate print and get functions
    ///in the cvarPrintFTable and cvarReadFTable, based on the type, using std bind.  This enables the user to access the variable then, using the string passed in.
    template <class T>
    void bindCVar(const std::string& str, T& var, const std::string& help);

    ///function which takes in a string and a variable we want to associate with it.  Places the appropriate print and get functions
    ///in the cvarPrintFTable and cvarReadFTable, based on the type, using std bind.  This enables the user to access the variable then, using the string passed in.
    template <class T>
    void bindCVar(const std::string& str, T& var);


    ///add a command to the console using a function object of type void(void).  set the help string for the command to the "help" argument
    void bindCommand(const std::string& str,std::function<void(void)> fun , const std::string& help );

    ///add a command to the console using a function pointer that takes no arguments.  set the associated help string to the "help" argument
    void bindCommand(const std::string& str, void (*fptr)(void) , const std::string& help );

    ///add a command to the console using a function pointer that takes arbitrary arguments, then set the associated help string to the "help" argument
    template<typename ...Args>
    void  bindCommand( const std::string& str, void (*fptr)(Args...) , const std::string& help );

    ///add a command to the console using a function object with arbitrary arguments, then set the associated help string to the "help" argument
    template <typename ...Args>
    void  bindCommand(const std::string& str,  std::function<void(Args...)> fun, const std::string& help );

    ///add a command to the console using a function object of type void(void).
    void bindCommand(const std::string& str,std::function<void(void)> fun);

    ///add a command to the console using a function pointer that takes no arguments.
    void bindCommand(const std::string& str, void (*fptr)(void) );

    ///add a command to the console using a function pointer that takes arbitrary arguments
    template<typename ...Args>
    void  bindCommand( const std::string& str, void (*fptr)(Args...));

    ///add a command to the console using a function object with arbitrary arguments
    template <typename ...Args>
    void  bindCommand(const std::string& str,  std::function<void(Args...)> fun );

    ///set the help string for a particular command or variable
    void setHelpTopic(const std::string& str, const std::string& data);

    ///execute commands from an ifstream until EOF
    void executeFile(std::ifstream& f);

    ///execute commands from a file named by input string until EOF
    void executeFile(const std::string& f);

    ///populate the command buffer from an input file named by string inFile
    void loadHistoryBuffer(const std::string& inFile);

    ///write the history buffer to file named by string outFile
    void saveHistoryBuffer(const std::string& outFile);

    ///populate the command buffer from an input file named by istream inFile
    void loadHistoryBuffer(std::istream& inFile);

    ///write the history buffer to file named by ostream outFile
    void saveHistoryBuffer(std::ofstream& outfile);

private:

    ///maps strings to std function objects, representing the available commands to the user.  eg, quit, set, etc
    std::map < std::string, std::function< void (void) > > commandTable;

    ///maps strings naming cVars to functions which read them from a std::istream.  This allows the console to parse variables of any type representable
    ///as text without modifying the console code or adding custom parsing code.
    std::map < std::string, std::function< void (void) > > cvarReadFTable;

    ///maps strings naming cVars to functions which write the variables of arbitrary type out to a std::ostream
    std::map < std::string, std::function< void (void) > > cvarPrintFTable;

    ///maps names of functions or cvars to string literals containing helpful information on their use
    std::map < std::string, std::string> helpTable;

    std::istream* consoleInput; ///the stream from which user input will be read to execute commands

    BaseLog* consoleOutput; ///the virtuoso log which output from console commands will be put to

    ///function which simply sets the value of an arbitrary type based on what's in the input stream
    ///the arguments are "eaten" by std bind, allowing it to be stored as type void (*x)(void) in the cvarReadFTable
    template <class T>
    void setCvar(T* var);

    ///function which simply prints the value of a variable to an output stream
    ///the arguments are "eaten" by std bind, allowing it to be stored as type void (*x)(void) in the cvarPrintFTable
    template <class T>
    void printCvar(T* var);

    ///dumps a list of available commands to the output stream
    void listCmd();

    ///dumps a list of bound cvars to the output stream
    void listCVars();

    ///dumps a list of available help topics to the output stream
    void listHelp();

    ///The function associated with the built in command "set" which parses the name of a cvar, and if it is bound, sets the value based on
    ///the value in the input stream
    void commandSet();

    ///the function associated with built in command "echo", which prints the value of a cvar if it is bound. if not, reports an error.
    void commandEcho();

    ///prints help on a topic if the user types help < topic >, or a generic help message if the user just types help
    void commandHelp();

    ///creates a string variable from the console
    void commandVar();

    ///wrapper function which parses arguments to a function object of arbitrary type from the console's input stream then executes the function if the parsing was successful
    template<typename... Args>
    void parse(std::function<void(Args...)> f);

    ///This function is called by populateAndExecute, and only executes the bound function if the parsing succeeds
    ///if parsing failed we do not want to pass in uninitialized garbage to the c++ function we bound
    template <typename... Args>
    void conditionalExecute(std::function< void(Args...) > f, const Args&... args);

    ///adds the built-in commands to the command table
    void bindBasicCommands();

    ///helper function to return a default-constructed temp variable of a particular type.  Used in parsing arguments for C++ functions bound to the console
    template <class T>
    T makeTemp();

    ///for parsing arguments to C++ functions bound to the console.  variadic template that recursively parses our function arguments in order
    template <typename FirstType, typename... Args>
    void populateTemps( FirstType& in,  Args&... Temps);

    ///for parsing arguments to C++ functions bound to the console. variadic template that recursively parses our function arguments in order.  base case
    template <typename FirstType>
    void populateTemps( FirstType& in);

    ///for parsing arguments to C++ functions bound to the console. variadic template that recursively parses our function arguments in order.  base case
    void populateTemps() {}

    ///for parsing arguments to C++ functions bound to the console.  call starts populating temp variables
    template <typename... Args>
    void goPopulateTemps(Args&... temps);

    ///for parsing arguments to C++ functions bound to the console.  Populates the temp variables using the istream, then calls the function with them
    template<typename... Args>
    void populateAndExecute(std::function<void(Args...)> f,
                            typename std::remove_const<typename std::remove_reference<Args>::type >::type...
                            temps);


    ///helper function that takes an input line containing commands and dereferences any console variables whose names appear prefixed by the $ symbol
    void dereferenceVariables(std::string& str);

    ///assigns the value of a dynamically created console variable using the console's input stream
    template<class T>
    void assignDynamicVariable(std::shared_ptr<T> var);

    ///writes a dynamically created console variable to the console output
    template <class T>
    void writeDynamicVariable(std::shared_ptr<T> var);

    ///bind dynamically created console variable to the variable table and set the associated help string
    template<class T>
    void bindDynamicCVar(const std::string& var, const T& value, const std::string& help);

    ///bind dynamically created console variable to the variable table.
    template<class T>
    void bindDynamicCVar(const std::string& var, const T& valueIn);

};

}
#include "GameConsole.tcc"

#endif
