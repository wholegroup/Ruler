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
CDlgOptions::CDlgOptions(CMainDlg *pMainDlg)
{
	ATLASSERT(NULL != pMainDlg);
	m_pMainDlg = pMainDlg;
}


//////////////////////////////////////////////////////////////////////////
// Инициализация диалога
//
LRESULT CDlgOptions::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	// выравниваем по центру
	CenterWindow();

	// вставка в заголовок диалога версии программы
	std::vector<WCHAR> wcTitle(GetWindowTextLength() + 1);
	
	::GetWindowText(m_hWnd, &wcTitle.front(), static_cast<int>(wcTitle.size()));
	
	std::wstring wsTitle = &wcTitle.front();
	
	wsTitle += L" v";
	wsTitle += PROGRAM_VERSION;
	SetWindowText(wsTitle.c_str());

	// заполнение списка языков
	m_cbLanguages = static_cast<CComboBox>(GetDlgItem(IDC_COMBO_LANGUAGES));

	m_wLanguageID   = m_pMainDlg->GetLanguageID();
	m_pMapLanguages = m_pMainDlg->GetMapLanguages();
	ATLASSERT(NULL != m_pMapLanguages);

	INT iSelect = 0;
	INT i       = 0;

	for (CMapWordStr::const_iterator lang = m_pMapLanguages->begin(); lang != m_pMapLanguages->end(); lang++, i++)
	{
		m_cbLanguages.AddString(lang->second.c_str());

		if (lang->first == m_wLanguageID)
		{
			iSelect = i;
		}
	}

	m_cbLanguages.SetCurSel(iSelect);

	// установка ползунка прозрачности
	m_bTransparency = m_pMainDlg->GetTransparency();
	
	m_tbTransparency = static_cast<CTrackBarCtrl>(GetDlgItem(IDC_SLIDER_TRANSPARENT));
	m_tbTransparency.SetRange(26, 255);
	m_tbTransparency.SetPos(m_bTransparency);

	std::wostringstream streamTransparency;
	streamTransparency << (100 - m_bTransparency * 100 / m_tbTransparency.GetRangeMax()) << "%";

	m_sTransparency = static_cast<CStatic>(GetDlgItem(IDC_STATIC_PERCENT_TRANSPARENCY));
	m_sTransparency.SetWindowText(streamTransparency.str().c_str());

	// установка ползунка ширины линейки
	m_fWidth = m_pMainDlg->GetWidth();

	m_tbWidth = static_cast<CTrackBarCtrl>(GetDlgItem(IDC_SLIDER_SIZE));
	m_tbWidth.SetRange(200, 1000);
	m_tbWidth.SetPos(static_cast<int>(m_fWidth * 1000));

	std::wostringstream streamWidth;
	streamWidth << (m_tbWidth.GetPos() * 100 / m_tbWidth.GetRangeMax()) << "%";

	m_sWidth = static_cast<CStatic>(GetDlgItem(IDC_STATIC_PERCENT_SIZE));
	m_sWidth.SetWindowText(streamWidth.str().c_str());

	// установка горячей клавиши
	DWORD dwHotkeyShow = m_pMainDlg->GetHotkeyShow();

	m_hkShow = static_cast<CHotKeyCtrl>(GetDlgItem(IDC_HOTKEY_SHOW));
	m_hkShow.SetHotKey(LOBYTE(LOWORD(dwHotkeyShow)), HIBYTE(LOWORD(dwHotkeyShow)));

	// установка переключателя режима ShowAlways
	m_btnShowAlways = static_cast<CButton>(GetDlgItem(IDC_CHECK_SHOWALWAYS));
	
	m_btnShowAlways.SetCheck(m_pMainDlg->GetShowAlways());

	// установка переключателя автозапуска
	m_btnAutostart = static_cast<CButton>(GetDlgItem(IDC_CHECK_AUTOSTART));
	
	HKEY hKey;

	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey))
	{
		if (ERROR_SUCCESS == RegQueryValueEx(hKey, PROGRAM_NAME, NULL, NULL, NULL, NULL))
		{
			m_btnAutostart.SetCheck(TRUE);
		}
		else
		{
			m_btnAutostart.SetCheck(FALSE);
		}
		
		RegCloseKey(hKey);
	}

	// изменение шрифтов
	CLogFont lfBold(static_cast<CStatic>(GetDlgItem(IDC_STATIC_TRANSPARENCY)).GetFont());
	lfBold.SetBold();

	static_cast<CStatic>(GetDlgItem(IDC_STATIC_LANGUAGE)).SetFont(lfBold.CreateFontIndirect());
	static_cast<CStatic>(GetDlgItem(IDC_STATIC_TRANSPARENCY)).SetFont(lfBold.CreateFontIndirect());
	static_cast<CStatic>(GetDlgItem(IDC_STATIC_HOTKEY)).SetFont(lfBold.CreateFontIndirect());
	static_cast<CStatic>(GetDlgItem(IDC_STATIC_SIZE)).SetFont(lfBold.CreateFontIndirect());
	static_cast<CStatic>(GetDlgItem(IDC_STATIC_WWW)).SetFont(lfBold.CreateFontIndirect());
	static_cast<CStatic>(GetDlgItem(IDC_STATIC_EMAIL)).SetFont(lfBold.CreateFontIndirect());

	m_hlEmail.SetHyperLink(m_pMainDlg->GetStringLang(IDS_EMAIL).c_str());
	m_hlEmail.SubclassWindow(GetDlgItem(IDC_HYPERLINK_EMAIL));

	m_hlWWW.SetHyperLink(m_pMainDlg->GetStringLang(IDS_WWW).c_str());
	m_hlWWW.SubclassWindow(GetDlgItem(IDC_HYPERLINK_WWW));

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка WM_DESTROY
//
LRESULT CDlgOptions::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка IDOK
//
LRESULT CDlgOptions::OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	// установка автозапуска
	SetAutoStart();

	// установка горячей клавиши
	m_pMainDlg->SetHotkeyShow(m_hkShow.GetHotKey());

	// установка режима ShowAlways
	m_pMainDlg->SetShowAlways(m_btnShowAlways.GetCheck());

	CloseDialog(IDOK);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Обработка IDCANCEL
