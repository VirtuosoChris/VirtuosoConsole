#include "GameConsole.h"
#include <cctype>

#include <sstream>

using namespace std::placeholders;

const unsigned int Virtuoso::GameConsole::defaultHistorySize = 10; ///default size of the history file

void Virtuoso::GameConsole::executeFile(std::ifstream& f)
{

    std::istream* old = consoleInput;

    consoleInput = &f;

    while(!f.eof())
    {
        commandExecute();
    }

    consoleInput = old;

}


void Virtuoso::GameConsole::executeFile(const std::string& x)
{

    std::ifstream f(x);

    if(!f.is_open())
    {
        output().error()<<"unable to open file!"<<std::endl;
    }
    else
    {
        executeFile(f);
    }

    f.close();

}

void Virtuoso::GameConsole::commandHelp()
{

    const char* genericHelp= "Type 'help' followed by the name of a command or variable to get help on that topic if available."
                             "\nType listCmd, listCVars, and listHelp to print lists of the available commands, variables, and help topics."
                             "\nUse $<varname> to dereference a variable in a command argument list and use # to comment the rest of a line";

    std::string x;
    if((*consoleInput)>>x)
    {

        std::map<std::string,std::string>::const_iterator it = helpTable.find(x);

        if(it != helpTable.end())
        {

            output().status()<<(it->second)<<std::endl;

        }
        else
        {
            output().error()<<"No help available on "<<x<<std::endl;
        }
    }

    else
    {

        output().status()<<genericHelp<<std::endl;

    }
}



void Virtuoso::GameConsole::listHelp()
{

    output().status()<<"\nAvailable help topics:";

    std::map<std::string, std::string >::const_iterator it = helpTable.begin();

    for(; it != helpTable.end(); it++)
    {

        output().status()<<"\n"<<it->first;

    }
    output().status()<<std::endl;

}


void Virtuoso::GameConsole::listCmd()
{

    output().status()<<"\nAvailable commands:";

    std::map<std::string, std::function< void (void) > >::const_iterator it = commandTable.begin();

    for(; it != commandTable.end(); it++)
    {

        output().status()<<"\n"<<it->first;

    }
    output().status()<<std::endl;

}




void Virtuoso::GameConsole::listCVars()
{

    output().status()<<"\nBound console variables:";

    std::map<std::string, std::function< void (void) > >::const_iterator it =
        cvarReadFTable.begin();


    for(; it != cvarReadFTable.end(); it++)
    {

        output().status()<<"\n"<<it->first;

    }

    output().status()<<std::endl;

}



void Virtuoso::GameConsole::commandSet()
{

    std::string x;


    if(!(*consoleInput>>x))
    {
        output().error()<<"Syntax error parsing argument"<<std::endl;
        return;
    }

    std::map< std::string, std::function< void (void) > >::iterator it = cvarReadFTable.find(x);

    if(it!= cvarReadFTable.end())
    {

        (it->second)();
    }
    else
    {
        output().error()<<"Variable "<<x<<" unknown."<<std::endl;
    }

}


///the function associated with built in command "echo", which prints the value of a cvar if it is bound. if not, reports an error.
void Virtuoso::GameConsole::commandEcho()
{

    std::string x;

    if(!(*consoleInput>>x))
    {
        output().error()<<"Syntax error parsing argument"<<std::endl;
        return;
    }

    std::map< std::string, std::function< void (void) > >::iterator it = cvarPrintFTable.find(x);

    if(it!= cvarPrintFTable.end())
    {
        (it->second)();
    }
    else
    {
        output().error()<<"Variable "<<x<<" unknown."<<std::endl;
    }

}


///reads a string from the input stream and executes the command associated with it, if there is one.  if not, reports an error.
void Virtuoso::GameConsole::commandExecute(std::istream* input/*, bool flush*/ )
{

    char ch;
    while(!(*consoleInput).eof()){

        ch=(*consoleInput).peek();

        if(ch == '#')
        {
            std::string tmp;
            getline( (*input), tmp);
            return;
        } //if newline we will not parse anything else

        else if(std::isspace(ch))
        {
            (*consoleInput).get(ch);
            continue;
        }
        else
        {
            break;
        }

    }

    std::stringstream lineStream;

    {
        std::string lineTemp;

        getline( (*input), lineTemp);

        history_buffer.push(lineTemp);

        output().echo()<<lineTemp<<std::endl;

        dereferenceVariables(lineTemp);


        lineStream.str(lineTemp);///\todo this constrains us to a single line.  way to go later might be to require user or
        ///generated command parser to return string that was parsed

    }

    std::istream* oldInput= consoleInput;

    consoleInput = &lineStream;

    std::string x;

    while((*consoleInput)>>x)
    {

        std::map<std::string, std::function< void (void)> >::const_iterator it = commandTable.find(x);

        if(it == commandTable.end())
        {
            output().error()<<"command "<<x<<" unknown"<<std::endl;
        }
        else
        {
            (it->second)(); //execute the command
        }

        output().status()<<'\n';
    }

    consoleInput =(std::istream*)oldInput;

}


