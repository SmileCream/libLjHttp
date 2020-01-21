#include "LjHttp.h"
#pragma comment(lib,"libcurl.lib")
static bool s_global_init_status = 0;        //判断是否已进行全局初始化，0表示未初始化，1表示已经初始化

LJHTTP_API CurlHeadList * CreateCurlHeadList()
{
	CurlHeadList * re = (CurlHeadList *)malloc(sizeof(CurlHeadList));
	re->_pHead = NULL;
	return re;
}
LJHTTP_API void PushBackCurlHeadList(CurlHeadList* s, char * key, char *value) {
	//找链表最后一个节点
	assert(s);
	PCurlHeadParamNode pNewNode = (PCurlHeadParamNode)malloc(sizeof(CurlHeadParamNode));

	pNewNode->key = (char*)malloc(strlen(key) + 1);
	memcpy(pNewNode->key, key, strlen(key)+1);
	
	pNewNode->value = (char*)malloc(strlen(value) + 1);
	memcpy(pNewNode->value, value, strlen(value) + 1);

	//pNewNode->key = key;
	//pNewNode->value = value;

	pNewNode->_PNext = NULL;
	//链表没有节点的情况
	if (s->_pHead == NULL) {
		s->_pHead = pNewNode;
	}
	else {
		PCurlHeadParamNode pCur = s->_pHead;
		while (pCur->_PNext) {
			pCur = pCur->_PNext;
		}
		//让最后一个节点指向新节点
		pCur->_PNext = pNewNode;
	}
}
LJHTTP_API void FreeCurlHeadList(CurlHeadList* s) {             //清空链表
	assert(s);
	if (s->_pHead == NULL) {
		return;
	}
	PCurlHeadParamNode pCur = s->_pHead;
	while (pCur->_PNext) {    //循环清空链表中的节点
		PCurlHeadParamNode Tmp = pCur->_PNext;
		if (Tmp != NULL)
		{
			if (pCur->key != NULL) 
			{
				free(pCur->key);
				pCur->key = NULL;
			}
			if (pCur->value != NULL) 
			{
				free(pCur->value);
				pCur->value = NULL;
			}
			free(pCur);
			pCur = NULL;
		}
		//free(pCur);
		pCur = Tmp;
	}
	free(s);
}
char* stristri(char * inBuffer, char * inSearchStr)
{
	char*  currBuffPointer = inBuffer;

	while (*currBuffPointer != 0x00)
	{
		char* compareOne = currBuffPointer;
		char* compareTwo = inSearchStr;
		//统一转换为小写字符
		while (tolower(*compareOne) == tolower(*compareTwo))
		{
			compareOne++;
			compareTwo++;
			if (*compareTwo == 0x00)
			{
				return (char*)currBuffPointer;
			}

		}
		currBuffPointer++;
	}
	return NULL;
}
/*
	* 设置消息头
	* 返回值int, 0表示成功, 其它表示失败
*/
static int CurlSetHeaders(CURL* curl, CurlHeadList *curlP, struct curl_slist** headers)
{
	if (NULL == curl || NULL == headers || NULL == curlP)
	{
		return -1;       //CURL_PARAMS指针为NULL
	}

	assert(curlP);
	PCurlHeadParamNode pCur = curlP->_pHead;
	while (pCur) {
		char *temp = (char*)malloc(sizeof(char) * (strlen(pCur->key) + strlen(pCur->value) + 2));
		sprintf(temp, "%s:%s", pCur->key, pCur->value);
		*headers = curl_slist_append(*headers, temp);
		/*if(pCur != NULL)
		{
			if (pCur->key != NULL)
			{
				free(pCur->key);
				pCur->key = NULL;
			}
			if (pCur->value != NULL)
			{
				free(pCur->value);
				pCur->value = NULL;
			}
		}	*/
		pCur = pCur->_PNext;
		free(temp);
	}

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, *headers);
	return 0;
}

struct curl_slist* SetCurlHeaders(struct curl_slist * headers,const char * key, const char *value)
{
	char *temp = (char*)malloc(sizeof(char) * (strlen(key) + strlen(value) + 2));
	sprintf(temp, "%s:%s", key, value);
	headers = curl_slist_append(headers, temp);
	free(temp);
}

