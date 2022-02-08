#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <termios.h>

#define SHELL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define SHELL_PIPE_DELIM "|"
#define MAX_HIST 10000

int execute(char **args, int position);
char ** shell_split_line(char * line, int *position);
char ** shell_split_pipe(char * line, int *size);
char* shell_read_line();
void shell_loop();
int show_history(char **args);
int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);
int shell_launch(char **args, int position, int in, int out);
int max(int a,int b);
pid_t child_pid;

struct termios saved_attributes;

void reset_input_mode(void) {
  tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}
void set_input_mode(void){
  struct termios tattr;
  tcgetattr(STDIN_FILENO, &saved_attributes);
  atexit(reset_input_mode);
  tcgetattr (STDIN_FILENO, &tattr);
  tattr.c_lflag &= ~(ICANON|ECHO);
  tattr.c_cc[VMIN] = 1;
  tattr.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &tattr);
}

struct History{
    int cnt;
    char** data;
};

struct History history;

char *built_in[] = 
{
    "cd",
    "exit",
    "help",
    "history"
};

int (*built_in_func[]) (char **) = 
{
    &shell_cd,
    &shell_exit,
    &shell_help,
    &show_history,
};

int shell_num_builtins()
{
    return sizeof(built_in) / sizeof(char *);
}

void init_history(){
    FILE* hist_ip;
    if(hist_ip = fopen("shell_history.txt","r")){
        fscanf(hist_ip,"%d",&(history.cnt));
        history.data=(char**)calloc(history.cnt+MAX_HIST,sizeof(char*));
        for(int i=0;i<history.cnt+10000;i++){
            history.data[i]=(char*)calloc(100,sizeof(char));
        }
        for(int i=0;i<history.cnt;i++){
            fgets(history.data[i],100,hist_ip);
        }
        fclose(hist_ip);
    }
    else{
        history.cnt=0;
        history.data=(char**)calloc(MAX_HIST,sizeof(char*));
        for(int i=0;i<history.cnt+10000;i++){
            history.data[i]=(char*)calloc(100,sizeof(char));
        }
    }
}

int main(int argc, char **argv)
{

    // Command loop
    init_history();
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    shell_loop();
    // shutdown actions
    return EXIT_SUCCESS;
}




void shell_loop()
{
    
    char *line;
    size_t line_size = 1024;
    char **args;
    char **pipes;
    int status=1;
    int position, pipe_size;
    line = (char *)(malloc(line_size * sizeof(char)));
    while(status)
    {
        printf("--> ");
        line = shell_read_line();
        if(line[0] == '\0'){
            continue;
        }
        // line = readline();
        // getline(&line, &line_size, stdin);
        // scanf("%[^\n]s", line);
        // printf("The command is %s\n", line);
        pipes = shell_split_pipe(line, &pipe_size);
        if (pipe_size == 1) {
            args = shell_split_line(line, &position);
            int i=0;
            /*printf("HELLO\n");
            for(i=0;i<position;++i)
            {
                printf("%s\n",args[i]);
            }
            */
            // printf("END\n");
            status = execute(args, position);
        }
        else if (pipe_size > 1) {
            // printf("%d\n", pipe_size);
            // for (int j=0; j<pipe_size;j++)
            //     printf("%s\n", pipes[j]);
            // printf("BEGIN\n");
            int i;
            pid_t pid;
            // in handles the file descriptor for input
            int in = 0, fd[2];
            for (i=0; i<pipe_size - 1; ++i) {
                pipe(fd);
                args = shell_split_line(pipes[i], &position);
                status = shell_launch(args, position, in, fd[1]);
                close(fd[1]);
                in = fd[0];
            }
            // if (in != 0)
            //     dup2 (in, 0);
            args = shell_split_line(pipes[i], &position);
            status = shell_launch(args, position, in, 1);
        }
        // memory cleaning
        free(line);
        free(args);
        fflush(stdin);
        fflush(stdout);
    } 

}

char * memory_failed_error()
{
    fprintf(stderr, "Shell: failed to allocate memory for line\n");
    exit(EXIT_FAILURE);
}

