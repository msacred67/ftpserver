#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<fcntl.h>

int main(int argc,char *argv[])
{

    struct sockaddr_in server, client;
    struct stat file;
    int sock, sockcli;
    int k, length, i;
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock == -1)
    {
        perror(strerror(errno));
        exit(1);
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(4444);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    k = bind(sock,(struct sockaddr *) &server,sizeof(server));

    if(k == -1)
    {
        perror(strerror(errno));
        exit(1);
    }

    k = listen(sock,1);

    if(k == -1)
    {
        perror(strerror(errno));
        exit(1);
    }

    length = sizeof(client);
    sockcli = accept(sock,(struct sockaddr*)&client, &length);
    i = 1;
    char buf[100], command[100], filename[20];
    int size, c;
    int filehandle;
    while(1)
    {
        read(sockcli, buf, 100);
        sscanf(buf, "%s", command);
        if(!strcmp(command,"get"))
        {
            sscanf(buf, "%s%s", filename, filename);
            stat(filename, &file);
            filehandle = open(filename, O_RDONLY);
            if(filehandle == -1)
                size = 0;
            size = file.st_size;
            write(sockcli, &size, sizeof(int));
            if(size)
                sendfile(sockcli, filehandle, NULL, size);
        }
        else if(!strcmp(command, "put"))
        {
            int c = 0, len;
            char *f;
            sscanf(buf, "%s%s", filename, filename);
            read(sockcli, &size, sizeof(int));
            i = 1;
            while(1)
            {
                filehandle = open(filename, O_CREAT | O_EXCL | O_WRONLY, 0666);
                if(filehandle == -1)
                {
                    sprintf(filename + strlen(filename), "%d", i);
                }
                else
                break;
            }
            f = malloc(size);
            read(sockcli, f, size);
            c = write(filehandle, f, size);
            close(filehandle);
            write(sockcli, &c, sizeof(int));
        }
        else if(!strcmp(command, "ls"))
        {
            system("ls >temps.txt");
            stat("temps.txt",&file);
            size = file.st_size;
            write(sockcli, &size, sizeof(int));
            filehandle = open("temps.txt", O_RDONLY);
            sendfile(sockcli,filehandle,NULL,size);
        }
        else if(!strcmp(command, "bye") || !strcmp(command, "quit"))
        {
            printf("FTP server ditutup..\n");
            i = 1;
            write(sockcli, &i, sizeof(int));
            exit(0);
        }
    }
    return 0;
}