/*
	* CURL公共操作
	* 参数1：curl指针，通过此指针进行相关设置
	* 参数2：curl参数结构体
	* 返回值int，0表示成功，其他表示失败
*/
static int CurlPublicMethod(CURL* curl, CurlParams* curl_params)
{
	if (NULL == curl || NULL == curl_params)
	{
		printf("CurlPublicMethod curl or curl_params ptr is null \n");
		return -1;       //CURL指针为NULL
	}

	//指定服务器url
	if (curl_params->serverUrl != NULL) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, curl_params->serverUrl);
	}

	curl_easy_setopt(curl, CURLOPT_PORT, curl_params->port);

	//禁用掉alarm这种超时
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	//设置超时时间
	if (0 >= curl_params->connect_timeout)
	{
		curl_params->connect_timeout = C_CONNECT_DEFAULT_TIMEOUT;
	}
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, curl_params->connect_timeout);

	if (0 >= curl_params->timeout)
	{
		curl_params->timeout = C_DEFAULT_TIMEOUT;
	}
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, curl_params->timeout);
	//设置cookie
	if (curl_params->cookie != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_COOKIE, curl_params->cookie);
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, curl_params->cookie);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, curl_params->cookie);
	}
	//设置代理服务器
	if (curl_params->proxyUrl != NULL) 
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, curl_params->proxyUrl);
		//判断是否存在https
		if (stristri(curl_params->proxyUrl, "https"))
		{
			//隧道
			curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L);
		}
	}
	//设置用户名密码
	if (curl_params->proxyNamePass != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, curl_params->proxyNamePass);
	}
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);//0忽略证书检查 1进行检查
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);     //可以看到调试信息
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, 0);
	/*
	默认情况下libcurl完成一个任务以后，出于重用连接的考虑不会马上关闭,如果每次目标主机不一样，这里禁止重连接
	每次执行完curl_easy_perform，licurl会继续保持与服务器的连接。接下来的请求可以使用这个连接而不必创建新的连接,如果目标主机是同一个的话。
	*/
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

	return 0;

}

/*
	* 回调函数，处理返回的数据
	* 参数1：缓存存放
	* 参数2：缓存块数
	* 参数3：缓存每块大小
	* 参数4：WRITEDATA对应的数据流
	* 返回值，数据所占字节数
*/
static size_t WriteCallback(void* buffer, size_t size, size_t nmemb, void* content)
{
	CurlResponse* p_rec = (CurlResponse*)content;
	long sizes = size * nmemb;
	if ((p_rec->len + sizes) > sizeof(CurlResponse))
	{
		printf("http-getbyte buff is to small !\n");
		return sizes;
	}
	memcpy(&(p_rec->rec[p_rec->len]), buffer, sizes);
	p_rec->len += sizes;
	return sizes;
}
/*
	* curl返回值处理
	* 参数1: curl返回码
	* 返回值int, 0表示成功, 其他表示失败
*/
static int DealResCode(CURL* curl, const CURLcode res, char * errorMessage)
{
	//输出返回码代表的意思
	int nCode = 0;
	
	errorMessage = (char *)curl_easy_strerror(res);
	printf("%s\n", errorMessage);

	//http返回码
	long lResCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &lResCode);

	if (CURLE_OK != res || 200 != lResCode)
	{
		//curl传送失败
		if (CURLE_OPERATION_TIMEOUTED == res)
		{
			nCode = 1;   //超时返回1
		}
		else
		{
			nCode = -1;    //其它错误返回-1
		}
		printf("curl send msg error: pRes=%s, lResCode=%ld \n", errorMessage, lResCode);
	}

	return nCode;
}

/*
	* 通过get的方式操作
	* 参数1: curl结构体
	* 参数2: curl参数结构体
	* 参数3: 返回的数据结构体
	* 参数4: 错误信息
	* 返回值int, 0表示成功, 1表示超时,其他表示失败
*/
static int CurlGetMessage(CURL* curl,
	CurlParams* curl_params,
	CurlResponse* curl_rec, char * errorMessage)
{
	CURLcode res = CURLE_OK;
	int nCode = 0;

	if (NULL == curl || NULL == curl_params || NULL == curl_rec)
	{
		printf("get_msg curl or curl_params or msg ptr is null \n");
		return -1;       //CURL_PARAMS指针为NULL
	}

	//设定传输方式
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

	//设置消息头
	struct curl_slist *headers = NULL;
	
	CurlSetHeaders(curl, curl_params->curlHeadP, &headers);
	

	//CURL公共操作方式
	nCode = CurlPublicMethod(curl, curl_params);
	if (0 != nCode)
	{
		//释放头结构体
		if (NULL != headers)
		{
			curl_slist_free_all(headers);
		}

		printf("CurlGetMessage call CurlPublicMethod failure \n");
		return -1;
	}

	// 设置回调函数
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_rec->len = 0;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)curl_rec);
	res = curl_easy_perform(curl);

	//释放头结构体
	if (NULL != headers)
	{
		curl_slist_free_all(headers);
		headers = NULL;
	}

	//处理curl返回值
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	//返回成功
	return nCode;

}

