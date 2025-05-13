#include "main.h"
#include "transfer.h"
#include "logMsg.h"
#include "httpUtils.h"
#include "ghts3client.h"

extern int s3_test();
extern string rgwHost;
static int g_svrPort = 8183;
static string g_startAt = "";
GhtS3Client s3;

/******************************************************************************
*
******************************************************************************/
void ErrorHandler(const Request &req, Response &res)
{
    logError("%s(), Server-error: method:%s, path:%s, status:%d\n", THIS_FUNC,
             req.method.c_str(), req.path.c_str(), res.status);

    if (res.status == 200 || res.status == 400 || res.status == 401 ||
        res.status == 402 || res.status == 403 || res.status == 404 )
    {
        ;
    }
    else    
    {
        SetResponseMsg(res, res.status, -1, "request path error");
    }
}

/*******************************************************************************
* REST interface work thread
*******************************************************************************/
void * http_server_thread(void *arg)
{    
    NONUSE_PARAM(arg)
    
    logInfo("%s(), Server REST listen on: 0.0.0.0:%d.\n", THIS_FUNC, g_svrPort);
    
    Server svr;
    
    // create file cache folder
    string cacheDir = "mkdir -p " + CACHE_DIR;
    cmd_system(cacheDir.c_str());

    cacheDir = string("mkdir -p ") + CACHE_TEMP_DIR;
    cmd_system(cacheDir.c_str());

    cacheDir = string("mkdir -p ") + CACHE_DIR + "im/";
    cmd_system(cacheDir.c_str());

    cacheDir = string("mkdir -p ") + CACHE_DIR + "conf/";
    cmd_system(cacheDir.c_str());

    cacheDir = string("mkdir -p /app/run/frs/log/");
    cmd_system(cacheDir.c_str());
    
    // setup error request handler 
    svr.set_error_handler(ErrorHandler);

    // set callback for uploading file
    svr.Post("/api/v1/:apptype/:restype/:filename", FileUploadCallback);

    // set callback for downloading file, GET/HEAD
    svr.Get("/api/v1/:apptype/:restype/:filename", FileGetHeadCallback);

    // set file attachment
    svr.Put("/api/v1/:apptype/:restype/:filename", MetadataPatchCallback);

    // set file attachment
    svr.Patch("/api/v1/:apptype/:restype/:filename", MetadataPatchCallback);

    // set file attachment
    svr.Delete("/api/v1/:apptype/:restype/:filename", FileDeleteCallback);

    // set callback for uploading file
    svr.Post("/api/v1/temp/:filename", TempFileUploadCallback);

    // set callback for downloading file
    svr.Get("/api/v1/temp/:filename", TempFileDownloadCallback);

    svr.Get("/version", [](const Request &req, Response &res) 
    {
        logInfo("Request Info, method:%s, path:%s\n", 
            req.method.c_str(), req.path.c_str());
        
        // get all of request parameters
        Value resp;
        resp["version"] = FRS_VERSION;
        resp["built"]   = FRS_BUILT_ON;
        resp["startAt"] = g_startAt;
            
        string json = json_unformat_string(resp);
        res.set_content(json, "application/json");
    });

    // get specified position parameters
    svr.Get("/test/:apptype/:restype", [](const Request &req, Response &res) 
    {
        // get all of parameters at url
        logInfo("url parameters:\n  apptype:%s\n  restype:%s\n", 
            req.path_params.at("apptype").c_str(),
            req.path_params.at("restype").c_str());
        
        res.set_content("Hello World!", "text/plain");
    });

#if 0 // for testing
    // test get /hello parameters
    // for examle: http://172.20.1.153:8081/hello?filename=222223xxx.txt&restype=video
    // 
    svr.Get("/hello", [](const Request &req, Response &res) 
    {
        // get all of request parameters
        string context;
        for(std::multimap<std::string, std::string>::const_iterator it = req.params.begin(); it != req.params.end(); ++it)
        {
            string params = it->first + " = " + it->second;
            context += params += "\n";
        }        
        res.set_content("Hello World!", "text/plain");
        
    });

    svr.Patch("/api/v1/test", TestPatchCallback);
#endif
    
    // set server config
    svr.set_keep_alive_max_count(50);
    svr.set_keep_alive_timeout(20);
  
    // listen and block main thread
    if (svr.listen("0.0.0.0", g_svrPort)) 
    {
        logInfo("%s(), Server is running at http://localhost:%d.\n", THIS_FUNC, g_svrPort);
    } 
    else 
    {
        logError("%s(), Failed to start server.\n", THIS_FUNC);
    }

    
    while (true)
    {
        logInfo("%s(), main thread running ....\n", THIS_FUNC);
        this_thread::sleep_for(chrono::seconds(2));
    }    
}

/*******************************************************************************
* upload file to bucket
*******************************************************************************/
int s3_upload_files(string bucket, string path)
{
    vector<string> list;

    get_all_files(path, list);

    for (size_t i = 0; i < list.size(); i++)
    {
        string localFile = list[i];
        string keyName = list[i].substr(path.length());
        logInfo("  upload file: %s -> (%s)\n", localFile.c_str(), keyName.c_str());
        
        if (NOT(s3.isFileExist(bucket, keyName)))
        {
            logInfo("  upload: %s -> (%s).\n", localFile.c_str(), keyName.c_str());
            s3.uploadFile(bucket, keyName, localFile);
        }
        else
        {
            logInfo(" ceph: %s existed, no upload.\n", keyName.c_str());
        }        
    }
    return 0;
}

/*******************************************************************************
* CEPH storage work thread
*******************************************************************************/
void * ceph_storage_thread(void *arg)
{    
    NONUSE_PARAM(arg)

    logInfo("%s(), Server start ceph storage thread.\n", THIS_FUNC);
    logInfo("%s(), Ceph RGW: %s.\n", THIS_FUNC, rgwHost.c_str());
    s3.listBuckets();

    while(true)
    {
        #if 1
        string imPath = CACHE_DIR + "/im";
        logInfo("upload dir: %s to ceph.\n", imPath.c_str());
        s3_upload_files(BUCKET_IM, imPath);

        string confPath = CACHE_DIR + "/conf";
        logInfo("upload dir: %s to ceph.\n", confPath.c_str());
        s3_upload_files(BUCKET_CONF, confPath);
        #endif
        
        sleep(60);
    }
}

/*******************************************************************************
* main()
*******************************************************************************/
int main(int argc, char* argv[])
{
    logInfo("FRS Server starting ....\n");

    if (argc >= 2)
    {
        int port = str2int(argv[1]);
        printf("server port:%d\n", port);
    }

    g_startAt = GetCurrentDTstring();

    log_init();

    // http server thread
    pthread_t tid;
    pthread_create(&tid, NULL, http_server_thread, NULL);

    //ceph storage thread
    pthread_create(&tid, NULL, ceph_storage_thread, NULL);

    while (1)
    {
        sleep(10);
    }
    
    return 0;
}



