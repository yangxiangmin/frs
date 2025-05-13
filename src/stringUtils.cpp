#include <iconv.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/file.h>


#include "main.h"
#include "md5.h"
#include "stringUtils.h"  



/*******************************************************************************
*
*******************************************************************************/
static void md5_digest(const char *input, char *output)
{
    MD5 m5;
    
    if ((NULL == output) || (NULL == input))
    {
        return;
    }
    
    MD5_CTX md5Context;
    unsigned char digest[16] = {0};

    m5.MD5Init(&md5Context);
    m5.MD5Update(&md5Context, (unsigned char *) input, strlen(input));
    m5.MD5Final(digest, &md5Context);

    char *ptr = output;
    for (int i = 0; i < 16; i++)
    {
        ptr += sprintf(ptr, "%2.2x", digest[i]);
    }
}

/*******************************************************************************
*
*******************************************************************************/
string md5(string str)
{
    if (str.empty())
        return "";
    
    char    obuf[64];
    memset(obuf, 0, sizeof(obuf));
    
    md5_digest(str.c_str(), obuf);
    string strout = obuf;
    return strout;
}

/*******************************************************************************
*
*******************************************************************************/
int lockFile(string fileName)
{
    int fd = open(fileName.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0)
    {
        return (-1);
    }

    int ret = flock(fd, LOCK_EX | LOCK_NB);
    if (0 == ret)
    {
        return fd;
    }

    close(fd);

    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
void unlockFile(int fd)
{
    if (fd <= 2)
    {
        return;
    }

    flock(fd, LOCK_UN);

    close(fd);
}

/*******************************************************************************
*
*******************************************************************************/
void generate_token(const char *seed, char *output)
{
    char    inbuf[1024];
    time_t  now;
        
    if ((NULL == output) || (NULL == seed))
    {
        return;
    }
    
    now = time(0);
    sprintf(inbuf, "%s%u", seed, (int)now);
    md5_digest(inbuf, output);
}

/*******************************************************************************
*
*******************************************************************************/
string trim(string str)
{
    if (str.empty())
    {
        return "";
    }
    return rtrim(ltrim(str));
}

/*******************************************************************************
*
*******************************************************************************/
string ltrim(string str)
{
    if (str.empty())
    {
        return "";
    }
    return str.erase(0, str.find_first_not_of(" "));
}

/*******************************************************************************
*
*******************************************************************************/
string rtrim(string str)
{
    if (str.empty())
    {
        return "";
    }
    return str.erase(str.find_last_not_of(" ") + 1);
}

/*******************************************************************************
*
*******************************************************************************/
string int2str(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

/*******************************************************************************
*
*******************************************************************************/
int str2int(string s)
{
    if (s.empty())
        return 0;
    
    int num = 0;
    stringstream ss(s);
    ss >> num;    
    return num;
}

/*******************************************************************************
*
*******************************************************************************/
string str2Upper(string str)
{
    string strTmp = str;
    transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::toupper);
    return strTmp;    
}

/*******************************************************************************
*
*******************************************************************************/
string str2Lower(string str)
{
    string strTmp = str;
    transform(strTmp.begin(), strTmp.end(), strTmp.begin(), ::tolower);
    return strTmp;    
}

/*******************************************************************************
*
*******************************************************************************/
bool strIsDigit(string str)
{
    int len = str.length();

    for (int i = 0; i < len; i++)
    {
        if (str[i] >= '0' && str[i] <= '9')
            continue;        
        else 
            return false;
    }
    
    return true;
}

/*******************************************************************************
* string to cppjson 
********************************************************************************/
int string2json(string str, Value& data)
{
    Reader      reader; 
    Value item;            
    if (true == reader.parse(str, data, false))
    {
        return 0;

    }
    else
    {
        return -1;
    }
    return 0;
}



/*******************************************************************************
*
*******************************************************************************/
void split(std::string src, std::string token, vector<std::string>& vect)   
{   
    int nend   = 0;   
    int nbegin = 0;   
    while (nend != -1)   
    {   
        nend = src.find_first_of(token, nbegin);   
        if (nend == -1)   
        {
            string sub = src.substr(nbegin, src.length()-nbegin);
            if (! sub.empty())
            {
                vect.push_back(sub);   
            }
        }
        else  
        {
            string sub = src.substr(nbegin, nend-nbegin);
            if (! sub.empty())
            {
                vect.push_back(sub);   
            }            
        }
        nbegin = nend + 1;   
    }   
}  

/*******************************************************************************
* enable empty
*******************************************************************************/
void split_ex(std::string src, std::string token, vector<std::string>& vect)   
{   
    int nend   = 0;   
    int nbegin = 0;   
    while (nend != -1)   
    {   
        nend = src.find_first_of(token, nbegin);   
        if (nend == -1)   
        {
            string sub = src.substr(nbegin, src.length()-nbegin);
            vect.push_back(sub);   
        }
        else  
        {
            string sub = src.substr(nbegin, nend-nbegin);
            vect.push_back(sub);   
        }
        nbegin = nend + 1;   
    }   
}  

/*******************************************************************************
*
*******************************************************************************/
string& replace_all(string& str, const string& old_value,const string& new_value)
{
    while (true)
    {
        size_t pos = 0;
        if ((pos = str.find(old_value,0)) != string::npos)
            str.replace(pos,old_value.length(),new_value);
        else 
            break;
    }
    return str;
}

/*******************************************************************************
* replace char
*******************************************************************************/
string Replace(const char * data)
{
    string after = "";
    int len =strlen(data);
    for (int i = 0; i < len; i++)
    {
        if (data[i] == '\\')
            after += "\\\\";
        else if (data[i] == '\"')
            after += "\"";
        else if ((data[i] == '/'))
            after += "\\/";
        else if ((data[i] == '\t'))
            after += "\\t";
        else if ((data[i] == '\n'))
            after += "\\n";
        else if ((data[i] == '\r'))
            after += "\\r";
        else if ((data[i] == '\b'))
            after += "\\b";
        else if ((data[i] == '\f'))
            after += "\\f";
        else
            after += data[i];        
    }
    return after;    
}

/*******************************************************************************
*
*******************************************************************************/
void dump_hex(const char *title, char *buff, int len)
{
    int i;
    char out[128];
    memset(out, 0, sizeof(out));

    printf("---- %s ----\r\n", title==NULL?"(NULL)":title);
    for (i = 0; i < len; i++)
    {
        printf("%02x ", buff[i]&0xFF);
        if ((i+1) % 8 == 0)
            printf("\n");
    }
    printf("\r\n");
}

/*******************************************************************************
*
*******************************************************************************/
string json_format_string(Json::Value data, bool is_format)
{
    string jsonstr = "";
    
    if (is_format)
    {
        Json::StreamWriterBuilder builder;
        builder.settings_["indentation"] = "";
        jsonstr = Json::writeString(builder, data);
        
        // old way
        //jsonstr = data.toStyledString();
    }
    else
    {
        Json::StreamWriterBuilder builder;
        jsonstr = Json::writeString(builder, data);

        // old way    
        //FastWriter  resp_writer;
        //jsonstr = resp_writer.write(data);
    }
    return jsonstr;
}

/*******************************************************************************
*
*******************************************************************************/
string json_unformat_string(Json::Value data)
{
    string jsonstr = "";

    /* cppjson 1.9.5 */
    Json::StreamWriterBuilder builder;
    jsonstr = Json::writeString(builder, data);

    // old way 
    //FastWriter  resp_writer;
    //jsonstr = resp_writer.write(data);
    return jsonstr;
}

/*******************************************************************************
* get current date and time to ms
*******************************************************************************/
string GetCurrentDTstring()
{
    struct timeval  tv;
    struct tm *     time_ptr;
    
    memset(&tv, 0, sizeof(timeval));
    gettimeofday(&tv, NULL);
    time_ptr = localtime(&tv.tv_sec);

    char buf[1024];
    memset(&buf, 0, sizeof(buf));
    
    sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d.%06ld",
        time_ptr->tm_year + 1900,
        time_ptr->tm_mon + 1,
        time_ptr->tm_mday,
        time_ptr->tm_hour,
        time_ptr->tm_min,
        time_ptr->tm_sec,
        tv.tv_usec);

    string v = buf;

    return v;
}

