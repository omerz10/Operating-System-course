/*
 * Omer Zucker
 * Ex31
 * 200876548
 */


#include <fcntl.h>
#include <zconf.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>


#define MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
 * Names of temporary files
 */
char* t_file1 = "temp1.txt";
char* t_file2 = "temp2.txt";

enum status {Failed = -1, Different = 1, Similar = 2, Identical = 3};

/*
 * The function read content from 2 files and compare between buffers.
 */
int compareFiles(char* pth1, char* pth2);

/*
 * The function check whether files are:
 * Identical - 3
 * Similar - 2
 * Different - 1
 */
int levelOfResemblance(char* pth1, char* pth2);

/*
 * The function check if files are Similar or Different
 */
int similarFiles(char* pth1, char* pth2);

/*
 * The function lowers capital letters
 */
void lowerCase(char* c);

/*
 * The function convert content from one file to a temporary file.
 * By removing chars which are not letters
 */
void convertTextIntoTempFiles(int fd, int t_fd);


int main(int args, char* argv[]) {
    int status;

    if (args != 3) {
        perror("no compatible args");
        exit(Failed);
    }

    char* path1 = argv[1];
    char* path2 = argv[2];

    status = levelOfResemblance(path1, path2);

    switch (status) {
        case 1:
            return Different;

        case 2:
            return Similar;

        case 3:
            return Identical;

        default:
            exit(Failed);
    }
}


void lowerCase(char* c) {
    // c1 is a big letter
    if ((*c >= 65) && (*c <= 90)) {
        *c += 32;
    }
}


void convertTextIntoTempFiles(int fd, int t_fd) {
    int rd;
    char buff = '\0';

    // run through first file and read its content
    while ((rd = (int)read(fd, &buff, sizeof(char))) > 0) {

        if (rd < 0) {
            perror("cannot read from file");
            exit(Failed);
        }

              // buff is not a letter
        if ( buff == ' ' || buff == '\0' || buff == '\n' || buff == '\t') {
            continue;
        }

        lowerCase(&buff);  // convert a big letter to small one

        if (write(t_fd, &buff, sizeof(char)) == Failed) {
            perror("cannot write to the file");
            exit(Failed);
        }
    }
    // close fd & t_fd
    if (close(fd) < 0 || close(t_fd) < 0){
        perror("cannot close one of the files");
        exit(Failed);
    }
}


int compareFiles(char* pth1, char* pth2) {
    char buff1 = '\0', buff2 = '\0';
    int f_read1, f_read2, status;

    // open the files
    int fd1 = open(pth1, O_RDONLY);
    int fd2 = open(pth2, O_RDONLY);
    if (fd1 == Failed || fd2 == Failed) {
        perror("cannot open one of the files");
        exit(Failed);
    }

    // run through files and read their content
   do {
        f_read1 = (int)read(fd1, &buff1, sizeof(char));
        f_read2 = (int)read(fd2, &buff2, sizeof(char));
        if (f_read1 == Failed || f_read2 == Failed) {
            perror("cannot read from one of the files");
            exit(Failed);
        }

        if (buff1 != buff2) {
           break;
        }

    } while (f_read1 != 0 && f_read2 != 0); // run until the end of 2 files

    // close the files
    if (close(fd1) == Failed || close(fd2) == Failed) {
        perror("cannot close one of the files");
        exit(Failed);
    }

    // temp buffers aren't the same
    return (f_read1 == 0 && f_read2 == 0) ? 1 : 0;
}


int levelOfResemblance(char* pth1, char* pth2) {
    int status, resemblance;

    resemblance = compareFiles(pth1, pth2);

     if (resemblance){
        return Identical;
    }
    // files are similar or different
    return similarFiles(pth1, pth2);

}


int similarFiles(char* pth1, char* pth2) {
    int s_status;

    // open the files
    int fd1 = open(pth1, O_RDONLY);
    int fd2 = open(pth2, O_RDONLY);
    if (fd1 < 0 || fd2 < 0) {
        perror("cannot open one of the files");
        exit(Failed);
    }
    // create 2 temporary "fixed" files without spaces and big letters
    int t_fd1 = open(t_file1, O_RDWR |O_TRUNC | O_CREAT, MODE);
    int t_fd2 = open(t_file2, O_RDWR | O_TRUNC | O_CREAT, MODE);
    if (t_fd1 < 0 || t_fd2 < 0) {
        perror("cannot create one of the temporary files");
        exit(Failed);
    }

    // copy only small letter into temporary files
    convertTextIntoTempFiles(fd1, t_fd1);
    convertTextIntoTempFiles(fd2, t_fd2);

    s_status = compareFiles(t_file1, t_file2 );

    // remove temporary files
    if (remove("temp1.txt") < 0 || remove("temp2.txt") < 0) {
        perror("cannot remove one of the files");
        exit(Failed);
    }

    return (s_status == 1 ? Similar : Different);

}







