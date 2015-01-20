#pragma once

template <class T> 
class CTrayIconImpl
{
	BEGIN_MSG_MAP(CTrayIconImpl)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(GetTaskbarCreatedMsg(), OnTaskbarCreated)
		MESSAGE_HANDLER(m_uMessage, OnNotifyIcon)
	END_MSG_MAP()

	enum
	{
		ToolTipStringLen = 128
	};

	private:

		UINT  m_uID;
		UINT  m_uMessage;
		HICON m_hIcon;
		WCHAR m_szTip[ToolTipStringLen];
		UINT  m_uIDMenu;
		BOOL  m_bAdded;

	public:
	
		// Конструктор по-умолчанию
		CTrayIconImpl() 
			:m_uID(0), m_hIcon(NULL), m_uMessage(WM_NULL), m_uIDMenu(0)
		{
			memset(m_szTip, 0, sizeof(m_szTip));

			m_bAdded = FALSE;
		}


		// Деструктор по-умолчанию
		~CTrayIconImpl()
		{
			DestroyNotifyIcon();

			m_hIcon = NULL;
		}

	// Messages
	protected:
		
		//////////////////////////////////////////////////////////////////////////
		// обработка WM_DESTROY
		//
		LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			DestroyNotifyIcon();
        
			bHandled = FALSE;

			return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		// обработка WM_SIZE
		//
		LRESULT OnSize(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
		{
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			// скрываем окно с панели задач если оно минимизировано
			if ((SIZE_MINIMIZED == wParam)  && IsAddedNotifyIcon())
			{
				pT->ShowWindow(SW_HIDE);
			}
        
			bHandled = FALSE;

			return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		// обработка создания Taskbar (например при его перезагрузке, чтобы иконка не терялась) 
		//
		LRESULT OnTaskbarCreated(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
		{
			if (IsAddedNotifyIcon())
			{
				RestoreNotifyIcon();
			}

			bHandled = FALSE;

			return 0;
		}

		//////////////////////////////////////////////////////////////////////////
		// Обработка сообщений от иконки в трее
		//
		LRESULT OnNotifyIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
		{
			bHandled = FALSE;
			
			if (!IsAddedNotifyIcon())
			{
            return 0;
			}

			if (m_uID != wParam)
			{
				return 0;
			}

			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));
        
			if(!IsWindow(pT->m_hWnd))
			{
				return 0;
			}

			POINT pt;
			ATLVERIFY(GetCursorPos(&pt));

			switch(lParam)
			{
				case WM_MOUSEMOVE:
					pT->OnNotifyIconMouseMove(pt);
					break;

				case WM_LBUTTONDBLCLK:
					pT->OnNotifyIconLButtonDblClk(pt);
					break;

				case WM_LBUTTONDOWN:
					pT->OnNotifyIconLButtonDown(pt);
					break;
			
				case WM_LBUTTONUP:
					pT->OnNotifyIconLButtonUp(pt);
					break;
	
				case WM_RBUTTONDOWN:
					pT->OnNotifyIconRButtonDown(pt);
					break;

				case WM_RBUTTONUP:
					pT->OnNotifyIconRButtonUp(pt);
					break;

				case WM_RBUTTONDBLCLK:
					pT->OnNotifyIconRButtonDblClk(pt);
					break;

				default:
					break;
			}

			return 0;
		}
    
	// Overridables
	public:

		//////////////////////////////////////////////////////////////////////////
		// Создание иконки в трее
		//
		BOOL CreateNotifyIcon(UINT uID = 1, UINT nMessage = WM_NULL)
		{
			m_uID = uID;

			// проверка существования окна приложения
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if (!IsWindow(pT->m_hWnd))
			{
				return FALSE;
			}

			// создание сообщения для иконки
			if (WM_NULL != nMessage)
			{
				m_uMessage = nMessage;
			}
			else
			{
				m_uMessage = GetDefaultNotifyIconMsg();
			}

			// Get window icon
			HICON hIcon = (HICON)pT->SendMessage(WM_GETICON, ICON_SMALL, 0);

			if (!hIcon)
			{
				hIcon = (HICON)pT->SendMessage(WM_GETICON, ICON_BIG, 0);
			}

			ATLASSERT(NULL != hIcon);
			if (NULL == hIcon)
			{
				return FALSE;
			}

			m_hIcon = hIcon;

			// Get window text
			TCHAR szTip[ToolTipStringLen] = { 0 };
			INT   nTitle = pT->GetWindowText(szTip, _countof(szTip));

			if (nTitle > _countof(szTip))
			{
				szTip[_countof(szTip) - 4] = _T('.');
				szTip[_countof(szTip) - 3] = _T('.');
				szTip[_countof(szTip) - 2] = _T('.');
				szTip[_countof(szTip) - 1] = _T('\0');
			}

			SaveNotifyIconTip(szTip);

			// добавление иконки
			if (IsAddedNotifyIcon())
			{
				return ModifyNotifyIcon();
			}

			return AddNotifyIcon();
		}


		//////////////////////////////////////////////////////////////////////////
		// Изменение иконки в трее
		//
		BOOL ModifyNotifyIcon(HICON hIcon)
		{
			ATLASSERT(hIcon);
			if (!hIcon)
			{
				return FALSE;
			}

			m_hIcon = hIcon;

			return ModifyNotifyIcon();
		}


		//////////////////////////////////////////////////////////////////////////
		// Удаление иконки из трея
		//
		BOOL DestroyNotifyIcon()
		{
			if (IsAddedNotifyIcon())
			{
				return DeleteNotifyIcon();
			}
			else
			{
				return TRUE;
			}
		}


		//////////////////////////////////////////////////////////////////////////
		// Установка меню
		//
		VOID SetNotifyIconContextMenu(UINT uIDMenu)
		{
			m_uIDMenu = uIDMenu;
		}


		//////////////////////////////////////////////////////////////////////////
		// Установка tooltip
		//
		BOOL SetNotifyIconTip(LPCTSTR lpszTip)
		{
			SaveNotifyIconTip(lpszTip);

			return ModifyNotifyIcon();
		}


		//////////////////////////////////////////////////////////////////////////
		// Возвращает TRUE, если иконка существует
		//
		BOOL IsAddedNotifyIcon()
		{
			return m_bAdded;
		}


		//////////////////////////////////////////////////////////////////////////
		// Регистрация сообщения для иконки
		//
		static UINT GetDefaultNotifyIconMsg(void)
		{
			static const UINT sc_uDefaultMsg = ::RegisterWindowMessage(_T("{407d42d2-30a8-4e30-a329-18bec2e35a9d}"));

			return sc_uDefaultMsg;
		}


		//////////////////////////////////////////////////////////////////////////
		// Получение сообщения для создания taskbar
		//
		static UINT GetTaskbarCreatedMsg(void)
		{
			static const UINT sc_uTaskbarCreatedMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));

