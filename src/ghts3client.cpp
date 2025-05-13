// example source code
// https://github.com/awsdocs/aws-doc-sdk-examples/tree/main/cpp/example_code/s3

#include "main.h"
#include "ghts3client.h"
#include "fileUtils.h"
#include "logMsg.h"
#include "transfer.h"

//const string rgwHost = "172.18.195.32:7480";
//const string access_key = "OOF8YTLHAJPXH1WDHFYA";
//const string access_secret = "52bXmC6zbXQ4GzKgsSiDzg0sxwArwOwJs0bmUqmY";

// 172.18.195.30(new)
string rgwHost = "k8s-node1:7480";
const string access_key = "5ZJI63QPTJQUT8T3NECN";
const string access_secret = "XWYEkt5iFQlWCP8uRopwHX08vBrdOwwVcu0Vs8eN";


/*******************************************************************************
*
*******************************************************************************/
int s3_test()
{
    GhtS3Client s3;

    // bucket test
    //string bucket = "temp";
    //s3.createBucket(bucket);   
    //string bucket = "temp";
    //s3.deleteBucket(bucket);   
    //s3.listBuckets();

    // object test 
    //s3.uploadFile("frs.temp", "aws_sdk_2024-10-24-07.log", "./aws_sdk_2024-10-24-07.log");
    //s3.uploadFile("frs.temp", "aws_sdk_2024-10-28-06.log", "./aws_sdk_2024-10-28-06.log");
    //s3.uploadFile("frs.temp", "aws_sdk_2024-10-28-07.log", "./aws_sdk_2024-10-28-07.log");
    //s3.uploadFile("frs.temp", "aws_sdk_2024-10-29-02.log", "./aws_sdk_2024-10-29-02.log");
    
    //s3.uploadFile("frs.temp", "aws_sdk_2024-10-28-07.log", "./aws_sdk_2024-10-28-07.log");
    //s3.downloadFile("frs.temp", "aws_sdk_2024-10-28-07.log", "./aws_sdk_2024-10-28-07.log.download");
    //s3.deleteFile("frs.temp", "aws_sdk_2024-10-28-07.log");
    

    Aws::Vector<Aws::String> keysResult;
    s3.listObjects(BUCKET_IM, keysResult);
    
    cout << "----------" << endl;    
    cout << "bucket:frs-im, objects: " << endl;
    for (size_t i = 0; i < keysResult.size(); i++)
        cout << keysResult[i] << endl;
    cout << "----------" << endl;    
    return 0;    
}

/*******************************************************************************
*
*******************************************************************************/
GhtS3Client::GhtS3Client()
{
    char *p = NULL; 
    if((p = getenv("NODE_IP")))
    {
        rgwHost = string(p) + ":7480";
    }
    logInfo("CEPH RGW host: %s\n", rgwHost.c_str());
    
    options.loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
    Aws::InitAPI(options);
}

/*******************************************************************************
*
*******************************************************************************/
GhtS3Client::~GhtS3Client()
{
    Aws::ShutdownAPI(options);
}

