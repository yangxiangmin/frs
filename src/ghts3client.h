#include <iostream>
#include <fstream>
#include <sys/stat.h>

#include <aws/s3/S3Client.h>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>

#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/CompletedPart.h>
#include <aws/s3/model/CreateMultipartUploadRequest.h>
#include <aws/s3/model/CompletedMultipartUpload.h>
#include <aws/s3/model/CompleteMultipartUploadRequest.h>
#include <aws/s3/model/AbortMultipartUploadRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>

using namespace Aws::S3;
using namespace Aws::S3::Model;
using namespace std;


class GhtS3Client
{
public:
    GhtS3Client();
    ~GhtS3Client();

    int listBuckets();
    bool createBucket(const Aws::String& bucketName);
    bool deleteBucket(const Aws::String& bucketName);
    bool listObjects(const Aws::String &bucketName,
                     Aws::Vector<Aws::String> &keysResult);

    int uploadFile(const string bucket, const string keyName, const string localFile );
    int downloadFile(const string bucket, const string keyName, const string localFile );
    int deleteFile(const string bucket, const string keyName);
    bool isFileExist(const string bucket, const string keyName);

    #if 0

    string createMultipartUpload(const Aws::String &bucket, const Aws::String &key,
                                 Aws::S3::Model::ChecksumAlgorithm checksumAlgorithm,
                                 const Aws::S3::S3Client &client);
        

    bool abortMultipartUpload(const Aws::String &bucket,
                              const Aws::String &key,
                              const Aws::String &uploadID,
                              const Aws::S3::S3Client &client);

    bool completeMultipartUpload(const Aws::String &bucket,
                                  const Aws::String &key,
                                  const Aws::String &uploadID,
                                  const Aws::Vector<Aws::S3::Model::CompletedPart> &parts,
                                  const Aws::S3::S3Client &client);
    #endif
    
private:
    Aws::SDKOptions  options;    
    

}; 

