#include "LjHttp.h"
#pragma comment(lib,"libcurl.lib")
static bool s_global_init_status = 0;        //�ж��Ƿ��ѽ���ȫ�ֳ�ʼ����0��ʾδ��ʼ����1��ʾ�Ѿ���ʼ��

LJHTTP_API CurlHeadList * CreateCurlHeadList()
{
	CurlHeadList * re = (CurlHeadList *)malloc(sizeof(CurlHeadList));
	re->_pHead = NULL;
	return re;
}
LJHTTP_API void PushBackCurlHeadList(CurlHeadList* s, char * key, char *value) {
	//���������һ���ڵ�
	assert(s);
	PCurlHeadParamNode pNewNode = (PCurlHeadParamNode)malloc(sizeof(CurlHeadParamNode));

	pNewNode->key = (char*)malloc(strlen(key) + 1);
	memcpy(pNewNode->key, key, strlen(key)+1);
	
	pNewNode->value = (char*)malloc(strlen(value) + 1);
	memcpy(pNewNode->value, value, strlen(value) + 1);

	//pNewNode->key = key;
	//pNewNode->value = value;

	pNewNode->_PNext = NULL;
	//����û�нڵ�����
	if (s->_pHead == NULL) {
		s->_pHead = pNewNode;
	}
	else {
		PCurlHeadParamNode pCur = s->_pHead;
		while (pCur->_PNext) {
			pCur = pCur->_PNext;
		}
		//�����һ���ڵ�ָ���½ڵ�
		pCur->_PNext = pNewNode;
	}
}
LJHTTP_API void FreeCurlHeadList(CurlHeadList* s) {             //�������
	assert(s);
	if (s->_pHead == NULL) {
		return;
	}
	PCurlHeadParamNode pCur = s->_pHead;
	while (pCur->_PNext) {    //ѭ����������еĽڵ�
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
		//ͳһת��ΪСд�ַ�
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
	* ������Ϣͷ
	* ����ֵint, 0��ʾ�ɹ�, ������ʾʧ��
*/
static int CurlSetHeaders(CURL* curl, CurlHeadList *curlP, struct curl_slist** headers)
{
	if (NULL == curl || NULL == headers || NULL == curlP)
	{
		return -1;       //CURL_PARAMSָ��ΪNULL
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
	* CURL��������
	* ����1��curlָ�룬ͨ����ָ������������
	* ����2��curl�����ṹ��
	* ����ֵint��0��ʾ�ɹ���������ʾʧ��
*/
static int CurlPublicMethod(CURL* curl, CurlParams* curl_params)
{
	if (NULL == curl || NULL == curl_params)
	{
		printf("CurlPublicMethod curl or curl_params ptr is null \n");
		return -1;       //CURLָ��ΪNULL
	}

	//ָ��������url
	if (curl_params->serverUrl != NULL) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, curl_params->serverUrl);
	}

	curl_easy_setopt(curl, CURLOPT_PORT, curl_params->port);

	//���õ�alarm���ֳ�ʱ
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);

	//���ó�ʱʱ��
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
	//����cookie
	if (curl_params->cookie != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_COOKIE, curl_params->cookie);
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, curl_params->cookie);
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, curl_params->cookie);
	}
	//���ô��������
	if (curl_params->proxyUrl != NULL) 
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, curl_params->proxyUrl);
		//�ж��Ƿ����https
		if (stristri(curl_params->proxyUrl, "https"))
		{
			//���
			curl_easy_setopt(curl, CURLOPT_HTTPPROXYTUNNEL, 1L);
		}
	}
	//�����û�������
	if (curl_params->proxyNamePass != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, curl_params->proxyNamePass);
	}
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);//0����֤���� 1���м��
	//curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);     //���Կ���������Ϣ
	curl_easy_setopt(curl, CURLOPT_SSLVERSION, 0);
	/*
	Ĭ�������libcurl���һ�������Ժ󣬳����������ӵĿ��ǲ������Ϲر�,���ÿ��Ŀ��������һ���������ֹ������
	ÿ��ִ����curl_easy_perform��licurl���������������������ӡ����������������ʹ��������Ӷ����ش����µ�����,���Ŀ��������ͬһ���Ļ���
	*/
	curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1);

	return 0;

}

