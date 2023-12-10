#include "syscall_util.h"
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/types.h>
int current_child_pid = -1;

void interruptOverride(int s) // creating the override function for the interupt signal
{
	signal(SIGINT, interruptOverride); // recreating override

	if (current_child_pid != -1)
	{
		kill(current_child_pid, SIGINT); // sending signal
	}
}
void stoppedOverride(int s) // creating the override function for the stopped signal
{
	signal(SIGTSTP, stoppedOverride); // recreating override
	if (current_child_pid != -1)
	{
		kill(current_child_pid, SIGSTOP); // sending signal
	}
}
bool isCustomProgram(com_instr command, std::vector<alias> *aliases, std::vector<history> *history_inst)
{
	// checking if it is create alias
	if (command.exec_command.arguments[0].compare("createalias") == 0)
	{

		alias c = createAlias(command);
		if (c.name.compare("") != 0) // creating alias and registering it
		{
			aliases->push_back(c);
		}
		return true;
	}
	// checking if it is dextroy alias funcction
	if (command.exec_command.arguments[0].compare("destroyalias") == 0)
	{
		deleteAlias(command, aliases);
		return 1;
	}
	// exit function added for exiting the shell
	if (command.exec_command.arguments[0].compare("exit") == 0)
	{
		std::cout << "Bye\n";
		exit(0);
	}
	// history command detected
	if (command.exec_command.arguments[0].compare("myHistory") == 0)
	{
		handleHistory(command, history_inst, aliases);
		return 1;
	}
	// implementig simple cd functionality for better execution
	if (command.exec_command.arguments[0].compare("cd") == 0)
	{
		handleCd(command);
		return 1;
	}
	// creating loop for checking whether ther is an alias to be executed
	for (int i = 0; i < aliases->size(); i++)
	{
		if ((*aliases)[i].name.compare(command.exec_command.arguments[0].c_str()) == 0)
		{
			// executing the alias with the correct
			std::cout << (*aliases)[i].com_string << "\n";
			execCommands((*aliases)[i].instructions, aliases, NULL, "");
			return true;
		}
	}
	return false;
}
// function for handling cd
void handleCd(com_instr command)
{

	if (command.exec_command.arguments.size() > 1)
	{
		// changing directory
		if (chdir(command.exec_command.arguments[1].c_str()))
		{
			perror("chdir");
		}
	}
}
// function for handling history
void handleHistory(com_instr command, std::vector<history> *history_inst, std::vector<alias> *aliases)
{

	if (command.exec_command.arguments.size() == 1)
	{
		// printing history
		for (int i = 0; i < history_inst->size(); i++)
		{
			std::cout << history_inst->size() - i << " : " << (*history_inst)[i].com_string;
		}
		(*history_inst)[HISTORY_LENGTH - 1].is_history = true;
	}
	else
	{
		// getting history to be reexecuted
		int numb = std::stoi(command.exec_command.arguments[1]);
		// std::cout << numb << "\n";
		if (numb > 0 && numb <= HISTORY_LENGTH)
		{
			// executing history
			//  std::cout << (*history_inst)[HISTORY_LENGTH - numb].com_string << "\n";
			execCommands((*history_inst)[HISTORY_LENGTH - numb].instructions, aliases, NULL, (*history_inst)[HISTORY_LENGTH - numb].com_string);

			history h = (*history_inst)[HISTORY_LENGTH - numb];
			history_inst->erase(history_inst->begin() + (HISTORY_LENGTH - numb));
			h.is_history = true;
			history_inst->push_back(h);
		}
		else
		{
			std::cout << "Invalid number of history\n";
		}
	}
}
// function for handling alias
alias createAlias(com_instr command)
{
	alias a;
	a.name = "";
	if (command.exec_command.arguments.size() < 3)
	{
		return a;
	}
	// creating alias struct
	a.name = command.exec_command.arguments[1];
	a.instructions = readLineString(command.exec_command.arguments[2]);
	a.com_string = command.exec_command.arguments[2];
	// printCommmands(a.instructions);
	return a;
}
// deleting aliases
void deleteAlias(com_instr command, std::vector<alias> *aliases)
{

	if (command.exec_command.arguments.size() < 2)
	{
		return;
	}
	// finding alias if it exists and erasing it
	for (int i = 0; i < aliases->size(); i++)
	{
		if (command.exec_command.arguments[1] == (*aliases)[i].name)
		{
			aliases->erase(aliases->begin() + i);
			return;
		}
	}
	return;
}
// recursive function that checks if a file name corresponds to the
int recWildcardMatch(std::string wildcard, int wildcard_id, std::string argument, int argument_id)
{
	if (wildcard_id == wildcard.size())
	{
		if (argument_id == argument.size())
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else if (wildcard[wildcard_id] == '*')
	{
		for (; argument_id != argument.size(); argument_id++)
		{
			if (recWildcardMatch(wildcard, wildcard_id + 1, argument, argument_id))
			{
				return 1;
			}
		}
		return recWildcardMatch(wildcard, wildcard_id + 1, argument, argument_id);
	}
	else if (wildcard[wildcard_id] == argument[argument_id] || wildcard[wildcard_id] == '?')
	{
		return recWildcardMatch(wildcard, wildcard_id + 1, argument, argument_id + 1);
	}
	else
	{
		return 0;
	}
}
// replacing wildcards
comm validateWildcards(comm c)
{
	std::vector<std::string> file_names;

	struct dirent *cur_file;
	DIR *cur_directory;
	// finding all possible words in directory
	cur_directory = opendir(".");
	if (cur_directory)
	{
		while ((cur_file = readdir(cur_directory)) != NULL)
		{
			std::string s = cur_file->d_name;
			file_names.push_back(s);
		}
		closedir(cur_directory);
	}
	// checking for every word if it matches with the  given wildcard
	for (int i = c.wildcard_arguments.size() - 1; i >= 0; i--)
	{
		std::vector<std::string> available_files;
		for (int a = 0; a < file_names.size(); a++)
		{

			if (recWildcardMatch(c.arguments[c.wildcard_arguments[i]], 0, file_names[a], 0))
			{
				// std::cout << c.arguments[c.wildcard_arguments[i]] << " " << file_names[a] << "\n";
				available_files.push_back(file_names[a]);
			}
		}
		// replacing the arguments
		if (available_files.size() > 0)
		{
			c.arguments.erase(c.arguments.begin() + c.wildcard_arguments[i]);
			for (int a = 0; a < available_files.size(); a++)
			{
				c.arguments.insert(c.arguments.begin() + c.wildcard_arguments[i] + a, available_files[a]);
			}
		}
	}
	return c;
}
// function for executing a list of commands
void execCommands(std::vector<com_instr> c, std::vector<alias> *aliases, std::vector<history> *history_inst, std::string com_string)
{

	bool has_command = false;
	int file_pointer_of_next_command = NO_PIPE;

	if (c.size() > 1)
	{
		c[c.size() - 1].is_pipe = false;
	}
	// executing its command and passing pipe fd in the next exec command
	for (int i = 0; i < c.size(); i++)
	{
		if (c[i].exec_command.arguments.size() > 0)
		{
			has_command = true;
		}
		file_pointer_of_next_command = execCommand(c[i], file_pointer_of_next_command, aliases, history_inst);
	}
	if (has_command && history_inst != NULL && com_string.compare("") != 0)
	{
		if ((*history_inst)[HISTORY_LENGTH - 1].is_history == false)
		{
			history_inst->erase(history_inst->begin());
			history h;
			h.com_string = com_string;
			h.instructions = c;
			h.is_history = false;
			history_inst->push_back(h);
		}
		else
		{
			(*history_inst)[HISTORY_LENGTH - 1].is_history = false;
		}
	}
}
int execCommand(com_instr command, int prev_file_pointer, std::vector<alias> *aliases, std::vector<history> *history_inst)
{
	int fpn = NO_PIPE;
	if (command.exec_command.arguments.size() < 1)
	{
		return fpn;
	}

	if (isCustomProgram(command, aliases, history_inst))
	{
		return fpn;
	}
	else
	{
		command.exec_command = validateWildcards(command.exec_command);
		char *argv[command.exec_command.arguments.size() + 1];
		argv[command.exec_command.arguments.size()] = NULL;
		for (int i = 0; i < command.exec_command.arguments.size(); i++)
		{
			argv[i] = (char *)malloc(command.exec_command.arguments[i].length());
			strcpy(argv[i], command.exec_command.arguments[i].c_str());
		}
		pid_t pid;
		int fd[2];
		if (command.is_pipe)
		{
			if (pipe(fd) == -1)
			{
				perror(" pipe ");
				exit(1);
			}
			fpn = fd[READ];
		}

		// int fd[command.redirections.size()][2];
		// for (int i = 0; i < command.redirections.size(); i++)
		// {
		// 	if (pipe(fd[i]) == -1)
		// 	{
		// 		perror(" pipe ");
		// 		exit(1);
		// 	}
		// }

		pid = fork();
		if (pid == -1)
		{
			perror("fork");
			exit(1);
		}

		if (pid != 0)
		{
			if (command.is_pipe)
			{
				close(fd[WRITE]);
			}
			if (prev_file_pointer != NO_PIPE)
			{
				close(prev_file_pointer);
			}
			if (!command.is_pipe && !command.background)
			{
				current_child_pid = pid;
				waitpid(pid, NULL, 0);
				current_child_pid = -1;
			}
		}
		else
		{
			if (command.is_pipe)
			{
				close(fd[READ]);
				dup2(fd[WRITE], 1);
				close(fd[WRITE]);
			}
			if (prev_file_pointer != NO_PIPE)
			{
				dup2(prev_file_pointer, 0);
				close(prev_file_pointer);
			}
			// Redirections take precedence
			for (int i = 0; i < command.exec_command.redirections.size(); i++)
			{
				if (command.exec_command.redirections[i].type == OUTPUT)
				{
					int filedes;
					if ((filedes = open(command.exec_command.redirections[i].file_name.c_str(), O_CREAT | O_WRONLY, 0644)) == -1)
					{
						perror("opening redirection file");
						exit(1);
					}
					dup2(filedes, STDOUT);
					close(filedes);
				}
				else if (command.exec_command.redirections[i].type == INPUT)
				{
					int filedes;
					if ((filedes = open(command.exec_command.redirections[i].file_name.c_str(), O_CREAT | O_RDONLY, 0644)) == -1)
					{
						perror("opening redirection file");
						exit(1);
					}
					dup2(filedes, STDIN);
					close(filedes);
				}
				else if (command.exec_command.redirections[i].type == CONCAT_OUTPUT)
				{
					int filedes;
					if ((filedes = open(command.exec_command.redirections[i].file_name.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644)) == -1)
					{
						perror("opening redirection file");
						exit(1);
					}
					dup2(filedes, STDOUT);
					close(filedes);
				}
			}
			int ret = execv(argv[0], argv);
			/*if (ret == -1)
			{
				perror("exec");
			}*/

			char *fpath = (char *)malloc(command.exec_command.arguments[0].length() + 5);
			strcpy(fpath, "/bin/");
			strcat(fpath, argv[0]);

			ret = execv(fpath, argv);
			char *fpath2 = (char *)malloc(command.exec_command.arguments[0].length() + 10);
			strcpy(fpath2, "/usr/bin/");
			strcat(fpath2, argv[0]);
			ret = execv(fpath2, argv);
			if (ret == -1)
			{
				perror("exec");
				exit(1);
			}

			exit(1);
		}
	}

	return fpn;
}