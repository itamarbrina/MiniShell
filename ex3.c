// Ex3 - mini shell with pipes:
/**
* The program manage a mini shell.\n
 * The idea is to get sentence from the user, and to manage basic terminal.\n
 * this terminal will perform basic functions that not includes direct interrupts from the hardware.\n
 * In addition the program will manage a history file that will hold the previous commands list,\n
 * the using of !<int> to run previous command will be define too.\n
 * There is option to using pipes and concat between different commands.\n \n
 * you can also use '&' to run process in the background , or using 'nohup' that makes the program
 * continue the running even if the shell exit,\n the output will appear in file called 'nohup.txt'
 * instead of the terminal.

@author Itamar Brina, 208560920*/
//includes :
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>


#define SIZE 512
#define GREEN "\x1B[32m"
#define RED "\x1B[31m"
#define WHITE "\x1B[0m"
#define BLACK "\x1B[30m"


// functions:
void starting();
void sentence_parser(const char*, char*, int*, int*);
void read_history();
void update_history(char*);
void user_buffer(char *);
void command_string_creator (const char*, const int*, char*** );
void command_array_freer(char ***, const int*);
void command_executer(char ***, const int* , const bool * , const bool *);
void history_command(char *);
void pipe_code(char * , int*, pid_t *  , pid_t  * ,pid_t * );
int checking_pipe(const char*);
void string_split(char*, char*, char* , const int * , const char *);
void split_by_pipe_func(const char *  , char * , int * );
void history_in_pipe (char * , char * , char* ,char *, int*);
void pipes_and_forks (char* ,const char * , const char * ,const char* , const int* ,pid_t *  , pid_t  * ,pid_t *);
void executer_for_pipes( char ***  , const int *  , const int * , const pid_t * ,const pid_t *,
                         const pid_t *, const bool * , const bool *);
void sig_child_handler(int sig);
void nohup_string_changer(char *);
void nohup_handler ();

/**
* The program manage a mini shell.\n
 * The idea is to get sentence from the user, and to manage basic terminal.\n
 * this terminal will perform basic functions that not includes direct interrupts from the hardware.\n
 * In addition the program will manage a history file that will hold the previous commands list,\n
 * the using of !<int> to run previous command will be define too.\n
 *  * There is option to using pipes and concat between different commands.\n \n
 * you can also use '&' to run process in the background , or using 'nohup' that makes the program
 * continue the running even if the shell exit,\n the output will appear in file called 'nohup.txt'
 * @return 0 when the program finished successfully and 1 if the program failed.
 */
int main() {
    /*defining general variables of the program:*/
    char    str [SIZE];
    char    first_word [10];
    char    ** commands;
    int     count_all_commands = 0;
    int     count_all_pipes = 0;
    pid_t     parent_pid = getpid();

    /*defining the environment :*/
    starting();

    while(1)
    {
        int     count_chars = 0;
        int     count_words = 0;
        int     count_pipes = 0;
        int     num_of_pipes = 0;
        pid_t   left = 0;
        pid_t   middle = 0;
        pid_t   right = 0;
        bool bg_flag = false;
        bool nohup_flag = false;

        /*accept string from the user:*/
        user_buffer(str);

        /*checks ampersand case:*/
        if ( str[strlen(str)-1] == '&') {
            bg_flag = true;
            update_history(str);
            str[strlen(str)-1] = '\0';
            while (str[strlen(str)-1] == ' ')
                str[strlen(str)-1] = '\0';
        }

        /*checks nohup case:*/
        if (!(strncmp(str , "nohup " , 6))){
            nohup_flag = true;
            update_history(str);
            nohup_string_changer(str);
        }

        /*checking and taking cares of pipes:*/
        pipe_code(str ,&num_of_pipes ,&left ,&middle ,&right);


        /*treat the history commands:*/
        if (str[0] == '!' && num_of_pipes == 0) {
            history_command(str);
            /*checking and taking cares of pipes:*/
            pipe_code(str ,&num_of_pipes ,&left ,&middle ,&right);
        }

        /*the system can support only 2 pipes*/
        if(num_of_pipes == -1) {
            printf (RED "too many pipes!\n"WHITE);
            continue;
        }

        /*adding the number of pipes to the count*/
        count_all_pipes += num_of_pipes;

        /*initialize first_word:*/
        first_word[0] = '\0';

        /*parsing the sentence::*/
        sentence_parser(str, first_word, &count_words, &count_chars);

        /*if there are no words so the line is blank , and we skip this iteration:*/
        if(count_words == 0)
            continue;

        /*checking if there is spaces before / after of the command:*/
        if (str[0] == ' ' || str[strlen(str)-1] == ' '){
            printf(RED"error: too many spaces in the command!\n"WHITE);
            continue;
        }

        /*updating the commands counter:*/
        count_all_commands ++;

        /*checking if the input is "done":*/
        if ((count_words == 1)&&(!(strcmp("done",first_word))))
            break;

        /*updating the words counter:*/
        count_all_pipes += count_pipes;

        /*updating the history*/
        if ((getpid() == parent_pid) && (!bg_flag) && (!nohup_flag))
            update_history(str);


        /*if the sentence contains only 1 word , the program checks
        * if the word is 'history'*/
        if ((count_words == 1)&&(!(strcmp("history",first_word)))){
            read_history(str);
            if(getpid()!= parent_pid)
                exit(0);
            continue;
        }

        /*creating the commands dynamic array in "commands":*/
        command_string_creator(str, &count_words, &commands);

        /*checking if the command is cd:*/
        if(!strcmp("cd", (commands)[0])){
            printf(RED"command not supported (yet)\n"WHITE);
            command_array_freer(&commands, &count_words);
            continue;
        }

        if (num_of_pipes == 0)
            /*send the command to be executing:*/
            command_executer(&commands, &count_words , &bg_flag , &nohup_flag);

        else
            executer_for_pipes(&commands , &count_words , &num_of_pipes , &parent_pid ,&middle , &right , &bg_flag , &nohup_flag);

        /*free the dynamic command array:*/
        command_array_freer(&commands, &count_words);
    }
    printf("Num of commands: %d\n"
           "Total number of pipes in all commands: %d !\n", count_all_commands , count_all_pipes);
    return 0;
}


