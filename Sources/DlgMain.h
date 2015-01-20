#pragma once

#include "resource.h"

//	класс основного окна программы
class CMainDlg : 
	public CIndirectDialogImpl<CMainDlg, CResDlgTemplate>,
	public CTrayIconImpl<CMainDlg>,
	public CSettingsImpl
{
	public:
		enum { IDD = IDD_MAINDLG };

		BEGIN_MSG_MAP(CMainDlg)
			CHAIN_MSG_MAP(CTrayIconImpl<CMainDlg>)
			MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_CONTEXTMENU, OnShowMenu)
			MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
			MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLeftButton)
			MESSAGE_HANDLER(WM_HOTKEY, OnHotKey)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_MOVING, OnMoving)
			MESSAGE_HANDLER(WM_CLOSE, OnClose)
			MESSAGE_HANDLER(WM_QUERYENDSESSION, OnClose)
			MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
			COMMAND_ID_HANDLER(IDOK, OnOK)
			COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
			COMMAND_ID_HANDLER(IDB_QUIT, OnQuit)
			COMMAND_ID_HANDLER(IDB_OPTIONS, OnOptions)
			COMMAND_ID_HANDLER(IDB_SHOWHIDE, OnShowHide)
			REFLECT_NOTIFICATIONS()
		END_MSG_MAP()

	public:

		// Конструктор по-умолчанию
		CMainDlg();

		// Деструктор по-умолчанию
		~CMainDlg();

		// Инициализация шаблона диалога
		VOID DoInitTemplate();

		// Инициализация контролов диалога
		VOID DoInitControls();

	private:

		CMapWordStr  m_mapLanguages;  // список языков программы(ID, язык)
		WORD         m_wLanguageID;   // ИД выбранного языка программы
		HCURSOR      m_hCursorHand;   // хендл курсора руки
		BYTE         m_bTransparency; // значение прозрачности (0-255)
		DWORD        m_dwHotkeyShow;  // горячая клавиша для показа линейки
		BOOL         m_bHotkeyOn;     // TRUE, если хоткей работает
		HENHMETAFILE m_hEnhMetafile;  // хендл на EMF
		FLOAT        m_fDiffEMF;      // отношение ширины и высоты изображения линейки
		FLOAT        m_fWidth;        // ширина линейки в % (от 0 до 1)
		BOOL         m_bShowAlways;   // показывать всегда (для dual и wide мониторов)
		BOOL         m_bTimerOn;      // TRUE, если таймер запущен
		HWND         m_hLastFgWnd;    // последнее активное окно
		CArrHWND     m_arrWnd;        // список окон в которых включена линейка
		INT          m_iTimerPeriod;  // частота таймера в мс
		INT          m_iDeltaY;       // количество пикселей при движении линейки с помощью колеса

	protected:
	
		// Инициализация диалога
		LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Уничтожение диалога
		LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Обработка IDOK
		LRESULT OnOK(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		// Обработка IDCANCEL
		LRESULT OnCancel(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		// Закрытие диалога
		VOID CloseDialog(int nVal);

		// Поиск всех возможных языков программы
		VOID SearchLanguages();

		static BOOL CALLBACK SearchLanguageCallback(HMODULE hModule, LPCTSTR lpszType, LPTSTR lpszName, 
			WORD wIDLanguage, LONG_PTR lParam);

		// Вывод контекстного меню
		LRESULT OnShowMenu(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Установка курсора руки
		LRESULT OnSetCursor(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Для перетаскивания окна в любой точке
		LRESULT OnLeftButton(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Выход из приложения
		LRESULT OnQuit(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		// Вызов диалога настроек
		LRESULT OnOptions(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		// Обработка нажатия горячей клавиши
		LRESULT OnHotKey(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Обработка таймера
		LRESULT OnTimer(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Обработка движения окна (из=за установки стиля WS_EX_NOACTIVATE)
		LRESULT OnMoving(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Транформирование идентификаторов управляющих клавиш
		UINT TransModifiers(WORD wMod);

		// Загрузка EMF файла
		BOOL LoadEMF(std::wstring wsFilename);

		// Растеризация EMF
		BOOL RenderingEMF();

		// Изменение ширины линейки
		BOOL SetSize();

		// Диалог регистрации программы
		LRESULT OnRegister(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		// Показывает/скрывает линейку
		LRESULT OnShowHide(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

		// Закрытие диалога по WM_CLOSE
		LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

		// Обработка колеса мыши
		LRESULT OnMouseWheel(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);

	public:

		// Вывод меню по нажатию правой кнопки мыши на значке в трее
		VOID OnNotifyIconRButtonUp(const POINT& rpt);

		// Загружает указанный ресурс
		HGLOBAL GetResourceLang(LPWSTR lpResType, DWORD dwResID);

		// Загружает строку по указанному идентификатору в соответствии с установленным языком
		std::wstring GetStringLang(DWORD dwResID);

		// Возвращает список доступных языков
		inline CMapWordStr* GetMapLanguages(){return &m_mapLanguages;}

		// Возвращает ИД выбранного языка программы
		inline WORD GetLanguageID() {return m_wLanguageID;}

		// Смена языка
		VOID ChangeLanguage(WORD wLanguageId);

		// Установка прозрачности
		VOID SetTransparency(BYTE bTransparency);

		// Возвращает текущую прозрачность
		inline BYTE GetTransparency(){return m_bTransparency;}

		// Установка горячей клавиши для показа линейки
		DWORD GetHotkeyShow(){return m_dwHotkeyShow;}

		// Установка горячей клавиши для показа линейки
		VOID SetHotkeyShow(DWORD dwHotkeyShow);

		// устанавливает новую ширину линейки
		VOID SetWidth(FLOAT fWidth);

		// возвращает коэффициент ширины линейки
		inline FLOAT GetWidth(){return m_fWidth;}

		// возвращает режим работы ShowAlways
		inline BOOL GetShowAlways() {return m_bShowAlways;}

		// устанавливает режим работы ShowAlways
		VOID SetShowAlways(BOOL bShowAlways);
};