//
LRESULT CDlgOptions::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	RestoreSettings();

	CloseDialog(IDCANCEL);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Инициализация шаблона диалога
//
VOID CDlgOptions::DoInitTemplate() 
{
	m_Template.Create((LPDLGTEMPLATE)m_pMainDlg->GetResourceLang(RT_DIALOG, this->IDD));
}


//////////////////////////////////////////////////////////////////////////
// Инициализация контролов диалога
//
VOID CDlgOptions::DoInitControls() 
{
	ATLASSERT(TRUE);
}


//////////////////////////////////////////////////////////////////////////
// Закрытие диалога
//
VOID CDlgOptions::CloseDialog(int nVal)
{
	EndDialog(nVal);
}


//////////////////////////////////////////////////////////////////////////
// Смена языка
//
LRESULT CDlgOptions::OnChangeLanguage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	RestoreSettings();

	ATLASSERT(CBN_SELCHANGE == wNotifyCode);

	INT iSelect = m_cbLanguages.GetCurSel();
	ATLASSERT(iSelect < static_cast<INT>(m_pMapLanguages->size()));

	CMapWordStr::const_iterator lang = m_pMapLanguages->begin();
	advance(lang, iSelect);
	m_pMainDlg->ChangeLanguage(lang->first);

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Изменение прозрачности
//
LRESULT CDlgOptions::OnChangeTransparent(WORD wID, LPNMHDR pnmh, BOOL& bHandled)
{
	ATLASSERT(IDC_SLIDER_TRANSPARENT == wID);

	m_pMainDlg->SetTransparency(m_tbTransparency.GetPos());

	std::wostringstream streamTransparency;
	streamTransparency << (100 - m_tbTransparency.GetPos() * 100 / m_tbTransparency.GetRangeMax()) << L"%";

	m_sTransparency.SetWindowText(streamTransparency.str().c_str());

	return 0;
}	


//////////////////////////////////////////////////////////////////////////
// Восстановление измененных настроек
//
VOID CDlgOptions::RestoreSettings()
{
	m_pMainDlg->SetTransparency(m_bTransparency);
	m_pMainDlg->SetWidth(m_fWidth);
}


//////////////////////////////////////////////////////////////////////////
// Изменение размера линейки
//
LRESULT CDlgOptions::OnChangeSize(WORD wID, LPNMHDR pnmh, BOOL& bHandled)
{
	ATLASSERT(IDC_SLIDER_SIZE == wID);

	if (0 == m_tbWidth.GetPos())
	{
		m_pMainDlg->SetWidth(0);
	}
	else
	{
		m_pMainDlg->SetWidth(m_tbWidth.GetPos() / static_cast<FLOAT>(m_tbWidth.GetRangeMax()));
	}

	std::wostringstream streamWidth;
	streamWidth << (m_tbWidth.GetPos() * 100 / m_tbWidth.GetRangeMax()) << "%";

	m_sWidth.SetWindowText(streamWidth.str().c_str());

	return 0;
}


//////////////////////////////////////////////////////////////////////////
// Автоматический запуск программы
//
BOOL CDlgOptions::SetAutoStart()
{
	HKEY  hKey;
	WCHAR ptcFileName[MAX_PATH];

	if (ERROR_SUCCESS == RegOpenKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey))
	{
		if (m_btnAutostart.GetCheck())
		{
			ZeroMemory(ptcFileName, sizeof(WCHAR) * MAX_PATH);
			GetModuleFileName(NULL, ptcFileName, sizeof(WCHAR) * (MAX_PATH - 1));
			RegSetValueEx(hKey, PROGRAM_NAME, 0, REG_SZ, (BYTE*)ptcFileName, static_cast<DWORD>(wcslen(ptcFileName) * sizeof(WCHAR)));
		}
		else
		{
			RegDeleteValue(hKey, PROGRAM_NAME);
		}

		RegCloseKey(hKey);
	}

	return TRUE;
}