/**The function take care of the creation of the starting*/
void starting(){
/*opening the history file once so the program will not collapse if the
     * first input is 'history'*/
    FILE* existing;
    existing = fopen("file.txt", "a");
    if (existing == NULL)
    {
        perror(RED"Cannot open file"WHITE);
        exit(1);
    }
    fclose(existing);
}


/**the function accept an sentence from the user and prepare it to work:
 * @return the string that it accept with '\0' in the end*/
void user_buffer(char * str){
    /*print the path-way to the screen:*/
    char cwd[PATH_MAX];
    if(getcwd(cwd, sizeof(cwd)) == NULL) {
        perror(RED"error : can not find program's path!"WHITE);
        exit(1);
    }
    printf (GREEN"%s>"WHITE, cwd);
    fgets(str,SIZE,stdin);
    /*at first i've change '\n' to '\0' so it will be more comfortable to work with the string*/
    str[strlen(str)-1] = '\0';
}


/**The function read from file.txt and show the content to the user.
@param str Is holding the sentence that have been reading currently from the file.*/
void read_history(){
    /*open file in update mode , that file will hold the history content:*/
    char str [SIZE];
    FILE* fp;
    fp = fopen("file.txt", "r");
    if (fp == NULL)
    {
        perror(RED"Cannot open file"WHITE);
        exit(1);
    }

    int line_num=1;
    while (fgets(str, SIZE, fp)!=NULL) {
        printf("%d: %s", line_num, str);
        line_num ++ ;
    }
    // if there are no lines , so there is no history:
    if(line_num == 0)
        printf(BLACK"<The history is empty!>\n"WHITE);
    fclose(fp);
}


/**The function handles with the concatenation of new sentence \n
 * to the history file. \file updating file.txt.
 * @param str Is the sentence we want to enter to history.
 */
void update_history(char * str){
    str[strlen(str)+1] = '\0';
    //I've return the '\n' instead of '\0' to the sentence before adding to history:
    str[strlen(str)] = '\n';
    /*open file in update mode , that file will hold the history content:*/
    FILE* fp;
    fp = fopen("file.txt", "a");
    if (fp == NULL)
    {
        perror(RED"Cannot open file"WHITE);
        exit(1);
    }

    fprintf(fp , "%s" , str);
    fclose(fp);
    /*at first, I've change back to '\n' to '\0'*/
    str[strlen(str)-1] = '\0';
}


/**
 * The function parser the sentence by the following way: \n
 * 1. skips all the spaces in the sentence\n
 * 2. count the words and update count_words var (from below)\n
 * 3. count the characters and update count_chas var (from below)\n
 * 4. update the first_word var (from below)\n
 * @param str - The sentence we want to parse
 * @param first_word - Will hold the first 8 characters of the first word in the sentence
 * @param count_words - Pointer to var that counts the words in the sentence
 * @param count_chars - Pointer to var that counts the chars in the sentence
 */