/*
	* 通过post的方式操作
	* 参数1: curl指针
	* 参数2: curl参数结构体
	* 参数3: 要发送的数据
	* 参数4: 返回的数据
	* 返回值int, 0表示成功, 1表示超时,其他表示失败
*/
static int CurlPostMessage(CURL* curl,
	CurlParams* curl_params,
	const char* msg,
	CurlResponse* curl_rec,
	char * errorMessage)
{
	CURLcode res = CURLE_OK;
	int nCode = 0;

	if (NULL == curl || NULL == curl_params || NULL == curl_rec)
	{
		printf("CurlPostMessage curl or curl_params or curl_rec ptr is null \n");
		return -1;       //CURL_PARAMS指针为NULL
	}

	//发送数据,以及发送方式
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	if (NULL != msg && '\0' != msg[0])
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	}
	//设置消息头
	struct curl_slist *headers = NULL;

	CurlSetHeaders(curl, curl_params->curlHeadP, &headers);

	//CURL公共操作方式
	nCode = CurlPublicMethod(curl, curl_params);
	if (0 != nCode)
	{
		//释放头结构体
		if (NULL != headers)
		{
			curl_slist_free_all(headers);
			headers = NULL;
		}

		printf("CurlPostMessage call CurlPublicMethod failure \n");
		return -1;
	}

	// 设置回调函数
	
	curl_rec->len = 0;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_rec);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	res = curl_easy_perform(curl);

	//释放头结构体
	if (NULL != headers)
	{
		curl_slist_free_all(headers);
		headers = NULL;

	}

	//处理curl返回值
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	//返回标识码
	return nCode;

}
///* 进行单个线程CURL简单资源进行初始化*/
CURL* CurlInitResource()
{
	return curl_easy_init();
}
///*进行单个线程CURL简单资源进行释放*/
int CurlReleaseResource(CURL* curl) 
{
	if (NULL == curl)
	{
		printf("CurlReleaseResource curl ptr is null \n");
		return -1;       //CURL指针为NULL
	}
	curl_easy_cleanup(curl);
	curl = NULL;

	return 0;
}
LJHTTP_API int CurlGlobalInitResource()
{
	if (0 == s_global_init_status)
	{

		if (CURLE_OK == curl_global_init(CURL_GLOBAL_ALL))
		{
			//返回成功
			s_global_init_status = 1;
			return 0;
		}
		else
		{
			printf("curl_global_init error(%d)\n", errno);
			return -1;                  //CURL全局资源初始化失败
		}

	}

	return 0;
}
LJHTTP_API int CurlGlobalCleanupResource()
{
	if (1 == s_global_init_status)
	{
		curl_global_cleanup();
		s_global_init_status = 0;
	}

	return 0;
}

int CurlInitParams(CurlParams * curl_params, const char * serverUrl, const int port, int connect_timeout, int timeout)
{
	if (NULL == curl_params || NULL == serverUrl)
	{
		printf("CurlParams curl_params or serverUrl ptr is null \n");
		return -1;
	}
	memset(curl_params, 0, sizeof(*curl_params));            //初始化整个结构体
	curl_params->serverUrl = serverUrl;
	curl_params->port = port;
	curl_params->connect_timeout = connect_timeout;
	curl_params->timeout = timeout;
	return 0;

}



