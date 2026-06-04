// WeComNotify.h : main header file for the WECOMNOTIFY DLL
//
// 飞秋企业微信消息转发插件 - DLL入口
//

#if !defined(AFX_WECOMNOTIFY_H__6D4F6E9A_0B3C_5D8F_A2C4_F9E7B6D5A3B1__INCLUDED_)
#define AFX_WECOMNOTIFY_H__6D4F6E9A_0B3C_5D8F_A2C4_F9E7B6D5A3B1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CWeComNotifyApp
class CWeComNotifyApp : public CWinApp
{
public:
	CWeComNotifyApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWeComNotifyApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CWeComNotifyApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

// 插件CLSID声明 (用于DllGetPluginClsids)
// 多个CLSID之间用 | 分隔
#define FQ_MODLE_CLSIDS  _T("{4B7E93A1-5C2D-4F8E-B1D6-A8C3E5F9D2B7}")

#endif // !defined(AFX_WECOMNOTIFY_H__6D4F6E9A_0B3C_5D8F_A2C4_F9E7B6D5A3B1__INCLUDED_)
