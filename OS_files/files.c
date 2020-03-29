#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


void listItems(const char* dirPath, int nrArg, char** argv);
void listRecursive(const char* dirPat2h);
void listRecursiveOption2(const char* dirPath, char* s);
void listRecursiveOption1(const char* dirPath, int value);
int permissionFunction(struct stat file, char *s);
void parseItems(int nrArg, char** argv);
void extractSection(int nrArg, char** argv);
void findallItems(const char* dirPath, int argc, char** argv, int* cntSuccess);
int isSF(int argc, char** argv, const char* dirPath);


struct section
{
    char sectName[15];
    short int sectType;
    int sectOffset;
    int sectSize;
};
typedef struct section section;

int main(int argc, char **argv)
{
    if(argc >= 2)
    {
        if(strcmp(argv[1], "variant") == 0)
        {
            printf("87463\n");
        }

        else if(strcmp(argv[1], "list") == 0)
        {
            char dirPath[400];
            strcpy(dirPath, argv[argc - 1] + 5);      // iau calea din argument fara "path="
            printf("SUCCESS\n");
            listItems(dirPath, argc, argv);
        }

        else if(strcmp(argv[1], "parse") == 0)
        {
            parseItems(argc, argv);
        }

        else if(strcmp(argv[1], "extract") == 0)
        {
            extractSection(argc, argv);
        }

        else if(strcmp(argv[1], "findall") == 0)
        {
            int cntSuccess = 0;
            char dirPath[400];
            strcpy(dirPath, argv[2] + 5);
            findallItems(dirPath,  argc, argv, &cntSuccess);
        }
    }
    return 0;
}


void listItems(const char* dirPath, int nrArg, char** argv)
{
    int value = 0;
    DIR *dir = NULL;
    struct dirent * entry = NULL;
    char filePath[512];
    struct stat statbuf;
    dir = opendir(dirPath);
    if( dir == NULL)
    {
        perror("Could not open directory");
        closedir(dir);
        exit(0);
    }

    if(nrArg == 5) //trebuie sa facem list recursiv + optiune
    {
        if(strcmp(argv[2], "recursive") == 0)  // => argv[3] e optiunea
        {
            if(strstr(argv[3], "permissions"))
            {
                //optiunea noastra e perm_string
                char *s = argv[3] + 12;
                listRecursiveOption2(dirPath, s);
            }
            else
            {
                //optiunea noastra e size_smaller
                int value;
                sscanf(argv[3] + 13, "%d", &value);
                listRecursiveOption1(dirPath, value);
            }
        }
        else  // =>argv[2] e optiunea
        {
            if(strstr(argv[2], "permissions"))
            {
                //optiunea noastra e perm_string
                char *s = argv[2] + 12;
                listRecursiveOption2(dirPath, s);
            }
            else
            {
                //optiunea noastra e size_small
                int value;
                sscanf(argv[2] + 13, "%d", &value);
                listRecursiveOption1(dirPath, value);
            }
        }
    }

    else if(nrArg == 4) //lipseste ori recursive ori optiunea
    {
        if(strcmp(argv[2], "recursive") == 0) //facem list recursiv fara optiune
        {
            listRecursive(dirPath);
        }
        else  //facem list iterativ + optiune
        {
            if(strstr(argv[2], "permissions"))
            {

                //optiunea noastra e perm_string
                char *s = argv[2] + 12;
                while((entry = readdir(dir)) != NULL)
                {
                    if((strcmp(entry->d_name, ".") != 0) && strcmp(entry->d_name, "..") != 0)
                    {
                        snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
                        if(lstat(filePath, &statbuf) == 0)
                        {
                            if(permissionFunction(statbuf, s) == 1)
                                printf("%s\n", filePath);
                        }
                    }
                }
            }
            else
            {
                //optiunea noastra e size_smaller
                while((entry = readdir(dir)) != NULL)
                {
                    if((strcmp(entry->d_name, ".") != 0) && strcmp(entry->d_name, "..") != 0)
                    {
                        snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
                        if(lstat(filePath, &statbuf) == 0)
                        {
                            value = 0;
                            sscanf(argv[2] + 13, "%d", &value);
                            if(S_ISREG(statbuf.st_mode))
                            {

                                if(statbuf.st_size < value)
                                    printf("%s\n", filePath);
                            }
                        }

                    }
                }
            }
        }
    }
    else if(nrArg == 3) //list iterativ fara optiune
    {
        while((entry = readdir(dir)) != NULL)
        {
            if((strcmp(entry->d_name, ".") != 0) && strcmp(entry->d_name, "..") != 0)
            {
                snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
                printf("%s\n", filePath);
            }
        }
    }
    closedir(dir);
}