			return sc_uTaskbarCreatedMsg;
		}


		//////////////////////////////////////////////////////////////////////////
		// Восстановление окна
		//
		BOOL RestoreWindow(bool bForeground = true)
		{
			// проверка существования окна
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if (!IsWindow(pT->m_hWnd))
			{
				return FALSE;
			}

			// If not visible, show main window
			if (!pT->IsWindowVisible())
			{
				pT->ShowWindow(SW_SHOW);
			}

			// If iconic, restore the main window
			if (pT->IsIconic())
			{
				pT->ShowWindow(SW_RESTORE);
			}

			if (bForeground)
			{
				ForegroundWindow();
			}

			return TRUE;
		}


		//////////////////////////////////////////////////////////////////////////
		// Окно на передний план
		//
		BOOL ForegroundWindow()
		{
			// проверка существования окна
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if (!IsWindow(pT->m_hWnd))
			{
				return FALSE;
			}

			// If so, does it have any popups?
			HWND hWndChild = pT->GetLastActivePopup();

			// Bring the main window or its popup to the foreground
			return ::SetForegroundWindow(hWndChild);
		}

		
		//////////////////////////////////////////////////////////////////////////
		// Обработка движения мыши на иконке
		//
		VOID OnNotifyIconMouseMove(const POINT& /*rpt*/)
		{
		}


		//////////////////////////////////////////////////////////////////////////
		// Обработка двойного нажатия левой кнопкой мыши
		//
		VOID OnNotifyIconLButtonDblClk(const POINT& /*rpt*/)
		{
		}


		//////////////////////////////////////////////////////////////////////////
		// Обработка нажатия левой кнопкой мыши
		//
		VOID OnNotifyIconLButtonDown(const POINT& /*rpt*/)
		{
		}


		//////////////////////////////////////////////////////////////////////////
		// Обработка отжатия левой кнопки мыши
		//
		VOID OnNotifyIconLButtonUp(const POINT& /*rpt*/)
		{
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if (!IsWindow(pT->m_hWnd))
			{
				return;
			}

			if (TRUE == pT->IsWindowVisible())
			{
				pT->ShowWindow(SW_HIDE);
			}
			else
			{
				pT->ShowWindow(SW_SHOWNOACTIVATE);
			}
		}


		//////////////////////////////////////////////////////////////////////////
		// Обработка нажатия правой кнопки мыши
		//
		VOID OnNotifyIconRButtonDown(const POINT& /*rpt*/)
		{
		}


		//////////////////////////////////////////////////////////////////////////
		// Обработка нажатия правой кнопки мыши
		//
		VOID OnNotifyIconRButtonUp(const POINT& rpt)
		{
			OnNotifyIconContextMenu(rpt);
		}


		//////////////////////////////////////////////////////////////////////////
		// Обработка вывода меню
		//
		VOID OnNotifyIconContextMenu(const POINT& rpt)
		{
			// проверка установленного меню
			if (0 == m_uIDMenu)
			{
				return;
			}

			// проверка существования окна
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if (!IsWindow(pT->m_hWnd))
			{
				return;
			}

			#if (_ATL_VER >= 0x0700)
			HINSTANCE hResource = ATL::_AtlBaseModule.GetResourceInstance();
			#else //!(_ATL_VER >= 0x0700)
			HINSTANCE hResource = _Module.GetResourceInstance();
			#endif //!(_ATL_VER >= 0x0700)
        
			HMENU hPopupMenu = ::LoadMenu(hResource, MAKEINTRESOURCE(m_uIDMenu));
			if(hPopupMenu)
			{
				HMENU hSubMenu;

				hSubMenu = ::GetSubMenu(hPopupMenu, 0);
				if (hSubMenu)
				{
					if (!pT->SendMessage(WM_INITMENUPOPUP, (WPARAM)hSubMenu, 0))
					{
						ForegroundWindow();
						::TrackPopupMenuEx(hSubMenu, TPM_LEFTALIGN, rpt.x, rpt.y, pT->m_hWnd, NULL);
						pT->PostMessage(WM_NULL, 0, 0);
					}
				}

				::DestroyMenu(hPopupMenu);
			}
		}


		//////////////////////////////////////////////////////////////////////////
		// Обработка двойного нажатия правой кнопкой мыши
		//
		VOID OnNotifyIconRButtonDblClk(const POINT& /*rpt*/)
		{
		}

	// Implementation
	private:
		
		//////////////////////////////////////////////////////////////////////////
		// Сохранение tooltip
		//
		VOID SaveNotifyIconTip(LPCTSTR lpszTip)
		{
			if (lpszTip)
			{
				lstrcpyn(m_szTip, lpszTip, _countof(m_szTip));
			}
			else
			{
				m_szTip[0] = _T('\0');
			}
		}


		//////////////////////////////////////////////////////////////////////////
		// Добавление иконки в трей
		//
		BOOL AddNotifyIcon()
		{
			// проверка существования окна
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if (!IsWindow(pT->m_hWnd))
			{
				return FALSE;
			}

			// формирование и отсылка сообщения
			NOTIFYICONDATA nid = { 0 };

			nid.cbSize           = sizeof(NOTIFYICONDATA);
			nid.hWnd             = pT->m_hWnd;
			nid.uID              = m_uID;
			nid.uFlags           = NIF_MESSAGE | (m_hIcon ? NIF_ICON : 0);
			nid.uCallbackMessage = m_uMessage;
			nid.hIcon            = m_hIcon;

			if(lstrlen(m_szTip))
			{
				lstrcpyn(nid.szTip, m_szTip, _countof(nid.szTip));
				nid.uFlags |= NIF_TIP;
			}

			return (m_bAdded = ::Shell_NotifyIcon(NIM_ADD, &nid));
		}


		//////////////////////////////////////////////////////////////////////////
		// Обновление иконки в трее
		//
		BOOL ModifyNotifyIcon()
		{
			// проверка существования окна
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if(!IsWindow(pT->m_hWnd))
			{
				return FALSE;
			}

			// формирование и отсылка сообщения
			NOTIFYICONDATA nid = { 0 };

			nid.cbSize           = sizeof(NOTIFYICONDATA);
			nid.hWnd             = pT->m_hWnd;
			nid.uID              = m_uID;
			nid.uFlags           = NIF_MESSAGE | (m_hIcon ? NIF_ICON : 0);
			nid.uCallbackMessage = m_uMessage;
			nid.hIcon            = m_hIcon;
        
			if(lstrlen(m_szTip))
			{
				lstrcpyn(nid.szTip, m_szTip, _countof(nid.szTip));

				nid.uFlags |= NIF_TIP;
			}

			return ::Shell_NotifyIcon(NIM_MODIFY, &nid);
		}


		//////////////////////////////////////////////////////////////////////////
		// Удаление иконки из трея
		//
		BOOL DeleteNotifyIcon(VOID)
		{
			// проверка существования окна
			T* pT = static_cast<T*>(this);
			ATLASSERT(::IsWindow(pT->m_hWnd));

			if (!IsWindow(pT->m_hWnd))
			{
				return FALSE;
			}

			// формирование и отсылка сообщения
			NOTIFYICONDATA nid = { 0 };

			nid.cbSize = sizeof(NOTIFYICONDATA);
			nid.hWnd   = pT->m_hWnd;
			nid.uID    = m_uID;
			m_bAdded   = !::Shell_NotifyIcon(NIM_DELETE, &nid);

			return !m_bAdded;
		}


		//////////////////////////////////////////////////////////////////////////
		// Восстановление иконки в трее
		//
		BOOL RestoreNotifyIcon(VOID)
		{
			if(!m_hIcon)
			{
				return FALSE;
			}

			return AddNotifyIcon();
		}
};