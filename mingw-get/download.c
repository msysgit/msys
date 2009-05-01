
#include "download.hh"

#include <curl/curl.h>
#include "error.hh"


typedef struct CallbackInfo
{
	int (*next_callback)(size_t, size_t, void*);
	void* callback_param;
} CallbackInfo;

int DLCallback
 (void* clientp,
  double dltotal,
  double dlnow,
  double ultotal,
  double ulnow)
{
	return ((CallbackInfo*)clientp)->next_callback((size_t)dltotal,
	 (size_t)dlnow, ((CallbackInfo*)clientp)->callback_param);
}


int DownloadFile
 (const char* url,
  const char* local,
  int (*prog_callback)(size_t, size_t, void*),
  void* callback_param)
{
	int ret = 1;

	CURL* curl = curl_easy_init();
	if (curl)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
		char errbuf[CURL_ERROR_SIZE];
		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);

		char* tempf = malloc(strlen(local) + 8);
		strcpy(tempf, local);
		strcpy(tempf + strlen(local), ".dltemp");
		FILE* outfile = fopen(tempf, "wb");
		if (outfile)
		{
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, outfile);

			if (prog_callback)
			{
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
				curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, DLCallback);
				CallbackInfo info;
				info.next_callback = prog_callback;
				info.callback_param = callback_param;
				curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &info);
			}
			else
				curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1);

			CURLcode success = curl_easy_perform(curl);

			fclose(outfile);

			if (success == 0)
			{
				if (MoveFileEx(tempf, local, MOVEFILE_REPLACE_EXISTING))
					ret = 0;
				else
					MGError("Failed to rename '%s' as '%s'", tempf, local);
			}
			else
			{
				DeleteFile(tempf);
				if (success == CURLE_ABORTED_BY_CALLBACK)
				{
					MGError("Download aborted by callback");
					ret = 2;
				}
				else
				{
					MGError(errbuf);
					MGError("Failed to download '%s'", url);
				}
			}

		}
		else
			MGError("Couldn't open '%s' for writing.", tempf);

		free(tempf);
		curl_easy_cleanup(curl);
	}
	else
		MGError("Couldn't initialize CURL downloader.");

	return ret;
}
