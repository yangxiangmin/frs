#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

string GetLocalIp(const char *netif);
string GetLocalMac(const char* netif);

string get_hostname();

string get_remove_ip_by_socket(int sock);
string get_local_ip_by_socket(int sock);

#endif /* __NET_UTILS_H__ */