/*******************************************************************************
* get current date and time to ms
* return YYYYMMDD_HHmm
*******************************************************************************/
string GetCurrentDTstring2()
{
    struct timeval  tv;
    struct tm *     time_ptr;
    
    memset(&tv, 0, sizeof(timeval));
    gettimeofday(&tv, NULL);
    time_ptr = localtime(&tv.tv_sec);

    char buf[1024];
    memset(&buf, 0, sizeof(buf));
    
    sprintf(buf, "%d%02d%02d_%02d%02d",
        time_ptr->tm_year + 1900,
        time_ptr->tm_mon + 1,
        time_ptr->tm_mday,
        time_ptr->tm_hour,
        time_ptr->tm_min);

    string v = buf;

    return v;
}

/*******************************************************************************
* 
*******************************************************************************/
string code_convert(const char *from_charset, const char *to_charset, const string& sourceStr)
{
    iconv_t cd = iconv_open(to_charset, from_charset);
    if (cd == 0)
        return "";

    size_t inlen = sourceStr.size();
    if (inlen == 0)
    {
        iconv_close(cd);
        return "";
    }

    size_t outlen = inlen*2 + 1;
    char* inbuf  = (char*)sourceStr.c_str();
    char* outbuf = (char*)malloc(outlen);
    memset(outbuf, 0, outlen);
    char *poutbuf = outbuf; 
    if (iconv(cd, &inbuf, &inlen, &poutbuf, &outlen) > 0)
    {
        string strTemp(outbuf);
        iconv_close(cd);
        return strTemp;
    }
    else
    {
        iconv_close(cd);
        return "";
    }

}