void listRecursive(const char* dirPath)
{
    DIR *dir = NULL;
    struct dirent * entry = NULL;
    char filePath[512];
    struct stat statbuf;
    dir = opendir(dirPath);
    if( dir == NULL)
    {
        perror("Could not open directory");
        closedir(dir);
        exit(0);
    }
    while((entry = readdir(dir)) != NULL)
    {
        if((strcmp(entry->d_name, ".") != 0) && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
            printf("%s\n", filePath);
            if(lstat(filePath, &statbuf) == 0)
            {
                if(S_ISDIR(statbuf.st_mode))
                {
                    listRecursive(filePath);
                }
            }
        }
    }
    closedir(dir);
}

void listRecursiveOption1(const char* dirPath, int value)
{
    DIR *dir = NULL;
    struct dirent * entry = NULL;
    char filePath[512];
    struct stat statbuf;
    dir = opendir(dirPath);
    if( dir == NULL)
    {
        perror("Could not open directory");
        closedir(dir);
        exit(0);
    }
    while((entry = readdir(dir)) != NULL)
    {
        if((strcmp(entry->d_name, ".") != 0) && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
            if(lstat(filePath, &statbuf) == 0)
            {
                if(S_ISREG(statbuf.st_mode))
                    if(statbuf.st_size < value)
                    {
                        printf("%s\n", filePath);
                    }
                if(S_ISDIR(statbuf.st_mode))
                {
                    listRecursiveOption1(filePath, value);
                }
            }
        }
    }
    closedir(dir);
}

void listRecursiveOption2(const char* dirPath, char* s)
{
    DIR *dir = NULL;
    struct dirent * entry = NULL;
    char filePath[512];
    struct stat statbuf;
    dir = opendir(dirPath);
    if( dir == NULL)
    {
        perror("Could not open directory");
        closedir(dir);
        exit(0);
    }
    while((entry = readdir(dir)) != NULL)
    {
        if((strcmp(entry->d_name, ".") != 0) && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
            if(lstat(filePath, &statbuf) == 0)
            {
                if(S_ISREG(statbuf.st_mode))
                    if(permissionFunction(statbuf, s) == 1)
                        printf("%s\n", filePath);

                if(S_ISDIR(statbuf.st_mode))
                {
                    listRecursiveOption2(filePath, s);
                }
            }
        }
    }
    closedir(dir);
}


int permissionFunction(struct stat file, char *s)
{
    for(int i = 0; i <= 8; i++)
    {
        if(s[i] != '-')
        {
            if((file.st_mode & (1 << (8 - i))) == 0)
            {
                return 0;
            }
        }
    }
    return 1;
}

int IsSF(int argc, char** argv, const char* dirPath)
{
    int cnt = 0;
    int fd = -1;
    fd = open(dirPath, O_RDONLY);

    if(fd == -1)
    {
        perror("Could not open input file");
        close(fd);
        exit(1);
    }
    lseek(fd, -6, SEEK_END);
    short int headerSize;
    char magicNumber[5];

    if(read(fd, &headerSize, 2) != 2)
    {
        perror("Reading error!");
        close(fd);
        exit(2);
    }

    if(read(fd, &magicNumber, 4) != 4)
    {
        perror("Reading error!");
        exit(2);
    }
    magicNumber[4] =0;
    if(strcmp(magicNumber, "4dnI") != 0)
    {
        return 0;
    }

    lseek(fd, -headerSize, SEEK_END);
    int version;
    if(read(fd, &version, 4) != 4)
    {
        perror("Reading error!");
        exit(2);
    }
    if(version < 63 || version > 157)
    {
        return 0;
    }
    int numberSections = 0;
    if(read(fd, &numberSections, 1) != 1)
    {
        perror("Reading error!");
        exit(2);
    }

    if(numberSections < 8 || numberSections > 20)
    {
        return 0;
    }

    int i;
    section sectArray[20];
    for(i = 0; i < numberSections; i++)
    {
        if(read(fd, sectArray[i].sectName, 14) != 14)
        {
            perror("Reading error!");
            exit(2);
        }
        sectArray[i].sectName[14] = 0;
        if(read(fd, &(sectArray[i].sectType), 2) != 2)
        {
            perror("Reading error!");
            exit(2);
        }

        if(sectArray[i].sectType != 29 && sectArray[i].sectType != 12 && sectArray[i].sectType != 45 && sectArray[i].sectType != 62 && sectArray[i].sectType != 72 && sectArray[i].sectType != 59)
        {
            //printf("ERROR\nwrong sect_types");
            return 0;
        }

        if(sectArray[i].sectType == 59)
            cnt++;

        if(read(fd, &(sectArray[i].sectOffset), 4) != 4)
        {
            perror("Reading error!");
            exit(2);
        }

        if(read(fd, &(sectArray[i].sectSize), 4) != 4)
        {
            perror("Reading error!");
            exit(2);
        }
    }

    if(cnt < 2)
    {
        return 0;
    }

    return 1;
    close(fd);
}

