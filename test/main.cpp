#include "../GameConsole.h"
#include "../SimpleLog.h"

using namespace std::placeholders;
using namespace Virtuoso;

bool running = true;

template<class T>
T& primitiveAssign(T& l, const T& r){
	return l = r;
}

template<class TargetType,class ArgType>
TargetType& do_setter(TargetType* var_to_set,const ArgType& value)
{
	*var_to_set=value;
	return *var_to_set;
}

template<class TargetType,class ArgType>
std::function<void ()> setter(TargetType* var_to_set,ArgType val)
{
	return std::bind(&do_setter<TargetType,ArgType>,var_to_set,val);
}


void printSum(const int& a, const int& b){

    std::cout<<a+b<<std::endl;

}


class Adder{

std::string str;
public:

Adder(const std::string& pr):str(pr){}

void add(int a, int b, int c, int d, int e){
   // std::cout<<"Arg order is "<<a<<","<<b<<','<<c<<','<<d<<','<<e<<std::endl;
    std::cout<<str<<a+b+c+d+e<<std::endl;
}
};



int main()
{
    Adder a("Sum is "); //a class

	int health = 0; //a variable like we might have in a game

	SimpleLog log; //the log we need for console output

	GameConsole console(std::cin,log); //create the console

    //binding a template function with a particular value as a command

    std::function<void (void)> set_bool = std::bind(&primitiveAssign<bool>, std::ref(running), false);

	console.bindCommand("quit",set_bool, "quits the program");

    //binding a free function that takes 2 arguments as a command.

    console.bindCommand("sum", printSum, "sums two input integer values");

    // binding an instance method as a command.
    console.bindCommand("sumFiveValues", std::function<void(int,int, int, int, int)>(std::bind(&Adder::add,&a, _1, _2, _3,_4,_5)), "Given five integers as input, sum them all");

    //bind the variable to the console
	console.bindCVar("health", health, "player life");

    #ifdef HISTORY_FILE
        //populate the history buffer with commands from a previous run of the program
        console.loadHistoryFile("COMMAND_HISTORY.txt");
    #endif

	while(running){ //command loop
        std::cout<<"\nexecute next instruction"<<std::endl;
	 	console.commandExecute();
	}

	std::cout<<"Saving history file"<<std::endl;
    #ifdef HISTORY_FILE
        console.saveHistoryFile("COMMAND_HISTORY.txt"); //save commands from this run
    #endif

}
