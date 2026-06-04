// WeComNotifyModule.cpp : implementation file
//

#include "stdafx.h"
#include "WeComNotifyModule.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "..\include\FQEventSinkImp.h"

/////////////////////////////////////////////////////////////////////////////
// CWeComNotifyModule

IMPLEMENT_DYNCREATE(CWeComNotifyModule, CCmdTarget)
DELEGATE_DUAL_INTERFACE(CWeComNotifyModule, FQModule)
DELEGATE_DUAL_INTERFACE(CWeComNotifyModule, MenuSink)

CWeComNotifyModule::CWeComNotifyModule()
{
	m_ptrMenu = NULL;
	m_ptrModuleSite = NULL;
	m_ptrRoot = NULL;
	m_AddMenuID = 0;
	m_bNotifyEnabled = FALSE;
	m_strWebhookUrl = _T("");
	m_strMentionIds = _T("");

	EnableAutomation();
	AfxOleLockApp();
}

CWeComNotifyModule::~CWeComNotifyModule()
{
	AfxOleUnlockApp();
}

void CWeComNotifyModule::OnFinalRelease()
{
	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(CWeComNotifyModule, CCmdTarget)
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(CWeComNotifyModule, CCmdTarget)
END_DISPATCH_MAP()

static const IID IID_IWeComNotifyModule =
{ 0xF1E2D3C4, 0xB5A6, 0x9780, { 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0 } };

BEGIN_INTERFACE_MAP(CWeComNotifyModule, CCmdTarget)
	INTERFACE_PART(CWeComNotifyModule, IID_IWeComNotifyModule, Dispatch)
	INTERFACE_PART(CWeComNotifyModule, IID_IFQModule, FQModule)
	INTERFACE_PART(CWeComNotifyModule, IID_IFQUICommand, MenuSink)
END_INTERFACE_MAP()

IMPLEMENT_OLECREATE(CWeComNotifyModule, "FeiQ.WeComNotifyModule",
	0x4B7E93A1, 0x5C2D, 0x4F8E, 0xB1, 0xD6, 0xA8, 0xC3, 0xE5, 0xF9, 0xD2, 0xB7)

/////////////////////////////////////////////////////////////////////////////
// FQModule interface handlers

STDMETHODIMP CWeComNotifyModule::XFQModule::raw_OnModuleEvent(BSTR eventType, BSTR Name, BSTR sParam, BSTR* pVal)
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->OnModuleEvent(eventType, Name, sParam, pVal);
}

STDMETHODIMP CWeComNotifyModule::XFQModule::raw_OnLoad(IDispatch* FQModuleSite)
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->OnLoad(FQModuleSite);
}

STDMETHODIMP CWeComNotifyModule::XFQModule::raw_OnUnload()
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->OnUnload();
}

STDMETHODIMP CWeComNotifyModule::XFQModule::raw_OnConfig()
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->OnConfig();
}

STDMETHODIMP CWeComNotifyModule::XFQModule::get_Identifier(BSTR* pVal)
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->get_Identifier(pVal);
}

STDMETHODIMP CWeComNotifyModule::XFQModule::get_Name(BSTR* pVal)
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->get_Name(pVal);
}

STDMETHODIMP CWeComNotifyModule::XFQModule::get_Description(BSTR* pVal)
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->get_Description(pVal);
}

STDMETHODIMP CWeComNotifyModule::XFQModule::get_Author(BSTR* pVal)
{
	METHOD_PROLOGUE(CWeComNotifyModule, FQModule)

	return pThis->get_Author(pVal);
}

/////////////////////////////////////////////////////////////////////////////
// MenuSink interface handlers

STDMETHODIMP CWeComNotifyModule::XMenuSink::raw_OnInvoke(enum FQ_UI_TYPE Type, long Id, VARIANT Parameter)
{
	METHOD_PROLOGUE(CWeComNotifyModule, MenuSink)
	pThis->MenuOnInvoke(Type, Id, Parameter);
	return S_OK;
}

