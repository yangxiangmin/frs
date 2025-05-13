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

#include "json/json.h"  // JSONCPP
using namespace Json;

#include "main.h"
#include "transfer.h"
#include "httpUtils.h"
#include "ghts3client.h"
#include "logMsg.h"
#include "stringUtils.h"

extern GhtS3Client s3;

/******************************************************************************
* support multipart/form-data(application/octet-stream + application/json)
******************************************************************************/
void FileUploadCallback(const Request &req, Response &res, const ContentReader &content_reader) 
{
    string apptype = req.path_params.at("apptype");
    string restype = req.path_params.at("restype");
    string filename = req.path_params.at("filename");
    string originFile = "";
    bool isUploadOk = true;
        
    logInfo("%s(),upload parameters£¬apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());

    // create folder if not exist
    string uploadDir = string("mkdir -p ") + CACHE_DIR + apptype + "/" + restype;
    cmd_system(uploadDir.c_str());
    
    string uploadFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;        
    string metaFile   = CACHE_DIR + apptype + "/" + restype + "/" + filename + ".metadata";

    string uploadTmpFile = CACHE_DIR + apptype + "/" + restype + "/" + filename + ".tmp";    
    
    logInfo("%s(), http header, Content-Type: %s \n", THIS_FUNC, 
            req.get_header_value("Content-Type").c_str());    

    // file exist
    if (file_exists(uploadFile))
    {
        logError("%s(), file already exist.\n", THIS_FUNC);
        SetResponseMsg(res, 400, -2, "file exist");
        isUploadOk = false;
        return;
    }
    
    //binary data can use: multipart/form-data and application/octet-stream
    if (req.is_multipart_form_data()) 
    {
        logInfo("%s(), request content type: is_multipart_form_data\n", THIS_FUNC);    
    
        MultipartFormDataItems files;
        
        // get the file info then get stream data
        content_reader([&](const MultipartFormData &file) 
            {
                // get the file info
                files.push_back(file);
                originFile = file.filename;
                
                logInfo("%s(), --- files.back: name:%s, content_type:%s, filename:%s, content:%s\n", THIS_FUNC,
                        file.name.c_str(),file.content_type.c_str(),file.filename.c_str(),file.content.c_str());     
                return true;
            },
            
            [&](const char *data, size_t data_length) 
            {
                MultipartFormData file;
                file = files.back();
                logInfo("%s(), --- files.back: name:%s, content_type:%s, filename:%s, content:%s\n", THIS_FUNC,
                        file.name.c_str(),file.content_type.c_str(),file.filename.c_str(),file.content.c_str());
                                
                // write to file                 
                if (file.content_type == CONTENT_TYPE_APP_OCTET) 
                {
                    // get the file content
                    file.content.append(data, data_length);
                    string localFile = uploadTmpFile;
                    
                    append_to_file2(localFile, data, data_length);
                    logInfo("%s(), upload binary data length:%d\n", THIS_FUNC, data_length);                    

                } 
                else if (file.content_type == CONTENT_TYPE_APP_JSON) 
                {  
                    char *buf = new char[data_length+1];
                    if (buf)
                    {
                        memset(buf, 0, data_length+1);
                        memcpy(buf, data, data_length);
                        logInfo("%s(), upload json data:\n%s\n", THIS_FUNC, buf);

                        // write metadata to file
                        write_to_file(metaFile, buf);
                        
                        delete[] buf; 
                    }
                }
                
                return true;
            });
            
    }     
    else 
    {
        logInfo("%s(), request type NOT is_multipart_form_data\n", THIS_FUNC);
        logInfo("%s(), upload header, Content-Type: %s\n", THIS_FUNC, req.get_header_value("Content-Type").c_str());

        string content_type = req.get_header_value("Content-Type");

        if (content_type == CONTENT_TYPE_APP_JSON)
        {
            string body;
            content_reader([&](const char *data, size_t data_length)
            {
                body.append(data, data_length);
                logInfo("%s(), upload read length:%d, body:%s\n", THIS_FUNC, data_length, body.c_str());
                    
                write_to_file(metaFile, data);                
                return true;
            });
        }
        else if (content_type == CONTENT_TYPE_APP_OCTET)
        {
            content_reader([&](const char *data, size_t data_length)
            {
                logInfo("%s(), upload read length:%d, octet data\n", THIS_FUNC);
                write_to_binary_file(uploadTmpFile, data, data_length);                    
                return true;
            });
        }
    }

    // check file md5
    string fileMd5 = calculate_file_md5(uploadTmpFile);
    if (fileMd5 != filename)
    {
        logError("%s(), upload file md5 not consistent.\n", THIS_FUNC);  
        logError("%s(), filename:%s, content md5:%s.\n", THIS_FUNC, filename.c_str(), fileMd5.c_str());  
        SetResponseMsg(res, 400, -5, "md5 is NOT consistent");
        isUploadOk = false;
    }   
    else
    {
        int rv = file_rename(uploadTmpFile, uploadFile);
        if (rv == 0)
        {
            ;
        }   
        logInfo("%s(), uploadTmpFile:%s, uploadFile:%s.\n", THIS_FUNC, uploadTmpFile.c_str(), uploadFile.c_str());  
    }


    if (isUploadOk == true)
	{
        SetResponseMsg(res, 200, 0, "file upload success1");
        
        return;     
    }
       
}

/******************************************************************************
*
******************************************************************************/
void TempFileUploadCallback(const Request &req, Response &res, const ContentReader &content_reader) 
{
    string filename = req.path_params.at("filename");
    string originFile = "";
    bool isUploadOk = true;
        
    logInfo("%s(),upload parameters£¬temp file name:%s\n", THIS_FUNC, filename.c_str());

    // create folder if not exist
    string uploadDir = string("mkdir -p ") + CACHE_DIR + "temp/";
    cmd_system(uploadDir.c_str());
    
    string uploadFile = CACHE_DIR + "temp/" + filename;        
    string metaFile   = CACHE_DIR + "temp/" + filename + ".metadata";

    string uploadTmpFile = CACHE_DIR + "temp/" + filename + ".tmp";    
    
    logInfo("%s(), http header, Content-Type: %s \n", THIS_FUNC, 
            req.get_header_value("Content-Type").c_str());    

    // file exist
    if (file_exists(uploadFile))
    {
        logError("%s(), file already exist.\n", THIS_FUNC);
        SetResponseMsg(res, 400, -2, "file exist");
        isUploadOk = false;
        return;
    }
    
    //binary data can use: multipart/form-data and application/octet-stream
    if (req.is_multipart_form_data()) 
    {
        logInfo("%s(), request content type: is_multipart_form_data\n", THIS_FUNC);    
    
        MultipartFormDataItems files;
        
        // get the file info then get stream data
        content_reader([&](const MultipartFormData &file) 
            {
                // get the file info
                files.push_back(file);
                originFile = file.filename;
                
                logInfo("%s(), --- files.back: name:%s, content_type:%s, filename:%s, content:%s\n", THIS_FUNC,
                        file.name.c_str(),file.content_type.c_str(),file.filename.c_str(),file.content.c_str());     
                return true;
            },
            
            [&](const char *data, size_t data_length) 
            {
                MultipartFormData file;
                file = files.back();
                logInfo("%s(), --- files.back: name:%s, content_type:%s, filename:%s, content:%s\n", THIS_FUNC,
                        file.name.c_str(),file.content_type.c_str(),file.filename.c_str(),file.content.c_str());
                                
                // write to file                 
                if (file.content_type == CONTENT_TYPE_APP_OCTET) 
                {
                    // get the file content
                    file.content.append(data, data_length);
                    string localFile = uploadTmpFile;
                    
                    append_to_file2(localFile, data, data_length);
                    logInfo("%s(), upload binary data length:%d\n", THIS_FUNC, data_length);                    

                } 
                else if (file.content_type == CONTENT_TYPE_APP_JSON) 
                {  
                    char *buf = new char[data_length+1];
                    if (buf)
                    {
                        memset(buf, 0, data_length+1);
                        memcpy(buf, data, data_length);
                        logInfo("%s(), upload json data:\n%s\n", THIS_FUNC, buf);

                        // write metadata to file
                        write_to_file(metaFile, buf);
                        
                        delete[] buf; 
                    }
                }
                
                return true;
            });
            
    }     
    else 
    {
        logInfo("%s(), request type NOT is_multipart_form_data\n", THIS_FUNC);
        logInfo("%s(), upload header, Content-Type: %s\n", THIS_FUNC, req.get_header_value("Content-Type").c_str());

        string content_type = req.get_header_value("Content-Type");

        if (content_type == CONTENT_TYPE_APP_JSON)
        {
            string body;
            content_reader([&](const char *data, size_t data_length)
            {
                body.append(data, data_length);
                logInfo("%s(), upload read length:%d, body:%s\n", THIS_FUNC, data_length, body.c_str());
                    
                write_to_file(metaFile, data);                
                return true;
            });
        }
        else if (content_type == CONTENT_TYPE_APP_OCTET)
        {
            content_reader([&](const char *data, size_t data_length)
            {
                logInfo("%s(), upload read length:%d, octet data\n", THIS_FUNC);
                write_to_binary_file(uploadTmpFile, data, data_length);                    
                return true;
            });
        }
    }

    // check file md5
    string fileMd5 = calculate_file_md5(uploadTmpFile);
    if (fileMd5 != filename)
    {
        logError("%s(), upload file md5 not consistent.\n", THIS_FUNC);  
        logError("%s(), filename:%s, content md5:%s.\n", THIS_FUNC, filename.c_str(), fileMd5.c_str());  
        SetResponseMsg(res, 400, -5, "md5 is NOT consistent");
        isUploadOk = false;
    }   
    else
    {
        int rv = file_rename(uploadTmpFile, uploadFile);
        if (rv == 0)
        {
            ;
        }   
        logInfo("%s(), uploadTmpFile:%s, uploadFile:%s.\n", THIS_FUNC, uploadTmpFile.c_str(), uploadFile.c_str());  
    }


    if (isUploadOk == true)
    {
        SetResponseMsg(res, 200, 0, "file upload success1");
        
        return;     
    }
       
}


/******************************************************************************
* using: handle GET/HEAD
******************************************************************************/
void FileGetHeadCallback(const Request &req, Response &res) 
{
    logInfo("%s(), request method:%s\n", THIS_FUNC, req.method.c_str());
    
    string filename = req.path_params.at("filename");

    if (req.method == "HEAD")
    {
        FileHeadCallback(req, res);
    }
    else    
    {
        if(strstr(filename.c_str(), ".metadata") == NULL)
        {
            logInfo("%s(), request download file :%s\n", THIS_FUNC, filename.c_str());
            FileChunkedDownloadCallback(req, res);
        }
        else
        {
            logInfo("%s(), request metadata :%s\n", THIS_FUNC, filename.c_str());
            FileGetMetadataCallback(req, res);
        }        
    }
    return;
}

/******************************************************************************
* using: handle GET metadata
******************************************************************************/
void FileGetMetadataCallback(const Request &req, Response &res) 
{
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");

    logInfo("%s(), request method:%s\n", THIS_FUNC, req.method.c_str());
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());

    string metaFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;

    // file not exist
    if (std::ifstream(metaFile, std::ifstream::binary | std::ifstream::in).good()) 
    {
        // response 
        string meta = read_from_file(metaFile);
        Value data;
        int rv =  string2json(meta, data);
        
        //SetResponseMsg(res, 200, 0, meta);    
        SetResponseMsgWithData(res, 200, 0, "OK", data);    
        return;
    }
    else
    {
        string bucket = GetBucketName(apptype);        
        string keyName = "/" + restype + "/" + filename;        
        if (s3.isFileExist(bucket, keyName))
        {
            // download from ceph
            s3.downloadFile(bucket, keyName, metaFile);
            if (file_exists(metaFile))
            {            
                // response 
                string meta = read_from_file(metaFile);
                Value data;
                int rv =  string2json(meta, data);
                
                //SetResponseMsg(res, 200, 0, meta);    
                SetResponseMsgWithData(res, 200, 0, "OK", data);
                return;
                ; // file downloaded successfully from ceph
            }
            else
            {
                SetResponseMsg(res, 400, -4, "file not found");
                return;
            }
        }
        else
        {
            SetResponseMsg(res, 400, -4, "file not found");
            return;
        }
    }    

    
}




