#include <iostream>
#include <vector>
#include <string>

// defininig history length
#define HISTORY_LENGTH 20
// creating typedefs for ease of use
typedef struct command comm;
typedef struct redirection_s redirection;
typedef struct command_with_instruction com_instr;
typedef struct alias_s alias;
typedef struct history_s history;

// initiating history struct
struct history_s
{
	std::string com_string;				 // line executed as written
	std::vector<com_instr> instructions; // list of decoted instructions
	bool is_history;					 // bool for knowing whether or not the instruction was executed by history
};
// Initializing alias struct
struct alias_s
{
	std::string name;					 // alias name
	std::string com_string;				 // line executed by alias as written
	std::vector<com_instr> instructions; // instructions executed by alias as decoded
};
// enum showing what are the possible redirection types
enum redirection_types
{
	NONE,
	INPUT,
	OUTPUT,
	CONCAT_OUTPUT
};
// initializing redirecton struct
struct redirection_s
{
	std::string file_name;	// file name that concatination is about
	redirection_types type; // type of redirection
};
// initializing command struct
struct command
{
	std::vector<std::string> arguments;	   // list of the arguments as strings
	std::vector<redirection> redirections; // list of all redirections
	std::vector<int> wildcard_arguments;   // list of wildcard arguments (their indexes in the argument vector)
};
// initializing a struct for a coomand and its assosiations with other commands
struct command_with_instruction
{
	bool is_pipe;			// whether there is a pipe after command
	bool background;		// whether it is run on the background
	comm exec_command;		// instructon to be executed
	std::string com_string; // line asstring
};
void printLine();									// printig line before new commands
std::vector<com_instr> readLine(std::string *);		// reading command's lines
std::vector<com_instr> readLineString(std::string); // reading command's lines from string
void printCommmands(std::vector<com_instr>);		// debuging function for printing commands
void printCommmand(com_instr);						// debuging instruction for printing command
comm commandFromString(std::string commandLine);	// creating a struct command from string