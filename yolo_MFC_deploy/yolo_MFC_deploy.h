
// yolo_MFC_deploy.h: PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"		// 主符号


// CyoloMFCdeployApp:
// 有关此类的实现，请参阅 yolo_MFC_deploy.cpp
//

class CyoloMFCdeployApp : public CWinApp
{
public:
	CyoloMFCdeployApp();

// 重写
public:
	virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CyoloMFCdeployApp theApp;
