
#include <curl/curl.h>
#include "ref.hpp"


struct CallbackInfo
{
	void (*next_callback)(size_t);
	int total_sent;
};

extern "C" int DLCallback
 (void* clientp,
  double dltotal,
  double dlnow,
  double ultotal,
  double ulnow)
{
	CallbackInfo* cbi = reinterpret_cast< CallbackInfo* >(clientp);
	if (!cbi->total_sent)
	{
		cbi->next_callback((size_t)dltotal);
		cbi->total_sent = 1;
	}
	cbi->next_callback((size_t)dlnow);
	return 0;
}


size_t DownloadFile
 (const char* url,
  const char* local,
  void (*prog_callback)(size_t) = 0)
{
	CURL* curl = curl_easy_init();
	if (!curl)
		return 0;
	RefType< CURL >::Ref curl_closer(curl, curl_easy_cleanup);

	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);

	FILE* outfile = fopen(local, "wb");
	if (!outfile)
		return 0;
	RefType< FILE >::Ref fcloser(outfile, fclose);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);

	if (prog_callback)
	{
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, DLCallback);
		CallbackInfo info;
		info.next_callback = prog_callback;
		info.total_sent = 0;
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &info);
	}
	else
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

	CURLcode success = curl_easy_perform(curl);

	if (success != 0)
	{
		fcloser = RefType< FILE >::Ref();
		DeleteFile(local);
		return 0;
	}

	double dltotal = 0.0;
	curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dltotal);
	return (size_t)dltotal;
}