STDMETHODIMP CWeComNotifyModule::XMenuSink::raw_OnQueryState(enum FQ_UI_TYPE Type, long Id, VARIANT Parameter,
								 BSTR* bstrText,
								 enum FQ_UI_ITEM_STATE* State)
{
	METHOD_PROLOGUE(CWeComNotifyModule, MenuSink)

	if (Id == pThis->m_AddMenuID)
	{
		if (pThis->m_bNotifyEnabled)
		{
			*bstrText = SysAllocString(L"企业微信消息转发 [已开启]");
		}
		else
		{
			*bstrText = SysAllocString(L"企业微信消息转发 [已关闭]");
		}
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Plugin logic implementation

// Helper: convert GBK/ANSI (from FeiQ) to UTF-8 (for webhook JSON)
static CStringA ANSIToUTF8(LPCSTR pszAnsi)
{
	if (!pszAnsi || !pszAnsi[0])
		return "";

	int nWide = MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, NULL, 0);
	if (nWide <= 0)
		return "";

	CStringW strW;
	wchar_t* pW = strW.GetBuffer(nWide);
	MultiByteToWideChar(CP_ACP, 0, pszAnsi, -1, pW, nWide);
	strW.ReleaseBuffer();

	int nUTF8 = WideCharToMultiByte(CP_UTF8, 0, strW, -1, NULL, 0, NULL, NULL);
	if (nUTF8 <= 0)
		return "";

	CStringA strA;
	char* pA = strA.GetBuffer(nUTF8);
	WideCharToMultiByte(CP_UTF8, 0, strW, -1, pA, nUTF8, NULL, NULL);
	strA.ReleaseBuffer();

	return strA;
}

// Helper: send message to WeCom webhook via WinHTTP
static BOOL SendWeComWebhook(LPCSTR pszWebhookUrl, LPCSTR pszJsonPayload)
{
	if (!pszWebhookUrl || !pszWebhookUrl[0])
		return FALSE;

	BOOL bResult = FALSE;
	HINTERNET hSession = NULL;
	HINTERNET hConnect = NULL;
	HINTERNET hRequest = NULL;
	CStringA strHeaders;  // Declare before any goto targets

	// Parse URL to extract host and path
	CStringA strUrl(pszWebhookUrl);
	CStringA strHost, strPath;
	INTERNET_PORT nPort = INTERNET_DEFAULT_HTTPS_PORT;

	if (strUrl.Left(8).CompareNoCase("https://") == 0)
	{
		int nSlash = strUrl.Find('/', 8);
		if (nSlash > 0)
		{
			strHost = strUrl.Mid(8, nSlash - 8);
			strPath = strUrl.Mid(nSlash);
		}
		else
		{
			strHost = strUrl.Mid(8);
			strPath = "/";
		}
	}
	else
	{
		// Fallback for non-https URLs
		int nSlash = strUrl.Find('/');
		if (nSlash > 0)
		{
			strHost = strUrl.Left(nSlash);
			strPath = strUrl.Mid(nSlash);
		}
		else
		{
			strHost = strUrl;
			strPath = "/";
		}
	}

	// Check for port in host
	int nColon = strHost.Find(':');
	if (nColon > 0)
	{
		nPort = (INTERNET_PORT)atoi(strHost.Mid(nColon + 1));
		strHost = strHost.Left(nColon);
	}

	hSession = WinHttpOpen(L"FeiQ-WeComNotify/1.0",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	if (!hSession)
		goto cleanup;

	hConnect = WinHttpConnect(hSession, CStringW(strHost), nPort, 0);
	if (!hConnect)
		goto cleanup;

	hRequest = WinHttpOpenRequest(hConnect, L"POST", CStringW(strPath),
		NULL, WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_SECURE);

	if (!hRequest)
		goto cleanup;

	// Disable cert validation for corporate proxies (optional)
	DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
		SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
		SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
		SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
	WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));

	strHeaders = "Content-Type: application/json; charset=utf-8";
	bResult = WinHttpSendRequest(hRequest,
		CStringW(strHeaders), -1L,
		(LPVOID)pszJsonPayload, (DWORD)strlen(pszJsonPayload),
		(DWORD)strlen(pszJsonPayload), 0);

	if (bResult)
	{
		bResult = WinHttpReceiveResponse(hRequest, NULL);
	}

cleanup:
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	return bResult;
}