void sentence_parser(const char* str, char * first_word, int* count_words, int* count_chars)
{
    for(int i=0 ; str[i]!='\0' ; )
    {
        /*skip the spaces*/
        while(str[i] == ' ')
            i++;

        /*checking if the next character is the end character*/
        if (str[i]=='\0')
            break;
        /*because we skip all the spaces ,and we checked if it '\0' now we can know
         * for sure that this is word , so we can raise the words counter by 1*/
        (*count_words)++;
        while((str[i]!=' ')&&(str[i]!='\0')) {
            (*count_chars)++;
            if((*count_words) == 1 && strlen(first_word)<9)
            {
                first_word[strlen(first_word)+1] = '\0';
                first_word[strlen(first_word)] = str[i];
            }
            i++;
        }
    }
}


/**This function taking care of allocating the dynamic array of strings that will hold the commands
 * @param str the string we want to change into dynamic array.
 * @param commands the pointer that will point on the dynamic array.
 * @param count_words the size of the commands array.*/
void command_string_creator (const char* str, const int* count_words , char*** commands) {
    /*define a dynamic array that will contain the words:*/
    *(commands) = (char**) malloc (sizeof(char*)*((*count_words)+1));
    if(*(commands)==NULL) {
        perror(RED"cannot allocate mem for commands array!\n"WHITE);
        exit(1);
    }
    /*define var that will contain one word of the comment:*/
    char word_holder[512];

    /*defining a var that will use as index of the command array:*/
    int word_num = 0;

    /*pass one more time on the input and finish defining the organs in the command array:*/
    for (int i = 0; str[i] != '\0';){
        /*restarting the word for accepting new word:*/
        word_holder[0] = '\0';
        /*copying the word to a different string:*/
        while(str[i]!='\0' && str[i]!=' '){
            word_holder[strlen(word_holder)+1] = '\0';
            word_holder[strlen(word_holder)] = str[i];
            i++;
        }

        /*skip the spaces*/
        while(str[i] == ' ')
            i++;

        /*allocating dynamic memory for the word in the array of commands:*/
        (*commands)[word_num] = (char*) malloc (sizeof(char)*(strlen(word_holder)+1));
        if ((*commands)[word_num]==NULL){
            perror(RED"problem with allocate for command's word!!"WHITE);
            exit(1);
        }
        strcpy((*commands)[word_num], word_holder);

        /*move up to the next place in the commands array:*/
        word_num++;
    }
    /*defining the last place in the commands array to be NULL:*/
    (*commands)[*count_words] = NULL;
}


/**This function frees the commands array.
 * @param commands holds the commands array.
 * @param size holds the size of the commands array.*/
void command_array_freer(char *** commands, const int* size){
    for(int i = 0; i < *size; i++)
        free((*commands)[i]);
    free(*commands);
}


/***This function fork the process and sent the son to execute the pleased command.
 * @param commands holds the commands array.
 * @param size holds the size of commands array.
 * @param bg_flag true if there is ampersand in the end of the string
 * @param nohup_flag true if nohup written in the beginning of the string
 * */
void command_executer(char ***commands, const int* size , const bool * bg_flag , const bool * nohup_flag){
    /*duplicate the process (creating 'son' process)*/
    pid_t son = fork();
    if(son < 0){
        perror(RED"error: can not fork() process!"WHITE);
        command_array_freer(commands, size);
        exit(1);
    }

    /*send the son to perform the command:*/
    if (son == 0 ){
        if(*nohup_flag)
            nohup_handler();
        execvp((*commands)[0], *commands);
        perror(RED"command is not valid!\n"WHITE);
        command_array_freer(commands, size);
        exit(0);
    }
    /*defining the signal catcher:*/
    signal(SIGCHLD , sig_child_handler);

    /*wait for son to finish!*/
    if(!(*bg_flag))
        pause();
}


/**This function handle the history commands:\n
 * reading from file.txt and change the input string.
 * @param str Is the input string from the user.
 * @result str will hold the final result.*/
