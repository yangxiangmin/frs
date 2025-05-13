#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <fcntl.h>   
#include <dirent.h>

#include <string>  
#include <fstream>  
#include <sstream>  
#include <vector>
using namespace std;

#include "json/json.h"
using namespace Json;

#include "main.h"
#include "httpUtils.h"  
#include "stringUtils.h"  


/*******************************************************************************
*
*******************************************************************************/
void SetResponseMsg(Response &res, int status, int code, string msg)
{
    Value resp;
    resp["msg"] = msg;
    resp["code"] = code;
    string json = json_unformat_string(resp);
    
    res.status = status;
    res.set_content(json, "application/json");

}

/*******************************************************************************
*
*******************************************************************************/
void SetResponseMsgWithData(Response &res, int status, int code, string msg, Value data)
{
    Value resp;
    resp["msg"] = msg;
    resp["code"] = code;
    resp["data"] = data;
    string json = json_unformat_string(resp);
    
    res.status = status;
    res.set_content(json, "application/json");

}

