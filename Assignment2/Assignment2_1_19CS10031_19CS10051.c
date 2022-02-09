#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <termios.h>
#include <sys/inotify.h>
#include <time.h>
#include <errno.h>

#define SHELL_BUFSIZE 1024
#define SHELL_TOK_BUFSIZE 64
#define SHELL_TOK_DELIM " \t\r\n\a"
#define SHELL_PIPE_DELIM "|"
#define MAX_HIST 10000
#define MAX_SHOW_HIST 1000
#define MAX_COMMAND 128
#define INF 100000000
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define BUF_LEN 128

int execute(char **args, int position);
char ** shell_split_line(char * line, int *position);
char ** shell_split_pipe(char * line, int *size);
char* shell_read_line();
void shell_loop();
void init_history();
void update_history();
int show_history(char **args);
int clear_history(char **args);
int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);
int shell_launch(char **args, int position, int in, int out, int wait);
int shell_multiWatch(char *line);
void search(char *cmd);

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
    int start;
    char** data;
};

struct History history;

char *built_in[] = 
{
    "cd",
    "exit",
    "help",
    "history",
    "hclear",
};

int (*built_in_func[]) (char **) = 
{
    &shell_cd,
    &shell_exit,
    &shell_help,
    &show_history,
    &clear_history,
};

int shell_num_builtins()
{
    return sizeof(built_in) / sizeof(char *);
}

void init_history(){
    history.start=0;
    FILE* hist_ip;
    int tmp;
    if(hist_ip = fopen("shell_history.txt","r")){
        fscanf(hist_ip,"%d",&(history.cnt));
        fgetc(hist_ip);
        history.data=(char**)calloc(MAX_HIST,sizeof(char*));
        for(int i=0;i<MAX_HIST;i++){
            history.data[i]=(char*)calloc(MAX_COMMAND,sizeof(char));
        }
        for(int i=0;i<history.cnt;i++){
            int j=0;
            while (1)
            {
                history.data[i][j] = (char) fgetc(hist_ip);
                if(history.data[i][j]==EOF){
                    history.data[i][j]='\0';
                    break;
                }
                if(history.data[i][j]=='\n'){
                    history.data[i][j]='\0';
                    if(j==0)
                    i--;
                    break;
                }
                j++;
            }
            printf("%d",j);
            
        }
        fclose(hist_ip);
    }
    else{
        history.cnt=0;
        history.data=(char**)calloc(MAX_HIST,sizeof(char*));
        for(int i=0;i<MAX_HIST;i++){
            history.data[i]=(char*)calloc(MAX_COMMAND,sizeof(char));
        }
    }
}

void update_history(){
    FILE* hist_op = fopen("shell_history.txt","w");
    if(history.cnt==MAX_HIST){
        fprintf(hist_op,"%d",MAX_HIST);
        for(int i=history.start;i<history.cnt;i++){
            fprintf(hist_op,"\n%s",history.data[i]);
        }
        for(int i=0;i<history.start;i++){
            fprintf(hist_op,"\n%s",history.data[i]);
        }
    }
    else{
        fprintf(hist_op,"%d",history.cnt);
        for(int i=history.start;i<history.cnt;i++){
            fprintf(hist_op,"\n%s",history.data[i]);
        }
    }
    fclose(hist_op);
}

void computeLPSArray(char *pat, int M, int *lps) {
  int len = 0;
  lps[0] = 0;
  int i = 1;
  while (i < M) {
    if (pat[i] == pat[len]) {
      len++;
      lps[i] = len;
      i++;
    } else {
      if (len != 0) {
        len = lps[len - 1];
      } else {
        lps[i] = 0;
        i++;
      }
    }
  }
}
int KMPSearch(char *pat, char *txt) {
  int M = strlen(pat);
  int N = strlen(txt);
  int lps[M];
  computeLPSArray(pat, M, lps);
  int maximum = 0;
  int i = 0, j = 0;
  while (i < N) {
    if (pat[j] == txt[i]) {
      j++;
      i++;
      maximum = (j>maximum)?j:maximum;
    }
    if (j == M) {
      return M;

    } else if (i < N && pat[j] != txt[i]) {
      if (j != 0)
        j = lps[j - 1];
      else
        i = i + 1;
    }
  }
  return maximum;
}

void search(char* cmd) {
    size_t line_size = 1024;    
    int maximum = 0, maxIdx = -1;

    
    for(int j=history.start-1;j>=0;j--){
        for (int i = 0; i < (int)strlen(cmd); i++) {
            char *cmdsubstring = cmd + i;
            int len = KMPSearch(cmdsubstring, history.data[j]);
            if (len > maximum) {
                maximum = len;
                maxIdx = j;
            }
        }
    }
    for(int j=history.cnt;j>=history.start;j--){
        for (int i = 0; i < (int)strlen(cmd); i++) {
            char *cmdsubstring = cmd + i;
            int len = KMPSearch(cmdsubstring, history.data[j]);
            if (len > maximum) {
                maximum = len;
                maxIdx = j;
            }
        }
    }

    if (maximum==0 || maximum==1) {
        fprintf(stdout, "No match for search term in history\n");
    } else {
        fprintf(stdout, "%s", history.data[maxIdx]);
    }
}

