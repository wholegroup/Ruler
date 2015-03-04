#pragma once

// минимально требуемые версии
#define WINVER        0x0500
#define _WIN32_WINNT  0x0501
#define _WIN32_IE     0x0501
#define _RICHEDIT_VER 0x0200

// WinAPI
#include <Windows.h>

// STL
#include <vector>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <ctime>

// тип списка языков (ИД, название)
typedef std::map<WORD, const std::wstring> CMapWordStr;

// список хендлов окон
typedef std::vector<HWND> CArrHWND;

// WTL
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlmisc.h>
#include <atldlgs.h>

// AGG
#include "agg-2.4/include/agg_scanline_u.h"
#include "agg-2.4/include/agg_renderer_scanline.h"
#include "agg-2.4/include/agg_pixfmt_rgba.h"
#include "agg-2.4/include/agg_rasterizer_scanline_aa.h"
#include "agg-2.4/include/agg_trans_affine.h"
#include "agg-2.4/examples/interactive_polygon.h"
#include "agg-2.4/include/agg_span_allocator.h"
#include "agg-2.4/include/agg_image_accessors.h"
#include "agg-2.4/include/agg_span_interpolator_linear.h"
#include "agg-2.4/include/agg_span_image_filter_rgba.h"
#include "agg-2.4/include/agg_image_filters.h"

#define image_resample_affine_type agg::span_image_resample_rgba_affine

typedef agg::pixfmt_bgra32                               pixfmt;
typedef pixfmt::color_type                               color_type;
typedef agg::span_allocator<color_type>                  span_alloc_type;
typedef agg::pixfmt_bgra32_pre                           pixfmt_pre;
typedef agg::renderer_base<pixfmt_pre>                   renderer_base_pre;
typedef agg::image_accessor_clone<pixfmt>                source_type;
typedef agg::span_interpolator_linear<agg::trans_affine> interpolator_type;
typedef image_resample_affine_type<source_type>          span_gen_type;


// для использования настроек программы
#include "WTLAddons/CSettingsImpl.h"

// для создания иконки в трее
#include "CTrayIconImpl.h"

// версия программы (генерируется при сборке MSBuild)
#include "Version.h"

// константы по-умолчанию
#include "Default.h"

// шаблон для создания мультиязычных диалогов
#include "DlgTemplate.h"

extern CAppModule _Module;

#if defined _M_IX86
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
	#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

