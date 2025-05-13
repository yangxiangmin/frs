#ifndef __CPP_HTTPLIB_UPLOAD_H__
#define __CPP_HTTPLIB_UPLOAD_H__
#include "httplib.h"

#define CHUNK_SIZE          (32*1024)
#define BUCKET_IM           "frs-im"
#define BUCKET_CONF         "frs-conf"


#define CONTENT_TYPE_IMAGE_JPEG   "image/jpeg"
#define CONTENT_TYPE_IMAGE_PNG    "image/png"
#define CONTENT_TYPE_IMAGE_GIF    "image/gif"
#define CONTENT_TYPE_IMAGE_BMP    "image/bmp"
#define CONTENT_TYPE_IMAGE_XICON  "image/x-icon"
#define CONTENT_TYPE_IMAGE_TIFF   "image/tiff"
#define CONTENT_TYPE_IMAGE_SVG    "image/svg+xml"
#define CONTENT_TYPE_IMAGE_WEBP   "image/webp"

#define CONTENT_TYPE_APP_JSON     "application/json"
#define CONTENT_TYPE_APP_OCTET    "application/octet-stream"



void FileDownloadCallback(const Request &req, Response &res);
void FileGetHeadCallback(const Request &req, Response &res);
void FileGetMetadataCallback(const Request &req, Response &res);

void FileChunkedDownloadCallback(const Request &req, Response &res);
void FileHeadCallback(const Request &req, Response &res);

void FileFullDownloadCallback(const Request &req, Response &res);
void TempFileDownloadCallback(const Request &req, Response &res);


void FileUploadCallback(const Request &req, Response &res, const ContentReader &content_reader);
void TempFileUploadCallback(const Request &req, Response &resp, const ContentReader &content_reader);

void MetadataPatchCallback(const Request &req, Response &res);
void FileDeleteCallback(const Request &req, Response &res);
void TestPatchCallback(const Request &req, Response &res);

string GetBucketName(string apptype);


#endif /* __CPP_HTTPLIB_UPLOAD_H__ */
