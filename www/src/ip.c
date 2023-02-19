#include <stdio.h>	//printf
#include <string.h>	//memset
#include <errno.h>	//errno
#include <sys/socket.h>	//socket
#include <netinet/in.h> //sockaddr_in
#include <arpa/inet.h>	//getsockname
#include <unistd.h>	//close
#include "ip.h"

const char* getIP()
{
    //Nos conectamos al servidor de google para obtener nuestra IP
    const char* google_dns_server = "8.8.8.8";
    int dns_port = 53;
    struct sockaddr_in name;
    socklen_t namelen = sizeof(name);
    char buffer[100];
    const char* ip;
	
	struct sockaddr_in serv;
    
    int sock = socket ( AF_INET, SOCK_DGRAM, 0);
    
    //Socket could not be created
    if(sock < 0)
    {
		perror("Socket error");
	}
    
	memset( &serv, 0, sizeof(serv) );
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr( google_dns_server );
    serv.sin_port = htons( dns_port );

    if(connect( sock , (const struct sockaddr*) &serv , sizeof(serv) ))
    {
        perror("connection can not be establised .\n");
        close(sock);
        return NULL;
    }
    
    if(getsockname(sock, (struct sockaddr*) &name, &namelen))
    {
        perror("unobtainable sockname.\n");
        close(sock);
        return NULL;
    }
    	
    ip = inet_ntop(AF_INET, &name.sin_addr, buffer, 100);
	if(!ip)
	{
		//Some error
		fprintf(stderr,"Error number : %d . Error message : %s \n" , errno , strerror(errno));
        close(sock);
        return NULL;
	}
    printf("%s\n", ip);
    close(sock);
    
    return ip;
}