/*******************************************************************************
* 
*******************************************************************************/
string gbk_utf8(string& strdata)
{
    string newcode = code_convert("gb18030", "utf-8", strdata);
    if (newcode.length() <= 0)
    {
        newcode = code_convert("gb2312", "utf-8", strdata);
    }
    
    return newcode;
}

/*******************************************************************************
* 
*******************************************************************************/
string utf8_gbk(string& strdata)
{
    string newcode = code_convert( "utf-8", "gb18030",strdata);
    if (newcode.length() <= 0)
    {
        newcode = code_convert( "utf-8", "gb2312",strdata); 
    }
    
    return newcode;
}

/*******************************************************************************
* 
*******************************************************************************/
string cmd_system(const char* command)
{
    if (NULL == command || strlen(command) == 0)
    {
        return "";
    }
    
    string result = "";
    FILE *fpRead = NULL;
    char buf[2048];
    memset(buf, '\0', sizeof(buf));
    
    fpRead = popen(command, "r");
    if(fpRead == NULL)
    {
        return "";
    }
    
    while(fgets(buf, sizeof(buf)-1, fpRead) != NULL)
    { 
        result = buf;
    }
    pclose(fpRead);
    
    return result;
}

/*******************************************************************************
* calculate file's md5 value
*******************************************************************************/
string calculate_file_md5(string file_path)
{    
    string cmd = "md5sum \"" + file_path + "\"";
    string md5 = cmd_system(cmd.c_str());

    vector<string> vect;
    string strrv = "";
    split(md5, " ", vect);
    if (vect.size() > 0)
        strrv = vect[0];
        
    return strrv;
}

/*******************************************************************************
*
*******************************************************************************/
bool IsIpAddress(const char* address)
{
	int a,b,c,d;
	if ((sscanf(address, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) && 
	    (a >=0 && a <= 255) &&
	    (b >=0 && b <= 255) &&
	    (c >=0 && c <= 255) &&
	    (d >=0 && d <= 255))
	{
		return true;
	}

    return false;
}

