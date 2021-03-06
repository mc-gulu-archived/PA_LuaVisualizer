
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include "output.h"

int nMaxCacheNode = 2;
int nCacheNode = 0;
lprof_PREVNODE sPrevNode = { 0,0 };

//double dTotalWriteConsuming = 0.0;

lprofP_OUTPUT pOutputHead = NULL;
lprofP_OUTPUT pOutputTail = NULL;

#ifdef __cplusplus
extern "C" {
#endif
	//void UnitySendMessage(const char* obj, const char* method, const char* msg);
#ifdef __cplusplus
}
#endif

//pOutputCallback = NULL;

void sendUnityMessage(const char* pMsg)
{
#ifdef __cplusplus
	if (pUnityMethod && pUnityObject)
		UnitySendMessage(pUnityObject, pUnityMethod, pMsg);
#endif
	if (pOutputCallback)
		pOutputCallback(pMsg);
}

void output(const char *format, ...) {
	//LARGE_INTEGER timestart;
	//lprofC_start_timer2(&timestart);
	va_list ap;
	va_start(ap, format);
	vfprintf(outf, format, ap);
	va_end(ap);

	fflush(outf);
	//dTotalWriteConsuming += lprofC_get_seconds2(&timestart);
}

void lprofP_addFrame(int id, char* str)
{
	lprof_NODE* pNode = (lprof_NODE*)malloc(sizeof(lprof_NODE));
	if(pNode)
	{
		memset(pNode, 0x0, sizeof(lprof_NODE));
		pNode->id = id;
		pNode->frame = str;
		pNode->data = NULL;
		pNode->next = NULL;
		if (pOutputTail)
		{
			pOutputTail->next = pNode;
			pOutputTail = pNode;
		}	
		else
		{
			pOutputTail = pNode;
		}
		if (pOutputHead == NULL)
			pOutputHead = pOutputTail;
		nCacheNode++;
		if (nCacheNode >= nMaxCacheNode)
			lprofP_output();
	}
	
}

void lprofP_addData(char* str)
{
	if (pOutputTail)
	{
		if (pOutputTail->data)
		{
			int len = strlen(pOutputTail->data) + strlen(str) + 2;
			char* psz = (char*)malloc(len);
			if(psz)
			{
				memset(psz, 0x0, len);
				strcpy(psz, pOutputTail->data);
				strcat(psz, ",");
				strcat(psz, str);
				free(str);
				free(pOutputTail->data);
				pOutputTail->data = psz;
			}
			
		}
		else
		{
			pOutputTail->data = str;
		}
		
	}
}

void lprofP_output()
{
	if (pOutputHead != NULL)
	{
		lprofP_OUTPUT pOut = pOutputHead;
		if (pOut->data)
		{
			int nLen = strlen(pOut->frame) + 2;
			char* psz = (char*)malloc(nLen);
			if(psz)
			{
				memset(psz, 0x0, nLen);
				strcpy(psz, pOut->frame);
				strcat(psz, ",");
				if (pOutputCallback)
				{
					pOutputCallback(psz);
				}
				output(psz);
				free(psz);
			}
			nLen = strlen(pOut->data) + 2;
			psz = (char*)malloc(nLen);
			if(psz)
			{
				memset(psz, 0x0, nLen);
				strcpy(psz, pOut->data);
				strcat(psz, ",");
				sendUnityMessage(psz);
				output(psz);
				free(psz);
			}
			sPrevNode.id = pOut->id;
			sPrevNode.data = 1;
			
		}
		else
		{
			if (sPrevNode.id != pOut->id && sPrevNode.data != 0)
			{
				int nLen = strlen(pOut->frame) + 2;
				char* psz = (char*)malloc(nLen);
				if(psz)
				{
					memset(psz, 0x0, nLen);
					strcpy(psz, pOut->frame);
					strcat(psz, ",");
					sendUnityMessage(psz);
					output(psz);
					free(psz);
					sPrevNode.id = pOut->id;
					sPrevNode.data = 0;
				}
			}
		}
		pOutputHead = pOut->next;
		if (pOutputTail == pOut)
			pOutputTail = pOutputHead;
		free(pOut);
		nCacheNode--;
	}
}

void lprofP_close()
{
	lprofP_OUTPUT pCurrent = NULL;
	sPrevNode.id = 0;
	sPrevNode.data = 0;
	pCurrent = pOutputHead;
	while (pCurrent)
	{
		lprofP_OUTPUT pNext = pCurrent->next;
		free(pCurrent);
		pCurrent = pNext;
	}
	pOutputHead = NULL;
	pOutputTail = NULL;
	//dTotalWriteConsuming = 0.0;
	nCacheNode = 0;
}