/******************************************************************************
* using: set_chunked_content_provider
******************************************************************************/
void FileHeadCallback(const Request &req, Response &res)
{
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");

    logInfo("%s(), request method:%s\n", THIS_FUNC, req.method.c_str());
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());

    string metaFile = CACHE_DIR + apptype + "/" + restype + "/" + filename + ".metadata";

    // file exist
    if (!std::ifstream(metaFile, std::ifstream::binary | std::ifstream::in).good()) 
    {
        string bucket = GetBucketName(apptype);
        
        string keyName = "/" + restype + "/" + filename;
        
        if (s3.isFileExist(bucket, keyName))
        {
            // download from ceph
            s3.downloadFile(bucket, keyName, metaFile);
            if (file_exists(metaFile))
            {            
                ; // file downloaded successfully from ceph
            }
            else
            {
                SetResponseMsg(res, 400, -4, "file not found");
                return;
            }
        }
        else
        {
            SetResponseMsg(res, 400, -4, "file not found");
            return;
        }
    }

    // response 
    string meta = read_from_file(metaFile);
    Value data;
    int rv =  string2json(meta, data);
    
    //SetResponseMsg(res, 200, 0, meta);    
    SetResponseMsgWithData(res, 200, 0, "OK", data);
    
}

/******************************************************************************
* using: set_chunked_content_provider
******************************************************************************/
void FileChunkedDownloadCallback(const Request &req, Response &res) 
{
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");


    logInfo("%s(), request method:%s\n", THIS_FUNC, req.method.c_str());
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());

    
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Content-Disposition", "attachment; filename="+filename);

    string downloadFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;
    
    // 404 not found, can't open file
    if (!std::ifstream(downloadFile, std::ifstream::binary | std::ifstream::in).good()) 
    {
        string bucket = GetBucketName(apptype);

        string keyName = "/" + restype + "/" + filename;
        
        if (s3.isFileExist(bucket, keyName))
        {
            // download from ceph
            s3.downloadFile(bucket, keyName, downloadFile);
            if (file_exists(downloadFile))
            {            
                ; // file downloaded successfully from ceph
            }
            else
            {
                SetResponseMsg(res, 400, -4, "file not found");
                return;
            }
        }
        else
        {
            SetResponseMsg(res, 400, -4, "file not found");
            return;
        }
    }
    
    res.set_chunked_content_provider("application/octet-stream", [downloadFile](size_t offset, httplib::DataSink& sink) 
    {
        // open file
        std::ifstream file_reader(downloadFile, std::ifstream::binary | std::ifstream::in);

        // can't open file, cancel process
        if (!file_reader.good())
            return false;

        // get file size
        file_reader.seekg(0, file_reader.end);
        size_t file_size = file_reader.tellg();
        file_reader.seekg(0, file_reader.beg);

        // check offset and file size, cancel process if offset >= file_size
        if (offset >= file_size)
            return false;
        
        // larger chunk size get faster download speed, 
        // more memory usage, more bandwidth usage, more disk I/O usage
        const size_t chunk_size = CHUNK_SIZE;

        // prepare read size of chunk
        size_t read_size = 0;
        bool last_chunk = false;
        if (file_size - offset > chunk_size) 
        {
            read_size = chunk_size;
            last_chunk = false;
        } 
        else
        {
            read_size = file_size - offset;
            last_chunk = true;
        }

        // allocate temp buffer, and read file chunk into buffer
        std::vector<char> buffer;
        buffer.reserve(chunk_size);
        file_reader.seekg(offset, file_reader.beg);
        file_reader.read(&buffer[0], read_size);
        file_reader.close();

        // write buffer to sink
        sink.write(&buffer[0], read_size);

        // done after last chunk had write to sink
        if (last_chunk)
            sink.done();

        return true;
    });

}

