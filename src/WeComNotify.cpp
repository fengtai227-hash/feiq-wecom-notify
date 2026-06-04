// WeComNotify.cpp : Defines the initialization routines for the DLL.
//
// 飞秋企业微信消息转发插件 - DLL主文件
//

#include "stdafx.h"
#include "WeComNotify.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWeComNotifyApp

BEGIN_MESSAGE_MAP(CWeComNotifyApp, CWinApp)
	//{{AFX_MSG_MAP(CWeComNotifyApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWeComNotifyApp construction

CWeComNotifyApp::CWeComNotifyApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWeComNotifyApp object

CWeComNotifyApp theApp;


/////////////////////////////////////////////////////////////////////////////
// CWeComNotifyApp initialization

BOOL CWeComNotifyApp::InitInstance()
{
	// ===== MFC DLL初始化 =====
	if (!CWinApp::InitInstance())
		return FALSE;

	// ===== 注册所有 OLE 类工厂 (飞秋插件必需) =====
	COleObjectFactory::RegisterAll();

	return TRUE;
}

int CWeComNotifyApp::ExitInstance()
{
	return CWinApp::ExitInstance();
}


/////////////////////////////////////////////////////////////////////////////
// DLL导出函数 - 飞秋插件系统必需

// 返回插件CLSID列表 (飞秋用于发现和加载插件)
extern "C" __declspec(dllexport) char* DllGetPluginClsids()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	static CString strCLSID = FQ_MODLE_CLSIDS;
	return strCLSID.GetBuffer(strCLSID.GetLength() + 1);
}


/////////////////////////////////////////////////////////////////////////////
// COM DLL注册/注销函数

STDAPI DllCanUnloadNow(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return AfxDllCanUnloadNow();
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	return AfxDllGetClassObject(rclsid, riid, ppv);
}

STDAPI DllRegisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!COleObjectFactoryEx::UpdateRegistryAll(TRUE))
		return SELFREG_E_CLASS;
	return S_OK;
}

STDAPI DllUnregisterServer(void)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	if (!COleObjectFactoryEx::UpdateRegistryAll(FALSE))
		return SELFREG_E_CLASS;
	return S_OK;
}
