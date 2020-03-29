#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/mman.h>

#define FIFO_NAME_1 "RESP_PIPE_87463"
#define FIFO_NAME_2 "REQ_PIPE_87463"
#define SHM_KEY 12804

int main(void)
{
    int fd[2] = {-1};
    char string[] = {0x07, 0x43, 0x4f, 0x4e, 0x4e, 0x45, 0x43, 0x54};
    char request[50] = {0};
    unsigned int size = 0;
    int shmId, number = 0;
    off_t sizeFile;

    if(mkfifo(FIFO_NAME_1, 0600) != 0)
    {
        perror("Err creating FIFO1");
        return 1;
    }

    fd[0] = open(FIFO_NAME_1, O_RDWR);
    fd[1] = open(FIFO_NAME_2, O_RDWR);

    if(fd[0] == -1)
    {
        perror("Could not open FIFO1");
        return 1;
    }

    if(fd[1] == -1)
    {
        perror("Could not open FIFO2");
        return 1;
    }

    write(fd[0], string, sizeof(string));
    printf("SUCCESS");

    char* data = NULL;
    char *datas = NULL;
    for(;;)
    {
        size = 0;
        read(fd[1], &size, sizeof(char) * 1);       //citesc size ca sa stiu unde pun caracterul null
        read(fd[1], request, sizeof(char) * size);  //citesc urmatorii size octeti pentru a lua doar request name
        request[size] = 0;                          //pun caracterul NULL pentru a putea lucra cu anumite functii

        if(strcmp(request, "PING") == 0)
        {
            char stringResponsePing[] = {0x04, 0x50, 0x49, 0x4e, 0x47};
            write(fd[0], stringResponsePing, sizeof(stringResponsePing));
            stringResponsePing[2] = 0x4f;
            int ResponseNr = 87463;
            write(fd[0], stringResponsePing, sizeof(stringResponsePing));
            write(fd[0], &ResponseNr, sizeof(ResponseNr));
        }

        if(strcmp(request, "CREATE_SHM") == 0)
        {
            read(fd[1], &number, sizeof(number));
            shmId = shmget(SHM_KEY, sizeof(char) * number, IPC_CREAT | 0664);
            char ResponseSharedMemory[] = {0x0A, 0x43, 0x52, 0x45, 0x41, 0x54, 0x45, 0x5f, 0x53, 0x48, 0x4d};  //CREATE_SHM
            if(shmId < 0)
            {
                //perror("Could not aquire shm");
                char ErrorMes[] = {0x05, 0x45, 0x52, 0x52, 0x4f, 0x52};  //ERROR
                write(fd[0], ResponseSharedMemory, sizeof(ResponseSharedMemory));
                write(fd[0], ErrorMes, sizeof(ErrorMes));
                //return 1;
            }

            else
            {
                char SuccessMes[] = {0x07, 0x53, 0x55, 0x43, 0x43, 0x45, 0x53, 0x53};  //SUCCESS
                write(fd[0], ResponseSharedMemory, sizeof(ResponseSharedMemory));
                write(fd[0], SuccessMes, sizeof(SuccessMes));
            }
        }

        if(strcmp(request, "WRITE_TO_SHM") == 0)
        {
            unsigned int offset = 0;
            unsigned int value = 0;

            read(fd[1], &offset, sizeof(offset));
            read(fd[1], &value, sizeof(value));
            //printf("\n%d\n%d\n", offset, value);
            char ResponseWriteMemory[] = {0x0C, 0x57, 0x52, 0x49, 0x54, 0x45, 0x5f, 0x54, 0x4f, 0x5f, 0x53, 0x48, 0x4d};  // WRITE_TO_SHM
            if((0 < offset && offset < number) && (offset + sizeof(value) < number))
            {
                //atunci putem scrie
                data = (char*)shmat(shmId, NULL, 0);

                if(data == (void*)-1)
                {
                    perror("Could not attach to shm");
                    return 1;
                }
                *(int*)(data + offset) = value;
                char SuccessMes[] = {0x07, 0x53, 0x55, 0x43, 0x43, 0x45, 0x53, 0x53};  //SUCCESS
                write(fd[0], ResponseWriteMemory, sizeof(ResponseWriteMemory));
                write(fd[0], SuccessMes, sizeof(SuccessMes));
            }
            else
            {
                char ErrorMes[] = {0x05, 0x45, 0x52, 0x52, 0x4f, 0x52};  //ERROR
                write(fd[0], ResponseWriteMemory, sizeof(ResponseWriteMemory));
                write(fd[0], ErrorMes, sizeof(ErrorMes));
            }
//            shmdt(data);
//            //data = NULL;
//            shmctl(shmId, IPC_RMID, 0);
        }


        if(strcmp(request, "MAP_FILE") == 0)
        {
            int file = -1;
            char file_name[20];
            unsigned int numberFile = 0;;
            read(fd[1], &numberFile, sizeof(char) * 1);
            read(fd[1], file_name, sizeof(char) * numberFile);
            file_name[numberFile] = 0;

            printf("%s", file_name);
            file = open(file_name, O_RDONLY);
            if(file == -1)
            {
                perror("Could not open input file");
                //return 1;
            }

            sizeFile = lseek(file, 0, SEEK_END);
            //lseek(file, 0, SEEK_SET);
            char ResponseWriteMemory[] = {0x08, 0x4d, 0x41, 0x50, 0x5f, 0x46, 0x49, 0x4c, 0x45};  // MAP_FILE
            datas = (char*)mmap(NULL, sizeFile, PROT_READ, MAP_PRIVATE, file, 0);
            if(datas == (void*)-1)
            {
                char ErrorMes[] = {0x05, 0x45, 0x52, 0x52, 0x4f, 0x52};  //ERROR
                write(fd[0], ResponseWriteMemory, sizeof(ResponseWriteMemory));
                write(fd[0], ErrorMes, sizeof(ErrorMes));
                close(file);
            }
            else
            {
                char SuccessMes[] = {0x07, 0x53, 0x55, 0x43, 0x43, 0x45, 0x53, 0x53};  //SUCCESS
                write(fd[0], ResponseWriteMemory, sizeof(ResponseWriteMemory));
                write(fd[0], SuccessMes, sizeof(SuccessMes));
            }

            //munmap(datas, size);
        }

        if(strcmp(request, "READ_FROM_FILE_OFFSET") == 0)
        {
            unsigned int fileOffset;
            unsigned int nrOfBytes;
            read(fd[1], &fileOffset, sizeof(fileOffset));
            read(fd[1], &nrOfBytes, sizeof(nrOfBytes));
            char ResponseOffsetFile[] = {0x15, 0x52, 0x45, 0x41, 0x44, 0x5f, 0x46, 0x52, 0x4f, 0x4d, 0x5f, 0x46, 0x49, 0x4c, 0x45, 0x5f, 0x4f, 0x46, 0x46, 0x53, 0x45, 0x54};
            int i;

            if(fileOffset + nrOfBytes < sizeFile)
            {
                char *data = (char *)shmat(shmId,NULL,0);
                if(data == (void *) -1){
                    perror("Could not attach to shm");
                    return 1;
                }

                for(i = fileOffset; i <= fileOffset + nrOfBytes; i++)
                {
                    data[i - fileOffset] = datas[i];
                }
                write(fd[0], ResponseOffsetFile, sizeof(ResponseOffsetFile));
                char SuccessMes[] = {0x07, 0x53, 0x55, 0x43, 0x43, 0x45, 0x53, 0x53};  //SUCCESS
                write(fd[0], SuccessMes, sizeof(SuccessMes));
            }

            else
            {
                char ErrorMes[] = {0x05, 0x45, 0x52, 0x52, 0x4f, 0x52};  //ERROR
                write(fd[0], ResponseOffsetFile, sizeof(ResponseOffsetFile));
                write(fd[0], ErrorMes, sizeof(ErrorMes));
            }
        }

        if(strcmp(request, "READ_FROM_FILE_SECTION") == 0)
        {
            unsigned int sectionNo;
            unsigned int sfOffset;
            unsigned int nrSFBytes;
            read(fd[1], &sectionNo, sizeof(sectionNo));
            read(fd[1], &sfOffset, sizeof(sfOffset));
            read(fd[1], &nrSFBytes, sizeof(nrSFBytes));
            char ResponseSFSection[] = {0x16, 0x52, 0x45, 0x41, 0x44, 0x5f, 0x46, 0x52, 0x4f, 0x4d, 0x5f, 0x46, 0x49, 0x4c, 0x45, 0x5f, 0x53, 0x45, 0x43, 0x54, 0x49, 0x4f, 0x4e};

            //afla: headerSize -> myNoOfSection -> ajungi la inceputul sectiunii dorite -> citesti offset-ul -> citesti informatia de lungimea ceruta de la offset-ul format din cei 4 bytes

            short int headerSize2 = 0, headerSize1 = 0, headerSize = 0;

            //acum merg la sfarsit pentru a afla header size
            headerSize2 = datas[sizeFile - 6];  //am luat cei doi octeti
            headerSize1 = datas[sizeFile - 5];
            headerSize = (headerSize1 * 256) + headerSize2;

            char myNoOfSection = datas[sizeFile - headerSize + 4];

            //printf("\n%d\n", myNoOfSections);

            if(sectionNo > myNoOfSection)
            {
                char ErrorMes[] = {0x05, 0x45, 0x52, 0x52, 0x4f, 0x52};  //ERROR
                write(fd[0], ResponseSFSection, sizeof(ResponseSFSection));
                write(fd[0], ErrorMes, sizeof(ErrorMes));
            }
            else
            {
		//iau 4 bytes pentru offset incepand de la 24*(sectionNo - 1) + 16 + 5 pentru ca acolo incepe offset-ul in header
                int myOffset = *((int*)(datas+(sizeFile - headerSize + 24 * (sectionNo - 1) + 16 + 5)));
                char *dataForOffset = (char *)shmat(shmId, NULL, 0);
                if(dataForOffset == (void *) -1)
                {

                    //perror("Could not attach to shm");
                    //return 1;
                    char ErrorMes[] = {0x05, 0x45, 0x52, 0x52, 0x4f, 0x52};  //ERROR
                    write(fd[0], ResponseSFSection, sizeof(ResponseSFSection));
                    write(fd[0], ErrorMes, sizeof(ErrorMes));
                }
                int i;

                for(i = 0; i < nrSFBytes; i++)
                {
                    dataForOffset[i] = datas[i +  sfOffset + myOffset];
                }

                char SuccessMes[] = {0x07, 0x53, 0x55, 0x43, 0x43, 0x45, 0x53, 0x53};  //SUCCESS
                write(fd[0], ResponseSFSection, sizeof(ResponseSFSection));
                write(fd[0], SuccessMes, sizeof(SuccessMes));
        }
}

        if(strcmp(request, "READ_FROM_LOGICAL_SPACE_OFFSET") == 0)
        {
            unsigned int logicalOffset, noOfBytes = 0;
            read(fd[1], &logicalOffset, sizeof(logicalOffset));
            read(fd[1], &noOfBytes, sizeof(noOfBytes));
            char SuccessMes[] = {0x07, 0x53, 0x55, 0x43, 0x43, 0x45, 0x53, 0x53};   //SUCCESS
            char ResponseLogicalSpace[] = {30, 'R', 'E', 'A' , 'D', '_', 'F', 'R', 'O', 'M', '_', 'L', 'O', 'G','I', 'C', 'A', 'L', '_', 'S', 'P', 'A', 'C', 'E', '_', 'O', 'F', 'F', 'S', 'E', 'T'};
            write(fd[0], ResponseLogicalSpace, sizeof(ResponseLogicalSpace));
            write(fd[0], SuccessMes, sizeof(SuccessMes));
        }

        if(strcmp(request, "EXIT") == 0)
        {
            close(fd[0]);
            close(fd[1]);
            unlink(FIFO_NAME_1);
            return 1;
        }
    }
}