/******************************************************************************
* using: set_content_provider, download temp file 
******************************************************************************/
void TempFileDownloadCallback(const Request &req, Response &res) 
{
    string filename = req.path_params.at("filename");
    
    logInfo("%s(), download file:%s\n", THIS_FUNC, filename.c_str());
    
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Content-Disposition", "attachment; filename=xxx.txt");

    string downloadFile = CACHE_TEMP_DIR + filename;
    
    // 404 not found, can't open file
    if (!std::ifstream(downloadFile, std::ifstream::binary | std::ifstream::in).good()) 
    {
        SetResponseMsg(res, 400, -4, "file not found");
        return;
    }

    res.set_chunked_content_provider("application/octet-stream", [downloadFile](size_t offset, httplib::DataSink& sink) 
    {
        // open file
        std::ifstream file_reader(downloadFile, std::ifstream::binary | std::ifstream::in);

        // can't open file, cancel process
        if (!file_reader.good())
            return false;

        // get file size
        file_reader.seekg(0, file_reader.end);
        size_t file_size = file_reader.tellg();
        file_reader.seekg(0, file_reader.beg);

        // check offset and file size, cancel process if offset >= file_size
        if (offset >= file_size)
            return false;
        
        // larger chunk size get faster download speed, 
        // more memory usage, more bandwidth usage, more disk I/O usage
        const size_t chunk_size = CHUNK_SIZE;

        // prepare read size of chunk
        size_t read_size = 0;
        bool last_chunk = false;
        if (file_size - offset > chunk_size) 
        {
            read_size = chunk_size;
            last_chunk = false;
        } 
        else
        {
            read_size = file_size - offset;
            last_chunk = true;
        }

        // allocate temp buffer, and read file chunk into buffer
        std::vector<char> buffer;
        buffer.reserve(chunk_size);
        file_reader.seekg(offset, file_reader.beg);
        file_reader.read(&buffer[0], read_size);
        file_reader.close();

        // write buffer to sink
        sink.write(&buffer[0], read_size);

        // done after last chunk had write to sink
        if (last_chunk)
            sink.done();

        return true;
    });
}

