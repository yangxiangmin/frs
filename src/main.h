#ifndef __FRS_INCLUDE__
#define __FRS_INCLUDE__
#include "httplib.h"

#include "fileUtils.h"
#include "netUtils.h"
#include "stringUtils.h"

#include "json/json.h"
using namespace Json;

using namespace std;
using namespace httplib;


#define FRS_VERSION     "V00.00.09"
#define FRS_BUILT_ON    __DATE__ " " __TIME__


#define THIS_FILE       __FILE__
#define THIS_FUNC       __FUNCTION__
#define THIS_LINE       __LINE__
#define NONUSE_PARAM(x) {if (x) { x = x;}}
#define NOT(x)          (!(x))

#define CACHE_DIR         string("/app/run/frs/dat/fileCache/")
#define CACHE_TEMP_DIR    string("/app/run/frs/dat/fileCache/temp/")

void ErrorHandler(const Request &req, Response &res);

#endif  /* __FRS_INCLUDE__ */
