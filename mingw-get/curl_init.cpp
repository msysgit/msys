
#include <curl/curl.h>

static struct CurlInit
{
	CurlInit()
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
	~CurlInit()
	{
		curl_global_cleanup();
	}
} curl_init;