#ifdef NON_USING
/******************************************************************************
*
******************************************************************************/
void FileDownloadCallback(const Request &req, Response &res) 
{
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");
    
    logInfo("%s(), download url parameters:", THIS_FUNC);
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
            apptype.c_str(), restype.c_str(), filename.c_str());
    
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Content-Disposition", "attachment; filename="+filename);

    string downloadFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;

    // file not found
    if (! std::ifstream(downloadFile, std::ifstream::binary | std::ifstream::in).good()) 
    {       
        string bucket = GetBucketName(apptype);

        string keyName = "/" + restype + "/" + filename;
        
        if (s3.isFileExist(bucket, keyName))
        {
            // download from ceph
            s3.downloadFile(bucket, keyName, downloadFile);
            if (file_exists(downloadFile))
            {            
                ; // file downloaded successfully from ceph
            }
            else
            {
                SetResponseMsg(res, 400, -4, "file not found");
                return;
            }
        }
        else
        {
            SetResponseMsg(res, 400, -4, "file not found");
            return;
        }
        
    }

    res.set_chunked_content_provider(
        "multipart/form-data", [](size_t offset, DataSink &sink) 
        {
            const char arr[] = "hello world";
            auto ret = sink.write(arr + offset, sizeof(arr));
            sink.done();
            logInfo("%s(), download write length:%d.\n", THIS_FUNC, sizeof(arr));
            return !!ret;
        });
}

