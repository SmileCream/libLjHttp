#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "curl/curl.h"
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
/*    * �궨��curl��������С*/
#define MAX_IP_LEN        (32)             //ip��ַ��󳤶�
#define MAX_URL_PATH_LEN  (2083)            //url·����󳤶�
#define MAX_USER_LEN      (64)             //�û�����󳤶�
#define MAX_PWD_LEN       (64)             //��֤������󳤶�
#define MAX_URL_LEN       (168)            //url��󳤶�
#define MAX_REC_LEN       (204800)          //����������󳤶�
#define MAX_TEMP_LEN      (128)            //��ʱ������󳤶�
#define C_CONNECT_DEFAULT_TIMEOUT  10  //�����ʱ��

#define C_DEFAULT_TIMEOUT          600 //�����ʱ��

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
//��װ������Ľṹ
typedef struct _CurlHeadList       
{
	PCurlHeadParamNode _pHead;//ָ�������һ���ڵ�
}CurlHeadList;

LJHTTP_API CurlHeadList * CreateCurlHeadList();			//��ʼ������
LJHTTP_API void PushBackCurlHeadList(CurlHeadList* s, char * key, char *value);//�������
LJHTTP_API void FreeCurlHeadList(CurlHeadList* s);            //�������

/*   
* �������ݽṹ�壬������ز���
*/
typedef struct st_curl_rec {
	char rec[MAX_REC_LEN];                  //curl������������ռ��С,���Ϊ200kb  
	int  len;                               //curlʵ�ʷ������ݴ�С
	
}CurlResponse;

/*
	���͵Ĳ���
*/
typedef struct st_curl_params {
	char *serverUrl;						//url·��  
	int  port;                              //curl����ip�Ķ˿� 
	int  connect_timeout;                   //���ӳ�ʱ  
	int  timeout;                           //�����ʱ��
	char * cookie;						    //cookie
	//�Ƿ�ʹ�ô���
	char * proxyUrl;						//�����ַ	 ��ʽ��"127.0.0.1:8888"
	char * proxyNamePass;						//�û������� ��ʽ��"user:password"
	CurlHeadList *curlHeadP;				//httpͷ����Ϣ

}CurlParams;


/* ��������CURL��ʼ֮ǰ��ȫ�ֱ�����ʼ�����������߳���*/
LJHTTP_API int  CurlGlobalInitResource(void);
/*��������CURL����֮ǰ��ȫ����Դ�ͷţ��������߳���*/
LJHTTP_API int  CurlGlobalCleanupResource();

/*
	POST ����
	��http���󣺶˿ڣ���ʱʱ��Ȳ���ΪĬ��ֵ
	����1��serverUrl�����ַ
	����2��curlHeadP����ͷ��
	����3��postMessage���͵���Ϣ
	����4��curl_recӦ������
	����5��errorMessage������Ϣ
	����ֵ��������
*/
LJHTTP_API bool HttpPostSimple( char *serverUrl,CurlHeadList* curlHeadP, const char* postMessage, CurlResponse* curl_rec, char * errorMessage);

/*
	POST ����
	���п����ò�����http����
	����1��CurlParams�������
	����2��postMessage���͵���Ϣ
	����3��curl_recӦ������
	����4��errorMessage������Ϣ
	����ֵ��������
*/
LJHTTP_API bool HttpPostAttach(CurlParams* curl_params, const char* postMessage, CurlResponse* curl_rec, char * errorMessage);

/*
	GET ����
	��http���󣺶˿ڣ���ʱʱ��Ȳ���ΪĬ��ֵ
	����1��serverUrl�����ַ
	����2��curlHeadP����ͷ��
	����3��curl_recӦ������
	����4��errorMessage������Ϣ
	����ֵ��������
*/
LJHTTP_API bool HttpGetSimple(char *serverUrl, CurlHeadList* curlHeadP, CurlResponse* curl_rec, char * errorMessage);

/*
	GET ����
	���п����ò�����http����
	����1��CurlParams�������
	����2��curl_recӦ������
	����3��errorMessage������Ϣ
	����ֵ��������
*/
LJHTTP_API bool HttpGetAttach(CurlParams* curl_params, CurlResponse* curl_rec, char * errorMessage);
/*
	�����ļ�
	���п����ò�����http����
	����1��CurlParams�������
	����2�������ļ���
	����3��errorMessage������Ϣ
	����ֵ��������
*/
LJHTTP_API bool HttpDownloadFile(CurlParams* curl_params, const char* filename, void * ProgressCallback, char * errorMessage);

/*
	�ϴ��ļ�
	���п����ò�����http����
	����1��CurlParams�������
	����2���ϴ����ļ���
	����3��curl_rec ��������,�����ʼ��,�ڲ���ʼ��
	����4��errorMessage������Ϣ
	����ֵ��������
*/
LJHTTP_API bool HttpUploadFile(CurlParams* curl_params, const char* file_fullname, CurlResponse* curl_rec, char * errorMessage);