LJHTTP_API bool HttpPostSimple(char *serverUrl, CurlHeadList* curlHeadP, const char* postMessage, CurlResponse* curl_rec, char * errorMessage) 
{
	if (NULL == serverUrl|| NULL == curlHeadP || NULL == curl_rec || NULL == errorMessage)
	{
		return false;
	}
	CURL * curl = CurlInitResource();
	//默认参数
	CurlParams curl_params;
	memset(&curl_params,0,sizeof(CurlParams));
	curl_params.serverUrl = serverUrl;
	curl_params.curlHeadP = curlHeadP;
	//判断是否存在https
	if (stristri(serverUrl, "https"))
	{
		//https 端口
		curl_params.port = 443;
	}
	else
	{
		//http 端口
		curl_params.port = 80;
	}
	curl_params.connect_timeout = C_CONNECT_DEFAULT_TIMEOUT;
	curl_params.timeout = C_DEFAULT_TIMEOUT;
	curl_params.proxyUrl = NULL;//代理服务器为空
	curl_params.proxyNamePass = NULL;//用户名密码为空
	curl_params.cookie = NULL;//cookie
	int nCode = CurlPostMessage(curl, &curl_params, postMessage, curl_rec, errorMessage);
	CurlReleaseResource(curl);
	return CURLE_OK == nCode;
}
LJHTTP_API bool HttpPostAttach(CurlParams* curl_params, const char* postMessage, CurlResponse* curl_rec, char * errorMessage) 
{
	if (NULL == curl_params  || NULL == curl_rec || NULL == errorMessage)
	{
		return false;
	}
	CURL * curl = CurlInitResource();
	int nCode = CurlPostMessage(curl, curl_params, postMessage, curl_rec, errorMessage);
	CurlReleaseResource(curl);
	return CURLE_OK == nCode;
}

/*
	简单http请求：端口，超时时间等参数为默认值
*/
LJHTTP_API bool HttpGetSimple(char *serverUrl, CurlHeadList* curlHeadP, CurlResponse* curl_rec, char * errorMessage) 
{
	if (NULL == serverUrl || NULL == curlHeadP || NULL == curl_rec || NULL == errorMessage)
	{
		return false;
	}
	CURL * curl = CurlInitResource();
	//默认参数
	CurlParams curl_params;
	curl_params.serverUrl = serverUrl;
	curl_params.curlHeadP = curlHeadP;
	//判断是否存在https
	if (stristri(serverUrl, "https"))
	{
		//https 端口
		curl_params.port = 443;
	}
	else
	{
		//http 端口
		curl_params.port = 80;
	}
	
	curl_params.connect_timeout = C_CONNECT_DEFAULT_TIMEOUT;
	curl_params.timeout = C_DEFAULT_TIMEOUT;
	curl_params.proxyUrl = NULL;//代理服务器为空
	curl_params.proxyNamePass = NULL;//用户名密码为空

	int nCode = CurlGetMessage(curl, &curl_params , curl_rec, errorMessage);
	CurlReleaseResource(curl);
	return CURLE_OK == nCode;
}

/*
	带有可配置参数的http请求
*/
LJHTTP_API bool HttpGetAttach(CurlParams* curl_params, CurlResponse* curl_rec, char * errorMessage) 
{
	if (NULL == curl_params || NULL == curl_rec || NULL == errorMessage)
	{
		return false;
	}
	CURL * curl = CurlInitResource();
	int nCode = CurlGetMessage(curl, curl_params , curl_rec, errorMessage);
	CurlReleaseResource(curl);
	return CURLE_OK == nCode;
}
/*
	* 回调函数，下载文件处理，写文件
	* 参数1：缓存存放
	* 参数2：缓存块数
	* 参数3：缓存每块大小
	* 参数4：WRITEDATA对应的数据流
	* 返回值，数据所占字节数
*/
static size_t write_file(void* buffer, size_t size, size_t nmemb, void* file)
{
	return fwrite(buffer, size, nmemb, (FILE *)file);
}
LJHTTP_API bool HttpDownloadFile(CurlParams* curl_params, const char* filename, void * ProgressCallback,char * errorMessage)
{
	CURL * curl = CurlInitResource();
	CURLcode res = CURLE_OK;
	FILE* pFile = NULL;
	int nCode = 0;

	if (NULL == curl || NULL == curl_params || NULL == filename || '\0' == filename[0])
	{
		printf("download_file curl or curl_params or filename ptr is null \n");
		return false;       //CURL_PARAMS指针为NULL
	}

	pFile = fopen(filename, "wb");         //打开文件,返回结果用文件存储
	if (NULL == pFile)
	{
		printf("download_file open file error(%d), %s\n", errno, filename);
		return -1;      //打开文件失败
	}

	//设置消息头
	struct curl_slist *headers = NULL;

	CurlSetHeaders(curl, curl_params->curlHeadP, &headers);

	// 设置重定向的最大次数  
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	// 设置301、302跳转跟随location  
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	// 将CURLOPT_NOPROCESS设置为0开启进度支持
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 10L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 50L);
	curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, 2000000L);/*下载最高速度*/

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);//0忽略证书检查 1进行检查
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
	//CURL公共操作方式
	nCode = CurlPublicMethod(curl, curl_params);
	if (0 != nCode)
	{
		//释放头结构体
		if (NULL != headers)
		{
			curl_slist_free_all(headers);
			headers = NULL;
		}

		if (NULL != pFile)
		{
			fclose(pFile);
			pFile = NULL;
		}

		printf("download_file call CurlPublicMethod failure \n");
		return -1;
	}

	// 设置回调函数
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, pFile);
	res = curl_easy_perform(curl);

	//释放资源
	{
		//释放头结构体
		if (NULL != headers)
		{
			curl_slist_free_all(headers);
			headers = NULL;
		}

		if (NULL != pFile)
		{
			fclose(pFile);
			pFile = NULL;
		}
	}

	//处理curl返回值
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	CurlReleaseResource(curl);
	return CURLE_OK == nCode;

}