/*******************************************************************************
*
*******************************************************************************/
int GhtS3Client::listBuckets() 
{
    printf("running listBuckets()\n\n");
    
    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
    #if 0
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";
    Aws::S3::S3Client client(cred, NULL, cfg);
    #else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;    
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    #endif
    
    auto response = client.ListBuckets();
    if (response.IsSuccess()) 
    {
        auto buckets = response.GetResult().GetBuckets();
        string outStr="";
        for (auto iter = buckets.begin(); iter != buckets.end(); ++iter) 
        {

            outStr += "name: " + iter->GetName() + " " +
                   iter->GetCreationDate().ToLocalTimeString(Aws::Utils::DateFormat::ISO_8601) + "\n";
            
        }
        logInfo("%s(), bucket list:\n%s\n", THIS_FUNC, outStr.c_str());
    } 
    else 
    {
        logError("%s(), Error while ListBuckets, Exception: %s: %s \n", THIS_FUNC,
                 response.GetError().GetExceptionName().c_str(),
                 response.GetError().GetMessage().c_str());
    }
    
    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
bool GhtS3Client::createBucket(const Aws::String& bucketName) 
{
    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
    #if 0
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";
    Aws::S3::S3Client client(cred, NULL, cfg);
    #else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;    
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    #endif
                              
    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(bucketName);

    if (config.region != "") 
    {
        Aws::S3::Model::CreateBucketConfiguration createBucketConfig;
        createBucketConfig.SetLocationConstraint(
                Aws::S3::Model::BucketLocationConstraintMapper::GetBucketLocationConstraintForName(
                        config.region));
        request.SetCreateBucketConfiguration(createBucketConfig);
    }

    Aws::S3::Model::CreateBucketOutcome outcome = client.CreateBucket(request);
    if (!outcome.IsSuccess()) 
    {
        auto err = outcome.GetError();
        logError("%s(), Error: createBucket, %s:%s\n", THIS_FUNC,
            err.GetExceptionName().c_str(),
            err.GetMessage().c_str() );                
    } 
    else 
    {
    
        logInfo("%s(), Created bucket(%s) in the specified AWS Region..\n", THIS_FUNC, bucketName.c_str());
    }

    return outcome.IsSuccess();
}

/*******************************************************************************
*
*******************************************************************************/
bool GhtS3Client::deleteBucket(const Aws::String &bucketName)
{
    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
    #if 0
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";

    Aws::S3::S3Client client(cred, NULL, cfg);
    #else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    #endif

    Aws::S3::Model::DeleteBucketRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::DeleteBucketOutcome outcome =
            client.DeleteBucket(request);

    if (!outcome.IsSuccess())
    {
        const Aws::S3::S3Error &err = outcome.GetError();
        logError("%s(), upload file failed, %s:%s\n", THIS_FUNC,
            err.GetExceptionName().c_str(),
            err.GetMessage().c_str() );        
    } 
    else 
    {
        logInfo("%s(), The bucket(%s) was deleted.\n", THIS_FUNC, bucketName.c_str());
    }

    return outcome.IsSuccess();
}

/*******************************************************************************
*
*******************************************************************************/
bool GhtS3Client::listObjects(const Aws::String &bucketName,
                             Aws::Vector<Aws::String> &keysResult)
{
    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
    #if 0
    // config and connect to endpoint
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";
    Aws::S3::S3Client client(cred, NULL, cfg);
    #else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;    
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    #endif

    Aws::S3::Model::ListObjectsV2Request request;
    request.WithBucket(bucketName);

    Aws::String continuationToken; // Used for pagination.
    Aws::Vector<Aws::S3::Model::Object> allObjects;

    do 
    {
        if (!continuationToken.empty()) 
        {
            request.SetContinuationToken(continuationToken);
        }

        auto outcome = client.ListObjectsV2(request);

        if (!outcome.IsSuccess()) 
        {
            std::cerr << "Error: listObjects: " <<
                      outcome.GetError().GetMessage() << std::endl;
            return false;
        } 
        else
        {
            Aws::Vector<Aws::S3::Model::Object> objects =
                    outcome.GetResult().GetContents();

            allObjects.insert(allObjects.end(), objects.begin(), objects.end());
            continuationToken = outcome.GetResult().GetNextContinuationToken();
        }
    } while (!continuationToken.empty());

    logInfo("%s(), %d object(s) found.\n", THIS_FUNC,allObjects.size());

    for (const auto &object: allObjects) 
    {
        logInfo("%s(), %s\n", THIS_FUNC,
            object.GetKey().c_str());
    
        keysResult.push_back(object.GetKey());
    }

    return true;
}

/*******************************************************************************
*
*******************************************************************************/
int GhtS3Client::uploadFile(const string bucket, const string keyName, const string localFile)
{
    const Aws::String s3_bucket_name = bucket;
    const Aws::String s3_object_name = keyName; 
    const string file_name = localFile; 
    const Aws::String region = "";     

    // check file exist
    if (!file_exists(file_name)) 
    {
        logInfo("%s(), file not found:%s\n", THIS_FUNC,
                 file_name.c_str());
        return 1;
    }

    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
    #if 0
    // config and connect to endpoint
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";
    Aws::S3::S3Client client(cred, NULL, cfg);
    #else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    #endif
    
    //S3Client (const Auth::AWSCredentials &credentials, 
    //const Client::ClientConfiguration &clientConfiguration=Client::ClientConfiguration(), 
    //bool signPayloads=false, bool useVirtualAdressing=true)

    // make request
    Aws::S3::Model::PutObjectRequest object_request;

    object_request.SetBucket(s3_bucket_name);
    object_request.SetKey(s3_object_name);
    
    const shared_ptr<Aws::IOStream> input_data = 
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag", 
                                      file_name.c_str(), 
                                      ios_base::in | ios_base::binary);
    object_request.SetBody(input_data);
    
    auto put_object_outcome = client.PutObject(object_request);
    
    // get result
    if (!put_object_outcome.IsSuccess()) 
    {
        auto error = put_object_outcome.GetError();
        logError("%s(), upload file failed, %s:%s\n", THIS_FUNC,
            error.GetExceptionName().c_str(),
            error.GetMessage().c_str() );
            
        return 1;
    }

    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
int GhtS3Client::deleteFile(const string bucket, const string keyName)
{
    const Aws::String bucket_name = bucket;
    const Aws::String key_name = keyName;

    logInfo("%s(), bucket:%s, keyName:%s\n", THIS_FUNC,
             bucket_name.c_str(), key_name.c_str());

    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
    #if 0
    // config and connect to endpoint
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";
    Aws::S3::S3Client client(cred, NULL, cfg);
    #else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;    
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    #endif

    // make request
    Aws::S3::Model::DeleteObjectRequest object_request;
    
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto delete_object_outcome = client.DeleteObject(object_request);

    // get result
    if (delete_object_outcome.IsSuccess())
    {
        logInfo("%s(), delete file success\n", THIS_FUNC);
    }
    else
    {
        logError("%s(), delete file failed, %s:%s\n", THIS_FUNC,
            delete_object_outcome.GetError().GetExceptionName().c_str(),
            delete_object_outcome.GetError().GetMessage().c_str() );
    }

    return 0;
}

/*******************************************************************************
*
*******************************************************************************/
int GhtS3Client::downloadFile(const string bucket, const string keyName, const string localFile)
{
    const Aws::String bucket_name = bucket;
    const Aws::String key_name = keyName;

    logInfo("%s(), bucket:%s, keyName:%s\n", THIS_FUNC,
             bucket_name.c_str(), key_name.c_str());

    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
    #if 0
    // config and connect to endpoint
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";
    Aws::S3::S3Client client(cred, NULL, cfg);
    #else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    #endif

    // make request
    Aws::S3::Model::GetObjectRequest object_request;
    
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto get_object_outcome = client.GetObject(object_request);

    // get result
    if (get_object_outcome.IsSuccess())
    {
        Aws::OFStream saveFile;

        saveFile.open(localFile, std::ios::out | std::ios::binary);
        saveFile << get_object_outcome.GetResultWithOwnership().GetBody().rdbuf();
         
         logInfo("%s(), upload file success.\n", THIS_FUNC);
         return true;
     } 
     else
     {
         auto error = get_object_outcome.GetError();
         logError("%s(), getObject failed, %s:%s\n", THIS_FUNC,
             error.GetExceptionName().c_str(),
             error.GetMessage().c_str() );
         
         return false;
     }    

    return 0;
}

/*******************************************************************************
* check if file exist in ceph
*******************************************************************************/
bool GhtS3Client::isFileExist(const string bucket, const string keyName)
{
    const Aws::String bucket_name = bucket;
    const Aws::String key_name = keyName;

    logInfo("%s(), bucket:%s, keyName:%s\n", THIS_FUNC,
             bucket_name.c_str(), key_name.c_str());

    Aws::Auth::AWSCredentials cred(access_key, access_secret); 
#if 0
    // config and connect to endpoint
    Aws::S3::S3ClientConfiguration  cfg;
    cfg.endpointOverride = rgwHost;  
    cfg.scheme = Aws::Http::Scheme::HTTP;
    cfg.verifySSL = false;
    cfg.region = "";

    Aws::S3::S3Client client(cred, NULL, cfg);
#else
    Aws::Client::ClientConfiguration config;
    config.region = Aws::String("");
    config.verifySSL = false;
    config.connectTimeoutMs = 60000;
    config.requestTimeoutMs = 10000;
    config.endpointOverride = Aws::String(rgwHost);
    config.scheme = Aws::Http::Scheme::HTTP;
    
    Aws::S3::S3Client client = Aws::S3::S3Client(cred, config, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
#endif
    
    // make request
    Aws::S3::Model::HeadObjectRequest object_request;
    
    object_request.WithBucket(bucket_name).WithKey(key_name);

    auto get_object_outcome = client.HeadObject(object_request);

    // get result
    if (get_object_outcome.IsSuccess())
    {
         logInfo("%s(), %s file exists.\n", THIS_FUNC, keyName.c_str());
         return true;
     } 
     else
     {
         logError("%s(), %s file NOT exists.\n", THIS_FUNC, keyName.c_str());
         return false;
     }    

    return 0;
}


#if 0
/*******************************************************************************
* //! Create a multipart upload.
    \param bucket: The name of the S3 bucket where the object will be uploaded.
    \param key: The unique identifier (key) for the object within the S3 bucket.
    \param client: The S3 client instance used to perform the upload operation.
    \return Aws::String: Upload ID or empty string if failed.
*
*******************************************************************************/
string GhtS3Client::createMultipartUpload(const Aws::String &bucket, const Aws::String &key,
                                  Aws::S3::Model::ChecksumAlgorithm checksumAlgorithm,
                                  const Aws::S3::S3Client &client) {

    Aws::S3::Model::CreateMultipartUploadRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);

    if (checksumAlgorithm != Aws::S3::Model::ChecksumAlgorithm::NOT_SET) {
        request.SetChecksumAlgorithm(checksumAlgorithm);
    }

    Aws::S3::Model::CreateMultipartUploadOutcome outcome =
            client.CreateMultipartUpload(request);

    Aws::String uploadID;
    if (outcome.IsSuccess()) {
        uploadID = outcome.GetResult().GetUploadId();
    } else {
        std::cerr << "Error creating multipart upload: " << outcome.GetError().GetMessage() << std::endl;
    }

    return string(uploadID);
}

/*******************************************************************************
*
*******************************************************************************/
bool GhtS3Client::abortMultipartUpload(const Aws::String &bucket,
                                      const Aws::String &key,
                                      const Aws::String &uploadID,
                                      const Aws::S3::S3Client &client) {
    Aws::S3::Model::AbortMultipartUploadRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);
    request.SetUploadId(uploadID);

    Aws::S3::Model::AbortMultipartUploadOutcome outcome =
            client.AbortMultipartUpload(request);

    if (outcome.IsSuccess()) {
        std::cout << "Multipart upload aborted." << std::endl;
    } else {
        std::cerr << "Error aborting multipart upload: " << outcome.GetError().GetMessage() << std::endl;
    }

    return outcome.IsSuccess();
}

/*******************************************************************************
*
*******************************************************************************/
bool GhtS3Client::completeMultipartUpload(const Aws::String &bucket,
                                          const Aws::String &key,
                                          const Aws::String &uploadID,
                                          const Aws::Vector<Aws::S3::Model::CompletedPart> &parts,
                                          const Aws::S3::S3Client &client) {
    Aws::S3::Model::CompletedMultipartUpload completedMultipartUpload;
    completedMultipartUpload.SetParts(parts);

    Aws::S3::Model::CompleteMultipartUploadRequest request;
    request.SetBucket(bucket);
    request.SetKey(key);
    request.SetUploadId(uploadID);
    request.SetMultipartUpload(completedMultipartUpload);

    Aws::S3::Model::CompleteMultipartUploadOutcome outcome =
            client.CompleteMultipartUpload(request);

    if (!outcome.IsSuccess()) {
        std::cerr << "Error completing multipart upload: " << outcome.GetError().GetMessage() << std::endl;
    }
    return outcome.IsSuccess(); //outcome;
}
#endif


