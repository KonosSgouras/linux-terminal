#include "general_util.h"
#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define NO_PIPE -1

int execCommand(com_instr command, int prev_file_pointer, std::vector<alias> *aliases, std::vector<history> *history_inst); // executing a single command
void execCommands(std::vector<com_instr> c, std::vector<alias> *aliases, std::vector<history> *history_inst, std::string);  // executing alist of commands

comm validateWildcards(comm);                                                                       // reorginizing a command to replace wildcards
int recWildcardMatch(std::string wildcard, int wildcard_id, std::string argument, int argument_id); // recursive function find wildcard matches

bool isCustomProgram(com_instr command, std::vector<alias> *aliases, std::vector<history> *history_inst); // a function that checks if it is a custom program
alias createAlias(com_instr command);                                                                     // custom program for creating an alias
void deleteAlias(com_instr command, std::vector<alias> *aliases);                                         // custom program for deleting alias
void handleHistory(com_instr command, std::vector<history> *history_inst, std::vector<alias> *aliases);   // custom program for handling history
void handleCd(com_instr command);                                                                         // handling cd custom program

void interruptOverride(int); // interrupt Override
void stoppedOverride(int);   // stopped Override