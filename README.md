# MiniShell
mini shell pipes, ampersand, and nuhup.<br>
This project was written as a part of the operation systems course at Azrieli's College.<br>

## Description

The program gets a sentence from the user and turns it into an array of commands, intending to fork the process
and send the son to perform the command from the user.<br><br>

The mini shell accepts "simple commands" that already exist on other files in the system,
due to that cause commands like: "cd" that change the current path of the shell could not perform.<br>
in addition, commands that use direct interrupts from the user (such as ctrl+c, ESC, ext...) will not
perform either.  <br>
 <br>
In addition, there is an option to concatenate commands through pipes using: <command1> | <command2>.<br>
the program can support up to 2 pipes only!<br><br>

The user also can perform '&' at the end of the sentence to send the process to run in the background,
or using 'nohup' at the beginning of the sentence, which will make the output appear in txt file followed by the name
"nohup.txt", in this state the command that already running will be not affected by the SIGHUP signal.<br><br>

The program manages a history file that records all the data the program gets from the user.<br>
The user can also use 2(3) following function from the console:<br>
<ol>
	<li> 'history' = prints the data recorded by the program.</li>
	<li>'done' = finishing the running of the program and printing a suitable massage.</li>
	<li>'![int num] = activate the 'num' command in the history. (can be performed only at part b!)</li>
</ol>

#### In this program, we use:
<ol>
	<li> an array of chars in size 512 (str), that array holding the current sentence given by the user.</li>
	<li> an array of chars in size 9 (first_word), that array helps the program to distinguish between user data 
	   to user command - the array uses minimal size (needed to distinguish between: 'history/0' to 'history.../0'.)</li>
	<li> pointer from type char ** (commands) that will be allocated dynamically in running time, and contains the 
	  user input after the portion to the words.</li>
	<li> two counters type 'int' to count the number of pipes the user uses, and the number of commands.</li>
	<li> two flags type 'bool' to mark the situations of background or nohup.</li>
	<li> one pid_t variable that holds the PID of the main process, which helps to distinguish between the main process and the 
	    other processes.</li>
</ol>

#### functions:
There are seven main functions:
<ul>
 	<li> sentence_parser - This method takes the sentence and counts the words and the chars the sentence contains,
				In addition, the method updates the value of the first_word variable to the first 8 characters
				of the first word of the sentence.</li>
	<li> command_string_creator - This function accepts the string and the size of the array, The output of this method
					 is allocate and create a dynamic array of strings that will contain all the commands.</li>
	<li> command_executer -  This method accepts the dynamic commands array, forks the process, and sends the son to 
					perform the command by the command 'execvp'.</li>
	<li> executer_for_pipes - This method takes care of doing the execution if there are pipes.</li>
	<li> command_array_freer -  This method has one very important purpose - to free the memory of the commands array.</li>
	<li> pipe_code - this is a critical function that combines calls to several different important functions, and contains all the code of pipes treatment.
			at the end of these functions, there will be 2/3 (according to num of pipes) processing that concat to each other
			running on the code, where every one of them holds a different string in str.</li>
	<li> pipes_and_forks - This function is really critical and it is the core of pipe functionality. The function is responsible of duplicate the process 
			two or three times depending on the number of pipes there is, and create the pipes that the processes will be concat
			 each output to other input with them.</li>
</ul>
In addition, there are several more helping functions:
<ul>
	<li> starting - This function simply opens the history file and closes it for the first time - for the purpose of 
			    preventing the program to collapse. 	</li>
	<li> read_history - This function reads the history from 'file.txt' line by line and shows the output in the console.</li>
	<li> update_history - This method intends to update the history file (file.txt) when it needed.</li>
	<li> user_buffer - This method is responsible to print the path that is shown in the terminal on the screen,
				in addition, it accepts a line from the user and changes the last character from '\n' to '\0'.</li>
	<li> checking_pipe - this function only counts the number of pipes (that no belongs to the user string in double quotes), and returns this value;</li>
	<li> string_split - This method aims to split the str string by two/three according to the pipe that appears in it, and taking care of creating 
				the new string without spaces.</li>
	<li> split_by_pipe_func - helps to string_split to split the strings. </li>
	<li> history_in_pipe - This function takes care of the appearance of history commands in pipes, The function builds the whole string after the 
			replace the command from the history.</li>
	<li> sig_child_handler - This function is handled with a child signal (number 17).</li>
	<li> nohup_string_changer - This method helps to remove "nohup" from str in no hup cases.</li>
	<li> nohup_handler -This function defines the new environment in "nohup" cases.</li>
</ul>

## Program files 
The program is in only one file: "Ex1.c"<br>
The program uses an external file "file.txt" to record the history.<br>
(The program creates a new process and sends it to a separate external code file).<br>
The program uses another external file "nohup.txt" that contains the output of nohup's commands.<br>

## How to compile?
two options are possible:<br>
1. compile through clion compiler (or other c editor) and use through the console.<br>
2. compile through the terminal using gcc ex3.c -o ex3. 
   and then: ./ex3.

## Input
array from the user with at most 510 characters.

## Output
1. if using 'history' the output is the recorded history<br>
2. if the command is suitable - the output will be the same as in the terminal.<br>
3. if the input will be cd the system will print 'command not supported (yet)'.<br>
4. if the input start or ends with ' ' the program will print 'error: too many spaces in the command!'.<br>
5. if the input contains pipes the output will be the output of the concat of the commands.<br>
6. if using 'done' command the output is a suitable string that shows the number of words and commands that 
   have been used.<br>
7. if the string begins with 'nohup' then the output will be printed in the file instead of on the terminal.<br>
8. if the program failed: probably the output will be exit(1) and the program will print a suitable string.<br>