void Virtuoso::GameConsole::bindBasicCommands()
{

    std::function<  void(const std::string&, const DynamicVariable& ) > f1 =

        std::bind(
            static_cast< void(GameConsole::*)(const std::string&, const DynamicVariable&) >
            ( &GameConsole::bindDynamicCVar<DynamicVariable>),
            this, std::placeholders::_1, std::placeholders::_2
        );

    bindCommand("var",f1,
                "Type var <varname> <value> to declare a dynamic variable with name <varname> and value <value>."
                "\nVariable names are any space delimited string and variable value is set to the remainder of the line."
               );

    bindCommand("set", std::bind(&GameConsole::commandSet, this ), "type set <identifier> <val> to change the value of a cvar");

    bindCommand("echo", std::bind(&GameConsole::commandEcho, this), "type echo <identifier> to print the value of a cvar");

    bindCommand("listCmd", std::bind(&GameConsole::listCmd, this), "lists the available console commands");

    bindCommand("listCVars", std::bind(&GameConsole::listCVars, this), "lists the bound cvars");

    bindCommand("help", std::bind(&GameConsole::commandHelp, this), "you're a smartass");

    bindCommand("listHelp", std::bind(&GameConsole::listHelp, this), "lists the available help topics");

    bindCommand("runFile", std::function<void(const std::string&)>(
                    std::bind(static_cast<void (GameConsole::*)(const std::string&)>(&GameConsole::executeFile), this, _1)
                ),
                "runs the commands in a text file named by the argument");

}


///constructor.  Requires an input stream to receive commands from and an output stream to put results of commands to.
///binds the default commands to the command table.
Virtuoso::GameConsole::GameConsole(std::istream& input, BaseLog& log)//was ostream
    :
    history_buffer(defaultHistorySize),
    consoleInput(&input), consoleOutput(&log)
{

    //create default console commands

    bindBasicCommands();
}


void Virtuoso::GameConsole::loadHistoryBuffer(const std::string& inFile)
{
    std::ifstream hfi(inFile);

    if(hfi.is_open())
    {
        loadHistoryBuffer(hfi);
        hfi.close();
    }
    else
    {
        output().error()<<"Unable to load console history file "<<inFile<<std::endl;
    }

}



void Virtuoso::GameConsole::saveHistoryBuffer(const std::string& outFile)
{
    if(history_buffer.size())
    {
        std::ofstream hfo(outFile);
        saveHistoryBuffer(hfo);
        hfo.close();
    }
}


void Virtuoso::GameConsole::loadHistoryBuffer(std::istream& inFile)
{
    while(!inFile.eof())
    {

        std::string tmp;
        std::getline(inFile,tmp);

        if(tmp.length())
        {
            history_buffer.push(tmp);
        }

    }
}

void Virtuoso::GameConsole::saveHistoryBuffer(std::ofstream& outfile)
{

    for(unsigned int i = 0; i < history_buffer.size(); i++)
    {
        outfile<<history_buffer[i]<<std::endl;
    }


}


void Virtuoso::GameConsole::dereferenceVariables(std::string& str)
{
    size_t varBase = 0;
    int n=0;

    while((varBase = str.find('$', varBase)) != str.npos)
    {

        size_t substrEnd = varBase;
        size_t dollar = varBase;
        varBase++;

        n++;

        for(;  (substrEnd < str.size()) &&  (!isspace(str[substrEnd])); substrEnd++);

        if(substrEnd == varBase)
        {
            consoleOutput->error()<<"EXPECTED IDENTIFIER AT $"<<std::endl;;
        }
        else
        {
            std::string substr(str.substr(varBase, substrEnd-varBase));

            BaseLog* tmp = consoleOutput;
            std::stringstream sstr;
            SimpleLog tempLog(sstr);
            consoleOutput = &tempLog;

            std::map< std::string, std::function< void (void) > >::iterator it = cvarPrintFTable.find(substr);

            //check that variable exists
            if(it != cvarPrintFTable.end())
            {
                it->second();

                str.replace( dollar,substrEnd, sstr.str());

                consoleOutput = tmp;

            }
            else
            {
                consoleOutput = tmp;

                consoleOutput->error()<<"Variable "<<substr<<" not found"<<std::endl;
            }
        }
    }
}


Virtuoso::GameConsole::GameConsole()
:
history_buffer(defaultHistorySize),
consoleInput(NULL),
consoleOutput(NULL)
{
    bindBasicCommands();
}




