/*
 * Copyright (C) 2015 Andrey Rychkov <wholegroup@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "stdafx.h"
#include "DlgMain.h"
#include "DlgOptions.h"

//////////////////////////////////////////////////////////////////////////
// Конструктор по-умолчанию
//
CMainDlg::CMainDlg()	: 
	m_hEnhMetafile(NULL),
	m_fDiffEMF(1),
	m_bHotkeyOn(FALSE),
	m_bTimerOn(FALSE),
	m_iDeltaY(DEFAULT_DELTAY)
{
	// открытие настроек
#ifdef _DEBUG
	OpenSettings(COMPANY_NAME, PROGRAM_NAME L" (debug)");
#else
	OpenSettings(COMPANY_NAME, PROGRAM_NAME);
#endif

	// Поиск всех возможных языков программы
	m_mapLanguages.clear();

	SearchLanguages();

	// установка языка программы
	m_wLanguageID = (WORD)GetSettingsInteger(L"m_wLanguageID", GetUserDefaultLangID());

	CMapWordStr::const_iterator item = m_mapLanguages.find(m_wLanguageID);
	if (m_mapLanguages.end() == item)
	{
		m_wLanguageID = DEFAULT_LANGUAGE;
	}

	// загрузка курсора руки
	m_hCursorHand = ::LoadCursor(NULL, IDC_HAND);
	ATLASSERT(NULL != m_hCursorHand);

	// прозрачность
	m_bTransparency = static_cast<BYTE>(GetSettingsInteger(L"m_bTransparency", DEFAULT_TRANSPARENCY));
	
	// hotkey
	m_dwHotkeyShow = static_cast<DWORD>(GetSettingsInteger(L"m_dwHotkeyShow", 0));

	// ширина
	m_fWidth = GetSettingsFloat(L"m_fWidth", DEFAULT_WIDTH);

	// режим показа окна (dual-wide или стандартный)
	m_bShowAlways = GetSettingsBoolean(L"m_bShowAlways", DEFAULT_SHOWALLWAYS);

	// частота таймера
	m_iTimerPeriod = GetSettingsInteger(L"m_iTimerPeriod", DEFAULT_TIMER);

	// дельта сдвига линейки колесом мыши
	m_iDeltaY = GetSettingsInteger(L"m_iDeltaY", DEFAULT_DELTAY);
}


//////////////////////////////////////////////////////////////////////////
// Деструктор по-умолчанию
//
CMainDlg::~CMainDlg()
{
	// уничтожаем переменные
	if (NULL != m_hEnhMetafile)
	{
		DeleteEnhMetaFile(m_hEnhMetafile);
	}
}


//////////////////////////////////////////////////////////////////////////
// Инициализация диалога
//
LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// установка иконки приложения
	HICON hIconSmall = reinterpret_cast<HICON>(::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDI_RULER), 
		IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR));

	SetIcon(hIconSmall, FALSE);

	// прозрачность окна
	ATLVERIFY(ModifyStyleEx(0, WS_EX_LAYERED));

	SetTransparency(m_bTransparency);

	// скрывает окно с панели задач
	ModifyStyleEx(0, WS_EX_TOOLWINDOW);

	// создание иконки в трее
	CreateNotifyIcon();
	SetNotifyIconTip(PROGRAM_NAME L" v" PROGRAM_VERSION);

	// регистрация hotkey
	SetHotkeyShow(m_dwHotkeyShow);

	// загрузка линейки
	ATLVERIFY(LoadEMF(L"ruler.emf"));

	// установка ширины окна
	ATLVERIFY(SetSize());

	// выбор режима отображения
	SetShowAlways(m_bShowAlways);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Уничтожение диалога
//
LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	// удаление таймера
	if (TRUE == m_bTimerOn)
	{
		ATLVERIFY(KillTimer(0));
		
		m_bTimerOn = FALSE;
	}

	// удаление хоткея
	if (m_bHotkeyOn)
	{
		ATLVERIFY(UnregisterHotKey(*this, 0));

		m_bHotkeyOn = FALSE;
	}

	// сохраняем настройки программы
	SetSettingsInteger(L"m_wLanguageID", m_wLanguageID);
	SetSettingsInteger(L"m_bTransparency", m_bTransparency);
	SetSettingsInteger(L"m_dwHotkeyShow", m_dwHotkeyShow);
	SetSettingsFloat(L"m_fWidth", m_fWidth);
	SetSettingsInteger(L"m_iTimerPeriod", m_iTimerPeriod);
	SetSettingsBoolean(L"m_bShowAlways", m_bShowAlways);
	SetSettingsInteger(L"m_iDeltaY", m_iDeltaY);

	CloseSettings(TRUE);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка IDOK
//
LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка IDCANCEL
//
LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Выход из приложения
//
LRESULT CMainDlg::OnQuit(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_CLOSE);

	return 0;
}



//////////////////////////////////////////////////////////////////////////
// Закрытие диалога
//
VOID CMainDlg::CloseDialog(int nVal)
{
	DestroyWindow();

	::PostQuitMessage(nVal);
}


//////////////////////////////////////////////////////////////////////////
// Поиск всех возможных языков программы
//
VOID CMainDlg::SearchLanguages()
{
	if (FALSE == ::EnumResourceLanguages(GetModuleHandle(NULL), RT_DIALOG, MAKEINTRESOURCE(IDD_OPTIONS), 
		(ENUMRESLANGPROC)CMainDlg::SearchLanguageCallback, (LONG_PTR)&m_mapLanguages))
	{
		MessageBox(L"Error::EnumResourceLanguages");
	}
	ATLASSERT(0 < m_mapLanguages.size());
}


BOOL CALLBACK CMainDlg::SearchLanguageCallback(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, 
	WORD wIDLanguage, LONG_PTR lParam)
{
	// неиспользуемые параметры
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpszType);
	UNREFERENCED_PARAMETER(lpszName);

	// проверка входных значений
	ATLASSERT(GetModuleHandle(NULL) == hModule);
	ATLASSERT(RT_DIALOG == lpszType);
	ATLASSERT(MAKEINTRESOURCE(IDD_OPTIONS) == lpszName);

	// получения указателя на массив языков
	CMapWordStr *pMap = (CMapWordStr*)lParam;

	// получение наименования языка по его идентификатору wIDLanguage
	std::vector<WCHAR> wcBuf(128);

	GetLocaleInfo(MAKELCID(wIDLanguage, 0), LOCALE_SNATIVELANGNAME, &wcBuf.front(), (INT)wcBuf.size());

	// перевод первой буквы в верхний регистр
	CharUpperBuff(&wcBuf.front(), 1);

	// добавление языка в массив
	pMap->insert(make_pair(wIDLanguage, &wcBuf.front()));

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Загружает строку по указанному идентификатору в соответствии с 
// установленным языком
//
std::wstring CMainDlg::GetStringLang(DWORD dwResID)
{
	LPCWSTR lpStr;

	lpStr = (LPCWSTR)GetResourceLang(RT_STRING, 1 + (dwResID >> 4));
	if (NULL == lpStr)
	{
		return L"";
	}

	for (DWORD dwStrPos = 0; dwStrPos < (dwResID & 0x000F); dwStrPos++)
	{
		lpStr += *lpStr + 1;
	}

	std::wstring wsStr = (lpStr + 1);

	wsStr.erase(*lpStr);

	return wsStr;
}


//////////////////////////////////////////////////////////////////////////
// Загружает указанный ресурс
//
HGLOBAL CMainDlg::GetResourceLang(LPWSTR lpResType, DWORD dwResID)
{
	HINSTANCE hAppInstance = GetModuleHandle(NULL);
	HRSRC     hrRes;

	hrRes = FindResourceEx(hAppInstance, lpResType, MAKEINTRESOURCE(dwResID), m_wLanguageID);
	if (NULL == hrRes)
	{
		hrRes = FindResourceEx(hAppInstance, lpResType, MAKEINTRESOURCE(dwResID), DEFAULT_LANGUAGE);
	}

	return LoadResource(hAppInstance, hrRes);
}


//////////////////////////////////////////////////////////////////////////
// Инициализация шаблона диалога
//
VOID CMainDlg::DoInitTemplate() 
{
	m_Template.Create((LPDLGTEMPLATE)GetResourceLang(RT_DIALOG, this->IDD));
}


//////////////////////////////////////////////////////////////////////////
// Инициализация контролов диалога
//
VOID CMainDlg::DoInitControls() 
{
	ATLASSERT(TRUE);
}


//////////////////////////////////////////////////////////////////////////
// Вывод контекстного меню
//
LRESULT CMainDlg::OnShowMenu(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	CPoint pt(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	
	OnNotifyIconRButtonUp(pt);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Установка курсора руки
//
LRESULT CMainDlg::OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	ATLVERIFY(NULL != ::SetCursor(m_hCursorHand));

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Для перетаскивания окна в любой точке
//
LRESULT CMainDlg::OnLeftButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	SendMessage(WM_NCLBUTTONDOWN, WPARAM(HTCAPTION), MAKELPARAM(0, 0));
	
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Вывод меню в трее по нажатию правой кнопки мыши
//
VOID CMainDlg::OnNotifyIconRButtonUp(const POINT& rpt)
{
	CMenu menuPopup;
	menuPopup.LoadMenuIndirect(GetResourceLang(RT_MENU, IDR_POPUPMENU));

	CMenuHandle menuVisible;
	menuVisible = menuPopup.GetSubMenu(0);
	
	if (TRUE == IsWindowVisible())
	{
		menuVisible.ModifyMenu(IDB_SHOWHIDE, MF_BYCOMMAND, static_cast<UINT_PTR>(IDB_SHOWHIDE), 
			GetStringLang(IDS_HIDE).c_str());
	}
	else
	{
		menuVisible.ModifyMenu(IDB_SHOWHIDE, MF_BYCOMMAND, static_cast<UINT_PTR>(IDB_SHOWHIDE), 
			GetStringLang(IDS_SHOW).c_str());
	}
	
	menuVisible.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, rpt.x, rpt.y, *this); 
}


//////////////////////////////////////////////////////////////////////////
// Обработка нажатия горячей клавиши
//
LRESULT CMainDlg::OnHotKey(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HWND hLastFgWnd = GetForegroundWindow();
	CArrHWND::const_iterator ciHWND = std::find(m_arrWnd.begin(), m_arrWnd.end(), hLastFgWnd);

	if (TRUE == IsWindowVisible())
	{
		ShowWindow(SW_HIDE);

		if (!m_bShowAlways && (m_arrWnd.end() != ciHWND))
		{
			m_arrWnd.erase(ciHWND);
		}
	}
	else
	{
		ShowWindow(SW_SHOWNOACTIVATE);

		if (!m_bShowAlways && (m_arrWnd.end() == ciHWND))
		{
			m_arrWnd.push_back(hLastFgWnd);
		}
	}

	// убираем из списка мертвые окна для исключения переполнения
	if (DEFAULT_MAX_HWND < m_arrWnd.size())
	{
		for (CArrHWND::iterator iterWndDie = m_arrWnd.begin(); iterWndDie != m_arrWnd.end(); )
		{
			if (!::IsWindow(*iterWndDie))
			{
				iterWndDie = m_arrWnd.erase(iterWndDie);

				continue;
			}

			iterWndDie++;
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Вызов диалога настроек
//
LRESULT CMainDlg::OnOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	CDlgOptions dlg(this);
	
	dlg.DoModal(*this);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Смена языка
//
VOID CMainDlg::ChangeLanguage(WORD wLanguageId)
{
	// поиск языка
	CMapWordStr::iterator find = m_mapLanguages.find(wLanguageId);

	// выходим, если указанный язык не поддерживаем
	if (find == m_mapLanguages.end())
	{
		return;
	}

	// смена языка
	m_wLanguageID = wLanguageId;

	CloseDialog(CHANGE_LANGUAGE);	
}


//////////////////////////////////////////////////////////////////////////
// Установка прозрачности
//
VOID CMainDlg::SetTransparency(BYTE bTransparency)
{
	m_bTransparency = bTransparency;

	ATLVERIFY(RenderingEMF());
}


//////////////////////////////////////////////////////////////////////////
// Установка горячей клавиши для показа линейки
//
VOID CMainDlg::SetHotkeyShow(DWORD dwHotkeyShow)
{
	m_dwHotkeyShow = dwHotkeyShow;

	if (m_bHotkeyOn)
	{
		ATLVERIFY(UnregisterHotKey(*this, 0));
		
		m_bHotkeyOn = FALSE;
	}

	if (RegisterHotKey(*this, 0, TransModifiers(HIBYTE(LOWORD(m_dwHotkeyShow))), LOBYTE(LOWORD(m_dwHotkeyShow))))
	{
		m_bHotkeyOn = TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
// Транформирование идентификаторов управляющих клавиш
// (из-за несовместимости GetHotkey and RegisterHotkey)
//
UINT CMainDlg::TransModifiers(WORD wMod)
{
	UINT uMod = 0;

	if (HOTKEYF_ALT & wMod)
	{
		uMod |= MOD_ALT;
	}

	if (HOTKEYF_SHIFT & wMod)
	{
		uMod |= MOD_SHIFT;
	}

	if (HOTKEYF_CONTROL & wMod)
	{
		uMod |= MOD_CONTROL;
	}

	return uMod;
}


//////////////////////////////////////////////////////////////////////////
// Загрузка EMF файла
//
BOOL CMainDlg::LoadEMF(std::wstring wsFilename)
{
	// формируем полный путь к файлу
	std::vector<WCHAR> wcPath(MAX_PATH);
	std::wstring       wsPath;
	
	ZeroMemory(&wcPath.front(), sizeof(WCHAR) * wcPath.size());
	GetModuleFileName(NULL, &wcPath.front(), static_cast<DWORD>(sizeof(WCHAR) * (wcPath.size() - 1)));

	wsPath = &wcPath.front();

	wstring::size_type stPos = wsPath.rfind(L"\\");
	if (wstring::npos != ++stPos)
	{
		wsPath.erase(stPos);
	}

	// открываем файл
	HANDLE hFile = INVALID_HANDLE_VALUE;

	hFile = CreateFile((wsPath + wsFilename).c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return FALSE;
	}

	// считываем файл в память
	DWORD               dwSize = GetFileSize(hFile, NULL);
	std::auto_ptr<BYTE> pBytes(new BYTE[dwSize]);

	if ((0 == dwSize) || (NULL == pBytes.get()))
	{
		CloseHandle(hFile);

		return FALSE;
	}

	// считываем файл в память
	DWORD dwRead;

	if (!ReadFile(hFile, pBytes.get(), dwSize, &dwRead, NULL) || (dwSize != dwRead))
	{
		CloseHandle(hFile);

		return FALSE;
	}

	// создаем в памяти EMF структуру
	LPENHMETAHEADER lpEMF = reinterpret_cast<LPENHMETAHEADER>(pBytes.get());
	if (ENHMETA_SIGNATURE == lpEMF->dSignature)
	{
		if (NULL != m_hEnhMetafile)
		{
			DeleteEnhMetaFile(m_hEnhMetafile);
		}

		m_hEnhMetafile = SetEnhMetaFileBits(dwSize, pBytes.get());
	}

	CloseHandle(hFile);

	if (NULL == m_hEnhMetafile)
	{
		return FALSE;
	}

	m_fDiffEMF = static_cast<FLOAT>((lpEMF->rclFrame.right - lpEMF->rclFrame.left)) / 
		(lpEMF->rclFrame.bottom - lpEMF->rclFrame.top);

	ATLVERIFY(RenderingEMF());

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Растеризация EMF
//
BOOL CMainDlg::RenderingEMF()
{
	// получение оконных координат
	CRect rectClient;
	CRect rectWindow;
	CRect rectImage;

	GetClientRect(&rectClient);
	GetWindowRect(&rectWindow);

	rectImage = rectClient;
	rectImage.InflateRect(0, 0, rectClient.Width(), rectClient.Height());

	// создание битмапа
	BITMAPINFO bmpInfo;

	bmpInfo.bmiHeader.biSize          = sizeof(BITMAPINFOHEADER); 
	bmpInfo.bmiHeader.biWidth         = rectImage.Width(); 
	bmpInfo.bmiHeader.biHeight        = rectImage.Height(); 
	bmpInfo.bmiHeader.biPlanes        = 1; 
	bmpInfo.bmiHeader.biBitCount      = 32; 
	bmpInfo.bmiHeader.biCompression   = BI_RGB; 
	bmpInfo.bmiHeader.biSizeImage     = 0; 
	bmpInfo.bmiHeader.biXPelsPerMeter = 0; 
	bmpInfo.bmiHeader.biYPelsPerMeter = 0; 
	bmpInfo.bmiHeader.biClrUsed       = 0; 
	bmpInfo.bmiHeader.biClrImportant  = 0; 

	HDC   dcMem     = ::CreateCompatibleDC(GetDC()); 
	PVOID pvBuf     = NULL;
	HBITMAP hBmp    = ::CreateDIBSection(dcMem, &bmpInfo, DIB_RGB_COLORS, &pvBuf, 0, 0);
	PVOID pvBuf2    = NULL;
	HBITMAP hBmp2   = ::CreateDIBSection(dcMem, &bmpInfo, DIB_RGB_COLORS, &pvBuf2, 0, 0);
	HBITMAP hBmpOld = static_cast<HBITMAP>(::SelectObject(dcMem, hBmp2));

	// очистка фона
	ZeroMemory(pvBuf2, rectImage.Width() * rectImage.Height() * 4);
	for (int i = 3; i < ((rectImage.Width() * rectImage.Height()) << 2); i+=4)
	{
		((PUCHAR)pvBuf2)[i] = m_bTransparency;
	}

	// рендеринг увеличенного EMF
	::PlayEnhMetaFile(dcMem, m_hEnhMetafile, &rectImage);

	// установка прозрачности
	for (int i = 3; i < ((rectImage.Width() * rectImage.Height()) << 2); i+=4)
	{
		((PUCHAR)pvBuf2)[i] ^= m_bTransparency;
	}

	// AGG
	agg::rendering_buffer aggRendBuf;
	aggRendBuf.attach((PUCHAR)pvBuf, rectImage.Width(), rectImage.Height(), rectImage.Width() * -4);

	agg::rendering_buffer aggRendImg;
	aggRendImg.attach((PUCHAR)pvBuf2, rectImage.Width(), rectImage.Height(), rectImage.Width() * -4);

	agg::interactive_polygon m_quad(4, 5.0);

	m_quad.xn(0) = rectClient.left;
	m_quad.yn(0) = rectClient.top;
	m_quad.xn(1) = rectClient.right;
	m_quad.yn(1) = rectClient.top;
	m_quad.xn(2) = rectClient.right;
	m_quad.yn(2) = rectClient.bottom;
	m_quad.xn(3) = rectClient.left;
	m_quad.yn(3) = rectClient.bottom;

	pixfmt_pre        pixf_pre(aggRendBuf);
	renderer_base_pre rb_pre(pixf_pre);

	rb_pre.clear(agg::rgba(0, 0, 0, 0));

	agg::rasterizer_scanline_aa<> rasterizer;
	rasterizer.move_to_d(m_quad.xn(0), m_quad.yn(0));
	rasterizer.line_to_d(m_quad.xn(1), m_quad.yn(1));
	rasterizer.line_to_d(m_quad.xn(2), m_quad.yn(2));
	rasterizer.line_to_d(m_quad.xn(3), m_quad.yn(3));

	span_alloc_type sa;
	agg::image_filter_bilinear filter_kernel;
	agg::image_filter_lut filter(filter_kernel, true);

	pixfmt pixf_img(aggRendImg);
	source_type source(pixf_img);

	agg::trans_affine tr(m_quad.polygon(), rectImage.left, rectImage.top, rectImage.right, rectImage.bottom);

	interpolator_type interpolator(tr);
	span_gen_type sg(source, interpolator, filter);

	agg::scanline_u8 scanline;
	agg::render_scanlines_aa(rasterizer, scanline, rb_pre, sa, sg);

	// обновление окна
	BLENDFUNCTION blend;

	blend.BlendOp             = AC_SRC_OVER;
	blend.BlendFlags          = 0;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat         = AC_SRC_ALPHA;

	static_cast<HBITMAP>(::SelectObject(dcMem, hBmp));
	if (FALSE == UpdateLayeredWindow(m_hWnd, NULL, &rectWindow.TopLeft(), &rectClient.Size(), 
		dcMem, &rectClient.TopLeft(), NULL, &blend, ULW_ALPHA))
	{
		MessageBox(L"Error in UpdateLayeredWindow", L"Update failed", MB_OK | MB_ICONERROR);
	}

	// удаление созданных ресурсов
	::SelectObject(dcMem, hBmpOld);
	::DeleteObject(hBmp);
	::DeleteObject(hBmp2);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// Изменение ширины линейки
//
BOOL CMainDlg::SetSize()
{
	// ширина десктопа
	RECT rectDesktop;
	::GetClientRect(GetDesktopWindow(), &rectDesktop);

	LONG lWidth = rectDesktop.right - rectDesktop.left;

	// изменение ширины линейки
	ResizeClient(static_cast<int>(lWidth * m_fWidth), static_cast<int>(lWidth * m_fWidth / m_fDiffEMF), FALSE);

	// рендеринг изображения
	ATLVERIFY(RenderingEMF());

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
// устанавливает новую ширину линейки
//
VOID CMainDlg::SetWidth(FLOAT fWidth)
{
	m_fWidth = fWidth;

	ATLVERIFY(SetSize());

	CenterWindow();
}


//////////////////////////////////////////////////////////////////////////
// Показывает/скрывает линейку
//
LRESULT CMainDlg::OnShowHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
{
	PostMessage(WM_HOTKEY);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка таймера
//
LRESULT CMainDlg::OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	HWND hLastFgWnd = GetForegroundWindow();

	if (hLastFgWnd != m_hLastFgWnd)
	{
		m_hLastFgWnd = hLastFgWnd;

		if ((*this == hLastFgWnd) || 
			(m_arrWnd.end() != std::find(m_arrWnd.begin(), m_arrWnd.end(), hLastFgWnd)) ||
			(*this == ::GetWindow(hLastFgWnd, GW_OWNER)))
		{
			ShowWindow(SW_SHOWNOACTIVATE);
		}
		else
		{
			ShowWindow(SW_HIDE);
		}
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка движения окна (из=за установки стиля WS_EX_NOACTIVATE)
//
LRESULT CMainDlg::OnMoving(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
{
	PRECT pRect = reinterpret_cast<PRECT>(lParam);

	SetWindowPos(NULL, pRect->left, pRect->top, pRect->right - pRect->left, pRect->bottom - pRect->top, 0);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// устанавливает режим работы ShowAlways
//
VOID CMainDlg::SetShowAlways(BOOL bShowAlways)
{
	if (TRUE == m_bTimerOn)
	{
		ATLVERIFY(KillTimer(0));

		m_bTimerOn = FALSE;
	}

	if (TRUE == bShowAlways)
	{
		ModifyStyleEx(WS_EX_NOACTIVATE, 0);

		ShowWindow(SW_SHOW);
	}
	else
	{
		ModifyStyleEx(0, WS_EX_NOACTIVATE);

		// запуск таймера
		if (SetTimer(0, m_iTimerPeriod, NULL))
		{
			m_bTimerOn = TRUE;
		}

		// вывод линейки
		ShowWindow(SW_HIDE);
	}

	CenterWindow();

	m_bShowAlways = bShowAlways;
}


//////////////////////////////////////////////////////////////////////////
// Закрытие диалога по WM_CLOSE
//
LRESULT CMainDlg::OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
{
	CloseDialog(0);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка колеса мыши
//
LRESULT CMainDlg::OnMouseWheel(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
{
	WORD wKeys  = GET_KEYSTATE_WPARAM(wParam);
	INT  iDelta = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA; 

	// с зажатым контролом меняем масштаб окна
	if (MK_CONTROL == wKeys)
	{
		FLOAT fNewWidth = m_fWidth + iDelta / 100.f;

		if ((fNewWidth > 0) && (fNewWidth <= 1))
		{
			SetWidth(fNewWidth);
		}
	}

	// иначе выполняем сдвиг окна
	else
	{
		// высота экрана
		INT iDisplayHeight = GetDeviceCaps(GetDC(), VERTRES);

		// координаты линейки
		CRect rectWindow;
		GetWindowRect(&rectWindow);

		// новая ордината линейки
		INT iNewY = rectWindow.top - iDelta * m_iDeltaY;

		// сдвиг
		if ((iNewY > 0) && (iDisplayHeight > (iNewY + rectWindow.Height())))
		{
			rectWindow.MoveToY(iNewY);

			SetWindowPos(NULL, rectWindow, 0);
		}
	}

	return 0;
}
