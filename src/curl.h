/*
 * simple curl functions
 *
 * (C) 2015-2017 M. Liebmann (micha-bbg)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __CURL_H__
#define __CURL_H__

#include <stdint.h>

#include <string>

#include <curl/curl.h>
#include <curl/easy.h>

using namespace std;

struct progressData {
	CURL *curl;
	curl_off_t last_dlnow;
};

class CCurl
{
	private:
		static size_t CurlWriteToString(void *ptr, size_t size, size_t nmemb, void *data);
#if LIBCURL_VERSION_NUM < 0x072000
		static int CurlProgressFunc_old(void *p, double dltotal, double dlnow, double ultotal, double ulnow);
#endif
		static int CurlProgressFunc(void *p, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow);
		static size_t CurlGetContentLengthFunc(void *ptr, size_t size, size_t nmemb, void *stream);

	public:
		enum {
			PRIV_CURL_OK              = 0,
			PRIV_CURL_ERR_HANDLE      = 1,
			PRIV_CURL_ERR_NO_URL      = 2,
			PRIV_CURL_ERR_CREATE_FILE = 3,
			PRIV_CURL_ERR_CURL        = 4
		};

		CCurl() {};
		~CCurl() {};

		int CurlDownload(string url,
			   	 string& output,
			   	 bool outputToFile=true,
			   	 string userAgent="",
			   	 bool silent=false,
				 bool verbose=false,
		  		 const char* range=NULL,
				 bool passHeader=false,
			   	 string postfields="",
			   	 int connectTimeout=20,
			   	 int ipv=10,
			   	 bool useProxy=false,
			   	 bool followRedir=true,
			   	 int maxRedirs=20);
};


#endif //__CURL_H__
