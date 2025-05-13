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

#include "main.h"
#include "stringUtils.h"  

/*******************************************************************************
*
*******************************************************************************/
int get_file_contents(const char *file, char *data, unsigned long len)
{
	int ret = -1;
	FILE *fp = NULL;
	int count = -1;

    if (NULL == file || NULL == data)
    {
        return (-1);
    }

	if (NULL == (fp = fopen(file, "r")))
	{
		return (-1);
	}

	if ((count = fread(data, 1, len, fp)) > 0)
	{
		ret = 0;
	}

	if (fp != NULL)
	{
		fclose(fp);
	}

	return ret;
}

/*******************************************************************************
*
*******************************************************************************/
int put_file_contents(const char *file, const char *data, unsigned long len)
{
    FILE *pf = fopen(file, "w");
    
    if (NULL == file || NULL == data || len <= 0)
    {
        return (-1);
    }

    if (pf)
    {
        fwrite(data, len, 1, pf);
        fflush(pf);
        fclose(pf);
    }

    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
unsigned long get_file_size(const char *file)  
{  
    unsigned long filesize = 0;      
    struct stat statbuff;  

    if (NULL == file)
    {
        return 0;
    }
    
    if (stat(file, &statbuff) < 0)
    {  
        return 0;  
    }
    else
    {  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}  

/*******************************************************************************
*
*******************************************************************************/
string get_file_dir(string full_path_file)  
{  
    size_t pos = full_path_file.find_last_of('/');
    string folder(full_path_file.substr(0, pos) );
    return folder;
}  

/*******************************************************************************
*
*******************************************************************************/
string get_file_name(string full_path_file)  
{  
    size_t pos = full_path_file.find_last_of('/');
    string filename(full_path_file.substr(pos + 1) );
        
    return filename;
}  

/*******************************************************************************
*
*******************************************************************************/
string get_file_ext(string file)  
{  
    // update database   
    vector<string> vect;
    split(file, ".", vect);
    if (vect.size() >= 2)
    {
        return vect[vect.size()-1];
    }
    return "";    
} 

/*******************************************************************************
*
*******************************************************************************/
bool is_file_exist(const char *file)   
{   
    if ((access(file, F_OK)) != (-1))
    {   
        return true;
    }   

    return false;
}

/*******************************************************************************
*
*******************************************************************************/
bool file_exists(const string& name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

/*******************************************************************************
* 
*******************************************************************************/
int file_rename(string oldName, string newName)
{
    std::fstream f;
    f.open(oldName.c_str());
 
    if (f.fail())
    {
        f.close();
        return -1;
    }
    else
    {
        f.close();
        if (-1 == rename(oldName.c_str(), newName.c_str()))
        {
            ;
        }
    }
    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
bool make_dir(const char *folder)   
{   
    if (is_file_exist(folder))
        return true;

    mkdir(folder, S_IRWXU);

    return is_file_exist(folder);
}

/*******************************************************************************
*
*******************************************************************************/
int write_to_file(string file, string str_write)   
{   
    ofstream ofs;
    ofs.open(file.c_str(), std::ofstream::out);
    if (ofs.is_open())
    {
        ofs << str_write;
        ofs.close();
    }

    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
int write_to_binary_file(string file, const char* data, size_t length)   
{   
    FILE *pf = NULL;
    pf = fopen(file.c_str(), "w+");
    if (pf)
    {
        fwrite(data, length, 1, pf);
        fclose(pf);
    }
    
    return 0;
}
 
/*******************************************************************************
*
*******************************************************************************/
int append_to_file(string file, const char* str_write)   
{   
    ofstream ofs;
    ofs.open(file.c_str(), ios::app);
    if (ofs.is_open())
    {
        ofs << str_write;
        ofs.close();
    }

    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
int append_to_file2(string file, const char* str_write, size_t length)   
{   
    FILE *pf = NULL;
    pf = fopen(file.c_str(), "a+");
    if (pf)
    {
        fwrite(str_write, length, 1, pf);
        fclose(pf);
    }
    
    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
string read_from_file(string file)   
{   
    ifstream t(file.c_str());  
    stringstream buffer;  
    buffer << t.rdbuf();  
    string contents(buffer.str());

    return contents;
}

/*******************************************************************************
*
*******************************************************************************/
string get_owner_path()
{
    char path[1024];
    int cnt = readlink("/proc/self/exe", path, 1024);
    if (cnt < 0 || cnt >= 1024)
    {
        return NULL;
    }
    for(int i = cnt-1; i >= 0; --i)
    {
        if (path[i] == '/')
        {
            path[i + 1] = '\0';
            break;
        }
    }
    string s_path(path);
    return s_path;
}

/*******************************************************************************
*
*******************************************************************************/
void dir_oper(char const*path, vector<string>& list )
{
	//printf("[%s] it is a dir\n",path);
	struct dirent *filename;
	struct stat s_buf;
	DIR *dp = opendir(path);

    if (dp == NULL)
    {
        printf("dir_oper dp = null\n");            
        return;        
    }
    
	/*readdir() MUST loop to call whole dir */
	while (NULL != (filename = readdir(dp)))
	{
		char file_path[200];
		bzero(file_path,200);
        
		strcat(file_path, path);
		strcat(file_path, "/");
		strcat(file_path, filename->d_name);
		
		if(strcmp(filename->d_name,".") == 0 ||
           strcmp(filename->d_name,"..") == 0)
		{
			continue;
		}

		/*get file info s_buf */
		stat(file_path, &s_buf);

		/* is dir */
		if(S_ISDIR(s_buf.st_mode))
		{
			dir_oper(file_path, list);
			//printf("\n");
		}

		/*is a file */
		if(S_ISREG(s_buf.st_mode))
		{
			//printf("[%s] is a regular file\n",file_path);
            printf("push_back: %s\n",file_path);            
            list.push_back(file_path);
		}
	}
}

/*******************************************************************************
* 
*******************************************************************************/
int get_dir_list(string dirpath, vector<string>& list)
{
    DIR *dir = opendir(dirpath.c_str());
    if (dir == NULL)
    {
        printf("%s(), opendir() failed.\n", THIS_FUNC);
        return -1;
    }
    struct dirent *entry = NULL;
    
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {
            //It's dir
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            
            string dirNew = dirpath + "/" + entry->d_name;
            vector<string> tempPath;
            int rv = get_dir_list(dirNew, tempPath);
            if (rv == 0)
            {
                list.push_back(dirNew);
                list.insert(list.end(), tempPath.begin(), tempPath.end());
            }
        }
    }
    closedir(dir);
    return 0;
}

/*******************************************************************************
* 
*******************************************************************************/
int get_files_list(string dirpath, vector<string>& dirlist)
{
    DIR *dir = opendir(dirpath.c_str());
    if (dir == NULL)
    {
        printf("%s(), opendir() failed.\n", THIS_FUNC);
        return -1;
    }

    struct dirent *entry = NULL;
    
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {//It's dir
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            
            string dirNew = dirpath + "/" + entry->d_name;
            vector<string> tempPath;
            int rv = get_files_list(dirNew, tempPath);
            if (rv == 0)
            {
                dirlist.insert(dirlist.end(), tempPath.begin(), tempPath.end());
            }
        }
        else 
        {
            string name = entry->d_name;
            string imgdir = dirpath +"/"+ name;
            //sprintf("%s",imgdir.c_str());
            dirlist.push_back(imgdir);
        }

    }
    closedir(dir);
    return 0;
}

/*******************************************************************************
* 
*******************************************************************************/
int get_all_files(string dirpath, vector<string>& dirlist)
{
    DIR *dir = opendir(dirpath.c_str());
    if (dir == NULL)
    {
        printf("%s(), opendir() failed.\n", THIS_FUNC);
        return -1;
    }

    //cout << "dir:" << dirpath << endl;

    struct dirent *entry = NULL;
    
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR)
        {//It's dir
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            
            string dirNew = dirpath + "/" + entry->d_name;
            vector<string> tempPath;
            int rv = get_all_files(dirNew, tempPath);
            if (rv == 0)
            {
                dirlist.insert(dirlist.end(), tempPath.begin(), tempPath.end());
            }
        }
        else 
        {
            string name = entry->d_name;
            string imgdir = "";
            imgdir = dirpath + "/" + name;
                
            dirlist.push_back(imgdir);
        }

    }
    closedir(dir);
    return 0;
}

/*******************************************************************************
* create multi level dir for linux
*******************************************************************************/
void make_dir_multi_level(const char *folder)   
{  
    int i,len;  
    char str[512];      
    strncpy(str, folder, 512);  
    
    len = strlen(str);  
    for(i = 0; i < len; i++ )  
    {  
        if (str[i] == '/' )  
        {  
            str[i] = '\0';  
            if (access(str,0)!=0)  
            {  
                mkdir(str, 0777);  
            }  
            str[i] = '/';  
        }  
    }  
    
    if( len > 0 && access(str,0) != 0)  
    {  
        mkdir(str, 0777);  
    }  
    
    return;  
}  

