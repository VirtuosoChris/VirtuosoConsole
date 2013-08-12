Virtuoso Console
by Christopher Pugh
======================
This code is the backend for a Quake-style console.  You can bind variables and functions from your C++ code with minimal application-side code,  and use them at runtime.  If nothing else, this code is a great workout for std::bind, std::function, and variadic templates.  

The frontend (GUI) is in a separate library to be released.  

A point-by-point explanation of the console's features is contained below.  When you are done reading this file the first thing you should do is to build and run the test program included.  Try playing with the various commands, especially "help", "listCmd", and "listCVars"  In the included CMake configuration, this is build target ConsoleTest.  Then you should take a look at test/main.cpp to see a good example of code that uses the console.  

Note that all classes and functions are in the Virtuoso namespace, so you should do 

	using namespace Virtuoso; 

in your code, or prefix everything with Virtuoso:: to resolve the namespace.  


Creating a console
====================
It is as simple as declaring a variable of the GameConsole class.  You need an istream and a Virtuoso::BaseLog for construction.

GameConsole console(std::cin,log);

The input can be any istream including std::cin.  

A Virtuoso::BaseLog is a pure virtual class that has several pstreams that can be written to.   They are each accessed by a particular virtual function. 

For instance:
log.error()<<"Error";
log.status()<<"general message";
log.warning()<<"you're doing something you shouldn't!";

This is useful to direct different kinds of messages to different places, or for a graphical console to adjust text formatting based on the type of message. 

Since the BaseLog class is pure virtual you have one of two options: 
1) Overload the BaseLog with the appropriate functions, based on the needs of your application
2) Use Virtuoso::SimpleLog.

The second option is really easy.  It simply uses a single output stream for all write operations.  The default constructor for this class points the ostream to std::cout.  Thus, if you want to test the console with standard io, the code is simply: 

SimpleLog log;
GameConsole console(std::cin, log);

SimpleLog allows you to construct an instance with a reference to an ostream, or to change the stream later on with the setStream method.  

You can change the input stream and log pointers of the console later with the following methods: 
	void setConsoleInput(std::istream& in); ///setter for console istream
    void setConsoleOutput(BaseLog& out); ///setter for console output log
	
You can get a reference to the console's log or istream with the methods:

	BaseLog& output(); ///log getter
    std::istream& input(); ///input stream getter

	
Built in commands: 
===================
The following commands built in to every instance of the GameConsole class

echo: prints the value of a variable

help:  With no arguments, prints a general help string on how to use the console. 
With an argument string naming a variable or a command, prints the help string for that variable or command.  

listCVars : list all variables bound to the console, including dynamically created ones

listCmd : lists all commands bound to the console. 

listHelp : lists all the commands and CVars that have an available help string for the "help" command.  

runFile: runs commands in a text file named by the argument. 
Example: runFile "Game.ini"

set: assigns a value to a variable.  Uses istream operator >> to parse.  

var: declare a variable dynamically.  

	
	
Writing to Console:
=========================
You can directly write to your log instance using the virtual functions, or the () operator. 
log()<<"Something";//writes to log.status()
log.error()<<"error";

Or, you can access the input through the console: 
console.output().status()<<"something\n";



CVars: 
========
Variables can be bound from your program to the console as a console variable ("cVar" as a shorthand).  They can then be printed, set, or dereferenced in console commands.  

You can bind any arbitrary variable in your C++ code to the console, allowing it to be printed, set, or dereferenced in console commands.  The only requirement is that the datatype of the variable in C++ code has the >> and << operators overloaded for iostreams.

For instance, in your code if you have the variable 
int health;

You can call:
console.bindCVar("health", health, "player life");

The first argument is the name of the variable in the console.  The second argument is the reference to the actual variable in your C++ code.  The third string is optional, and is the help string associated with the variable.  If the user were to type 

"help health" 

into the console, the help string would be printed to the console's output stream. 

Variable names should not contain whitespace, since whitespace is a delimiter during parsing.  


Dynamic Variables
==================
You can also create variables using the console directly, rather than binding variables from your C++ code.
Dynamic variables are all implicitly of type string.  
They can be echo'ed, set, and dereferenced like any other variable. 

Example: 
var x hello world

This creates a variable called 'x'.  After the whitespace delimiter, the rest of the input line is assigned as the value of the variable.  

Thus, "echo x" will print "hello world"


Using Variables
=================
--Variables can be dereferenced into a command by prefixing the variable name with $.

For instance:

	var temp 10
	set health $temp

--Variables can be printed with the echo command.

	echo health
	>10

--Variables can be set with the set command

	set health 200
	echo health
	>200


Commands: 
=========
You can add arbitrary C++ functions to your code by giving the console a std::function object.  
The functions should return void, but they may take any argument type.  The requirement is that the variable types used as arguments have the >> operator overloaded for istream input.  They must also all have a default constructor.  Code will be automatically generated that parses all the arguments to the function from the input stream and pass them in for you.  

You use the bindCommand method on the console like the following example: 
	
	//function we want to bind as a console command
	void printSum(const int& a, const int& b){

		std::cout<<a+b<<std::endl;

	}

	//....
	
   console.bindCommand("sum", printSum);
  
An optional third argument gives the help string for the command.  Once the command is bound to the console, you simply type the name of the command followed by any arguments.  For instance, 
	
	sum 1 1

prints 2 when it is called from the demo program.	

Note that since printSum is a free C++ function rather than an instance method, a std::function object did not explicitly need to be created.  

You can add commands that are methods called on instance variables of a class as well.  See the documentation of std::function and std::bind for more information on this.  Also see the implementation of the "sumFiveValues" command in test/main.cpp using the Adder class.  All you really need is a C style function pointer or a std::function object that returns void and the library takes care of the rest for you.  



Reading and Executing Commands:
=================================
Simply call console.commandExecute() to read a line from the console's input stream and execute commands.  
This is what you would call in a graphical console when the user hits enter.  Or you can just call it in a loop when you're using cin and cout.  



History File: 
===============
The CMake configuration will try and find Boost.  If it is found, the circular buffer container will be included, and HISTORY_FILE will be defined.  This enables support for a cache of previously used console commands to be created.  There is also support for saving and loading this command buffer, so that multiple runs of the program have access to recently used commands.  

At the beginning of your program you would call:   
console.loadHistoryFile("COMMAND_HISTORY.txt");

At the end of your program you would call:
console.saveHistoryFile("COMMAND_HISTORY.txt");

You can also give these functions iostream references instead of strings containing file names.  


Comments
===========
The '#' character at the beginning of a line causes the line to be regarded as a comment and ignored for execution. 
This is useful for ini files and the like.  


Misc
======
You can call the setHelpTopic method on a console instance to change the help string for a command or cVar at any time, even if you don't specify one at creation time.  

For instance, 
console.setHelpTopic("health", "the player's health");


  