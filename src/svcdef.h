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
 * 获得日志服务的实例。
 */
CLogService * GetLogService();

/**
 * 获取类数据库管理器
 */
CClassDatabaseManager * GetClassDatabaseManager();

/**
 * 是否大端
 */
bool IsBigEndian();