/*
	* �ص������������ص�����
	* ����1��������
	* ����2���������
	* ����3������ÿ���С
	* ����4��WRITEDATA��Ӧ��������
	* ����ֵ��������ռ�ֽ���
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
	* curl����ֵ����
	* ����1: curl������
	* ����ֵint, 0��ʾ�ɹ�, ������ʾʧ��
*/
static int DealResCode(CURL* curl, const CURLcode res, char * errorMessage)
{
	//���������������˼
	int nCode = 0;
	
	errorMessage = (char *)curl_easy_strerror(res);
	printf("%s\n", errorMessage);

	//http������
	long lResCode = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &lResCode);

	if (CURLE_OK != res || 200 != lResCode)
	{
		//curl����ʧ��
		if (CURLE_OPERATION_TIMEOUTED == res)
		{
			nCode = 1;   //��ʱ����1
		}
		else
		{
			nCode = -1;    //�������󷵻�-1
		}
		printf("curl send msg error: pRes=%s, lResCode=%ld \n", errorMessage, lResCode);
	}

	return nCode;
}

/*
	* ͨ��get�ķ�ʽ����
	* ����1: curl�ṹ��
	* ����2: curl�����ṹ��
	* ����3: ���ص����ݽṹ��
	* ����4: ������Ϣ
	* ����ֵint, 0��ʾ�ɹ�, 1��ʾ��ʱ,������ʾʧ��
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
		return -1;       //CURL_PARAMSָ��ΪNULL
	}

	//�趨���䷽ʽ
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

	//������Ϣͷ
	struct curl_slist *headers = NULL;
	
	CurlSetHeaders(curl, curl_params->curlHeadP, &headers);
	

	//CURL����������ʽ
	nCode = CurlPublicMethod(curl, curl_params);
	if (0 != nCode)
	{
		//�ͷ�ͷ�ṹ��
		if (NULL != headers)
		{
			curl_slist_free_all(headers);
		}

		printf("CurlGetMessage call CurlPublicMethod failure \n");
		return -1;
	}

	// ���ûص�����
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	curl_rec->len = 0;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)curl_rec);
	res = curl_easy_perform(curl);

	//�ͷ�ͷ�ṹ��
	if (NULL != headers)
	{
		curl_slist_free_all(headers);
		headers = NULL;
	}

	//����curl����ֵ
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	//���سɹ�
	return nCode;

}

/*
	* ͨ��post�ķ�ʽ����
	* ����1: curlָ��
	* ����2: curl�����ṹ��
	* ����3: Ҫ���͵�����
	* ����4: ���ص�����
	* ����ֵint, 0��ʾ�ɹ�, 1��ʾ��ʱ,������ʾʧ��
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
		return -1;       //CURL_PARAMSָ��ΪNULL
	}

	//��������,�Լ����ͷ�ʽ
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	if (NULL != msg && '\0' != msg[0])
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "");
	}
	//������Ϣͷ
	struct curl_slist *headers = NULL;

	CurlSetHeaders(curl, curl_params->curlHeadP, &headers);

	//CURL����������ʽ
	nCode = CurlPublicMethod(curl, curl_params);
	if (0 != nCode)
	{
		//�ͷ�ͷ�ṹ��
		if (NULL != headers)
		{
			curl_slist_free_all(headers);
			headers = NULL;
		}

		printf("CurlPostMessage call CurlPublicMethod failure \n");
		return -1;
	}

	// ���ûص�����
	
	curl_rec->len = 0;
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, curl_rec);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
	res = curl_easy_perform(curl);

	//�ͷ�ͷ�ṹ��
	if (NULL != headers)
	{
		curl_slist_free_all(headers);
		headers = NULL;

	}

	//����curl����ֵ
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	//���ر�ʶ��
	return nCode;

}
///* ���е����߳�CURL����Դ���г�ʼ��*/
CURL* CurlInitResource()
{
	return curl_easy_init();
}
///*���е����߳�CURL����Դ�����ͷ�*/
int CurlReleaseResource(CURL* curl) 
{
	if (NULL == curl)
	{
		printf("CurlReleaseResource curl ptr is null \n");
		return -1;       //CURLָ��ΪNULL
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
			//���سɹ�
			s_global_init_status = 1;
			return 0;
		}
		else
		{
			printf("curl_global_init error(%d)\n", errno);
			return -1;                  //CURLȫ����Դ��ʼ��ʧ��
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
	memset(curl_params, 0, sizeof(*curl_params));            //��ʼ�������ṹ��
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
	//Ĭ�ϲ���
	CurlParams curl_params;
	memset(&curl_params,0,sizeof(CurlParams));
	curl_params.serverUrl = serverUrl;
	curl_params.curlHeadP = curlHeadP;
	//�ж��Ƿ����https
	if (stristri(serverUrl, "https"))
	{
		//https �˿�
		curl_params.port = 443;
	}
	else
	{
		//http �˿�
		curl_params.port = 80;
	}
	curl_params.connect_timeout = C_CONNECT_DEFAULT_TIMEOUT;
	curl_params.timeout = C_DEFAULT_TIMEOUT;
	curl_params.proxyUrl = NULL;//���������Ϊ��
	curl_params.proxyNamePass = NULL;//�û�������Ϊ��
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
	��http���󣺶˿ڣ���ʱʱ��Ȳ���ΪĬ��ֵ
*/
LJHTTP_API bool HttpGetSimple(char *serverUrl, CurlHeadList* curlHeadP, CurlResponse* curl_rec, char * errorMessage) 
{
	if (NULL == serverUrl || NULL == curlHeadP || NULL == curl_rec || NULL == errorMessage)
	{
		return false;
	}
	CURL * curl = CurlInitResource();
	//Ĭ�ϲ���
	CurlParams curl_params;
	curl_params.serverUrl = serverUrl;
	curl_params.curlHeadP = curlHeadP;
	//�ж��Ƿ����https
	if (stristri(serverUrl, "https"))
	{
		//https �˿�
		curl_params.port = 443;
	}
	else
	{
		//http �˿�
		curl_params.port = 80;
	}
	
	curl_params.connect_timeout = C_CONNECT_DEFAULT_TIMEOUT;
	curl_params.timeout = C_DEFAULT_TIMEOUT;
	curl_params.proxyUrl = NULL;//���������Ϊ��
	curl_params.proxyNamePass = NULL;//�û�������Ϊ��

	int nCode = CurlGetMessage(curl, &curl_params , curl_rec, errorMessage);
	CurlReleaseResource(curl);
	return CURLE_OK == nCode;
}

/*
	���п����ò�����http����
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
	* �ص������������ļ�����д�ļ�
	* ����1��������
	* ����2���������
	* ����3������ÿ���С
	* ����4��WRITEDATA��Ӧ��������
	* ����ֵ��������ռ�ֽ���
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
		return false;       //CURL_PARAMSָ��ΪNULL
	}

	pFile = fopen(filename, "wb");         //���ļ�,���ؽ�����ļ��洢
	if (NULL == pFile)
	{
		printf("download_file open file error(%d), %s\n", errno, filename);
		return -1;      //���ļ�ʧ��
	}

	//������Ϣͷ
	struct curl_slist *headers = NULL;

	CurlSetHeaders(curl, curl_params->curlHeadP, &headers);

	// �����ض����������  
	curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
	// ����301��302��ת����location  
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	// ��CURLOPT_NOPROCESS����Ϊ0��������֧��
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
	curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_TIME, 10L);
	curl_easy_setopt(curl, CURLOPT_LOW_SPEED_LIMIT, 50L);
	curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, 2000000L);/*��������ٶ�*/

	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);//0����֤���� 1���м��
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
	//CURL����������ʽ
	nCode = CurlPublicMethod(curl, curl_params);
	if (0 != nCode)
	{
		//�ͷ�ͷ�ṹ��
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

	// ���ûص�����
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, ProgressCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, pFile);
	res = curl_easy_perform(curl);

	//�ͷ���Դ
	{
		//�ͷ�ͷ�ṹ��
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

	//����curl����ֵ
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	CurlReleaseResource(curl);
	return CURLE_OK == nCode;

}

/*
	* ���ļ�ȫ·���л�ȡ�ļ���ָ��
	* �����ļ�����ַ
*/
static const char* get_file_name_ptr(const char *path)
{
	if (!path)
	{
		return NULL;
	}

	//�����һ��б��/
	char *pname = strrchr(path, '/');
	if (!pname)
	{
		//û�ҵ�б��,����Ϊpath�����ļ���
		return path;
	}

	//�ҵ����һ��б��, ����ָ���1
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

	//�����Ϸ��Լ��
	if (NULL == curl || NULL == curl_params || NULL == file_fullname || '\0' == file_fullname[0] || NULL == curl_rec)
	{
		printf("upload_file_content curl or curl_params or file_fullname or curl_rec ptr is null \n");
		return -1;            //CURL_PARAMSָ��ΪNULL
	}

	//��ȡ�ļ���
	const char* file_name = get_file_name_ptr(file_fullname);
	if (NULL == file_name || '\0' == file_name[0])
	{
		printf("uploadFile call get_file_name failure, file_fullname=%s \n", file_fullname);
		return -1;
	}
	printf("file_name=%s \n", file_name);

	//CURL����������ʽ
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

	//����ֵ
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

	//����curl����ֵ
	nCode = DealResCode(curl, res, errorMessage);
	if (0 > nCode)
	{
		printf("deal response code error \n");
	}

	CurlReleaseResource(curl);
	return CURLE_OK == nCode;
}