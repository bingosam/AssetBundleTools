#pragma once

/**
 * �ļ������˻ص�����
 *
 * @param pFileName �ļ���
 * @return �Ƿ�����
 */
typedef bool (*FILE_NAME_FILTER)(const char *pFileName);

/**
 * �г��ļ����µ��ļ�
 *
 * @param pDirectoryPath �ļ���·��
 * @param bRecursion �Ƿ�ݹ�
 * @param pFileNamePattern �ļ�����ʽ
 * @param filter �ļ������˻ص�����
 */
set<string> ListDirectory(const char *pDirectoryPath, bool bRecursion = false, const char *pFileNamePattern = "*", FILE_NAME_FILTER filter = NULL);

/**
 * �����ļ����µ��ļ�
 *
 * @param pDirectoryPath �ļ���·��
 * @param recursion �Ƿ�ݹ�
 * @param pFileNamePattern �ļ�����ʽ
 * @param filter �ļ������˻ص�����
 */
void ClearDirectory(const char *pDirectoryPath, bool recursion = false, const char *pFileNamePattern = "*", FILE_NAME_FILTER filter = NULL);

/**
 * �����ļ���
 *
 * @param pPath ·��
 * @param isDirectory pPath�Ƿ�Ϊ�ļ���·������Ĭ��Ϊ��
 */
int CreateDirectoryRecursion(const char *pDirectoryPath, bool isDirectory = true);

/**
 * ��ȡ�ļ���С
 *
 * @param pFilePath �ļ�·��
 * @return �ļ���С
 */
size_t GetFileSize(const char * pFilePath);

/**
 * �����ļ�,��Ŀ���ļ�����Ŀ¼�������򴴽�
 *
 * @param pSrcFile �ļ�·��
 * @param pNewFile ���ļ�·��
 * @param bFailIfExists �������Ƿ񷵻�ʧ��, falseʱ����
 * @return �����Ƿ�ɹ�
 */
BOOL __CopyFile(const char * pSrcFile, const char * pNewFile, BOOL bFailIfExists);

/**
 * �����ļ���
 *
 * @param pSrcDir �ļ���·��
 * @param pNewDir ���ļ���·��
 * @param bRecursion �Ƿ�ݹ鿽��
 * @return �����Ƿ�ɹ�
 */
BOOL CopyDirectory(const char * pSrcDir, const char * pNewDir, BOOL bRecursion);