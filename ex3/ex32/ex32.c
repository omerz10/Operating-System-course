/*
 * Omer Zucker
 * ID: 200876548
 * Ex3-part 2
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <dirent.h>
#include <stdbool.h>
#include <wait.h>
#include <unistd.h>

#define LINE_LENGTH 512  // max length of single line in configurations file
#define FLAGS (O_RDWR |O_TRUNC | O_CREAT | O_APPEND)
#define MODE 0666

enum status { DEFAULT = 0, BAD_OUTPUT = 1, SIMILAR_OUTPUT = 2 , GREAT_JOB = 3, NO_C_FILE = 4,
    COMPILATION_ERROR = 5, TIMEOUT = 6};

/*
 * globals
 */
char path_all_students[LINE_LENGTH] = "\0";
char path_input[LINE_LENGTH] = "\0";
char path_output[LINE_LENGTH] = "\0";
char cwd[LINE_LENGTH];
int fd_input;

void getConfigFromFile(char* pth_c);
void errorInSysCall();
void readLine(int fd, char* pth);
void resultsOfAllStudents(int fd_res);
void unlinkTemporaryFiles(char *pth_std);
void putResultInCSV(int fd_res, char* std_name, enum status std_result);
bool checkExtension(char *file);
bool compileCFile(char *d_pth, char* f_name);
enum status handleSingleStudent(char* pth_std);
enum status executeAndCompareFiles(char* pth_std);
enum status compare2Files(char* new_out_pth);


int main(int argc, char* argv[]) {
    int fd_res;
    getcwd(cwd, sizeof(cwd));

    if (argc != 2) {
        perror("no compatible args");
        exit(-1);
    }
    getConfigFromFile(argv[1]);

    // open file for results of all students
    if ((fd_res = open("results.csv", FLAGS, MODE)) < 0) {
        errorInSysCall();
    }
    resultsOfAllStudents(fd_res);

    // close file descriptors
    if(close(fd_res) < 0 || close(fd_input < 0)) {
        errorInSysCall();
    }
    return 0;
}

void resultsOfAllStudents(int fd_res) {
    char path_std[LINE_LENGTH];
    enum status student_result;
    struct dirent *entry;
    DIR *dp;

    // open dir with all students
    if ((dp = opendir(path_all_students)) == NULL) {
        perror("cannot open dir");
        exit(-1);
    }

    // run through all students directories
    while ((entry = readdir(dp)) != NULL) {
        // check whether this is a student directory
        if (entry->d_type ==  DT_DIR) {
            if(strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // create path for student
                sprintf(path_std, "%s/%s", path_all_students, entry->d_name);
                // open file of args input to the c files
                if ((fd_input = open(path_input, O_RDONLY)) < 0) {
                    errorInSysCall();
                }
                student_result = handleSingleStudent(path_std);
                putResultInCSV(fd_res, entry->d_name, student_result);
            }
        }
    }
}

enum status handleSingleStudent(char* pth_std){
    char sub_path[LINE_LENGTH];
    enum status std_result = NO_C_FILE; // default status
    bool has_been_compiled;
    struct dirent *entry;
    DIR *dp;

    // open dir of student
    if ((dp = opendir(pth_std)) == NULL) {
        perror("cannot open dir");
        exit(-1);
    }
    // run through all directories in student
    while((entry = readdir(dp)) != NULL) {
        // is a file
        if (entry->d_type != DT_DIR) {
            // ensure that is a c file
            if (checkExtension(entry->d_name)) {
                has_been_compiled = compileCFile(pth_std, entry->d_name);
                if (has_been_compiled) {
                    // return BAD_OUTPUT or SIMILAR_OUTPUT or GREAT_JOB or TIMEOUT
                    std_result = executeAndCompareFiles(pth_std);
                }
                else {
                    std_result = COMPILATION_ERROR;
                    break;
                }
                unlinkTemporaryFiles(pth_std);
                break;
            }
        }
            // is a directory- go into this directory recursively
        else if (entry->d_type ==  DT_DIR ) {
            // an empty or father directory
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                sprintf(sub_path, "%s/%s", pth_std ,entry->d_name);
                std_result = handleSingleStudent(sub_path);
                // there is a c file this in student directory
                if (std_result != NO_C_FILE) {
                    break;
                }
            }
        }
    }
    if (closedir(dp) < 0 ) {
        errorInSysCall();
    }
    return std_result;
}

enum status executeAndCompareFiles(char* pth_std) {
    char* argv[] = {"./a.out", NULL}, new_out_pth[LINE_LENGTH];
    int d1, d2, fd_output, state_ptr;
    pid_t father_res;
    enum status result = DEFAULT;