/******************************************************************************
* using: set_content_provider
******************************************************************************/
void FileFullDownloadCallback(const Request &req, Response &res) 
{
    // open file
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");
    
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());


    string contDis = "attachment; filename=" + filename;
    
    res.set_header("Cache-Control", "no-cache");
    res.set_header("Content-Disposition", contDis);

    string downloadFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;

    
    // 404 not found, can't open file
    if (!file_exists(downloadFile))
    {
        logInfo("%s(), download file not found, filename:%s\n", THIS_FUNC,
                filename.c_str());
        SetResponseMsg(res, 400, -4, "file not found");
        return;
    }

    std::ifstream file_reader(downloadFile, std::ifstream::binary | std::ifstream::in);
    // can't open file, cancel process
    if (!file_reader.good())
    {
        SetResponseMsg(res, 400, -3, "file open failed");
        return;
    }
    
    // get file size
    file_reader.seekg(0, file_reader.end);
    size_t file_size = file_reader.tellg();
    file_reader.seekg(0, file_reader.beg);

    res.set_content_provider(file_size, 
        "application/octet-stream",
        [downloadFile](size_t offset, size_t length, httplib::DataSink& sink) {

        if (length > 0) length=length;
        
        // open file
        std::ifstream file_reader(downloadFile, std::ifstream::binary | std::ifstream::in);

        // can't open file, cancel process
        if (!file_reader.good())
            return false;

        // get file size
        file_reader.seekg(0, file_reader.end);
        size_t file_size = file_reader.tellg();
        file_reader.seekg(0, file_reader.beg);

        // check offset and file size, cancel process if offset >= file_size
        if (offset >= file_size)
            return false;

        // larger chunk size get faster download speed, 
        // more memory usage, more bandwidth usage, more disk I/O usage
        const size_t chunk_size = CHUNK_SIZE;

        // prepare read size of chunk
        size_t read_size = 0;
        if (file_size - offset > chunk_size){
            read_size = chunk_size;
        } else {
            read_size = file_size - offset;
        }

        // allocate temp buffer, and read file chunk into buffer
        std::vector<char> buffer;
        buffer.reserve(chunk_size);
        file_reader.seekg(offset, file_reader.beg);
        file_reader.read(&buffer[0], read_size);
        file_reader.close();

        // write buffer to sink
        sink.write(&buffer[0], read_size);

        return true;
    });
}
#endif

