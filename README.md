Virtuoso Console
by Christopher Pugh
======================
This code is a single file header only library containint the backend for a Quake-style console.  You can bind variables and functions from your C++ code with minimal application-side code,  and use them at runtime.  If nothing else, this code is a great workout for std::bind, std::function, and variadic templates.  

A graphical console widget using IMGUI is coming to this repo soon.

Look at the test programs in the "demos" folder, read the library headers, and this file to understand usage.


Creating a console
====================
It is as simple as declaring a variable of the GameConsole class.  

Virtuoso::QuakeStyleConsole console;


Reading and Executing Commands:
=================================
Simply call console.commandExecute(inputStream, outputStream) to read a line from the console's input stream and execute commands.

This is what you would call in a graphical console when the user hits enter.  Or you can just call it in a loop when you're using cin and cout in a terminal program for example.

The input can be any istream including std::cin. And the output can be any ostream, including cout or a file

	
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


CVars: 
========
Variables can be bound from your program to the console as a console variable.  They can then be printed, set, or dereferenced in console commands.  

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
You can add arbitrary C++ functions to your code by giving the console a function pointer or std::function object.
You can add member functions too.  The console automatically parses any argument list of any datatype that can be read from an istream, so you never have to write a parser.

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


If you want to bind a member function as a console command, you need to call console.bindMemberCommand() and pass the caller as an argument.  For example : 

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

... 

console.bindMemberCommand("sumFiveValues", a, &Adder::add, "Given five integers as input, sum them all.  This demonstrates bindMemberCommand() using an object");


History File: 
===============
 The console has a cache of previously used console commands.  There is also support for saving and loading this command buffer, so that multiple runs of the program have access to recently used commands.  

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


  
