
// CircleMaker.h: PROJECT_NAME 애플리케이션에 대한 주 헤더 파일입니다.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH에 대해 이 파일을 포함하기 전에 'pch.h'를 포함합니다."
#endif

#include "resource.h"		// 주 기호입니다.
#include <gdiplus.h>

// CCircleMakerApp:
// 이 클래스의 구현에 대해서는 CircleMaker.cpp을(를) 참조하세요.
//

class CCircleMakerApp : public CWinApp
{
public:
	CCircleMakerApp();

// 재정의입니다.
public:
	BOOL InitInstance() final;
	int ExitInstance() final;

// 구현입니다.

	DECLARE_MESSAGE_MAP()

private:
	ULONG_PTR m_gdiplusToken;
};

extern CCircleMakerApp theApp;