/******************************************************************************
* set file's metadata
******************************************************************************/
void MetadataPatchCallback(const Request &req, Response &res)
{
    logInfo("%s(), test PATCH method\n", THIS_FUNC );
    logInfo("%s(), path:\n", THIS_FUNC, req.path);
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");
    
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());
    
    string reqFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;
    string metaFile = CACHE_DIR + apptype + "/" + restype + "/" + filename + ".metadata";

    if (! file_exists(reqFile))
    {
        logError("%s(), file:%s not found\n", THIS_FUNC, reqFile.c_str());
        SetResponseMsg(res, 400, -4, "file not found");
        return;
    }
    else
    {    
        string meta = req.body;
        
        logInfo("%s(), save to file:%s\nmetadata:\n%s\n\n", THIS_FUNC, metaFile.c_str(), meta.c_str());
        write_to_file(metaFile, meta);
        SetResponseMsg(res, 200, 0, "metadata saved");
    }
    
    return;
}

;
/******************************************************************************
* test PATCH
******************************************************************************/
void TestPatchCallback(const Request &req, Response &res)
{    
    logInfo("%s(), test PATCH\n", THIS_FUNC );
    logInfo("%s(), path:\n", THIS_FUNC, req.path);

    /*
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");
    
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());
    
    string reqFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;
    string metaFile = CACHE_DIR + apptype + "/" + restype + "/" + filename + ".metadata";
    */
    
    SetResponseMsg(res, 200, 0, "PATCH received");
    
    return;
}

/******************************************************************************
* set file's metadata
******************************************************************************/
void FileDeleteCallback(const Request &req, Response &res)
{
    string apptype  = req.path_params.at("apptype");
    string restype  = req.path_params.at("restype");
    string filename = req.path_params.at("filename");
    
    logInfo("%s(), apptype: %s, restype:%s, filename:%s\n", THIS_FUNC,
             apptype.c_str(), restype.c_str(), filename.c_str());
    
    string fullFile = CACHE_DIR + apptype + "/" + restype + "/" + filename;

    // file not found
    if (! std::ifstream(fullFile, std::ifstream::binary | std::ifstream::in).good())
    {
        SetResponseMsg(res, 400, -4, "file not found");
        return;
    }

    // delete file on cache and ceph
    if (0 == remove(fullFile.c_str()))
    {
        SetResponseMsg(res, 200, 0, "file remove ok");
    }

    return;
}

/******************************************************************************
* get bucket name by apptype
******************************************************************************/
string GetBucketName(string apptype)
{
    if (apptype == "im") 
        return  BUCKET_IM;
    else if (apptype == "conf")
        return BUCKET_CONF;
    else
        return "";
}

