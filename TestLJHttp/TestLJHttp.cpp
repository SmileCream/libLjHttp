// TestLJHttp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>
#include <string.h>
#include <iostream>
using namespace std;
extern "C" {
	#include "LjHttp.h"
}
#pragma comment(lib,"LJHTTP.lib")


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

static int ProgressCallback(void *pParam, double dltotal, double dlnow, double ultotal, double ulnow)
{
	printf("---------------------------------");
	double bPercent = (dlnow / dltotal) * 100;
	printf("下载进度：%0.2lf%%\n", bPercent);
	return 0;
}

int main()
{
	CurlHeadList *chl = CreateCurlHeadList();
	//测试头部，这里无实际意义
	char key[] = "zm-auth-name";
	char value[] = "jg";
	PushBackCurlHeadList(chl, key, value);
	//目标服务器
	string url = "http://myip.top/";
	char *serverUrl = (char*)malloc(url.size() + 1);
	memcpy(serverUrl, url.c_str(), url.size() + 1);

	//代理服务器地址和端口
	string proxyUrl = "106.60.18.184:4228";
	char *_proxyUrl = (char*)malloc(proxyUrl.size() + 1);
	memcpy(_proxyUrl, proxyUrl.c_str(), proxyUrl.size() + 1);

	

	//错误信息
	char  errorMessage[1024];

	/********************************************************************************************/
	//设置返回数据结构体
	CurlResponse *curl_rec = new CurlResponse;
	memset(curl_rec, 0, sizeof(CurlResponse));

	//简单请求
	bool flag = HttpPostSimple(serverUrl , chl, "", curl_rec, errorMessage);
	printf("%s %d\n", curl_rec->rec,strlen(curl_rec->rec));
	free(curl_rec);
	/********************************************************************************************/
	//带有自定义的请求参数的（POST）方法
	//请求参数
	CurlParams curl_params;
	memset(&curl_params, 0, sizeof(CurlParams));
	curl_params.serverUrl = serverUrl;
	curl_params.curlHeadP = chl;
	curl_params.port = 80;
	curl_params.connect_timeout = 600;
	curl_params.timeout = 600;
	//curl_params.proxyUrl = _proxyUrl;

	//设置返回数据结构体
	CurlResponse *curl_rec_1 = new CurlResponse;
	memset(curl_rec_1, 0, sizeof(CurlResponse));
	bool flag_1 = HttpPostAttach(&curl_params, "", curl_rec_1, errorMessage);

	printf("%s %d\n", curl_rec_1->rec,strlen(curl_rec_1->rec));
	free(curl_rec_1);
	/********************************************************************************************/
	//带有自定义的请求参数的（GET方法）
	CurlResponse *curl_rec_2 = new CurlResponse;
	memset(curl_rec_2, 0, sizeof(CurlResponse));
	bool flag_2 = HttpGetAttach(&curl_params, curl_rec_2, errorMessage);

	printf("%s %d\n", curl_rec_2->rec, strlen(curl_rec_2->rec));
	free(curl_rec_2);
	/********************************************************************************************/

	//测试下载
	string pUrl = "https://download.tortoisegit.org/tgit/2.9.0.0/TortoiseGit-LanguagePack-2.9.0.0-64bit-zh_CN.msi";
	char *downUrl = (char*)malloc(pUrl.size() + 1);
	memcpy(downUrl, pUrl.c_str(), pUrl.size() + 1);
	curl_params.serverUrl = downUrl;
	const char * fileName = "d:\\2.txt";
	HttpDownloadFile(&curl_params,fileName, ProgressCallback,errorMessage);

	printf("%s\n","----------------------end------------------------");
	FreeCurlHeadList(chl);
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件


	/*CurlGlobalInitResource();
	for (int i = 0; i < 10; i++) {
		HANDLE  hHandle = NULL;
		hHandle = CreateThread(NULL, NULL, doHttpRequest, NULL, NULL, NULL);
		CloseHandle(hHandle);
	}

	while (1) {
		Sleep(1);
	}

	CurlGlobalCleanupResource();*/