char* shell_read_line()
{
    int bufsize = SHELL_BUFSIZE;
    int position = 0,word_pos=0;
    char *buffer = calloc(bufsize, sizeof(char) );
    char *word_buf = calloc(bufsize, sizeof(char));
    int c;
    
    if (!buffer)
    {
        memory_failed_error();
    }
    set_input_mode();
    while(1)
    {
        c = getchar();

        if (c==EOF || c=='\n')
        {
            //printf("%d",position);
            printf("\n");
            buffer[position] = '\0';
            reset_input_mode();
            return buffer;
        }
        else if(c=='\033'){     //Arrow
            // printf("ARROW");
            c=getchar();
            c=getchar();
            //A for up, B for down, C for right, D for left
            // printf("%c",c);
        }
        else if(c=='\t'){
            // printf("%d",position);
            //tab completion
            reset_input_mode();

            //getting the last word
            int word_pos=0,tmp=position-1;
            memset(word_buf,'\0',bufsize);
            while(tmp>=0&&buffer[tmp]!=' '){
                tmp--;
            }
            if(tmp) tmp++;
            for(;word_pos+tmp<position;word_pos++){
                word_buf[word_pos]=buffer[word_pos+tmp];
            }
            // printf("%d",word_pos);

            //storing the names of all the files matching initially with the last word
            char **file_name;
            file_name=(char **)malloc(1000*sizeof(char*));
            for(int i=0;i<1000;i++){
                file_name[i]=(char *)calloc(100,sizeof(char));
            }
            //iterating over all the files in the directory
            DIR *d;
            struct dirent *dir;
            d = opendir(".");
            int file_count=0;
            if (d)
            {
                while ((dir = readdir(d)) != NULL)
                {
                    if(strncmp(word_buf,dir->d_name,word_pos)==0){
                        strcpy(file_name[file_count],dir->d_name);
                        file_count++;
                    }
                }
                closedir(d);
            }


            // checking if there were any matches
            if(file_count==0){
                set_input_mode();
                continue;
            }
            
            //finding the longest common substring
            int lcs=(int)1e8;
            for(int i=1;i<file_count;i++){
                int tmp=word_pos;
                while(file_name[0][tmp]==file_name[i][tmp]){
                    tmp++;
                }
                lcs=(lcs<tmp)?lcs:tmp;
            }
            if(file_count==1){
                int word_length=strlen(file_name[0]);
                // position++;word_pos++;
                for(;word_pos<word_length;word_pos++,position++){
                    buffer[position]=file_name[0][word_pos];
                    printf("%c",file_name[0][word_pos]);
                    // printf("%c",buffer[position]);
                }
            }   
            else if(lcs>word_pos&&lcs!=1e8){
                // position++;word_pos++;
                for(;word_pos<lcs;word_pos++,position++){
                    buffer[position]=file_name[0][word_pos];
                    printf("%c",file_name[0][word_pos]);
                    // printf("%c",buffer[position]);
                }
            }
            else{
                printf("\n");
                for(int i=0;i<file_count;i++){
                    printf("%d) %s\n",i+1,file_name[i]);
                }
                int filename_input_num;
                scanf("%d",&filename_input_num);
                filename_input_num--;
                if(filename_input_num<0||filename_input_num>file_count){
                    printf("Incorrect Input\n");
                    set_input_mode();
                    return NULL;
                }
                int word_length=strlen(file_name[filename_input_num]);
                // position++;word_pos++;
                for(;word_pos<word_length;word_pos++,position++){
                    buffer[position]=file_name[filename_input_num][word_pos];
                    // printf("%c",buffer[position]);
                }
                printf("--> %s",buffer);
                c=getchar();
            }
            
            set_input_mode();
            continue;

        }
        else if(c==127){
            if(position<=0)
            continue;
            printf("\b \b");
            position--;
        }
        else
        {
            buffer[position] = c;
            printf("%c",c);
            position++;
        }
        
        // handle buffer limit extension
        if (position >= bufsize)
        {
            bufsize += SHELL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer){
                memory_failed_error();
            }
        }
    }
}

char** shell_split_line(char* line, int *position)
{
    int bufsize = SHELL_TOK_BUFSIZE;
    *position = 0;

    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if (!tokens) 
    {
        memory_failed_error();
    }

    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL)
    {
        //Trying to manually tokenise the '<' without spaces as bash accepts those
        /*int l = strlen(token);
        int last=0;
        int i;
        int j=0;
        int flag=0;
        for(i=0;i<l;++i)
        {
            if(token[i]=='<'||token[i]=='>')
            {
                flag=1;
                if(i!=0)
                {
                    tokens[*position] = (char*)malloc(sizeof(char)*l);
                    strncpy(tokens[*position],token+last,i-last);
                    (*position)++;
                }
                tokens[*position] = (char*)malloc(1);
                tokens[*position][0] = token[i];
                (*position)++;
                last = i+1;
            }
        }
        
        if(flag==1) 
        {
            token = strtok(NULL, SHELL_TOK_DELIM);
            if(last<l)
            {
                tokens[*position] = (char*)malloc(sizeof(char)*l);
                strncpy(tokens[*position],token+last,l-last);
                (*position)++;
            }
            continue;
        }
    */

        tokens[*position] = token;
        (*position)++;
        
        if ((*position) >= bufsize)
        {
            bufsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                memory_failed_error();
            }
        }
        token = strtok(NULL, SHELL_TOK_DELIM);
    }
    tokens[(*position)] = NULL;
    return tokens;
}

