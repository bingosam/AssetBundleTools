#include "StdAfx.h"

#include "UnityError.h"

/**
 * 获取Unity错误信息
 *
 * @param iUnityError Unity错误码
 @ @return 错误文本信息
 */
const char *GetUnityErrorMessage(int iUnityError)
{
	switch (iUnityError)
	{
	case SUCCESSED: return "Successed.";
	case FILE_OPEN_FAILED: return "Unable to open file!";
	case FILE_WRITE_FAILED: return "Failed to write file!";
	case ASSETS_BUNDLE_FILE_READ_FAILED: return "Unable to read the assets bundle file!";
	case ASSETS_BUNDLE_FILE_UNPACK_FAILED: return "Failed to unpack the assets bundle file!";
	case ASSETS_BUNDLE_FILE_VERSION_UNSUPPORTED: return "The version of the assets bundle file is unsupported!";
	case ASSETS_BUNDLE_FILE_NOT_COMPRESSED: return "The assets bundle file is not compressed!";
	case MAKE_ASSETS_FILE_READER_FAILED: return "Unable to make an AssetsFileReader!";
	case ASSETS_DATA_READ_FAILED: return "Failed to read the assets data!";
	case INVALID_ASSETS_FILE: return "It's not a valid assets file!";
	case ASSETS_FILE_NOT_FOUND: return "Assets file not found!";
	case ASSET_NOT_FOUND: return "Asset not found!";
	case PICTURE_ASSET_READ_FAILED: return "Failed to read picture asset!";
	case PICTURE_EXPORT_FAILED: return "Unable to export picture asset!";
	default: return "Unknown Unnity Error!";
	}
}
