#include "../QuakeStyleConsole.h"

using namespace Virtuoso;

bool running = true;

void printSum(const int& a, const int& b)
{
    std::cout<<a+b<<std::endl;
}

class Adder
{
    std::string str;

    public:

        Adder(const std::string& pr):str(pr){}

        void add(int a, int b, int c, int d, int e)
        {
            std::clog << str << a+b+c+d+e << std::endl;
        }
};

int main()
{
    std::clog << "VirtuosoConsole test program.\n"<<std::endl;
    std::clog << "Type help, listCmd, or listCVars for usage."<<std::endl;
    std::clog << "Try runFile with TestCommands.txt."<<std::endl;
    std::clog << "When you quit, COMMAND_HISTORY.txt will save your history file (which you can also run using runFile)."<<std::endl;
    
    Adder a("Sum is "); //a class

	int health = 0; // a variable like we might have in a game

	QuakeStyleConsole console; //create the console
    
    //binding a template function with a particular value as a command

    console.bindCommand("quit", [](){running = false;}, "quits the program");
	
    //binding a free function that takes 2 arguments as a command.

    console.bindCommand("sum", printSum, "Sums two input integer values.  Function bound with bindCommand()");
    
    
    std::function <void (void)> printHistory = [&console]()
    {
        for (int i = 0; i < console.historyBuffer().size(); i++)
        {
            std::clog << console.historyBuffer()[i] << std::endl;
        }
    };
    
    console.bindCommand("printHistory", printHistory , "Print the current command history buffer for the console.");

    console.bindMemberCommand("sumFiveValues", a, &Adder::add, "Given five integers as input, sum them all.  This demonstrates bindMemberCommand() using an object");
    
    //bind the variable to the console
	console.bindCVar("health", health, "Player health.  Example variable bound from c++ code using bindCVar()");

    //populate the history buffer with commands from a previous run of the program
    console.loadHistoryBuffer("COMMAND_HISTORY.txt");

	while(running)
    {
        std::cout<<"\nexecute next instruction"<<std::endl;

        // you can pass in whatever istream and ostream you want
        console.commandExecute(std::cin, std::clog);
	}

    std::cout<<"Saving history file"<<std::endl;
    console.saveHistoryBuffer("COMMAND_HISTORY.txt"); //save commands from this run
}
