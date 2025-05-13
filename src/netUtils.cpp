#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <errno.h>

#include <string>  
#include <fstream>  
#include <sstream>  
using namespace std;

#include "main.h"
#include "logMsg.h"


/*******************************************************************************
* get local IP by NIC interface name
*******************************************************************************/
string GetLocalIp(const char *netif)  
{  
    int sock_get_ip;  
    char ipaddr[50];  
  
    struct sockaddr_in *sin;  
    struct ifreq ifr_ip;     
  
    if ((sock_get_ip=socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {  
         printf("socket create failse...GetLocalIp!\n");  
         return "";  
    }  
     
    memset(&ifr_ip, 0, sizeof(ifr_ip));     
    strncpy(ifr_ip.ifr_name, netif, sizeof(ifr_ip.ifr_name) - 1);     
   
    if( ioctl( sock_get_ip, SIOCGIFADDR, &ifr_ip) < 0 )     
    {     
         return "";     
    }       
    sin = (struct sockaddr_in *)&ifr_ip.ifr_addr;     
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));         
      
    printf("local ip:%s \n",ipaddr);      
    close( sock_get_ip );  
      
    return string( ipaddr );  
}  

/*******************************************************************************
* get local MAC by NIC interface name
*******************************************************************************/
string GetLocalMac(const char* netif)
{
    int sockfd;
    struct ifreq tmp;
    char macAddr[30];

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if (sockfd<0)
    {
        // cout<<"create socket fail"<<endl;
        return "";
    }

    memset(&tmp,0,sizeof(struct ifreq));

    strncpy(tmp.ifr_name, netif, sizeof(tmp.ifr_name)-1);

    if ((ioctl(sockfd,SIOCGIFHWADDR,&tmp))<0)
    {
        //cout<<"mac ioctl error"<<endl;
		close(sockfd);
        return "";
    }

    sprintf(macAddr,"%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)tmp.ifr_hwaddr.sa_data[0],
            (unsigned char)tmp.ifr_hwaddr.sa_data[1],
            (unsigned char)tmp.ifr_hwaddr.sa_data[2],
            (unsigned char)tmp.ifr_hwaddr.sa_data[3],
            (unsigned char)tmp.ifr_hwaddr.sa_data[4],
            (unsigned char)tmp.ifr_hwaddr.sa_data[5]
           );
    close(sockfd);
   	
	string strmac = macAddr;
    return strmac;
}


/*******************************************************************************
* 
*******************************************************************************/
string get_hostname()
{
    char szHostName[256]={0};
    gethostname(szHostName, 256);
    logInfo("get hostname: %s\n",szHostName);
    return string(szHostName);
}


/*******************************************************************************
* 
*******************************************************************************/
string get_remove_ip_by_socket(int sock)
{
    struct sockaddr_in guest;
    char guest_ip[20];

    if (sock <= 0)
        return "0.0.0.0";
    
    socklen_t guest_len = sizeof(guest);
    
    getpeername(sock, (struct sockaddr *)&guest, &guest_len);
    
    inet_ntop(AF_INET, &guest.sin_addr, guest_ip, sizeof(guest_ip));

    string clientip = guest_ip;
    return clientip;
}

/*******************************************************************************
* 
*******************************************************************************/
string get_local_ip_by_socket(int sock)
{
    struct sockaddr_in serv;
    char serv_ip[20];

    if (sock <= 0)
        return "0.0.0.0";
    
    socklen_t serv_len = sizeof(serv);
    
    getsockname(sock, (struct sockaddr *)&serv, &serv_len);
    
    inet_ntop(AF_INET, &serv.sin_addr, serv_ip, sizeof(serv_ip));

    string clientip = serv_ip;
    return clientip;
}

