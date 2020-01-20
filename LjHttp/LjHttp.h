#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "curl/curl.h"
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
/*    * 宏定义curl个参数大小*/
#define MAX_IP_LEN        (32)             //ip地址最大长度
#define MAX_URL_PATH_LEN  (2083)            //url路径最大长度
#define MAX_USER_LEN      (64)             //用户名最大长度
#define MAX_PWD_LEN       (64)             //验证密码最大长度
#define MAX_URL_LEN       (168)            //url最大长度
#define MAX_REC_LEN       (204800)          //返回数据最大长度
#define MAX_TEMP_LEN      (128)            //临时变量最大长度
#define C_CONNECT_DEFAULT_TIMEOUT  10  //连接最长时间

#define C_DEFAULT_TIMEOUT          600 //传输最长时间

#ifdef LJHTTP_EXPORTS
#define LJHTTP_API __declspec(dllexport)
#else
#define LJHTTP_API __declspec(dllimport)
#endif

enum MethodType
{
	METHOD_NO = 0,
	METHOD_GET,
	METHOD_POST,
};
typedef struct _CurlHeadParamNode {
	char *key;
	char *value;
	struct _CurlHeadParamNode * _PNext;
}CurlHeadParamNode,*PCurlHeadParamNode;
//封装了链表的结构
typedef struct _CurlHeadList       
{
	PCurlHeadParamNode _pHead;//指向链表第一个节点
}CurlHeadList;

LJHTTP_API CurlHeadList * CreateCurlHeadList();			//初始化链表
LJHTTP_API void PushBackCurlHeadList(CurlHeadList* s, char * key, char *value);//添加链表
LJHTTP_API void FreeCurlHeadList(CurlHeadList* s);            //清空链表

/*   
* 返回数据结构体，包含相关参数
*/
typedef struct st_curl_rec {
	char rec[MAX_REC_LEN];                  //curl返回数据数组空间大小,最大为200kb  
	int  len;                               //curl实际返回数据大小
	
}CurlResponse;

/*
	发送的参数
*/
typedef struct st_curl_params {
	char *serverUrl;						//url路径  
	int  port;                              //curl请求ip的端口 
	int  connect_timeout;                   //连接超时  
	int  timeout;                           //传输最长时间
	char * cookie;						    //cookie
	//是否使用代理
	char * proxyUrl;						//代理地址	 格式："127.0.0.1:8888"
	char * proxyNamePass;						//用户名密码 格式："user:password"
	CurlHeadList *curlHeadP;				//http头部信息

}CurlParams;


/* 进行所有CURL开始之前，全局变量初始化，放在主线程中*/
LJHTTP_API int  CurlGlobalInitResource(void);
/*进行所有CURL结束之前，全局资源释放，放在主线程中*/
LJHTTP_API int  CurlGlobalCleanupResource();

/*
	POST 方法
	简单http请求：端口，超时时间等参数为默认值
	参数1：serverUrl请求地址
	参数2：curlHeadP请求头部
	参数3：postMessage发送的信息
	参数4：curl_rec应答数据
	参数5：errorMessage错误信息
	返回值：请求结果
*/
LJHTTP_API bool HttpPostSimple( char *serverUrl,CurlHeadList* curlHeadP, const char* postMessage, CurlResponse* curl_rec, char * errorMessage);

/*
	POST 方法
	带有可配置参数的http请求
	参数1：CurlParams请求参数
	参数2：postMessage发送的信息
	参数3：curl_rec应答数据
	参数4：errorMessage错误信息
	返回值：请求结果
*/
LJHTTP_API bool HttpPostAttach(CurlParams* curl_params, const char* postMessage, CurlResponse* curl_rec, char * errorMessage);

/*
	GET 方法
	简单http请求：端口，超时时间等参数为默认值
	参数1：serverUrl请求地址
	参数2：curlHeadP请求头部
	参数3：curl_rec应答数据
	参数4：errorMessage错误信息
	返回值：请求结果
*/
LJHTTP_API bool HttpGetSimple(char *serverUrl, CurlHeadList* curlHeadP, CurlResponse* curl_rec, char * errorMessage);

/*
	GET 方法
	带有可配置参数的http请求
	参数1：CurlParams请求参数
	参数2：curl_rec应答数据
	参数3：errorMessage错误信息
	返回值：请求结果
*/
LJHTTP_API bool HttpGetAttach(CurlParams* curl_params, CurlResponse* curl_rec, char * errorMessage);
/*
	下载文件
	带有可配置参数的http请求
	参数1：CurlParams请求参数
	参数2：下载文件名
	参数3：errorMessage错误信息
	返回值：请求结果
*/
LJHTTP_API bool HttpDownloadFile(CurlParams* curl_params, const char* filename, void * ProgressCallback, char * errorMessage);

/*
	上传文件
	带有可配置参数的http请求
	参数1：CurlParams请求参数
	参数2：上传的文件名
	参数3：curl_rec 返回数据,无需初始化,内部初始化
	参数4：errorMessage错误信息
	返回值：请求结果
*/
LJHTTP_API bool HttpUploadFile(CurlParams* curl_params, const char* file_fullname, CurlResponse* curl_rec, char * errorMessage);
