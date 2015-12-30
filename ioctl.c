#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <string.h>
#include <net/if_arp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <netinet/in.h>
int main(int argc, char *argv[])
{
    int s;
    int error = -1;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if( s < 0)
    {
        printf("socket errot!\n");
	return -1;
    }
    {
        struct ifreq ifr;
	memcpy(ifr.ifr_name, "eth2", 5);
        
	//get MTU
	error = ioctl(s, SIOCGIFMTU, &ifr);
	if(error)
	{
	    printf("SIOCGIFMTU error!\n");
	}
	else
	{
	    printf("MTU:%d\n",ifr.ifr_mtu);
	}
        
	//get MAC address
	error = ioctl(s, SIOCGIFHWADDR, &ifr);
	if(error)
	{
	    printf("SIOCGIFHWADDR error!\n");
	}
	else
	{
	    char *hw = ifr.ifr_hwaddr.sa_data;
	    printf("mac address:%02x:%02x:%02x:%02x:%02x:%02x\n",hw[0],hw[1],hw[2],hw[3],hw[4],hw[5]);
	}

	//get IP address
	error = ioctl(s, SIOCGIFADDR, &ifr);
	if(error)
	{
	    printf("SIOCGIFADDR error!\n");
	}
	else
	{
	    struct sockaddr_in *sin = (struct sockaddr_in *) &ifr.ifr_addr;
	    char ip[16];
	    memset(ip, 0, 16);

	    inet_ntop(AF_INET, &sin_addr.s_addr, ip, 16);
	    printf("IP address: %s\n", ip);
	}

    }
}