HRESULT CWeComNotifyModule::get_Identifier(BSTR* pVal)
{
	if (NULL == pVal)
	{
		ASSERT(FALSE);
		return E_INVALIDARG;
	}

	*pVal = SysAllocString(FQ_MODULE_IDENTIFIER_PLUGINFEEDBACK);

	return S_OK;
}

HRESULT CWeComNotifyModule::get_Name(BSTR* pVal)
{
	if (NULL == pVal)
	{
		ASSERT(FALSE);
		return E_INVALIDARG;
	}

	*pVal = SysAllocString(FQ_MODULE_NAME_PLUGINFEEDBACK);

	return S_OK;
}

HRESULT CWeComNotifyModule::get_Description(BSTR* pVal)
{
	if (NULL == pVal)
	{
		ASSERT(FALSE);
		return E_INVALIDARG;
	}

	*pVal = SysAllocString(FQ_MODULE_DESCRIPTION_PLUGINFEEDBACK);

	return S_OK;
}

HRESULT CWeComNotifyModule::get_Author(BSTR* pVal)
{
	if (NULL == pVal)
	{
		ASSERT(FALSE);
		return E_INVALIDARG;
	}

	*pVal = SysAllocString(FQ_MODULE_AUTHOR_PLUGINFEEDBACK);

	return S_OK;
}

HRESULT CWeComNotifyModule::OnLoad(IDispatch* FQModuleSite)
{
	if (NULL == FQModuleSite)
	{
		ASSERT(FALSE);
		return E_INVALIDARG;
	}

	FQ_TRY
	{
		m_ptrModuleSite = FQModuleSite;
		m_ptrModuleSite->get_FQRoot(&m_ptrRoot);

		// ===== Register tray context menu item =====
		CComBSTR identeriferMenu("FeiQ.Menu");
		m_ptrMenu = m_ptrRoot->Module[(_bstr_t)identeriferMenu];
		IFQUICommand* pUICmd = dynamic_cast<IFQUICommand*>(&m_xMenuSink);
		_bstr_t bstrParent(L"");
		_bstr_t bstrMenuText(L"企业微信消息转发 [已关闭]");
		m_AddMenuID = m_ptrMenu->AddItem(FQ_UI_TYPE_TRAY_MENU, bstrParent, pUICmd, bstrMenuText, 100);

		// ===== Hook events =====
		m_RootEventSink.HookEvent(evt_OnBeforeRecvMsg, this, &CWeComNotifyModule::OnBeforeRecvMsg);
		m_RootEventSink.HookEvent(evt_OnRootEvent, this, &CWeComNotifyModule::OnRootEvent);

		if (!m_RootEventSink.Advise(m_ptrRoot))
		{
			throw FQ_UNSPECIFIC_ERROR;
		}

		// ===== Load configuration =====
		IFQDataPtr cfgPtr;
		m_ptrRoot->get_UserCustomConfig(&cfgPtr);
		if (cfgPtr)
		{
			m_bNotifyEnabled = cfgPtr->GetLong(L"WECOM_NOTIFY_ENABLED");

			long nUrlLen = cfgPtr->GetBufferLength(L"WECOM_WEBHOOK_URL");
			if (nUrlLen > 0)
			{
				CStringA strUrl;
				char* pUrl = strUrl.GetBuffer(nUrlLen);
				cfgPtr->GetBuffer(L"WECOM_WEBHOOK_URL", (unsigned char*)pUrl, nUrlLen);
				strUrl.ReleaseBuffer();
				m_strWebhookUrl = CString(strUrl);
			}

			long nMentionLen = cfgPtr->GetBufferLength(L"WECOM_MENTION_IDS");
			if (nMentionLen > 0)
			{
				CStringA strMention;
				char* pMention = strMention.GetBuffer(nMentionLen);
				cfgPtr->GetBuffer(L"WECOM_MENTION_IDS", (unsigned char*)pMention, nMentionLen);
				strMention.ReleaseBuffer();
				m_strMentionIds = CString(strMention);
			}
		}

		return S_OK;
	}
	FQ_CATCH_ALL(return E_FAIL)
}