/*
	* 从文件全路径中获取文件名指针
	* 返回文件名地址
*/
static const char* get_file_name_ptr(const char *path)
{
	if (!path)
	{
		return NULL;
	}

	//找最后一个斜杠/
	char *pname = strrchr(path, '/');
	if (!pname)
	{
		//没找到斜杠,则认为path就是文件名
		return path;
	}

	//找到最后一个斜杠, 反回指针加1
	return (char*)(pname + 1);
}
LJHTTP_API bool HttpUploadFile(CurlParams* curl_params,const char* file_fullname,CurlResponse* curl_rec, char * errorMessage)
{
	CURL * curl = CurlInitResource();
	CURLcode res = CURLE_OK;
	int nCode = 0;
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";

	//参数合法性检查
	if (NULL == curl || NULL == curl_params || NULL == file_fullname || '\0' == file_fullname[0] || NULL == curl_rec)
	{
		printf("upload_file_content curl or curl_params or file_fullname or curl_rec ptr is null \n");
		return -1;            //CURL_PARAMS指针为NULL
	}

	//获取文件名
	const char* file_name = get_file_name_ptr(file_fullname);
	if (NULL == file_name || '\0' == file_name[0])
	{
		printf("uploadFile call get_file_name failure, file_fullname=%s \n", file_fullname);
		return -1;
	}
	printf("file_name=%s \n", file_name);

	//CURL公共操作方式
	nCode = CurlPublicMethod(curl, curl_params);
	if (0 != nCode)
	{
		printf("upload_file_content call CurlPublicMethod failure \n");
		return -1;
	}

	/*
	Fill in the file upload field. This makes libcurl load data from
	 the given file name when curl_easy_perform() is called.
	*/
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "sendfile",
		CURLFORM_FILE, file_fullname,
		CURLFORM_END);

	/* Fill in the filename field */
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "filename",
		CURLFORM_COPYCONTENTS, file_name,
		CURLFORM_END);

	/* Fill in the submit field too, even if this is rarely needed */
	curl_formadd(&formpost,
		&lastptr,
		CURLFORM_COPYNAME, "submit",
		CURLFORM_COPYCONTENTS, "send",
		CURLFORM_END);

	headerlist = curl_slist_append(headerlist, buf);

	/* only disable 100-continue header if explicitly requested */
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	//返回值
	memset(curl_rec, 0, sizeof(CurlResponse));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)curl_rec);


	/* Perform the request, res will get the return code */
	res = curl_easy_perform(curl);

	/* then cleanup the formpost chain */

	if (NULL != formpost)
	{
		curl_formfree(formpost);
		formpost = NULL;
	}

	/* free slist */
	if (NULL != headerlist)
	{
		curl_slist_free_all(headerlist);
		headerlist = NULL;
	}

	//处理curl返回值
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	CurlReleaseResource(curl);
	return CURLE_OK == nCode;
}