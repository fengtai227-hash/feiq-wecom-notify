// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__5C3E5D8F_9A2B_4C7E_B1D3_E8F6A2C4D9B7__INCLUDED_)
#define AFX_STDAFX_H__5C3E5D8F_9A2B_4C7E_B1D3_E8F6A2C4D9B7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Insert your headers here
#define _CRT_SECURE_NO_WARNINGS  // suppress fopen/strcpy deprecation
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include <afxctl.h>         // MFC support for ActiveX Controls
#include <afxmt.h>          // MFC multithreading support
#include <afxdisp.h>        // MFC automation support
#include <atlconv.h>        // ATL string conversion (CA2W, USES_CONVERSION)
#include <winhttp.h>        // WinHTTP for HTTPS requests

#pragma comment(lib, "winhttp.lib")

#include "resource.h"       // main symbols

// 引入飞秋COM类型库 (生成智能指针包装类 IFQ*Ptr)
#import "..\tlb\FeiQ.tlb" no_namespace named_guids

// FeiQ SDK headers
#include "..\include\mfcdual.h"         // DELEGATE_DUAL_INTERFACE macro
#include "..\include\EventSupport.h"
#include "..\include\FQDataKeys.h"
#include "..\include\FQEventSink.h"
#include "..\include\FQEventSinkInc.h"
#include "..\include\FQMisc.h"

// FeiQ Plugin Identifiers (used by plugin metadata methods)
// NOTE: Use L"..." wide literals to avoid UTF-8 source vs MBCS runtime encoding mismatch
#define FQ_MODULE_IDENTIFIER_PLUGINFEEDBACK		L"FeiQ.WeComNotify"
#define FQ_MODULE_NAME_PLUGINFEEDBACK			L"企业微信消息转发"
#define FQ_MODULE_DESCRIPTION_PLUGINFEEDBACK	L"离开电脑时自动转发飞秋消息到企业微信"
#define FQ_MODULE_AUTHOR_PLUGINFEEDBACK			L"Aria"

// Note: FQ_MODLE_CLSIDS is defined in WeComNotify.h and must match IMPLEMENT_OLECREATE GUID

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5C3E5D8F_9A2B_4C7E_B1D3_E8F6A2C4D9B7__INCLUDED_)
