#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


//this program is used to find the file with the specified file name given a directory, 
//it takes 2 arguments as input, the directory name/path and the file name


/*
format the file name from a path-to-file
*/
char*
fmtname(char *path)
{
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;
  return p+1;
}

/*
finds the file given the path(to a directory or file) and returns the path to the file
*/
void 
find(char *path, char *findname)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if (fd = open(path, O_RDONLY) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_FILE:
        if (!strcmp(findname, fmtname(path))) printf("%s\n", path);
        break;
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            printf("path is too long \n");
            break;
        } 

        //constructing the buffer to store the path for recursive calls
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';
        while (read(fd, &de, sizeof(de) == sizeof(de))){
            if (
                de.inum == 0 || 
                de.inum == 1 || 
                !strcmp(de.name, ".") || 
                !strcmp(de.name, "..")
                )continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;       //null-terminate the path
            find(buf, findname); //recurse to subdirectories
        }
        break;
    default:
        break;
    }
    close(fd);
}


int 
main(int argc, char *argv[])
{
    if (argc != 3){
        printf("find: find <path> <filename>\n");
        exit(0);
    } 
    find(argv[1], argv[2]);
    exit(0);
}