char * error_in_memory(){
    fprintf(stderr, "Shell: failed to allocate memory for line\n");
    exit(EXIT_FAILURE);
}

int multiWatch_c_detect = 1;
void multiWatch_ctrl_c(){
    multiWatch_c_detect = 0;
    //signal(SIGTSTP, SIG_IGN);

    signal(SIGINT, SIG_IGN);
    

    int status = 0;
    int i=1;
    while(status==0){
        char * str = malloc(sizeof(char)*BUF_LEN);
        sprintf(str,".temp.PID%d.txt",i);
        status = remove(str);
    }
}

int shell_multiWatch(char *line){

    //signal(SIGTSTP, multiWatch_ctrl_c);
    signal(SIGINT, multiWatch_ctrl_c);
    char **cammands;
    cammands = (char**)malloc(sizeof(char *)*BUF_LEN);
    if (!cammands) 
    {
        error_in_memory();
    }

    char *temp;
    temp = strtok(line,"\"");
    temp = strtok(NULL,"\"");
    int size=0;
    while(temp!=NULL){
        cammands[size] = temp;
        //printf("%s\n",temp);
        size++;
        temp = strtok(NULL,"\"");
        temp = strtok(NULL,"\"");
        
    }
    cammands[size] = NULL;
    char ***arguments;
    arguments = (char***)malloc(sizeof(char **)*size+1);
    int positions[size];

    int fd = inotify_init();
    int inotify_id[size];
    int readFds[size];
    
    for(int i=0;i<size;i++){
        //printf("%s\n",cammands[i]);
        //arguments[i] = cammands[i];
        char **args;
        int position = 0;
        args = shell_split_line(cammands[i], &position);
        arguments[i] = args;
        positions[i] = position;
        
        char *str;
        str = malloc(sizeof(char)*BUF_LEN);
        for(int i=0;i<BUF_LEN;i++)
            str[i] = '\0';
        sprintf(str, ".temp.PID%d.txt", i+1);
        int discriptor = open(str,O_CREAT | O_APPEND, 0666);
        readFds[i] = discriptor;
        //close(discriptor);
        //watchFiles(str,cammands[i]);
        int wd = inotify_add_watch( fd, str, IN_MODIFY);
        inotify_id[i] = wd;
    }
    pid_t pid_to_console = fork();

    if(pid_to_console==0){
        //signal(SIGTSTP, SIG_DFL);
        signal(SIGINT, SIG_DFL);
        char buf[EVENT_BUF_LEN]
                    __attribute__((aligned(__alignof__(struct inotify_event))));
        for(int i=0;i<EVENT_BUF_LEN;i++) buf[i] = '\0';
        while(1){    
            int len = read( fd, buf, EVENT_BUF_LEN );
            if(len<=0) perror("read error");
            int i=0;
            while(i<len){
                struct inotify_event *event = (struct inotify_event *)&buf[i];
                
                if(event->mask & IN_MODIFY){

                    int wd = event->wd;
                    int fileIndex =0;
                    for(int j=0;j<size;j++){
                        if(wd==inotify_id[j]){
                            fileIndex = j;
                            break;
                        }
                    }
                    time_t t;   // not a primitive datatype
                    time(&t);
                    char read_buf[BUF_LEN + 1];
                    for (int i = 0; i <= BUF_LEN; i++) {
                        read_buf[i] = '\0';
                    }
                    
                    fprintf(stdout,"\"%s\", %s\n<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-\n",cammands[fileIndex],ctime(&t));
                    while (read(readFds[fileIndex], read_buf, BUF_LEN) > 0) {
                        fprintf(stdout, "%s", read_buf);
                        for (int i = 0; i <= BUF_LEN; i++) {
                            read_buf[i] = '\0';
                        }
                    }
                    fprintf(stdout, "->->->->->->->->->->->->->->->->->->->->\n\n");
                }
                i += sizeof(struct inotify_event) + event->len;
            }
        }
    }

    while(multiWatch_c_detect){
        for(int i=0;i<size;i++){

            //char **args;
            //int position = 0;
            //args = shell_split_line(cammands[i], &position);
            //arguments[i] = args;
            //positions[i] = position;

            char *str;
            str = malloc(sizeof(char)*BUF_LEN);
            sprintf(str, ".temp.PID%d.txt", i+1);
            int discriptor = open(str,O_CREAT| O_WRONLY | O_APPEND, 0644);
            //printf("%d\n",positions[i]);
            shell_launch(arguments[i],positions[i],0,discriptor,0);
        }
        sleep(1);

        

        
        
        //char final[EVENT_BUF_LEN+1000];
        //for(int i=0;i<size;i++)
        //sprintf(final,"\"%s\", %s\n<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-\n%s\n->->->->->->->->->->->->->->->->->->->\n",cammand,ctime(&t),buffer);
        //printf("%s",final);


    }
    multiWatch_c_detect=1;
    
    return 1;
}