HRESULT CWeComNotifyModule::OnUnload()
{
	// Remove menu items
	IFQUICommand* pUICmd = dynamic_cast<IFQUICommand*>(&m_xMenuSink);
	if (m_ptrMenu && m_AddMenuID != 0)
	{
		m_ptrMenu->DelItem(FQ_UI_TYPE_TRAY_MENU, m_AddMenuID, pUICmd);
	}

	// Unhook events
	m_RootEventSink.Unadvise();
	m_RootEventSink.UnhookEvent(evt_OnBeforeRecvMsg, this, &CWeComNotifyModule::OnBeforeRecvMsg);
	m_RootEventSink.UnhookEvent(evt_OnRootEvent, this, &CWeComNotifyModule::OnRootEvent);

	return S_OK;
}

HRESULT CWeComNotifyModule::OnConfig()
{
	// ===== Show configuration dialog =====
	// Switch resource handle to our DLL so dialog template can be found
	HINSTANCE hOldResource = AfxGetResourceHandle();
	HMODULE hOurDll = ::GetModuleHandle(_T("WeComNotify.dll"));
	if (hOurDll)
		AfxSetResourceHandle(hOurDll);

	CWeComConfigDlg dlg;

	// Load current values
	dlg.m_strWebhookUrl = m_strWebhookUrl;
	dlg.m_strMentionIds = m_strMentionIds;

	if (dlg.DoModal() == IDOK)
	{
		// Save to member variables
		m_strWebhookUrl = dlg.m_strWebhookUrl;
		m_strMentionIds = dlg.m_strMentionIds;

		// Persist to FeiQ config
		if (m_ptrRoot)
		{
			IFQDataPtr cfgPtr;
			m_ptrRoot->get_UserCustomConfig(&cfgPtr);
			if (cfgPtr)
			{
				CStringA strUrl(m_strWebhookUrl);
				cfgPtr->SetBuffer(L"WECOM_WEBHOOK_URL", (unsigned char*)(LPCSTR)strUrl, strUrl.GetLength());

				CStringA strMention(m_strMentionIds);
				cfgPtr->SetBuffer(L"WECOM_MENTION_IDS", (unsigned char*)(LPCSTR)strMention, strMention.GetLength());
			}
		}
	}

	// Restore original resource handle
	AfxSetResourceHandle(hOldResource);

	return S_OK;
}

HRESULT CWeComNotifyModule::OnModuleEvent(BSTR eventType, BSTR Name, BSTR sParam, BSTR* pVal)
{
	*pVal = SysAllocString(L"");
	return S_OK;
}

HRESULT CWeComNotifyModule::MenuOnInvoke(enum FQ_UI_TYPE Type, long Id, VARIANT Parameter)
{
	if (Id == m_AddMenuID)
	{
		m_bNotifyEnabled = !m_bNotifyEnabled;

		// Save state to config
		if (m_ptrRoot)
		{
			IFQDataPtr cfgPtr;
			m_ptrRoot->get_UserCustomConfig(&cfgPtr);
			if (cfgPtr)
			{
				cfgPtr->SetLong(L"WECOM_NOTIFY_ENABLED", m_bNotifyEnabled);
			}
		}
	}
	return S_OK;
}

