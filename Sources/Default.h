#pragma once

// компания
#define COMPANY_NAME L"www.wholegroup.com"

// название программы
#define PROGRAM_NAME L"Ruler Reader"

// язык по-умолчанию
#define DEFAULT_LANGUAGE MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL)

// код выхода для смены языка
#define CHANGE_LANGUAGE 123

// прозрачность по-умолчанию 0-255
#define DEFAULT_TRANSPARENCY 255

// ширина по-умолчанию
#define DEFAULT_WIDTH 0.5

// сдвиг колесом мыши в пикселах
#define DEFAULT_DELTAY 5

// частота таймера для обновления активного окна в мс
#define DEFAULT_TIMER 300

// показывать линейку всегда (для dual и wide мониторов)
#define DEFAULT_SHOWALLWAYS TRUE

// максимальное количество элементов в списке HWND
#define DEFAULT_MAX_HWND 100

// макросы для отладки
#ifdef _DEBUG
#define _RPT_API_TRACE(name)                                                     \
	do                                                                               \
	{                                                                                \
	unsigned __err = GetLastError();                                              \
	WCHAR    wcMsg[512];                                                          \
	\
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, __err, 0, wcMsg, 512, NULL); \
	\
	_RPTFW3(_CRT_WARN, L"%S() failed; ERR=%d %s\n", #name, __err, wcMsg);         \
	\
	SetLastError(__err);                                                          \
	} while(0, 0)
#else
#define _RPT_API_TRACE(name)	__noop
#endif

#ifdef _DEBUG
#define _RPT_API_ERROR(name)	                                                   \
	do                                                                               \
	{                                                                                \
	unsigned __err = GetLastError();                                              \
	WCHAR     wcMsg[512];                                                         \
	\
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, __err, 0, wcMsg, 512, NULL); \
	\
	_RPTFW3(_CRT_ERROR, L"%S() failed; ERR=%d %s\n", #name, __err, wcMsg);        \
	\
	SetLastError(__err);                                                          \
	} while(0, 0)
#else
#define _RPT_API_ERROR(name)	__noop
#endif
