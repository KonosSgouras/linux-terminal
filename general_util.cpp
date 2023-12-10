#include "general_util.h"
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
void printLine()
{
	// getting path and printing line with colors
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL)
	{
		std::cout << "\033[32;1mmyshell$\033[34m" << cwd << ">\033[0m";
	}
	else
	{
		perror("getcwd");
		*cwd = '\0';
		std::cout << "myshell$" << cwd << ">";
	}
}
void printCommmands(std::vector<com_instr> c)
{
	// looping through commands
	for (int i = 0; i < c.size(); i++)
	{
		printCommmand(c[i]);
	}
}
void printCommmand(com_instr c) // function for debug purposes
{
	std::cout << "\n-----------------------------\n";
	std::cout << "Is Pipe : " << c.is_pipe << "\n";
	std::cout << "Arguments\n";
	for (int i = 0; i < c.exec_command.arguments.size(); i++)
	{
		std::cout << c.exec_command.arguments[i] << "\n";
	}

	std::cout << "\nRedirects\n";
	for (int i = 0; i < c.exec_command.redirections.size(); i++)
	{
		std::cout << c.exec_command.redirections[i].type << " " << c.exec_command.redirections[i].file_name << "\n";
	}
	std::cout << "\nWildcard Ids\n";
	for (int i = 0; i < c.exec_command.wildcard_arguments.size(); i++)
	{
		std::cout << c.exec_command.wildcard_arguments[i] << "\n";
	}
	std::cout << "\n-----------------------------\n";
}
std::vector<com_instr> readLineString(std::string lines) // reading line as a sting
{
	// initializing variables
	std::vector<com_instr> commands;
	char c = ' ';
	std::string line = "";
	bool is_in_quotes = false;
	// std::cout << lines << "\n";
	// creating the loop for the string
	for (int i = 0; i < lines.length(); i++)
	{
		// detecting special characters
		if (lines[i] == '\n' || lines[i] == '|' || lines[i] == ';')
		{
			// std::cout << line << "\n";

			// finding coded instruction from string
			com_instr cur_inst;
			cur_inst.exec_command = commandFromString(line);
			cur_inst.is_pipe = false;
			// checking if it is pipe
			if (lines[i] == '|')
				cur_inst.is_pipe = true;
			// checking if last argument is a background operator()working if background operator & is after a space character
			if (!cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].compare("&"))
			{
				cur_inst.exec_command.arguments.pop_back();
				cur_inst.background = true;
			}
			else if (cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1][cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].length() - 1] == '&')
			{
				cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].pop_back();
				cur_inst.background = true;
			}
			else
			{
				cur_inst.background = false;
			}
			// pushing back command
			commands.push_back(cur_inst);
			line = "";
			continue;
		}
		// inserting in line
		line += lines[i];
	}
	// registering last command with the same process
	if (line.length() > 0)
	{
		// std::cout << line << "\n";

		com_instr cur_inst;
		cur_inst.exec_command = commandFromString(line);

		if (!cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].compare("&"))
		{
			cur_inst.exec_command.arguments.pop_back();
			cur_inst.background = true;
		}
		else if (cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1][cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].length() - 1] == '&')
		{
			cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].pop_back();
			cur_inst.background = true;
		}
		else
		{
			cur_inst.background = false;
		}

		commands.push_back(cur_inst);
		line = "";
	}
	return commands;
}
// reading line not from string
std::vector<com_instr> readLine(std::string *comma_s)
{
	// initializing variables
	std::vector<com_instr> commands;
	bool is_in_quotes = false;
	char c = ' ';
	(*comma_s) = "";
	// creating a loop for every character and starting scanning
	while (c != '\n')
	{
		std::string line = "";
		scanf("%c", &c);
		(*comma_s) += c;
		// checking the existance of special characters
		while ((c != '\n' && c != '|' && c != ';') || is_in_quotes == true)
		{
			line += c;
			scanf("%c", &c);
			(*comma_s) += c;
			// checking for quotes to create aliases. strings in quotes are considered a single argument despite of spaces
			if (c == '"')
			{
				is_in_quotes = !is_in_quotes;
			}
		}
		// checking last command with the same process
		if (line.size() != 0)
		{
			com_instr cur_inst;

			cur_inst.exec_command = commandFromString(line);

			cur_inst.is_pipe = false;

			if (c == '|')
				cur_inst.is_pipe = true;
			if (!cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].compare("&"))
			{
				cur_inst.exec_command.arguments.pop_back();
				cur_inst.background = true;
			}
			else if (cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1][cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].length() - 1] == '&')
			{
				cur_inst.exec_command.arguments[cur_inst.exec_command.arguments.size() - 1].pop_back();
				cur_inst.background = true;
			}
			else
			{
				cur_inst.background = false;
			}

			commands.push_back(cur_inst);
		}
	}
	return commands;
}
// decoding a command from string
comm commandFromString(std::string commandLine)
{
	std::string cur_word = "";
	comm cur_com;
	char c;
	bool is_wildcard = false;
	bool in_quotes = false;
	redirection_types redirection_of_next_word = NONE;
	// running through command
	for (int i = 0; i < commandLine.length(); i++)
	{
		c = commandLine[i];
		// std::cout << c << " " << redirection_of_next_word << "\n";
		if (c == ' ' && in_quotes == false) // detecting blanks
		{
			if (cur_word.compare("") != 0) // checking if empty and doing a process that is used in the future (redirections) with slight modifications
			{
				// checking if it is a redirection word
				if (redirection_of_next_word == NONE)
				{
					// putting arg in arguments
					cur_com.arguments.push_back(cur_word);
					cur_word = "";
					is_wildcard = false;
					redirection_of_next_word = NONE;
					continue;
				}
				else
				{
					// putting word in redirections
					redirection re;
					re.file_name = cur_word;
					re.type = redirection_of_next_word;
					cur_word = "";
					redirection_of_next_word = NONE;
					cur_com.redirections.push_back(re);
					continue;
				}
			}
			continue;
		}
		else if (c == '>' && in_quotes == false)
		{

			if (cur_word.compare("") != 0) // same process
			{
				if (redirection_of_next_word == NONE)
				{
					is_wildcard = false;
					cur_com.arguments.push_back(cur_word);
					cur_word = "";
					redirection_of_next_word = NONE;
				}
				else
				{
					redirection re;
					re.file_name = cur_word;
					re.type = redirection_of_next_word;
					cur_word = "";
					redirection_of_next_word = NONE;

					cur_com.redirections.push_back(re);
				}
			}
			// changing redirections
			if (redirection_of_next_word == OUTPUT)
			{
				redirection_of_next_word = CONCAT_OUTPUT;
			}
			else
			{
				redirection_of_next_word = OUTPUT;
			}
			continue;
		}
		else if (c == '<' && in_quotes == false)
		{

			if (cur_word.compare("") != 0) // same process
			{
				if (redirection_of_next_word == NONE)
				{
					cur_com.arguments.push_back(cur_word);
					cur_word = "";
					redirection_of_next_word = NONE;
					is_wildcard = false;
				}

				else
				{
					redirection re;
					re.file_name = cur_word;
					re.type = redirection_of_next_word;
					cur_word = "";
					redirection_of_next_word = NONE;
					cur_com.redirections.push_back(re);
				}
			}
			redirection_of_next_word = INPUT;
			continue;
		}
		else if (c == '"')
		{
			if (in_quotes == true)
			{
				in_quotes = false;
				if (cur_word.compare("") != 0) // same process
				{
					if (redirection_of_next_word == NONE)
					{
						cur_com.arguments.push_back(cur_word);
						cur_word = "";
						is_wildcard = false;
						redirection_of_next_word = NONE;
						continue;
					}
					else
					{
						redirection re;
						re.file_name = cur_word;
						re.type = redirection_of_next_word;
						cur_word = "";
						redirection_of_next_word = NONE;
						cur_com.redirections.push_back(re);
						continue;
					}
				}
				continue;
			}
			else
			{
				in_quotes = true;
				if (cur_word.compare("") != 0) // same process
				{
					if (redirection_of_next_word == NONE)
					{
						cur_com.arguments.push_back(cur_word);
						cur_word = "";
						is_wildcard = false;
						redirection_of_next_word = NONE;
						continue;
					}
					else
					{
						redirection re;
						re.file_name = cur_word;
						re.type = redirection_of_next_word;
						cur_word = "";
						redirection_of_next_word = NONE;
						cur_com.redirections.push_back(re);
						continue;
					}
				}
				continue;
			}
			// checking the existance of system calls
		}
		if ((c == '*' || c == '?') && is_wildcard == false)
		{
			// marking the existance of wildcard arguments
			cur_com.wildcard_arguments.push_back(cur_com.arguments.size());
			is_wildcard = true;
		}
		cur_word += c;
	}

	if (cur_word.compare("") != 0) // same process
	{
		if (redirection_of_next_word == NONE)
		{
			cur_com.arguments.push_back(cur_word);
		}
		else
		{
			redirection re;
			re.file_name = cur_word;
			re.type = redirection_of_next_word;
			cur_com.redirections.push_back(re);
		}
	}

	return cur_com;
}