#pragma once 

#include "resource.h"

class CDlgOptions :
	public CIndirectDialogImpl<CDlgOptions, CResDlgTemplate>
{
	public:

		enum {IDD = IDD_OPTIONS};

		BEGIN_MSG_MAP(CDlgOptions)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			COMMAND_ID_HANDLER(IDOK, OnOK)
			COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
			COMMAND_HANDLER(IDC_COMBO_LANGUAGES, CBN_SELCHANGE, OnChangeLanguage)
			NOTIFY_ID_HANDLER(IDC_SLIDER_TRANSPARENT, OnChangeTransparent)
			NOTIFY_ID_HANDLER(IDC_SLIDER_SIZE, OnChangeSize)
		END_MSG_MAP()

		// Конструктор по-умолчанию
		CDlgOptions(CMainDlg *pMainDlg);

		// Инициализация шаблона диалога
		VOID DoInitTemplate();

		// Инициализация контролов диалога
		VOID DoInitControls();

	private:

		CMainDlg*     m_pMainDlg;       // указатель на главный диалог
		CMapWordStr*  m_pMapLanguages;  // массив доступных языков
		CComboBox     m_cbLanguages;    // выпадающий список языков
		WORD          m_wLanguageID;    // ИД выбранного языка
		CTrackBarCtrl m_tbTransparency; // ползунок изменения прозрачности
		CStatic       m_sTransparency;  // static для вывода процента прозрачности
		BYTE          m_bTransparency;  // значение прозрачности
		CHotKeyCtrl   m_hkShow;         // контрл выбора горячей клавиши для показа сокрытия окна
		CTrackBarCtrl m_tbWidth;        // ползунок изменения ширины линейки
		CStatic       m_sWidth;         // static для вывода коэффициента ширины линейки
		FLOAT         m_fWidth;         // коэффициент ширины линейки
		CHyperLink    m_hlEmail;        // ссылка на e-mail службы поддержки
		CHyperLink    m_hlWWW;          // ссылка на сайт программы
		CButton       m_btnShowAlways;  // режим ShowAlways
		CButton       m_btnAutostart;   // автозапуск программы

	protected:

		// Инициализация диалога
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		// Обработка WM_DESTROY
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		// Обработка IDOK
		LRESULT OnOK(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		// Обработка IDCANCEL
		LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		// Закрытие диалога
		VOID CloseDialog(int nVal);

		// Смены языка
		LRESULT OnChangeLanguage(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

		// Изменение прозрачности линейки
		LRESULT OnChangeTransparent(WORD wID, LPNMHDR pnmh, BOOL& bHandled);

		// Изменение размера линейки
		LRESULT OnChangeSize(WORD wID, LPNMHDR pnmh, BOOL& bHandled);

		// Автоматический запуск программы
		BOOL SetAutoStart();

		// Восстановление измененных настроек
		VOID RestoreSettings();
};