int main(int argc, char **argv)
{

    // Command loop
    init_history();
    signal(SIGTSTP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    shell_loop();
    update_history();
    // shutdown actions
    return EXIT_SUCCESS;
}

void shell_loop()
{
    
    char *line,*temp;
    size_t line_size = 1024;
    char **args;
    char **pipes;
    int status=1;
    int position, pipe_size;
    line = (char *)(malloc(line_size * sizeof(char)));
    temp = (char *)(malloc(line_size * sizeof(char)));
    while(status)
    {
        printf("--> ");
        line = shell_read_line();
        if(line[0] == '\0'){
            continue;
        }

        //Adding the command in history
        if(history.cnt==MAX_HIST){
            strcpy(history.data[history.start],line);
            (history.start)++;
            history.start%=MAX_HIST;
        }
        else{
            strcpy(history.data[history.cnt],line);
            history.cnt++;
        }
        
        for(int i=0;i<line_size;i++)
            temp[i] = line[i];
        

        if(strcmp(strtok(temp," "),"multiWatch")==0){
            shell_multiWatch(line);
            continue;
        }

        pipes = shell_split_pipe(line, &pipe_size);
        if (pipe_size == 1) {
            args = shell_split_line(line, &position);
            int i=0;
            
            status = execute(args, position);
        }
        else if (pipe_size > 1) {
            int i;
            pid_t pid;
            // in handles the file descriptor for input
            int in = 0, fd[2];
            for (i=0; i<pipe_size - 1; ++i) {
                pipe(fd);
                args = shell_split_line(pipes[i], &position);
                status = shell_launch(args, position, in, fd[1], 1);
                close(fd[1]);
                in = fd[0];
            }
            // if (in != 0)
            //     dup2 (in, 0);
            args = shell_split_line(pipes[i], &position);
            status = shell_launch(args, position, in, 1, 1);
        }
        // memory cleaning
        free(line);
        free(args);
        fflush(stdin);
        fflush(stdout);
    } 

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
        error_in_memory();
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
            int lcs=INF;
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
            else if(lcs>word_pos&&lcs!=INF){
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
        else if(c==127){    //for backspace
            if(position<=0)
            continue;
            printf("\b \b");
            position--;
        }
        else if(c==18){ //Search function if ctrl+r is pressed
            reset_input_mode();
            printf("Enter search term: ");
            char search_term[100];
            scanf("%s",search_term);
            search(search_term);
            set_input_mode();

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
                error_in_memory();
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
        error_in_memory();
    }

    token = strtok(line, SHELL_TOK_DELIM);
    while (token != NULL)
    {
        tokens[*position] = token;
        (*position)++;
        
        if ((*position) >= bufsize)
        {
            bufsize += SHELL_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                error_in_memory();
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
        error_in_memory();
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
                error_in_memory();
            }
        }
        token = strtok(NULL, SHELL_PIPE_DELIM);
    }
    tokens[(*size)] = NULL;
    return tokens;
}

int shell_launch(char **args, int position, int in, int out, int wait) 
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

        char ** args2 = (char**)malloc(sizeof(char*)*(size+1));
        for(int i=0;i<=size;i++)
            args2[i] = NULL;

        for(i=0;i<size-count;++i) 
        {
            args2[i] = args[i];
        }

        
        
        if (execvp(args2[0], args2) == -1){

            perror("shell");
        }
        exit(EXIT_FAILURE);
    }
    else if (pid < 0) perror("forking error!");
    else
    {
        // Parent process
        //signal(SIGTSTP, SIG_DFL);
    	if(!strcmp(args[size-1],"&")==0 && wait)
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
        return shell_launch(args, position, 0, 1, 1);
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
    if(history.cnt<MAX_SHOW_HIST){
        for(int i=history.cnt-1;i>=0;i--){
            printf("%s\n",history.data[i]);
        }
        
    }
    else{
        if(history.start>MAX_SHOW_HIST+1){
            for(int i=history.start-1,j=MAX_SHOW_HIST;j>0;i--,j--){
            printf("%s\n",history.data[i]);
            }
        }
        else{
            for(int i=history.start-1;i>=0;i--){
            printf("%s\n",history.data[i]);
            }
            for(int i=history.cnt-1,j=MAX_SHOW_HIST-history.start;j>0;i--,j--){
            printf("%s\n",history.data[i]);
            }

        }
    }
}

int clear_history(char **args){
    history.start=0;
    history.cnt=0;
    for(int i=0;i<MAX_HIST;i++){
        memset(history.data[i],'\0',MAX_COMMAND);
    }
    

}