#ifndef __HTTP_UTILS_H__
#define __HTTP_UTILS_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <string>  
#include <fstream>  
#include <sstream>  
#include <vector>  
#include <string>  
#include <iostream>
using namespace std;

void SetResponseMsg(Response &res, int status, int code, string msg);
void SetResponseMsgWithData(Response &res, int status, int code, string msg, Value data);

#endif /* __HTTP_UTILS_H__ */