/////////////////////////////////////////////////////////////////////////////
// Helper: detect binary (image/file) content vs readable text
// Returns TRUE if content appears to be binary (image data, file bytes, etc.)
//
// WARNING: strlen(szContent) stops at first 0x00 byte — we CANNOT detect
// embedded nulls in binary data by scanning beyond strlen. The string
// terminator '\0' itself must NOT be treated as an embedded null.
//
// Detection strategy (ordered by priority):
//   P0: FeiQ binary protocol pattern detection — screenshots and file
//       transfers use '~' frame delimiters and '/' protocol prefixes that
//       are NEVER the first char of human text in a chat message.
//   P1: Check well-known image/video file headers (JPEG, PNG, GIF, BMP, RIFF, ICO)
//   P2: Extreme length (> 10KB) → almost certainly binary (image/file payload)
//   P3: Text readability fallback — if no natural language indicators
//       (letters, CJK, spaces) in first 64 bytes → encoded/binary data
//   P4: Non-printable character ratio analysis, with proper GBK pair skipping.
//       If >30% of bytes are non-printable → binary.
BOOL CWeComNotifyModule::IsBinaryContent(LPCTSTR szContent, int nSampleLen)
{
	if (!szContent || !szContent[0])
		return FALSE;

	int nLen = (int)strlen(szContent);
	if (nLen <= 0)
		return FALSE;

	unsigned char* p = (unsigned char*)szContent;

	// ---- P0: FeiQ binary protocol pattern detection ----
	// FeiQ encodes screenshots and file data using ASCII framing characters
	// (like '~' and '/') within the message body. These patterns are NEVER
	// the first character of a natural-language chat message.
	// Examples seen in practice: "/~#>hexdata<B~", "/-#>token<B~"
	if (nLen >= 1)
	{
		// '~' is FeiQ's universal frame delimiter
		if (p[0] == '~')
			return TRUE;

		// '/' prefix + protocol sub-marker → FeiQ binary data
		// ('/~', '/-', '/#' are FeiQ protocol, not text)
		if (p[0] == '/' && nLen >= 2)
		{
			if (p[1] == '~' || p[1] == '-' || p[1] == '#')
				return TRUE;
		}
	}

	// ---- P1: Check well-known image/video file headers ----
	// JPEG: FF D8 FF
	if (p[0] == 0xFF && p[1] == 0xD8)
		return TRUE;

	// PNG: 89 50 4E 47
	if (p[0] == 0x89 && p[1] == 'P' && p[2] == 'N' && p[3] == 'G')
		return TRUE;

	// GIF: GIF8 (GIF89a or GIF87a)
	if (p[0] == 'G' && p[1] == 'I' && p[2] == 'F' && p[3] == '8')
		return TRUE;

	// BMP: BM (note: "BM" could be rare text, but in FeiQ chat context it's
	// far more likely to be a bitmap than someone typing "BM" as first chars)
	if (p[0] == 'B' && p[1] == 'M')
		return TRUE;

	// RIFF container (WEBP, AVI, etc.)
	if (p[0] == 'R' && p[1] == 'I' && p[2] == 'F' && p[3] == 'F')
		return TRUE;

	// ICO icon: 00 00 01 00
	if (nLen >= 4 && p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x01 && p[3] == 0x00)
		return TRUE;

	// ---- P2: Extreme length (> 10KB) → almost certainly image/file payload ----
	if (nLen > 10240)
		return TRUE;

	// ---- P3: Text readability fallback ----
	// FeiQ encodes some binary data as ALL printable ASCII (e.g., base64-like
	// encoding of screenshots), which passes P1 and won't trigger P4.
	// We check if the content lacks ANY natural language indicators.
	{
		int nReadCheck = min(nLen, 64);
		int nWordChars = 0;    // ASCII letters (potential words)
		int nCJKChars = 0;     // GBK Chinese characters
		int nSpaces = 0;

		for (int i = 0; i < nReadCheck; i++)
		{
			unsigned char c = p[i];
			if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
				nWordChars++;
			else if (c == ' ' || c == '\t')
				nSpaces++;
			else if (c >= 0x81 && c <= 0xFE && i + 1 < nReadCheck)
			{
				unsigned char c2 = p[i + 1];
				if (c2 >= 0x40 && c2 <= 0xFE)
				{
					nCJKChars++;
					i++;  // skip trail byte
					continue;
				}
			}
		}

		// Natural text ALWAYS has at least one of:
		//   - ASCII letters forming words
		//   - CJK characters
		//   - Spaces between words
		// Encoded binary data (hex strings, base64, protocol markers)
		// has NONE of these — just digits + special chars.
		if (nWordChars == 0 && nCJKChars == 0 && nSpaces == 0 && nReadCheck >= 4)
			return TRUE;
	}

	// ---- P4: Non-printable character ratio analysis ----
	// Skip valid GBK double-byte sequences to avoid false-flagging Chinese text.
	// GBK encoding: lead byte 0x81-0xFE, trail byte 0x40-0xFE.
	// Bytes 0x80 and 0xFF are invalid in GBK — treat them as suspicious.
	int nCheck = min(nLen, nSampleLen);
	int nNonPrintable = 0;
	int nTotalChecked = 0;

	for (int i = 0; i < nCheck; )
	{
		unsigned char c = p[i];

		// Valid GBK lead byte followed by valid trail byte → skip pair entirely
		if (c >= 0x81 && c <= 0xFE && i + 1 < nCheck)
		{
			unsigned char c2 = p[i + 1];
			if (c2 >= 0x40 && c2 <= 0xFE)
			{
				// Valid GBK pair — Chinese character or full-width symbol
				i += 2;
				nTotalChecked += 2;
				continue;
			}
			else
			{
				// Lead byte with invalid trail → suspicious
				nNonPrintable++;
				i += 2;
				nTotalChecked += 2;
				continue;
			}
		}

		// 0x80 or 0xFF: invalid in any GBK position → suspicious
		if (c == 0x80 || c == 0xFF)
		{
			nNonPrintable++;
			i++;
			nTotalChecked++;
			continue;
		}

		// Control characters (except TAB, CR, LF) → suspicious
		if (c < 0x20 && c != '\t' && c != '\r' && c != '\n')
		{
			nNonPrintable++;
		}
		i++;
		nTotalChecked++;
	}

	if (nTotalChecked > 0 && (nNonPrintable * 100 / nTotalChecked) > 30)
		return TRUE;

	return FALSE;
}

