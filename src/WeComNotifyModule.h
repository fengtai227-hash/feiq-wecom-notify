#pragma once

// WeComNotifyModule.h : header file

#include "..\include\FQEventSinkInc.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CWeComConfigDlg -- configuration dialog
class CWeComConfigDlg : public CDialog
{
public:
	CString m_strWebhookUrl;
	CString m_strMentionIds;

	CWeComConfigDlg(CWnd* pParent = NULL) : CDialog(IDD_WECOM_CONFIG, pParent)
	{
		m_strWebhookUrl = _T("");
		m_strMentionIds = _T("");
	}

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
};

/////////////////////////////////////////////////////////////////////////////
// CWeComNotifyModule command target

class CWeComNotifyModule : public CCmdTarget
{
	DECLARE_DYNCREATE(CWeComNotifyModule)
	DECLARE_EVENT_RECEIVER(CWeComNotifyModule)

	CWeComNotifyModule();           // protected constructor used by dynamic creation
	~CWeComNotifyModule();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWeComNotifyModule)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
protected:
	BEGIN_DUAL_INTERFACE_PART(FQModule, IFQModule)
	STDMETHOD(raw_OnModuleEvent)(/*[in]*/ BSTR eventType, /*[in]*/ BSTR Name, /*[in]*/ BSTR sParam, /*[out, retval]*/ BSTR* pVal);
	STDMETHOD(raw_OnLoad)(/*[in]*/ IDispatch* FQModuleSite);
	STDMETHOD(raw_OnUnload)();
	STDMETHOD(raw_OnConfig)();
	STDMETHOD(get_Identifier)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Name)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Description)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_Author)(/*[out, retval]*/ BSTR *pVal);
	STDMETHOD(get_ModuleSite)(/*[out, retval]*/ IDispatch* *pVal)
	{
		METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

		FQ_CHECK_INVALIDARG_NULL(pVal);

		*pVal = pThis->m_ptrModuleSite;

		if (*pVal != NULL)
		{
			(*pVal)->AddRef();
		}

		return *pVal != NULL ? S_OK : E_FAIL;
	}
	END_DUAL_INTERFACE_PART(FQModule)

	BEGIN_DUAL_INTERFACE_PART(MenuSink, IFQUICommand)
    STDMETHOD(raw_OnInvoke)( enum FQ_UI_TYPE Type, long Id, VARIANT Parameter );
    STDMETHOD(raw_OnQueryState) (enum FQ_UI_TYPE Type, long Id, VARIANT Parameter,
                                 BSTR * bstrText,
                                 enum FQ_UI_ITEM_STATE * State );
	END_DUAL_INTERFACE_PART(MenuSink)

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(CWeComNotifyModule)

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(CWeComNotifyModule)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()

	// Embedded member variables (must be after DECLARE macros)
	IFQModuleSitePtr m_ptrModuleSite;
	IFQRootPtr m_ptrRoot;
	CFQEventSinkRoot m_RootEventSink;

	IFQMenuPtr m_ptrMenu;
	long m_AddMenuID;

	// Plugin state
	BOOL m_bNotifyEnabled;
	CString m_strWebhookUrl;
	CString m_strMentionIds;

public:
	HRESULT OnModuleEvent(BSTR eventType, BSTR Name, BSTR sParam, BSTR* pVal);
	HRESULT OnLoad(IDispatch* FQModuleSite);
	HRESULT OnUnload();
	HRESULT OnConfig();
	HRESULT get_Identifier(BSTR* pVal);
	HRESULT get_Name(BSTR* pVal);
	HRESULT get_Description(BSTR* pVal);
	HRESULT get_Author(BSTR* pVal);

	HRESULT MenuOnInvoke(enum FQ_UI_TYPE Type, long Id, VARIANT Parameter);
	HRESULT MenuOnQueryState(enum FQ_UI_TYPE Type, long Id, VARIANT Parameter,
		BSTR* bstrText, enum FQ_UI_ITEM_STATE* State);

	HRESULT OnBeforeRecvMsg(LPCTSTR Memo, LPCTSTR Host, LPCTSTR Group, LPCTSTR IP, LPCTSTR MAC,
		LPCTSTR sendMsg, LPCTSTR font, enum FQ_BEFORERECVMSG_RESULT* pResult);
	HRESULT OnRootEvent(LPCTSTR eventType, LPCTSTR Name, LPCTSTR sParam, IFQData* pData);

	// Helper — detect binary (image/file) content vs text
	static BOOL IsBinaryContent(LPCTSTR szContent, int nSampleLen);
	// Send a notification string to WeCom (handles JSON + webhook)
	void SendWeComNotification(LPCSTR pszSenderInfo, LPCTSTR szContent);
};

/////////////////////////////////////////////////////////////////////////////
