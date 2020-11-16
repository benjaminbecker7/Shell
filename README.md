# Project 04: Shell

## Overview
For this project, you will implement a simple shell. The shell should operate in the following basic way: when you type in a command (in response to the shell prompt), the shell creates a child process that executes the command that was entered, then prompts for more user input when finished. The shell you implement will be similar to one that you use every day in Linux (e.g., `bash`), but with many simplifications. 

### Learning objectives
After completing this project, you should be able to:
* Write programs that create and manage processes using the UNIX process API

### Important tips
* Read the entirety of the project description before you write any code.
* Work on the project over multiple sessions. Do not try to complete the project in a single session on the day (before) the project is due. This will give you time to think about how to solve problems, allow you to ask questions, and result in better outcomes.
* Ensure you follow good program design, coding, testing, and debugging practices (details below).

## Getting started 
Clone your Project 4 repository on a **birds** server. 

To compile the shell, run: `make` 

To start the shell, run: `./shell`

Your shell will run in an interactive fashion. It will display a prompt (`shell>`) and wait for the user to enter a command. The user will enter the program to execute (e.g., `/bin/ls`) or a "built-in" command (e.g., `exit`). Non-built-in commands should contain the full path to the executable file (e.g., `/bin/ls` rather than just `ls`) and may contain an arbitrary number of command-line options (e.g., `/bin/ls -l -r`).  If the user enters a non-built-in command, a new process will be created and the specified program will be started. The shell will wait for the program to finish, then display the prompt and allow the user to enter another command.

## Part 1: running commands
Your first task is to execute commands entered by the user.

### Starter code
The main function in `main.c` includes code for:
1. Displaying the prompt (`shell>`)
2. Reading a line of user input into the char array `buffer`
3. Breaking the command into tokens, where tokens are delimited by one or more whitespace characters

### Non-built-in commands
When a user enters a non-built-in command (e.g., `/bin/ls -l`), your shell should create a new process and invoke the specified executable program in the new process. The shell should wait for the process to complete before prompting for a new command. (Hint: use `waitpid` instead of `wait`, otherwise you'll run into problems when you complete part 2.)

Your code should gracefuly handle the situation where invoking the specified executable fails.

### Built-in commands
There is one built-in command you need to handle for this part of the project: `exit`.  To quit the shell, the user can type `exit`. The effect should be to quit the shell. (The `exit()` system call may be useful here.)

Note that built-in commands must be handled differently than other commands; they are **not to be executed in a separate process** like `/bin/ls` or other programs invoked through the shell.  

If the command is "empty" (i.e., all spaces) you should just ignore it.

### Defensive programming
Defensive programming is an important concept in computer systems: a system can't simply fail when it encounters an error; it must check all parameters before it trusts them. In general, there should be no circumstances in which your C program will core dump (crash), hang indefinitely, or prematurely terminate. Therefore, your program must respond to all input in a reasonable manner; "reasonable" means print an understandable error message and either continue processing or exit, depending upon the situation. For example, if a command that a user types does not exist, you should print an error message to the user and continue processing any further commands.

Moreover, your program should not have any memory leaks or any memory corruption problems of any kind.  Be sure to test your shell using valgrind to check for memory-related problems.

## Part 2: background commands
Your second task is add the capability to run processes in the "background". That is, when you start a command, you should be able to type a new command in the shell immediately, without waiting for the child process to finish.

### Identifying background commands
You will indicate that a command should be executed as a background process by including the ampersand (`&`) character at the end of the command. For simplicity, you can assume the ampersand will always be a separate token. For example:
```
shell> /bin/ls -l -r &
```

Only non-built-in commands can be run in the background.

### Running a command in the background
When a command is run in the background:
* You should **not** immediately wait for it to finish.
* You should store the child process's pid in an array of active background process. You can assume a fixed upper bound on the number of active background processes, as defined by the constant `MAX_BACKGROUND`.
* Each time through the main loop of your shell (i.e., when you are printing the prompt and getting the next command), you should scan the list of background processes. For each process on the list, do the following:
	* Check whether it has completed using the `waitpid` system call, which takes three parameters: the process id, a pointer to an int (which will be the exit status of the child process), and an integer flag.  For the flag value you should use `WNOHANG`, which tells `waitpid` not to "hang" or wait if the process is not finished.  If the child is done, `waitpid` will return the child's pid.  If the child is not yet done, `waitpid` will return `0`.  If there's an error with `waitpid`, it will return `-1`.  (See `man 2 waitpid` for more details.)
	* For any child processes that have finished, you should:
		* Print `PID finished with exit code STATUS`, replacing `PID` with the pid of the process that finished and `STATUS` with the exit status of the completed process.
		* Remove the pid from the array of active background processes
	* Since this process array scanning will only happen when you go through the main loop, you might need to hit return on an empty command line for your shell to figure out that a child process has finished.

### Built-in commands
You'll need to implement two new shell built-in commands:
* `jobs`

	When this built-in command is entered the shell should print a list of processes that are currently running in the background. For example:
	```
	shell> jobs
	Process currently active: 13346, 13347
	```

* `kill pid` (where pid is some integer)

	When this built-in command is entered, you should check whether a child process with the given pid is running. If a child is running with that pid, you should issue the `kill` system call to kill the child process.  You should use `SIGKILL` for the `sig` parameter, which will kill the child process definitively. See `man 2 kill` to learn more.

### Testing your code
You can use the command `/bin/sleep SECONDS` (replacing `SECONDS` with an integer) to help you test your handling of background processes and the built-in commands `jobs` and `kill`.

## Program design

You **must follow good program design and coding practices**, including:
* Checking the return values of all system calls (excluding `printf`) --- This will often catch errors in how you are inviking these systems calls.
* Using multiple functions --- Do not put all of your code in the `mian` function. You should use multiple functions, where each function: is Short, does One thing, takes Few parameters, and maintains a single level of Abstraction. In other words, follow the _SOFA_ criteria from COSC 101.
* Freeing all heap-allocated memory before exiting --- Run `valgrind` to confirm all memory is freed and no other memory errors exist in your program.
* Properly indenting your code --- Recall that indentation is not semantically significant in C, but it makes your code much easier to read.
* Including comments --- If you write more than a few lines of code in the body of a function, then you must include comments; generally, you should include a comment before each conditional statement, loop, and set of statements that perform some calculation. **Do not** include a comment for every line of code, and **do not** simply restate the code.
* Making multiple commits to your git repository --- Do not wait until your entire program is working before you commit it to your git repository. You should commit your code each time you write and debug a piece of functionality.

## Challenge problem
Challenge problems in projects are an opportunity to earn a small amount of extra credit toward your grade for _this_ project.

The challenge problem for this project is to add support for output redirection. In other words, if the user includes a greater than symbol `>` and a filename at the end of the command, then the output of the command is put in the file instead of displayed on the screen. To accomplish this, you'll need to replace the `stdout` file descriptor with a pointer to the output file after you call `fork` but before you call `execv`. (See `man stdout` and `man fopen` for details.)

## Submission instructions
You should **commit and push** your updated files to your git repository. However, as noted above, do not wait until your entire program is working before you commit it to your git repository; you should commit your code each time you write and debug a piece of functionality. 
