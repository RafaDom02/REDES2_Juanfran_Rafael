#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include "ip.h"

/********
* FUNCIÓN: char *getIP(char *interface)
* ARGS_IN: char *interface - nombre de la interfaz en la que se desea usar el socket,
*							 en caso de pasar "Default" se usará para que este disponible en el router, ejemplo: eth0
* DESCRIPCIÓN: Dada una interfaz, obtenemos su IP en una string
* ARGS_OUT: char * - IP de la interfaz
********/
char *getIP(char *interface)
{

	int fd;
	struct ifreq ifr;
	FILE *f;
	char line[100], *p = NULL, *c;
	char *ip = NULL;

	if (strcmp(interface, "Default") == 0)
	{

		f = fopen("/proc/net/route", "r");

		while (fgets(line, 100, f))
		{
			p = strtok(line, " \t");
			c = strtok(NULL, " \t");

			if (p != NULL && c != NULL)
			{
				if (strcmp(c, "00000000") == 0)
				{
					break;
				}
			}
		}
	}
    else{
        p = interface;
    }
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, p, IFNAMSIZ - 1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	if (!ip)
	{
		return NULL;
	}
	return ip;
}