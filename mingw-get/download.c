
#include <curl/curl.h>
#include "error.hh"


typedef struct CallbackInfo
{
	void (*next_callback)(size_t);
	int total_sent;
} CallbackInfo;

int DLCallback
 (void* clientp,
  double dltotal,
  double dlnow,
  double ultotal,
  double ulnow)
{
	CallbackInfo* cbi = (CallbackInfo*)clientp;
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
  void (*prog_callback)(size_t))
{
	size_t ret = 0;

	CURL* curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		char errbuf[CURL_ERROR_SIZE];
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

		FILE* outfile = fopen(local, "wb");
		if (outfile)
		{
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
			if (success == 0)
			{
				double dltotal;
				curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dltotal);
				ret = dltotal;
			}
			else
			{
				DeleteFile(local);
				MGSetError("Failed to download '%s':\n%s", url, errbuf);
			}

			fclose(outfile);
		}
		else
			MGSetError("Couldn't open '%s' for writing.", local);

		curl_easy_cleanup(curl);
	}
	else
		MGSetError("Couldn't initialize CURL downloader.");

	return ret;
}