    chdir(pth_std);
    if((fd_output = open("output.txt",FLAGS, MODE)) < 0) {
        errorInSysCall();
    }
    d1 = dup2(fd_input, 0);
    d2 = dup2(fd_output, 1);
    if (d1 < 0 || (d2 < 0)) {
        errorInSysCall();
    }
    pid_t child_pid = fork();
    if (child_pid < 0) {
        errorInSysCall();
    }
        // child process
    else if (child_pid == 0) {
        execvp(argv[0], argv);
    }
        // father process
    else {
        sleep(5);
        father_res = waitpid(child_pid, &state_ptr, WNOHANG);
        d1 = dup2(0,fd_input);
        d2 = dup2(1,fd_output);
        if (d1 < 0 || (d2 < 0)) {
            errorInSysCall();
        }
        if (father_res < 0) {
            errorInSysCall();
        }
        if(father_res == 0)
            return TIMEOUT;
        // execution ended within 5 seconds
        if ((father_res > 0)) {
            snprintf(new_out_pth, sizeof(new_out_pth),"%s/output.txt", pth_std);
            // compare between files
            result = compare2Files(new_out_pth);
        }
        else {
            result = TIMEOUT;
        }
    }
    if (close(fd_output) < 0) {
        errorInSysCall();
    }
    return result;
}

enum status compare2Files(char* new_out_pth) {
    char *argv[] = {"./comp.out", new_out_pth, path_output, NULL};
    int state_ptr = 0;

    enum status result = DEFAULT;
    pid_t pid = fork();
    if (pid < 0) {
        errorInSysCall();
    }
        // child process
    else if (pid == 0) {
        if(chdir(cwd) < 0)
            errorInSysCall();
        execvp(argv[0], argv);
    }
        // father process
    else {
        waitpid(pid, &state_ptr, 0);
        result = (enum status) WEXITSTATUS(state_ptr);
    }
    return result;
}

bool checkExtension(char *file) {
    int size = (int)strlen(file);

    if(file[size-2] == '.' && file[size-1] == 'c'){
        return true;
    }
    return false;
}

bool compileCFile(char *d_pth, char* f_name) {
    char *argv[] = {"gcc", f_name, NULL};
    int state_ptr;

    pid_t pid = fork();
    if (pid < 0) {
        errorInSysCall();
    }
        // child process
    else if (pid == 0) {
        chdir(d_pth);
        execvp(argv[0], argv);
    }
        // father process
    else {
        waitpid(pid, &state_ptr, WUNTRACED);
        // compilation succeed
        if (WIFEXITED(state_ptr) == 1 && WEXITSTATUS(state_ptr) == 0) {
            return true;
        }
    }
    return false;
}

void unlinkTemporaryFiles(char *pth_std) {
    char a_out_f[LINE_LENGTH] = "\0";
    char output_f[LINE_LENGTH] = "\0";

    snprintf(a_out_f, sizeof(a_out_f), "%s/a.out", pth_std);
    snprintf(output_f, sizeof(output_f), "%s/output.txt", pth_std);

    // unlink a.out file
    if (unlink(a_out_f) < 0) {
        errorInSysCall();
    }
    // unlink output file
    if (unlink(output_f) < 0) {
        errorInSysCall();
    }
}

void getConfigFromFile(char* pth_c) {
    int fd;
    if ((fd = open(pth_c, O_RDONLY)) < 0) {
        errorInSysCall();
    }
    // read 3 lines from file
    readLine(fd, path_all_students);
    readLine(fd, path_input);
    readLine(fd, path_output);

    if (close(fd) < 0) {
        errorInSysCall();
    }
}

void readLine(int fd, char* pth) {
    ssize_t f_read;
    int i=0;

    do {
        f_read = read(fd, &pth[i], 1);
        if (f_read < 0) {
            errorInSysCall();
        }
        i++;
    } while (pth[i-1] != '\n' && pth[i] != EOF);
    pth[i-1] = '\0';
}

void errorInSysCall() {
    char buff[] = "Error in system call";
    write(2, buff,sizeof(buff));
}

void putResultInCSV(int fd_res, char* std_name, enum status std_result) {
    int d = dup2(fd_res, 1);
    if (d < 0) {
        errorInSysCall();
    }
    switch (std_result) {
        case BAD_OUTPUT: {
            printf("%s,60,BAD_OUTPUT\n", std_name);
            break;
        }
        case SIMILAR_OUTPUT: {
            printf("%s,80,SIMILAR_OUTPUT\n", std_name);
            break;
        }
        case GREAT_JOB: {
            printf("%s,100,GREAT_JOB\n", std_name);
            break;
        }
        case NO_C_FILE: {
            printf("%s,0,NO_C_FILE\n", std_name);
            break;
        }
        case COMPILATION_ERROR: {
            printf("%s,0,COMPILATION_ERROR\n",std_name);
            break;
        }
        case TIMEOUT: {
            printf("%s,0,TIMEOUT\n", std_name);
            break;
        }
        default: { }
    }
    d = dup2(1, fd_res);
    if (d < 0) {
        errorInSysCall();
    }
}
