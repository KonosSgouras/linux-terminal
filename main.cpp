#include "syscall_util.h"
#include <signal.h>
#include <stdio.h>
int main(int argc, char **argv)
{
	// Creating signal overrides for keyboard interupt and keyboard stop
	signal(SIGINT, interruptOverride);
	signal(SIGTSTP, stoppedOverride);

	// creating lists for aliases and history
	std::vector<alias> aliases;
	std::vector<history> history_inst;
	// initializing history
	for (int i = 0; i < HISTORY_LENGTH; i++)
	{
		history h;
		h.com_string = "NO INSTRUCTION\n";
		std::vector<com_instr> instr;
		h.instructions = instr;
		h.is_history = false;
		history_inst.push_back(h);
	}
	// infinite while loop for handling command
	while (1)
	{
		printLine(); // printing line

		// reading line
		std::string command = "";
		std::vector<com_instr> cur = readLine(&command);

		// printCommmands(cur); // for debugging

		execCommands(cur, &aliases, &history_inst, command); // executing commands
	}

	return 0;
}