// Helper: send a notification text to WeCom webhook
// Builds the JSON payload with sender info and calls the webhook
void CWeComNotifyModule::SendWeComNotification(LPCSTR pszSenderInfo, LPCTSTR szContent)
{
	if (m_strWebhookUrl.IsEmpty())
		return;

	// Build sender info header from current message context
	CStringA strMemo = ANSIToUTF8(CStringA(pszSenderInfo ? pszSenderInfo : ""));

	// Convert wide notification text to UTF-8
	CStringA strContent;
	if (szContent && szContent[0])
	{
		strContent = ANSIToUTF8(CStringA(szContent));
	}
	else
	{
		strContent = ANSIToUTF8(CStringA(""));
	}

	// Escape JSON
	strContent.Replace("\\", "\\\\");
	strContent.Replace("\"", "\\\"");
	strContent.Replace("\r", "\\r");
	strContent.Replace("\n", "\\n");
	strContent.Replace("\t", "\\t");

	// Build @mentions JSON
	CStringA strMentionList;
	if (!m_strMentionIds.IsEmpty())
	{
		CStringA strIds(m_strMentionIds);
		CStringA strList;
		int nStart = 0;
		while (nStart < strIds.GetLength())
		{
			while (nStart < strIds.GetLength() && (strIds[nStart] == ' ' || strIds[nStart] == '\t'))
				nStart++;
			if (nStart >= strIds.GetLength())
				break;

			int nEnd = strIds.Find(',', nStart);
			if (nEnd < 0)
				nEnd = strIds.GetLength();

			CStringA strId = strIds.Mid(nStart, nEnd - nStart);
			strId.Trim();
			if (!strId.IsEmpty())
			{
				if (!strList.IsEmpty())
					strList += ",";
				strList += "\"" + strId + "\"";
			}
			nStart = nEnd + 1;
		}
		if (!strList.IsEmpty())
			strMentionList.Format(",\"mentioned_list\":[%s]", (LPCSTR)strList);
	}

	CStringA strJson;
	strJson.Format(
		"{"
		"\"msgtype\":\"text\","
		"\"text\":{"
		"\"content\":\"%s\\n%s\""
		"%s"
		"}"
		"}",
		(LPCSTR)strMemo,
		(LPCSTR)strContent,
		(LPCSTR)strMentionList);

	SendWeComWebhook(CStringA(m_strWebhookUrl), strJson);
}