void history_command(char* str){
    /*change the string into number:*/
    str[0] = '0';
    int len = atoi(str);
    if (len == 0){
        printf(RED"history command not valid!");
        return;
    }

    /*open the history file and read the lines into temp:*/
    FILE* fp;
    fp = fopen("file.txt", "r");

    if (fp == NULL){
        perror(RED"Cannot open file"WHITE);
        exit(1);
    }
    char temp[SIZE];
    for(int i = 0; i < len; i++) {
        if(!fgets(temp, SIZE, fp)) {
            printf(RED"NOT IN.HISTORY"WHITE);
            fclose(fp);
            return;
        }
    }

    /*change str to temp:*/
    temp[strlen(temp)-1] ='\0';
    strcpy(str, temp);
    fclose(fp);
}

/**This function checks if there is pipes in the string from the user.\n
 * After that, it continue to performing the actions that needed depending on vary the situations.
 * @param str is the string that the user enter
 * @param num_of_pipes holds the number of pipes in str.
 * @param left after forking (if needed) this pointer will hold the left son's pid.
 * @param middle after forking (if needed) this pointer will hold the middle son's pid.
 * @param right after forking (if needed) this pointer will hold the right son's pid.
 * @result * In *generally* the function result is:\n
 * 1.if there is too many (above 2) pipes or not pipes at all the program just return.\n
 * 2. if there 1 or 2 pipes the function taking care of creating the fork and the pipes.
 * */
void pipe_code(char * str ,int * num_of_pipes ,pid_t * left ,pid_t  * middle ,pid_t * right){
    /*counting the number of pipe that appearing in the string:*/
    *num_of_pipes = checking_pipe(str);

    /*if there are too many pipes:*/
    if (*num_of_pipes == -1)
        return;

    /*creating splits arrays:*/
    if (*num_of_pipes == 1 || *num_of_pipes == 2){
        char s1[SIZE] ,s2[SIZE] , s3[SIZE];

        string_split(s1, s2 ,s3 , num_of_pipes , str);
        /*history possibilities:*/
        if (s1[0] == '!' || s2[0] == '!' || s3[0] == '!' )
            history_in_pipe(str , s1 , s2 , s3 , num_of_pipes);

        if (*num_of_pipes == -1)
            return;

        pipes_and_forks(str , s1, s2 , s3 ,num_of_pipes, left, middle, right);
    }
}

/**The checking_pipe method counting the pipes appears in (if there are),
 * that appears in str. \n <b>pay attention , pipe inside double quotation marks will not be counted!\b
 * @param str Is the string that the checking will be on.
 * @return returning the number of pipes , if there are above 3 the return value will be -1.*/
int checking_pipe(const char* str){

    bool belongs_to_user_string = false;
    int num_of_pipes = 0 ;
    for (int i = 0 ; str[i] != '\0' ;i++) {
        if (str[i] == '|' && !belongs_to_user_string)
            num_of_pipes++;
        if (str[i] == '"')
            belongs_to_user_string = ! belongs_to_user_string;
    }
    switch (num_of_pipes) {
        case 0:
            return 0;
        case 1:
            return 1;
        case 2:
            return 2;
        default:
            return -1;
    }
}

/**This method aims to split the str string by two/three according to the pipe that appears in it,\n
 * and taking care of creating the new string without spaces.
 * @param s1 is first string of the new strings.
 * @param s2 is second string of the new strings.
 * @param s3 is third string of the new strings.
 * @param str is the string you want to split
 * @result str will hold the firs command , str 2 the second and s3 the third.*/
void string_split(char* s1, char* s2 , char * s3 , const int *pipes , const char* str) {

    int index = 0;

    //initialize:
    s1[0] = '\0';
    s2[0] = '\0';
    s3[0] = '\0';

    // creating s1:
    split_by_pipe_func(str , s1 , &index);
    /*clean the spaces in the end of s1:*/
    while (s1[strlen(s1) - 1] == ' ')
        s1[strlen(s1) - 1] = '\0';


    /*clean the spaces before s2:*/
    while (str[index] == ' ')
        index++;
    //creating s2:
    split_by_pipe_func(str , s2 , &index);

    if(*pipes == 2) {

        /*clean the spaces in the end of s2:*/
        while (s2[strlen(s2) - 1] == ' ')
            s2[strlen(s2) - 1] = '\0';

        /*clean the spaces before s3:*/
        while (str[index] == ' ')
            index++;
        //crating s3:
        split_by_pipe_func(str, s3, &index);
    }
}

