#pragma once

#include "utils/FileUtils.h"
#include "utils/thread.h"

#include "UABEWapper/UnityError.h"
#include "utils/logging.h"
#include "UABEWapper/AssetBundleInfo.h"
#include "UABEWapper/ClassDatabaseManager.h"
#include "lodepng/lodepng.h"
#include "UABEWapper/AssetExporter.h"
#include "UABEWapper/AssetsFile.h"
#include "UABEWapper/AssetBundle.h"
#include "UABEWapper/AssetBundle5.h"
#include "UABEWapper/AssetBundleFactory.h"
#include "UABEWapper/AssetsBundleTools.h"



/**
 * �����־�����ʵ����
 */
CLogService * GetLogService();

/**
 * ��ȡ�����ݿ������
 */
CClassDatabaseManager * GetClassDatabaseManager();

/**
 * �Ƿ���
 */
bool IsBigEndian();