/////////////////////////////////////////////////////////////////////////////
HRESULT CWeComNotifyModule::OnBeforeRecvMsg(LPCTSTR Memo, LPCTSTR Host, LPCTSTR Group, LPCTSTR IP, LPCTSTR MAC,
	LPCTSTR sendMsg, LPCTSTR font, enum FQ_BEFORERECVMSG_RESULT* pResult)
{
	// Always show message in FeiQ (never block)
	*pResult = FQ_BEFORERECVMSG_RESULT_NORMAL;

	// Skip if forwarding is disabled or webhook not configured
	if (!m_bNotifyEnabled)
		return S_OK;

	if (m_strWebhookUrl.IsEmpty())
		return S_OK;

	// Skip empty messages
	if (!sendMsg || !sendMsg[0])
		return S_OK;

	// ===== Detect binary content (images, screenshots, files) =====
	BOOL bIsBinary = IsBinaryContent(sendMsg, 512);


	if (bIsBinary)
	{
		// It's an image or file — send a clean notification instead of raw binary data.
		// WeCom webhooks only support text messages (no inline images), so we just
		// notify WHO sent a screenshot/picture.
		CStringA strSender;
		strSender.Format("[飞秋] %s (%s / %s)", CStringA(Memo), CStringA(Host), CStringA(IP));

		const char* szType = "[Screenshot/Picture] Sent a screenshot or picture. Check FeiQ to view.";

		// If we can identify the exact image format from headers, be specific
		unsigned char* pHead = (unsigned char*)sendMsg;
		if (pHead[0] == 0xFF && pHead[1] == 0xD8)
			szType = "[Screenshot/JPEG] Sent a JPEG picture. Check FeiQ to view.";
		else if (pHead[0] == 0x89 && pHead[1] == 'P' && pHead[2] == 'N' && pHead[3] == 'G')
			szType = "[Screenshot/PNG] Sent a PNG picture. Check FeiQ to view.";
		else if (pHead[0] == 'G' && pHead[1] == 'I' && pHead[2] == 'F' && pHead[3] == '8')
			szType = "[Screenshot/GIF] Sent a GIF picture. Check FeiQ to view.";
		else if (pHead[0] == 'B' && pHead[1] == 'M')
			szType = "[Screenshot/BMP] Sent a BMP picture. Check FeiQ to view.";

		SendWeComNotification((LPCSTR)strSender, szType);
		return S_OK;
	}

	// ===== Normal text message =====
	CStringA strMemo = ANSIToUTF8(CStringA(Memo));
	CStringA strHost = ANSIToUTF8(CStringA(Host));
	CStringA strGroup = ANSIToUTF8(CStringA(Group));
	CStringA strIP = ANSIToUTF8(CStringA(IP));
	CStringA strMsg = ANSIToUTF8(CStringA(sendMsg));

	// Escape JSON special characters in message content
	strMsg.Replace("\\", "\\\\");
	strMsg.Replace("\"", "\\\"");
	strMsg.Replace("\r", "\\r");
	strMsg.Replace("\n", "\\n");
	strMsg.Replace("\t", "\\t");

	// Build WeCom text message JSON with optional @mentions
	CStringA strJson;
	CStringA strMentionList;

	if (!m_strMentionIds.IsEmpty())
	{
		// Build "mentioned_list":["id1","id2",...] from comma-separated IDs
		CStringA strIds(m_strMentionIds);
		CStringA strList;
		int nStart = 0;
		while (nStart < strIds.GetLength())
		{
			while (nStart < strIds.GetLength() && (strIds[nStart] == ' ' || strIds[nStart] == '\t'))
				nStart++;
			if (nStart >= strIds.GetLength())
				break;

			int nEnd = strIds.Find(',', nStart);
			if (nEnd < 0)
				nEnd = strIds.GetLength();

			CStringA strId = strIds.Mid(nStart, nEnd - nStart);
			strId.Trim();
			if (!strId.IsEmpty())
			{
				if (!strList.IsEmpty())
					strList += ",";
				strList += "\"" + strId + "\"";
			}
			nStart = nEnd + 1;
		}

		if (!strList.IsEmpty())
		{
			strMentionList.Format(",\"mentioned_list\":[%s]", (LPCSTR)strList);
		}
	}

	strJson.Format(
		"{"
		"\"msgtype\":\"text\","
		"\"text\":{"
		"\"content\":\"[%s] %s\\nIP: %s\\nHost: %s\\nGroup: %s\""
		"%s"
		"}"
		"}",
		(LPCSTR)strMemo,
		(LPCSTR)strMsg,
		(LPCSTR)strIP,
		(LPCSTR)strHost,
		(LPCSTR)strGroup,
		(LPCSTR)strMentionList);

	// Send asynchronously (don't block FeiQ message processing)
	SendWeComWebhook(CStringA(m_strWebhookUrl), strJson);

	return S_OK;
}