/**This function taking care of appearance of history commands in pipes.\n
 * The function build the whole string after the replace of the command from the history.
 * @param str is pointer to the address of the string that comes from the user.
 * @param s1 is pointer to the firs part of the pipe
 * @param s2 is pointer to the second part of the pipe
 * @param s2 is pointer to the third part of the pipe (if there is)
 * @param num_of_pipes is pointer that holds the number of pipes there are in the string.
 * @result <\b str  will hold the string after the exchanging , \n
 * <\b num_of_pipe will hold the total number of pipes appearing in str after the exchange. \n
 * <\b s1 , <\b s2 , <\b s3 s[i] will hold the s[i] after the exchange. */
void history_in_pipe (char * str, char * s1, char* s2,char *s3, int* num_of_pipes){
    /*checking history possibilities:*/
    if (s1[0] == '!')
        history_command(s1);

    if (s2[0] == '!')
        history_command(s2);

    if (s3[0] == '!')
        history_command(s3);

    strncpy (str , s1 , strlen(s1)+1);
    str[strlen(str)+1]='\0';
    str[strlen(str)] ='|';
    strncat (str , s2 , strlen(s2)+1);

    if (*num_of_pipes == 2) {
        str[strlen(str) + 1] = '\0';
        str[strlen(str)] = '|';
        strncat (str , s3 , strlen(s3)+1);
    }
    // update the number of pipes appearing in str:
    *num_of_pipes = checking_pipe(str);

    // split again ,(if there is pipe in the history that we concat to another):
    string_split(s1, s2 ,s3 , num_of_pipes , str);
}

/**This method helps to <\b string_split to split the arrays. \n
 * @param src this is the string you want to perform the action on.
 * @param dest this is a pointer to string that will contain the result.
 * @param index this is pointer to int that will hold the starting point.
 * @result dest will hold the part of the src that start in src[index] and finished with pipe or \0 \n
 * <b> (only if the pipe is really pipe and not part of other string.) \n
 * */
void split_by_pipe_func(char const * src , char * dest, int * index){
    // define bool variable that show if the pipe is part of a string or not:
    bool belongs_to_user = false;

    /*running on the string and update the output until src[index] is available '|' or '\0' */
    for (; src[*index]!= '\0'; (*index) ++){
        if(src[*index] == '|' && !belongs_to_user){
            (*index) ++;
            break;
        }
        if(src[*index] == '"')
            belongs_to_user = !belongs_to_user;

        dest[strlen(dest)+1] = '\0';
        dest[strlen(dest)] = src[*index];
    }
}

/**This function is really critical and it is the core of pipe functionality.\n
 * The function is responsible of duplicate the process two or three times , \n
 * depends on the number of pipes there is , and create the pipes that the  \n
 * processes will be concat each output to other input with them.\n \n
 * pay attention if there is three processes the third will not be created from the main process but from the middle son.
 * @param str a pointer to the string coming from the user.
 * @param s1 a pointer to the first command in the pipe.
 * @param s2 a pointer to the second command in the pipe.
 * @param s3 a pointer to the third command in the pipe.
 * @param number_of_pipes holds the amount of pipes there are (distinguish 1 or 2).
 * @param left this variable will hold the first new process pid.
 * @param middle this variable will hold the second new process pid.
 * @param right this variable will hold the third new (if there is) process pid.
 * @result every son will hold different str and running it . \n
 * the communication is as shown here : <b>left -> middle (-> right). */