char** shell_split_pipe(char* line, int *size)
{
    int bufsize = SHELL_TOK_BUFSIZE;
    *size = 0;

    char **tokens = malloc(sizeof(char*) * bufsize);
    char *token;

    if (!tokens) 
    {
        memory_failed_error();
    }

    token = strtok(line, SHELL_PIPE_DELIM);
    while (token != NULL)
    {
        tokens[*size] = token;
        (*size)++;
        
        if ((*size) >= bufsize)
        {
            bufsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                memory_failed_error();
            }
        }
        token = strtok(NULL, SHELL_PIPE_DELIM);
    }
    tokens[(*size)] = NULL;
    return tokens;
}

int shell_launch(char **args, int position, int in, int out) 
{
    pid_t pid, wpid;
    int status;
    long int size = position;
    int i=0;
    //Pipe not implemented ;_;_;_;
    /*
    int num_pipe=0;
    for(i=0;i<size;++i) if(strcmp(args[i],"|")==0) num_pipe++;
    
    for(int i=0;i<num_pipe+1;++i)
    {
        char ** args2 = (char**)malloc(sizeof(char*)*(size-1));
    }
    */

    pid = fork();
    if (pid == 0) 
    {
        signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        if (in != 0)
        {
          dup2 (in, 0);
          close (in);
        }

        if (out != 1)
        {
          dup2 (out, 1);
          close (out);
        }
    	// Child process
        int count=0;
        for(i=0;i<size;++i)
        {
            if(strcmp(args[i],"&")==0) count++;
            if(strcmp(args[i],">")==0)
            {
                close(1);
                dup(open(args[i+1], O_WRONLY));
                count+=2;
            }
            if(strcmp(args[i],"<")==0)
            {
                close(0);
                dup(open(args[i+1], O_RDONLY));
                count+=2;
            }
        }

        char ** args2 = (char**)malloc(sizeof(char*)*(size-1));
        for(i=0;i<size-count;++i) 
        {
            args2[i] = args[i];
        }
        if (execvp(args2[0], args2) == -1) perror("shell");

        exit(EXIT_FAILURE);
    }
    else if (pid < 0) perror("forking error!");
    else
    {
        // Parent process
        //signal(SIGTSTP, SIG_DFL);
    	if(!strcmp(args[size-1],"&")==0)
    	{
    		do 
	        {
	            wpid = waitpid(pid, &status, WUNTRACED);
	        } while (!WIFEXITED(status) && !WIFSIGNALED(status) && !WIFSTOPPED(status));
            if(WIFSTOPPED(status)){
                kill(pid,SIGCONT);
            }
    	} 
    }

    return 1;
}

int execute(char **args, int position)
{
    if (args[0] == NULL)
    {
        return 1;
    } else
    {
        for (int i = 0; i<shell_num_builtins(); i++)
        {
            if (strcmp(built_in[i], args[0]) == 0)
            {
                return (*built_in_func[i])(args);
            }
        }
        return shell_launch(args, position, 0, 1);
    }
    
}

int shell_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "shell: expected argument to \"cd\" command!");
    } else
    {
        if (chdir(args[1]) != 0)
        {
            perror("shell");
        }
    }
    return 1;
}

int shell_help(char **args)
{
    printf("Shell help!\n");
    printf("The builtin commands are: \n");
    for (int i = 0; i<shell_num_builtins(); i++)
    {
        printf(" %s\n", built_in[i]);
    }

    printf("Use the man command for knowing about other commands!");
}

int shell_exit(char **args)
{
    return 0;
}

int show_history(char **args){
    if(history.cnt<1000){
        for(int i=history.cnt-1;i>=0;i--){
            printf("%s\n",history.data[i]);
        }
    }
    else{
        for(int i=1;i<=1000;i++){
            printf("%s\n",history.data[history.cnt-i]);
        }
    }
    return 0;
}