void parseItems(int nrArg, char** argv)
{
    int fd = -1;
    char dirPath[400];
    strcpy(dirPath, argv[nrArg - 1] + 5);
    fd = open(dirPath, O_RDONLY);

    if(fd == -1)
    {
        perror("Could not open input file");
        close(fd);
        exit(1);
    }

    lseek(fd, -6, SEEK_END);
    short int headerSize;
    char magicNumber[5];

    if(read(fd, &headerSize, 2) != 2)
    {
        perror("Reading error!");
        exit(2);
    }

    if(read(fd, &magicNumber, 4) != 4)
    {
        perror("Reading error!");
        exit(2);
    }
    magicNumber[4] =0;
    if(strcmp(magicNumber, "4dnI") != 0)
    {
        printf("ERROR\nwrong magic\n");
        return;
    }

    lseek(fd, -headerSize, SEEK_END);
    int version;
    if(read(fd, &version, 4) != 4)
    {
        perror("Reading error!");
        exit(2);
    }
    if(version < 63 || version > 157)
    {
        printf("ERROR\nwrong version");
        return;
    }
    int numberSections = 0;
    if(read(fd, &numberSections, 1) != 1)
    {
        perror("Reading error!");
        exit(2);
    }
    if(numberSections < 8 || numberSections > 20)
    {
        printf("ERROR\nwrong sect_nr");
        return;
    }

    int i;
    section sectArray[20];
    for(i = 0; i < numberSections; i++)
    {
        if(read(fd, sectArray[i].sectName, 14) != 14)
        {
            perror("Reading error!");
            exit(2);
        }
        sectArray[i].sectName[14] = 0;
        if(read(fd, &(sectArray[i].sectType), 2) != 2)
        {
            perror("Reading error!");
            exit(2);
        }

        if(sectArray[i].sectType != 29 && sectArray[i].sectType != 12 && sectArray[i].sectType != 45 && sectArray[i].sectType != 62 && sectArray[i].sectType != 72 && sectArray[i].sectType != 59)
        {
            printf("ERROR\nwrong sect_types");
            return;
        }

        if(read(fd, &(sectArray[i].sectOffset), 4) != 4)
        {
            perror("Reading error!");
            exit(2);
        }
        if(read(fd, &(sectArray[i].sectSize), 4) != 4)
        {
            perror("Reading error!");
            exit(2);
        }
    }
    printf("SUCCESS\n");
    printf("version=%d\n",version);
    printf("nr_sections=%d\n", numberSections);
    for(i = 0; i < numberSections; i++)
        printf("section%d: %s %d %d\n", i + 1, sectArray[i].sectName, sectArray[i].sectType, sectArray[i].sectSize);

    close(fd);
}