void pipes_and_forks (char* str ,const char * s1 ,const char * s2 ,const char* s3 ,const int * number_of_pipes ,
                      pid_t * left ,pid_t * middle ,pid_t * right){
    /*creating pipe:*/
    int index_in_file_descriptor [2];
    if (pipe(index_in_file_descriptor) < 0) {
        perror(RED"error with creating pipe in the main"WHITE);
        exit(1);
    }

    /*create first son:*/
    *left = fork();
    if (*left < 0){
        perror(RED"failed: creation fork in the pipe"WHITE);
        exit(1);
    }

    //son's code:
    if (*left == 0){
        close(index_in_file_descriptor[0]);
        dup2(index_in_file_descriptor[1] , STDOUT_FILENO);
        close(index_in_file_descriptor[1]);
        strncpy(str, s1 , strlen(s1)+1);
    }
    //dad's code:
    if(*left > 0){
        /*create second son:*/
        *middle = fork();
        if (*middle < 0){
            perror(RED"failed: creation fork in the pipe"WHITE);
            exit(1);
        }

        //son's code:
        if (*middle == 0){
            close(index_in_file_descriptor[1]);
            dup2(index_in_file_descriptor[0] , STDIN_FILENO);
            close(index_in_file_descriptor[0]);
            strncpy(str, s2 , strlen(s2)+1);
        }
        //dad's code:
        if (*middle > 0){
            close(index_in_file_descriptor[0]);
            close(index_in_file_descriptor[1]);
        }
    }
    if (*number_of_pipes == 2){
        // middle son code:
        if(*left > 0 && *middle == 0){
            int index_in_file_descriptor_2 [2];
            if (pipe(index_in_file_descriptor_2) < 0) {
                perror(RED"error with creating pipe in the main"WHITE);
                exit(1);
            }
            *right= fork();
            if (*right < 0){
                perror(RED"failed: creation fork in the pipe"WHITE);
                exit(1);
            }
            //the middle son code:
            if (*right > 0){
                close(index_in_file_descriptor_2[0]);
                dup2(index_in_file_descriptor_2[1] , STDOUT_FILENO);
                close(index_in_file_descriptor_2[1]);
            }

            //the right son code:
            if (*right == 0){
                close(index_in_file_descriptor_2[1]);
                dup2(index_in_file_descriptor_2[0] , STDIN_FILENO);
                close(index_in_file_descriptor_2[0]);
                strncpy(str, s3 , strlen(s3)+1);
            }
        }
    }
}

/**This function fork every one of the processes (except the main) and sent the son to execute the pleased command.
 * @param commands holds the commands array of current process.
 * @param size holds the size of commands array of current process.
 * @param num_of_pipes holds the amount of pipe in use.
 * @param parent_pid holds the main process pid.
 * @param right holds the third command process's pid.
 * @param bg_flag true if there is ampersand in the end of the string
 * @param nohup_flag true if nohup written in the beginning of the string
 * @result after the execute have finished every process (except the main) is taking care of freeing the dynamic memory,\n
 * and exit . the main process wait for all the processes that created to finished.*/
void  executer_for_pipes( char *** commands , const int * size , const int *num_of_pipes , const pid_t * parent_pid ,
                         const pid_t * middle ,  const pid_t * right , const bool* bg_flag , const bool * nohup_flag){
    /*send the son to perform the command:*/
    if (getpid() != *parent_pid ){
        if((*nohup_flag) && (*num_of_pipes == 2)&& (getpid() == *right))
            nohup_handler();
        if((*nohup_flag) && (*num_of_pipes == 1)&& (getpid() == *middle))
            nohup_handler();
        pid_t son = fork();
        if(son < 0){
            perror(RED"error: can not fork() process!"WHITE);
            command_array_freer(commands, size);
            exit(1);
        }

        /*send the son to perform the command:*/
        if (son == 0 ){
            execvp((*commands)[0], *commands);
            perror(RED"command is not valid!\n"WHITE);
            command_array_freer(commands, size);
            exit(0);
        }
        /*father code:*/
        if (son > 0 ){
            close(0);
            close(1);
            command_array_freer(commands ,size);

            /*wait for son to finish!*/
            waitpid(son , NULL ,0);

            /*if there are two pipes the middle son needs to wait for the right son that he have been forked:*/
            if(*num_of_pipes == 2 && (*right) > 0)
                /*wait for the third son:*/
                waitpid(*right , NULL , 0);

            exit(0);
        }

    }

    /*defining the signal catcher:*/
    signal(SIGCHLD , sig_child_handler);

    /*wait for son to finish!*/
    if(!(*bg_flag)) {
        pause();
        pause();
    }
}

/**This function is handling with child signal (number 17).
 * @param sig is the number of signal that arrived.*/
void sig_child_handler(int sig){
    int status;
    waitpid(-1,&status , WNOHANG);
}

/**This method helps to remove "nohup" from str in no hup cases:
 * @param str holds the string we want to change.*/
void nohup_string_changer(char * str){
    int num_of_spaces = 6;
    while (str[num_of_spaces] == ' ')
        num_of_spaces ++;
    strncpy(str , str + num_of_spaces , strlen(str)-num_of_spaces +1);

}

/**This function define the new environment in "nohup" cases:*/
void nohup_handler (){
    /*ignoring SIGHUP signal:*/
    signal(SIGHUP, SIG_IGN);

    /*opening the file that will contain the output:*/
    int fd = open("nohup.txt" , O_WRONLY | O_CREAT | O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);

    /*change the output to be the file:*/
    if (dup2(fd , STDOUT_FILENO) == -1) {
        fprintf(stderr, "dup2 failed");
        exit(1);
    }
    close(fd);
}
