#include <stdio.h>
#include <unistd.h>
#include <string.h> /* for strncpy */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

char *getIP(char *interface)
{

	int fd;
	struct ifreq ifr;
	FILE *f;
	char line[100], *p, *c;
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

	/* I want to get an IPv4 IP address */
	ifr.ifr_addr.sa_family = AF_INET;

	/* I want IP address attached to "eth0" */
	strncpy(ifr.ifr_name, p, IFNAMSIZ - 1);

	ioctl(fd, SIOCGIFADDR, &ifr);

	close(fd);

	/* display result */
	ip = inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
	if (!ip)
	{
		return NULL;
	}

	return ip;
}