void extractSection(int nrArg, char** argv)
{
    char filePath[400];

    strcpy(filePath, argv[2] + 5);

    int sectNr = 0;
    sscanf(argv[3] + 8, "%d", &sectNr);

    int lineNr = 0;
    sscanf(argv[4] + 5, "%d", &lineNr);
    int fd = -1;

    fd = open(filePath, O_RDONLY);
    if(fd == -1)
    {
        perror("ERROR\ninvalid file");
        close(fd);
        exit(1);
    }
    int deplasament = 0;
    deplasament = (sectNr - 1) * 24 + 16;
    short int headerSize;
    lseek(fd, -6, SEEK_END);
    if(read(fd, &headerSize, 2) != 2)
    {
        perror("Reading error!");
        return;
    }
    lseek(fd, -headerSize, SEEK_END);
    int NoSection = 0;
    lseek(fd, 4, SEEK_CUR);
    if(read(fd, &NoSection, 1) != 1)
    {
        perror("Reading error!");
        exit(2);
    }
    if(NoSection < sectNr)
    {
        printf("ERROR\ninvalid section");
        return;
    }
    //citim offset si sect_size

    lseek(fd, deplasament, SEEK_CUR); //am ajuns la inceput de offset
    int sectOffset, sectSize = 0;
    if(read(fd, &sectOffset, 4) != 4)
    {
        perror("Reading error!");
        exit(2);
    }
    if(read(fd, &sectSize, 4) != 4)
    {
        perror("Reading error!");
        exit(2);
    }

    lseek(fd, sectOffset, SEEK_SET);  //am ajuns la sectiunea cautata
    //am ajuns la sectiunea cautata
    //parcurg liniile din sectiune pentru a afla cate sunt
    char c;
    int cntLines = 1;
    int cntChar = sectSize;
    while(cntChar > 0)
    {
        if(read(fd, &c, 1) != 1)
        {
            perror("Reading error");
            exit(2);
        }
        //printf("%c", c);
        cntChar--;
        if(c == '\n')
        {
            //avem o linie noua
            cntLines++;
        }
    }
    if(cntLines < lineNr)
    {
        printf("ERROR\ninvalid line");
        return;
    }

    int found = 0;
    cntChar = 0;
    //char* line = (char *) malloc ((sectSize+2) * sizeof(char));

    lseek(fd, sectOffset, SEEK_SET);
    //printf("%d ", cntLines);

    //de aici
    /*int cnt_temp = cntLines;

    while(cnt_temp != lineNr)
    {
        if(read(fd, &c, 1) != 1)
        {

            perror("Could not read!");
            exit(2);
        }
        if(c == '\n')
            cnt_temp--;
    }
    printf("SUCCESS\n");

    while(cnt_temp == lineNr)
    {
        if(read(fd, &c, 1) != 1)
        {

            perror("Could not read!");
            exit(2);
        }

        if(lseek(fd, 0, SEEK_CUR) > sectSize + sectOffset)
            break;

        if(!(c == '\n' || c == '\0'))
            printf("%c", c);
        else
        {
            printf("\n");
            cnt_temp--;
        }
    }*/



    while(!found)
    {
        if(read(fd, &c, 1) != 1)
        {
            perror("Reading error");
            exit(2);
        }
       cntChar++;
       if(c == '\n')
        {
            if(cntLines == lineNr)
            {
                printf("SUCCESS\n");
                found = 1;
                lseek(fd, -cntChar, SEEK_CUR);
                //printf("%d!!!!\n", cntChar);
                while(cntChar)
                {
                    if(lseek(fd, 0, SEEK_CUR) >= sectOffset + sectSize)
                        break;
                    if(read(fd, &c, 1) != 1)
                    {
                        perror("Reading error");
                        exit(2);
                    }
                    printf("%c", c);
                    fflush(stdout);
                    cntChar--;
                }
                printf("\n");
                //printf("\0");
            }
            cntLines--;
            cntChar = 0;

        }
    }
    close(fd);
    //free(line);
}

void findallItems(const char* dirPath, int argc, char** argv, int* cntSuccess)
{
    DIR *dir = NULL;
    struct dirent * entry = NULL;
    char filePath[512];
    struct stat statbuf;
    dir = opendir(dirPath);
    if( dir == NULL)
    {
        perror("Could not open directory");
        closedir(dir);
        exit(0);
    }

    while((entry = readdir(dir)) != NULL)
    {
        if((strcmp(entry->d_name, ".") != 0) && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(filePath, 512, "%s/%s", dirPath, entry->d_name);
            if(lstat(filePath, &statbuf) == 0)
            {
                if(S_ISDIR(statbuf.st_mode))
                {
                    findallItems(filePath, argc, argv, cntSuccess);
                }
                else if(S_ISREG(statbuf.st_mode))
                {
                    if(IsSF(argc, argv, filePath) == 1)
                    {
                        if((*cntSuccess) == 0)
                        {
                            printf("SUCCESS\n");
                            (*cntSuccess) = 1;
                        }
                        printf("%s\n", filePath);
                        fflush(stdout);
                    }
                }
            }
        }
    }
    closedir(dir);
}