HRESULT CWeComNotifyModule::OnRootEvent(LPCTSTR eventType, LPCTSTR Name, LPCTSTR sParam, IFQData* pData)
{
	// Skip if forwarding is disabled or webhook not configured
	if (!m_bNotifyEnabled || m_strWebhookUrl.IsEmpty())
		return S_OK;

	if (!eventType || !eventType[0])
		return S_OK;

	// Catch file transfer and other system events
	CStringA strType(eventType);
	CStringA strName(Name ? Name : "");
	CStringA strParam(sParam ? sParam : "");

	// ---- Expanded file transfer event matching ----
	// Check both English and Chinese event type keywords
	BOOL bIsFileEvent = FALSE;

	// English keywords (case-insensitive substring search)
	if (strType.Find("File") >= 0 || strType.Find("file") >= 0 ||
		strType.Find("FILE") >= 0 ||
		strType.Find("Transfer") >= 0 || strType.Find("transfer") >= 0 ||
		strType.Find("SendData") >= 0 || strType.Find("Send") >= 0 ||
		strType.Find("RecvData") >= 0 || strType.Find("Recv") >= 0 ||
		strType.Find("Data") >= 0 || strType.Find("DATA") >= 0)
	{
		bIsFileEvent = TRUE;
	}

	// Chinese keywords (MBCS — eventType is in system code page)
	if (strType.Find("文件") >= 0 ||
		strType.Find("传输") >= 0 ||
		strType.Find("发送") >= 0 ||
		strType.Find("接收") >= 0 ||
		strType.Find("数据") >= 0 ||
		strType.Find("图片") >= 0 ||
		strType.Find("截图") >= 0)
	{
		bIsFileEvent = TRUE;
	}

	if (bIsFileEvent)
	{
		CStringA strSender;
		strSender.Format("[飞秋系统事件] %s / %s", (LPCSTR)strName, (LPCSTR)strType);

		// Send notification even if Name is empty (the eventType itself is informative)
		if (!strName.IsEmpty() || !strType.IsEmpty())
		{
			const char* szNotify = "[File Transfer / System Event] Received file or data transfer. Check FeiQ for details.";
			SendWeComNotification((LPCSTR)strSender, szNotify);
		}
	}

	return S_OK;
}/////////////////////////////////////////////////////////////////////////////
// CWeComConfigDlg -- configuration dialog implementation

void CWeComConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_WEBHOOK_URL, m_strWebhookUrl);
	DDX_Text(pDX, IDC_MENTION_IDS, m_strMentionIds);
}

BOOL CWeComConfigDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set placeholder hint text for Webhook URL
	if (m_strWebhookUrl.IsEmpty())
	{
		GetDlgItem(IDC_WEBHOOK_URL)->SetWindowText(
			_T("https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=..."));
	}

	// Set placeholder hint text for mention IDs
	if (m_strMentionIds.IsEmpty())
	{
		GetDlgItem(IDC_MENTION_IDS)->SetWindowText(
			_T("zhangsan,lisi"));
	}

	return TRUE;
}

void CWeComConfigDlg::OnOK()
{
	// Read data from controls
	UpdateData(TRUE);

	// Strip whitespace
	m_strWebhookUrl.Trim();
	m_strMentionIds.Trim();

	// Basic URL validation
	if (!m_strWebhookUrl.IsEmpty())
	{
		if (m_strWebhookUrl.Find(_T("https://qyapi.weixin.qq.com/cgi-bin/webhook/send?key=")) < 0)
		{
			AfxMessageBox(_T("Webhook URL 格式不正确，请粘贴完整的企业微信群机器人 Webhook 地址。"));
			return;
		}
	}

	CDialog::OnOK();
}
