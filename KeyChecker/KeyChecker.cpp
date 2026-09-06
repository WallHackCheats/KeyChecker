// ============================================================
// KeyChecker
// Native Windows C++ Desktop Application
//
// Keyboard + Gaming Mouse Tester
//
// Requirements:
//
// KeyChecker.cpp
// github.png
// discord.png
// youtube.png
// assets\font.ttf
//
// No Qt
// No CMake required
// ============================================================

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <windowsx.h>
#include <shellapi.h>
#include <wincodec.h>

#include <d2d1.h>
#include <dwrite.h>

#include <vector>
#include <string>
#include <cmath>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shell32.lib")


// ============================================================
// CONSTANTS
// ============================================================

constexpr float KEY_HEIGHT = 54.0f;
constexpr float KEY_SPACING = 6.0f;

constexpr float KEY_RADIUS = 20.0f;
constexpr float BORDER_THICKNESS = 2.0f;

constexpr float TITLE_FONT_SIZE = 38.0f;
constexpr float KEY_FONT_SIZE = 17.0f;
constexpr float BUTTON_FONT_SIZE = 15.0f;
constexpr float COPYRIGHT_FONT_SIZE = 15.0f;

constexpr float BUTTON_WIDTH = 135.0f;
constexpr float BUTTON_HEIGHT = 46.0f;
constexpr float BUTTON_SPACING = 10.0f;

constexpr float BUTTON_MARGIN_LEFT = 25.0f;
constexpr float BUTTON_MARGIN_BOTTOM = 14.0f;


// ============================================================
// MOUSE CONSTANTS
// ============================================================

constexpr float MOUSE_X = 1325.0f;
constexpr float MOUSE_Y = 155.0f;


constexpr float MOUSE_WIDTH = 190.0f;
constexpr float MOUSE_HEIGHT = 330.0f;

constexpr float MOUSE_RADIUS = 82.0f;

constexpr float MOUSE_BUTTON_HEIGHT = 135.0f;

constexpr float MOUSE_WHEEL_WIDTH = 30.0f;
constexpr float MOUSE_WHEEL_HEIGHT = 58.0f;

constexpr float MOUSE_SIDE_WIDTH = 35.0f;
constexpr float MOUSE_SIDE_HEIGHT = 72.0f;

constexpr float MOUSE_LABEL_FONT_SIZE = 14.0f;

constexpr DWORD MOUSE_FLASH_TIME = 180;


// ============================================================
// STRUCTURES
// ============================================================

struct KeyboardKey
{
    std::wstring name;

    int virtualKey;

    float x;
    float y;
    float width;

    bool pressed;
};


struct SocialButton
{
    std::wstring name;
    std::wstring url;
    std::wstring imageFile;

    float x;
    float y;

    float width;
    float height;

    ID2D1Bitmap* bitmap;

    bool hovered;
};


// ============================================================
// GLOBALS
// ============================================================

HWND g_hWnd = nullptr;


// ============================================================
// DIRECT2D
// ============================================================

ID2D1Factory* g_d2dFactory = nullptr;

ID2D1HwndRenderTarget* g_renderTarget = nullptr;

ID2D1SolidColorBrush* g_whiteBrush = nullptr;
ID2D1SolidColorBrush* g_blackBrush = nullptr;

ID2D1SolidColorBrush* g_buttonBrush = nullptr;
ID2D1SolidColorBrush* g_buttonHoverBrush = nullptr;

ID2D1LinearGradientBrush* g_backgroundBrush = nullptr;



// Mouse brushes

ID2D1SolidColorBrush* g_mouseBrush = nullptr;
ID2D1SolidColorBrush* g_mousePressedBrush = nullptr;
ID2D1SolidColorBrush* g_mouseHoverBrush = nullptr;
ID2D1SolidColorBrush* g_mouseWheelBrush = nullptr;
ID2D1SolidColorBrush* g_mouseLineBrush = nullptr;
ID2D1SolidColorBrush* g_scrollUpBrush = nullptr;
ID2D1SolidColorBrush* g_scrollDownBrush = nullptr;


// ============================================================
// DIRECTWRITE
// ============================================================

IDWriteFactory* g_writeFactory = nullptr;

IDWriteTextFormat* g_keyTextFormat = nullptr;
IDWriteTextFormat* g_titleTextFormat = nullptr;
IDWriteTextFormat* g_copyrightTextFormat = nullptr;
IDWriteTextFormat* g_buttonTextFormat = nullptr;

IDWriteTextFormat* g_mouseLabelTextFormat = nullptr;


// ============================================================
// WIC
// ============================================================

IWICImagingFactory* g_wicFactory = nullptr;


// ============================================================
// DATA
// ============================================================

std::vector<KeyboardKey> g_keyboard;
std::vector<SocialButton> g_buttons;


// ============================================================
// CUSTOM FONT
// ============================================================

bool g_customFontLoaded = false;

std::wstring g_customFontPath;


// ============================================================
// MOUSE STATE
// ============================================================

struct MouseState
{
    bool leftPressed = false;
    bool rightPressed = false;
    bool middlePressed = false;

    bool sideBackPressed = false;
    bool sideForwardPressed = false;

    bool scrollUp = false;
    bool scrollDown = false;

    DWORD scrollUpTime = 0;
    DWORD scrollDownTime = 0;
};

MouseState g_mouse;


// ============================================================
// SAFE RELEASE
// ============================================================

template <typename T>
void SafeRelease(T*& object)
{
    if (object != nullptr)
    {
        object->Release();
        object = nullptr;
    }
}


// ============================================================
// APPLICATION DIRECTORY
// ============================================================

std::wstring GetApplicationDirectory()
{
    wchar_t path[MAX_PATH] = {};

    DWORD length =
        GetModuleFileNameW(
            nullptr,
            path,
            MAX_PATH
        );

    if (length == 0)
    {
        return L".";
    }

    std::wstring fullPath(
        path,
        length
    );

    size_t position =
        fullPath.find_last_of(
            L"\\/"
        );

    if (position == std::wstring::npos)
    {
        return L".";
    }

    return fullPath.substr(
        0,
        position
    );
}


// ============================================================
// FONT
// ============================================================

bool LoadCustomFont()
{
    std::wstring directory =
        GetApplicationDirectory();

    g_customFontPath =
        directory +
        L"\\assets\\font.ttf";

    DWORD attributes =
        GetFileAttributesW(
            g_customFontPath.c_str()
        );

    if (
        attributes ==
        INVALID_FILE_ATTRIBUTES
        )
    {
        return false;
    }

    if (
        attributes &
        FILE_ATTRIBUTE_DIRECTORY
        )
    {
        return false;
    }

    int result =
        AddFontResourceExW(
            g_customFontPath.c_str(),
            FR_PRIVATE,
            nullptr
        );

    if (result > 0)
    {
        g_customFontLoaded = true;
        return true;
    }

    return false;
}


void UnloadCustomFont()
{
    if (
        g_customFontLoaded &&
        !g_customFontPath.empty()
        )
    {
        RemoveFontResourceExW(
            g_customFontPath.c_str(),
            FR_PRIVATE,
            nullptr
        );

        g_customFontLoaded = false;
    }
}


// ============================================================
// KEY CREATION
// ============================================================

void AddKey(
    const std::wstring& name,
    int virtualKey,
    float x,
    float y,
    float width
)
{
    KeyboardKey key{};

    key.name = name;
    key.virtualKey = virtualKey;

    key.x = x;
    key.y = y;
    key.width = width;

    key.pressed = false;

    g_keyboard.push_back(
        key
    );
}


// ============================================================
// CREATE KEYBOARD
// ============================================================

void CreateKeyboard()
{
    g_keyboard.clear();

    constexpr float K = 54.0f;

    constexpr float ROW_F = 125.0f;
    constexpr float ROW_NUMBER = 190.0f;
    constexpr float ROW_QWERTY = 250.0f;
    constexpr float ROW_HOME = 310.0f;
    constexpr float ROW_SHIFT = 370.0f;
    constexpr float ROW_BOTTOM = 430.0f;

    constexpr float START_X = 25.0f;


    float x = START_X;

    AddKey(
        L"ESC",
        VK_ESCAPE,
        x,
        ROW_F,
        70.0f
    );

    x += 86.0f;

    AddKey(L"F1", VK_F1, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F2", VK_F2, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F3", VK_F3, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F4", VK_F4, x, ROW_F, K);

    x += K + 16.0f;

    AddKey(L"F5", VK_F5, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F6", VK_F6, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F7", VK_F7, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F8", VK_F8, x, ROW_F, K);

    x += K + 16.0f;

    AddKey(L"F9", VK_F9, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F10", VK_F10, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F11", VK_F11, x, ROW_F, K);
    x += K + KEY_SPACING;

    AddKey(L"F12", VK_F12, x, ROW_F, K);

    x += K + 20.0f;

    AddKey(
        L"PRINT",
        VK_SNAPSHOT,
        x,
        ROW_F,
        75.0f
    );

    x += 81.0f;

    AddKey(
        L"SCROLL",
        VK_SCROLL,
        x,
        ROW_F,
        75.0f
    );

    x += 81.0f;

    AddKey(
        L"PAUSE",
        VK_PAUSE,
        x,
        ROW_F,
        75.0f
    );


    // ========================================================
    // NUMBER ROW
    // ========================================================

    x = START_X;

    AddKey(L"`", VK_OEM_3, x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"1", '1', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"2", '2', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"3", '3', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"4", '4', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"5", '5', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"6", '6', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"7", '7', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"8", '8', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"9", '9', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"0", '0', x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"-", VK_OEM_MINUS, x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(L"=", VK_OEM_PLUS, x, ROW_NUMBER, K);
    x += K + KEY_SPACING;

    AddKey(
        L"BACKSPACE",
        VK_BACK,
        x,
        ROW_NUMBER,
        108.0f
    );


    // ========================================================
    // QWERTY
    // ========================================================

    x = START_X;

    AddKey(
        L"TAB",
        VK_TAB,
        x,
        ROW_QWERTY,
        82.0f
    );

    x += 88.0f;

    AddKey(L"Q", 'Q', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"W", 'W', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"E", 'E', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"R", 'R', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"T", 'T', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"Y", 'Y', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"U", 'U', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"I", 'I', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"O", 'O', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"P", 'P', x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"[", VK_OEM_4, x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(L"]", VK_OEM_6, x, ROW_QWERTY, K);
    x += K + KEY_SPACING;

    AddKey(
        L"\\",
        VK_OEM_5,
        x,
        ROW_QWERTY,
        82.0f
    );


    // ========================================================
    // HOME ROW
    // ========================================================

    x = START_X;

    AddKey(
        L"CAPS",
        VK_CAPITAL,
        x,
        ROW_HOME,
        95.0f
    );

    x += 101.0f;

    AddKey(L"A", 'A', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"S", 'S', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"D", 'D', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"F", 'F', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"G", 'G', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"H", 'H', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"J", 'J', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"K", 'K', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"L", 'L', x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L";", VK_OEM_1, x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(L"'", VK_OEM_7, x, ROW_HOME, K);
    x += K + KEY_SPACING;

    AddKey(
        L"ENTER",
        VK_RETURN,
        x,
        ROW_HOME,
        108.0f
    );


    // ========================================================
    // SHIFT ROW
    // ========================================================

    x = START_X;

    AddKey(
        L"SHIFT",
        VK_LSHIFT,
        x,
        ROW_SHIFT,
        125.0f
    );

    x += 131.0f;

    AddKey(L"Z", 'Z', x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L"X", 'X', x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L"C", 'C', x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L"V", 'V', x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L"B", 'B', x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L"N", 'N', x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L"M", 'M', x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L",", VK_OEM_COMMA, x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L".", VK_OEM_PERIOD, x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(L"/", VK_OEM_2, x, ROW_SHIFT, K);
    x += K + KEY_SPACING;

    AddKey(
        L"SHIFT",
        VK_RSHIFT,
        x,
        ROW_SHIFT,
        125.0f
    );


    // ========================================================
    // BOTTOM ROW
    // ========================================================

    x = START_X;

    AddKey(
        L"CTRL",
        VK_LCONTROL,
        x,
        ROW_BOTTOM,
        80.0f
    );

    x += 86.0f;

    AddKey(
        L"WIN",
        VK_LWIN,
        x,
        ROW_BOTTOM,
        65.0f
    );

    x += 71.0f;

    AddKey(
        L"ALT",
        VK_LMENU,
        x,
        ROW_BOTTOM,
        65.0f
    );

    x += 71.0f;

    AddKey(
        L"SPACE",
        VK_SPACE,
        x,
        ROW_BOTTOM,
        365.0f
    );

    x += 371.0f;

    AddKey(
        L"ALT",
        VK_RMENU,
        x,
        ROW_BOTTOM,
        65.0f
    );

    x += 71.0f;

    AddKey(
        L"WIN",
        VK_RWIN,
        x,
        ROW_BOTTOM,
        65.0f
    );

    x += 71.0f;

    AddKey(
        L"MENU",
        VK_APPS,
        x,
        ROW_BOTTOM,
        65.0f
    );

    x += 71.0f;

    AddKey(
        L"CTRL",
        VK_RCONTROL,
        x,
        ROW_BOTTOM,
        80.0f
    );


    // ========================================================
    // NAVIGATION
    // ========================================================

    constexpr float NAV_X = 935.0f;
    constexpr float NAV_Y = 190.0f;
    constexpr float NAV_WIDTH = 72.0f;

    AddKey(L"INS", VK_INSERT, NAV_X, NAV_Y, NAV_WIDTH);

    AddKey(
        L"HOME",
        VK_HOME,
        NAV_X + 78.0f,
        NAV_Y,
        NAV_WIDTH
    );

    AddKey(
        L"PGUP",
        VK_PRIOR,
        NAV_X + 156.0f,
        NAV_Y,
        NAV_WIDTH
    );

    AddKey(
        L"DEL",
        VK_DELETE,
        NAV_X,
        NAV_Y + 60.0f,
        NAV_WIDTH
    );

    AddKey(
        L"END",
        VK_END,
        NAV_X + 78.0f,
        NAV_Y + 60.0f,
        NAV_WIDTH
    );

    AddKey(
        L"PGDN",
        VK_NEXT,
        NAV_X + 156.0f,
        NAV_Y + 60.0f,
        NAV_WIDTH
    );


    // ========================================================
    // ARROW KEYS
    // ========================================================

    constexpr float ARROW_X = 1013.0f;
    constexpr float ARROW_Y = 330.0f;
    constexpr float ARROW_WIDTH = 72.0f;

    AddKey(
        L"UP",
        VK_UP,
        ARROW_X,
        ARROW_Y,
        ARROW_WIDTH
    );

    AddKey(
        L"LEFT",
        VK_LEFT,
        ARROW_X - 78.0f,
        ARROW_Y + 60.0f,
        ARROW_WIDTH
    );

    AddKey(
        L"DOWN",
        VK_DOWN,
        ARROW_X,
        ARROW_Y + 60.0f,
        ARROW_WIDTH
    );

    AddKey(
        L"RIGHT",
        VK_RIGHT,
        ARROW_X + 78.0f,
        ARROW_Y + 60.0f,
        ARROW_WIDTH
    );
}


// ============================================================
// SOCIAL BUTTONS
// ============================================================

void CreateButtons()
{
    g_buttons.clear();


    SocialButton github{};

    github.name = L"Github";

    github.url =
        L"https://github.com/WallHackCheats";

    github.imageFile =
        L"github.png";

    github.width =
        BUTTON_WIDTH;

    github.height =
        BUTTON_HEIGHT;

    github.bitmap = nullptr;
    github.hovered = false;

    g_buttons.push_back(github);


    SocialButton discord{};

    discord.name = L"Discord";

    discord.url =
        L"https://x.com/TwistedGamingX1";

    discord.imageFile =
        L"discord.png";

    discord.width =
        BUTTON_WIDTH;

    discord.height =
        BUTTON_HEIGHT;

    discord.bitmap = nullptr;
    discord.hovered = false;

    g_buttons.push_back(discord);


    SocialButton youtube{};

    youtube.name = L"YouTube";

    youtube.url =
        L"https://www.youtube.com/@TwistedGamingOfficial1";

    youtube.imageFile =
        L"youtube.png";

    youtube.width =
        BUTTON_WIDTH;

    youtube.height =
        BUTTON_HEIGHT;

    youtube.bitmap = nullptr;
    youtube.hovered = false;

    g_buttons.push_back(youtube);
}


// ============================================================
// LOAD PNG
// ============================================================

HRESULT LoadPNG(
    const std::wstring& filename,
    ID2D1Bitmap** bitmap
)
{
    if (
        g_wicFactory == nullptr ||
        g_renderTarget == nullptr ||
        bitmap == nullptr
        )
    {
        return E_INVALIDARG;
    }

    *bitmap = nullptr;


    IWICBitmapDecoder* decoder = nullptr;
    IWICBitmapFrameDecode* frame = nullptr;
    IWICFormatConverter* converter = nullptr;


    HRESULT hr =
        g_wicFactory->CreateDecoderFromFilename(
            filename.c_str(),
            nullptr,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &decoder
        );

    if (FAILED(hr))
    {
        goto cleanup;
    }


    hr =
        decoder->GetFrame(
            0,
            &frame
        );

    if (FAILED(hr))
    {
        goto cleanup;
    }


    hr =
        g_wicFactory->CreateFormatConverter(
            &converter
        );

    if (FAILED(hr))
    {
        goto cleanup;
    }


    hr =
        converter->Initialize(
            frame,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            nullptr,
            0.0,
            WICBitmapPaletteTypeMedianCut
        );

    if (FAILED(hr))
    {
        goto cleanup;
    }


    hr =
        g_renderTarget->CreateBitmapFromWicBitmap(
            converter,
            nullptr,
            bitmap
        );


cleanup:

    SafeRelease(converter);
    SafeRelease(frame);
    SafeRelease(decoder);

    return hr;
}


// ============================================================
// LOAD BUTTON IMAGES
// ============================================================

void LoadButtonImages()
{
    std::wstring directory =
        GetApplicationDirectory();


    for (
        SocialButton& button :
        g_buttons
        )
    {
        std::wstring path =
            directory +
            L"\\" +
            button.imageFile;


        LoadPNG(
            path,
            &button.bitmap
        );
    }
}


// ============================================================
// CREATE DEVICE RESOURCES
// ============================================================

HRESULT CreateDeviceResources(
    HWND hwnd
)
{
    HRESULT hr = S_OK;


    if (!g_d2dFactory)
    {
        hr =
            D2D1CreateFactory(
                D2D1_FACTORY_TYPE_SINGLE_THREADED,
                &g_d2dFactory
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_writeFactory)
    {
        hr =
            DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown**>(
                    &g_writeFactory
                    )
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_wicFactory)
    {
        hr =
            CoCreateInstance(
                CLSID_WICImagingFactory,
                nullptr,
                CLSCTX_INPROC_SERVER,
                IID_PPV_ARGS(
                    &g_wicFactory
                )
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_renderTarget)
    {
        RECT rect{};
        GetClientRect(hwnd, &rect);

        UINT width =
            static_cast<UINT>(
                rect.right - rect.left
                );

        UINT height =
            static_cast<UINT>(
                rect.bottom - rect.top
                );

        if (width == 0 || height == 0)
        {
            return S_OK;
        }

        hr = g_d2dFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(
                    DXGI_FORMAT_B8G8R8A8_UNORM,
                    D2D1_ALPHA_MODE_IGNORE
                ),
                96.0f,
                96.0f,
                D2D1_RENDER_TARGET_USAGE_NONE,
                D2D1_FEATURE_LEVEL_DEFAULT
            ),
            D2D1::HwndRenderTargetProperties(
                hwnd,
                D2D1::SizeU(width, height),
                D2D1_PRESENT_OPTIONS_NONE
            ),
            &g_renderTarget
        );

        if (FAILED(hr))
        {
            return hr;
        }
    }



    // ========================================================
    // STANDARD BRUSHES
    // ========================================================

    if (!g_whiteBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    D2D1::ColorF::White
                ),
                &g_whiteBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_blackBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    D2D1::ColorF::Black
                ),
                &g_blackBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_buttonBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0x160F32,
                    0.72f
                ),
                &g_buttonBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_buttonHoverBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0xFFFFFF,
                    0.18f
                ),
                &g_buttonHoverBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    // ========================================================
    // MOUSE BRUSHES
    // ========================================================

    if (!g_mouseBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0x111827,
                    1.0f
                ),
                &g_mouseBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_mousePressedBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0x8B5CF6,
                    1.0f
                ),
                &g_mousePressedBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_mouseHoverBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0x293548,
                    1.0f
                ),
                &g_mouseHoverBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_mouseWheelBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0xA855F7,
                    1.0f
                ),
                &g_mouseWheelBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_mouseLineBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0xFFFFFF,
                    0.70f
                ),
                &g_mouseLineBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_scrollUpBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0x22C55E,
                    1.0f
                ),
                &g_scrollUpBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    if (!g_scrollDownBrush)
    {
        hr =
            g_renderTarget->CreateSolidColorBrush(
                D2D1::ColorF(
                    0xEF4444,
                    1.0f
                ),
                &g_scrollDownBrush
            );

        if (FAILED(hr))
        {
            return hr;
        }
    }


    // ========================================================
    // BACKGROUND
    // ========================================================

    if (!g_backgroundBrush)
    {
        ID2D1GradientStopCollection* gradientStops = nullptr;

        D2D1_GRADIENT_STOP stops[3];

        stops[0].position = 0.0f;
        stops[0].color = D2D1::ColorF(
            0x300642,
            1.0f
        );

        stops[1].position = 0.5f;
        stops[1].color = D2D1::ColorF(
            0x170A3B,
            1.0f
        );

        stops[2].position = 1.0f;
        stops[2].color = D2D1::ColorF(
            0x041936,
            1.0f
        );

        hr = g_renderTarget->CreateGradientStopCollection(
            stops,
            3,
            D2D1_GAMMA_2_2,
            D2D1_EXTEND_MODE_CLAMP,
            &gradientStops
        );

        if (FAILED(hr))
        {
            SafeRelease(gradientStops);
            return hr;
        }

        RECT rect{};
        GetClientRect(hwnd, &rect);

        const float width =
            static_cast<float>(rect.right - rect.left);

        const float height =
            static_cast<float>(rect.bottom - rect.top);

        hr = g_renderTarget->CreateLinearGradientBrush(
            D2D1::LinearGradientBrushProperties(
                D2D1::Point2F(0.0f, 0.0f),
                D2D1::Point2F(width, height)
            ),
            D2D1::BrushProperties(),
            gradientStops,
            &g_backgroundBrush
        );

        SafeRelease(gradientStops);

        if (FAILED(hr))
        {
            return hr;
        }
    }


    // ========================================================
    // FONT
    // ========================================================

    const wchar_t* fontName =
        g_customFontLoaded
        ? L"font"
        : L"Segoe UI";


    if (!g_keyTextFormat)
    {
        hr =
            g_writeFactory->CreateTextFormat(
                fontName,
                nullptr,
                DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                KEY_FONT_SIZE,
                L"en-us",
                &g_keyTextFormat
            );


        if (FAILED(hr))
        {
            return hr;
        }


        g_keyTextFormat->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT_CENTER
        );


        g_keyTextFormat->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT_CENTER
        );
    }


    if (!g_titleTextFormat)
    {
        hr =
            g_writeFactory->CreateTextFormat(
                fontName,
                nullptr,
                DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                TITLE_FONT_SIZE,
                L"en-us",
                &g_titleTextFormat
            );


        if (FAILED(hr))
        {
            return hr;
        }


        g_titleTextFormat->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT_CENTER
        );


        g_titleTextFormat->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT_CENTER
        );
    }


    if (!g_copyrightTextFormat)
    {
        hr =
            g_writeFactory->CreateTextFormat(
                fontName,
                nullptr,
                DWRITE_FONT_WEIGHT_NORMAL,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                COPYRIGHT_FONT_SIZE,
                L"en-us",
                &g_copyrightTextFormat
            );


        if (FAILED(hr))
        {
            return hr;
        }


        g_copyrightTextFormat->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT_TRAILING
        );


        g_copyrightTextFormat->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT_CENTER
        );
    }


    if (!g_buttonTextFormat)
    {
        hr =
            g_writeFactory->CreateTextFormat(
                fontName,
                nullptr,
                DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                BUTTON_FONT_SIZE,
                L"en-us",
                &g_buttonTextFormat
            );


        if (FAILED(hr))
        {
            return hr;
        }


        g_buttonTextFormat->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT_CENTER
        );


        g_buttonTextFormat->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT_CENTER
        );
    }


    // ========================================================
    // MOUSE LABEL FONT
    // ========================================================

    if (!g_mouseLabelTextFormat)
    {
        hr =
            g_writeFactory->CreateTextFormat(
                fontName,
                nullptr,
                DWRITE_FONT_WEIGHT_BOLD,
                DWRITE_FONT_STYLE_NORMAL,
                DWRITE_FONT_STRETCH_NORMAL,
                MOUSE_LABEL_FONT_SIZE,
                L"en-us",
                &g_mouseLabelTextFormat
            );


        if (FAILED(hr))
        {
            return hr;
        }


        g_mouseLabelTextFormat->SetTextAlignment(
            DWRITE_TEXT_ALIGNMENT_CENTER
        );


        g_mouseLabelTextFormat->SetParagraphAlignment(
            DWRITE_PARAGRAPH_ALIGNMENT_CENTER
        );
    }


    // ========================================================
    // LOAD IMAGES
    // ========================================================

    bool needsImages = false;


    for (
        const SocialButton& button :
        g_buttons
        )
    {
        if (!button.bitmap)
        {
            needsImages = true;
            break;
        }
    }


    if (needsImages)
    {
        LoadButtonImages();
    }


    return S_OK;
}


// ============================================================
// DISCARD DEVICE RESOURCES
// ============================================================

void DiscardDeviceResources()
{
    for (
        SocialButton& button :
        g_buttons
        )
    {
        SafeRelease(
            button.bitmap
        );
    }


    SafeRelease(g_backgroundBrush);

    SafeRelease(g_whiteBrush);
    SafeRelease(g_blackBrush);

    SafeRelease(g_buttonBrush);
    SafeRelease(g_buttonHoverBrush);

    SafeRelease(g_mouseBrush);
    SafeRelease(g_mousePressedBrush);
    SafeRelease(g_mouseHoverBrush);
    SafeRelease(g_mouseWheelBrush);
    SafeRelease(g_mouseLineBrush);
    SafeRelease(g_scrollUpBrush);
    SafeRelease(g_scrollDownBrush);

    SafeRelease(g_keyTextFormat);
    SafeRelease(g_titleTextFormat);
    SafeRelease(g_copyrightTextFormat);
    SafeRelease(g_buttonTextFormat);
    SafeRelease(g_mouseLabelTextFormat);

    SafeRelease(g_renderTarget);

    SafeRelease(g_wicFactory);

    SafeRelease(g_writeFactory);

    SafeRelease(g_d2dFactory);
}


// ============================================================
// DRAW TEXT
// ============================================================

void DrawTextCentered(
    const std::wstring& text,
    const D2D1_RECT_F& rect,
    IDWriteTextFormat* format,
    ID2D1Brush* brush
)
{
    if (
        !g_renderTarget ||
        !format ||
        !brush
        )
    {
        return;
    }


    g_renderTarget->DrawText(
        text.c_str(),
        static_cast<UINT32>(
            text.length()
            ),
        format,
        rect,
        brush
    );
}


// ============================================================
// DRAW KEY
// ============================================================

void DrawKey(
    const KeyboardKey& key
)
{
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(
            D2D1::RectF(
                key.x,
                key.y,
                key.x + key.width,
                key.y + KEY_HEIGHT
            ),
            KEY_RADIUS,
            KEY_RADIUS
        );


    D2D1_RECT_F textRect =
        D2D1::RectF(
            key.x,
            key.y,
            key.x + key.width,
            key.y + KEY_HEIGHT
        );


    if (key.pressed)
    {
        g_renderTarget->FillRoundedRectangle(
            roundedRect,
            g_whiteBrush
        );


        g_renderTarget->DrawRoundedRectangle(
            roundedRect,
            g_whiteBrush,
            BORDER_THICKNESS
        );


        DrawTextCentered(
            key.name,
            textRect,
            g_keyTextFormat,
            g_blackBrush
        );
    }
    else
    {
        g_renderTarget->DrawRoundedRectangle(
            roundedRect,
            g_whiteBrush,
            BORDER_THICKNESS
        );


        DrawTextCentered(
            key.name,
            textRect,
            g_keyTextFormat,
            g_whiteBrush
        );
    }
}


// ============================================================
// DRAW SOCIAL BUTTON
// ============================================================

void DrawSocialButton(
    const SocialButton& button
)
{
    D2D1_ROUNDED_RECT roundedRect =
        D2D1::RoundedRect(
            D2D1::RectF(
                button.x,
                button.y,
                button.x + button.width,
                button.y + button.height
            ),
            KEY_RADIUS,
            KEY_RADIUS
        );


    if (button.hovered)
    {
        g_renderTarget->FillRoundedRectangle(
            roundedRect,
            g_buttonHoverBrush
        );
    }
    else
    {
        g_renderTarget->FillRoundedRectangle(
            roundedRect,
            g_buttonBrush
        );
    }


    g_renderTarget->DrawRoundedRectangle(
        roundedRect,
        g_whiteBrush,
        BORDER_THICKNESS
    );


    if (button.bitmap)
    {
        constexpr float ICON_SIZE = 28.0f;
        constexpr float ICON_MARGIN = 9.0f;


        D2D1_RECT_F iconRect =
            D2D1::RectF(
                button.x + ICON_MARGIN,
                button.y +
                (button.height -
                    ICON_SIZE) /
                2.0f,

                button.x +
                ICON_MARGIN +
                ICON_SIZE,

                button.y +
                (button.height -
                    ICON_SIZE) /
                2.0f +
                ICON_SIZE
            );


        g_renderTarget->DrawBitmap(
            button.bitmap,
            iconRect,
            1.0f,
            D2D1_BITMAP_INTERPOLATION_MODE_LINEAR
        );
    }


    D2D1_RECT_F textRect =
        D2D1::RectF(
            button.x + 42.0f,
            button.y,
            button.x +
            button.width -
            8.0f,
            button.y +
            button.height
        );


    DrawTextCentered(
        button.name,
        textRect,
        g_buttonTextFormat,
        g_whiteBrush
    );
}


// ============================================================
// DRAW SOCIAL BUTTONS
// ============================================================

void DrawSocialButtons()
{
    for (
        const SocialButton& button :
        g_buttons
        )
    {
        DrawSocialButton(
            button
        );
    }
}


// ============================================================
// DRAW COPYRIGHT
// ============================================================

void DrawCopyright()
{
    D2D1_SIZE_F size =
        g_renderTarget->GetSize();


    constexpr float RIGHT_MARGIN = 22.0f;
    constexpr float BOTTOM_MARGIN = 12.0f;
    constexpr float COPYRIGHT_HEIGHT = 30.0f;


    D2D1_RECT_F rect =
        D2D1::RectF(
            500.0f,

            size.height -
            COPYRIGHT_HEIGHT -
            BOTTOM_MARGIN,

            size.width -
            RIGHT_MARGIN,

            size.height -
            BOTTOM_MARGIN
        );


    const std::wstring text =
        L"WallHack \x00A9 2026 - Twisted0097";


    DrawTextCentered(
        text,
        rect,
        g_copyrightTextFormat,
        g_whiteBrush
    );
}


// ============================================================
// MOUSE HELPER
// ============================================================

bool MouseFlashActive(
    DWORD time
)
{
    return
        time != 0 &&
        (GetTickCount() - time) <
        MOUSE_FLASH_TIME;
}


// ============================================================
// DRAW LINE + LABEL
// ============================================================

void DrawMouseLabelLine(
    const D2D1_POINT_2F& start,
    const D2D1_POINT_2F& end,
    const std::wstring& text,
    const D2D1_RECT_F& textRect
)
{
    g_renderTarget->DrawLine(
        start,
        end,
        g_mouseLineBrush,
        1.5f
    );


    // Small endpoint dot.

    g_renderTarget->FillEllipse(
        D2D1::Ellipse(
            end,
            3.0f,
            3.0f
        ),
        g_mouseLineBrush
    );


    DrawTextCentered(
        text,
        textRect,
        g_mouseLabelTextFormat,
        g_whiteBrush
    );
}


// ============================================================
// DRAW ARROW
// ============================================================

void DrawArrow(
    D2D1_POINT_2F from,
    D2D1_POINT_2F to,
    ID2D1Brush* brush
)
{
    g_renderTarget->DrawLine(
        from,
        to,
        brush,
        3.0f
    );


    float dx =
        to.x - from.x;

    float dy =
        to.y - from.y;

    float length =
        std::sqrt(
            dx * dx +
            dy * dy
        );

    if (length <= 0.0f)
    {
        return;
    }


    dx /= length;
    dy /= length;


    float px = -dy;
    float py = dx;


    constexpr float ARROW_SIZE = 9.0f;


    D2D1_POINT_2F p1 =
        D2D1::Point2F(
            to.x -
            dx * ARROW_SIZE +
            px * 5.0f,

            to.y -
            dy * ARROW_SIZE +
            py * 5.0f
        );


    D2D1_POINT_2F p2 =
        D2D1::Point2F(
            to.x -
            dx * ARROW_SIZE -
            px * 5.0f,

            to.y -
            dy * ARROW_SIZE -
            py * 5.0f
        );


    g_renderTarget->DrawLine(
        p1,
        to,
        brush,
        3.0f
    );


    g_renderTarget->DrawLine(
        p2,
        to,
        brush,
        3.0f
    );
}


// ============================================================
// DRAW GAMING MOUSE
// ============================================================

void DrawGamingMouse()
{
    const float x = MOUSE_X;
    const float y = MOUSE_Y;


    // ========================================================
    // OUTER MOUSE BODY
    // ========================================================

    D2D1_ROUNDED_RECT body =
        D2D1::RoundedRect(
            D2D1::RectF(
                x,
                y,
                x + MOUSE_WIDTH,
                y + MOUSE_HEIGHT
            ),
            MOUSE_RADIUS,
            MOUSE_RADIUS
        );


    g_renderTarget->FillRoundedRectangle(
        body,
        g_mouseBrush
    );


    g_renderTarget->DrawRoundedRectangle(
        body,
        g_whiteBrush,
        2.5f
    );


    // ========================================================
    // LEFT CLICK
    // ========================================================

    D2D1_RECT_F leftButton =
        D2D1::RectF(
            x + 8.0f,
            y + 8.0f,
            x + MOUSE_WIDTH / 2.0f - 2.0f,
            y + MOUSE_BUTTON_HEIGHT
        );


    if (g_mouse.leftPressed)
    {
        g_renderTarget->FillRoundedRectangle(
            D2D1::RoundedRect(
                leftButton,
                22.0f,
                22.0f
            ),
            g_mousePressedBrush
        );
    }


    g_renderTarget->DrawRoundedRectangle(
        D2D1::RoundedRect(
            leftButton,
            22.0f,
            22.0f
        ),
        g_whiteBrush,
        1.5f
    );


    // ========================================================
    // RIGHT CLICK
    // ========================================================

    D2D1_RECT_F rightButton =
        D2D1::RectF(
            x + MOUSE_WIDTH / 2.0f + 2.0f,
            y + 8.0f,
            x + MOUSE_WIDTH - 8.0f,
            y + MOUSE_BUTTON_HEIGHT
        );


    if (g_mouse.rightPressed)
    {
        g_renderTarget->FillRoundedRectangle(
            D2D1::RoundedRect(
                rightButton,
                22.0f,
                22.0f
            ),
            g_mousePressedBrush
        );
    }


    g_renderTarget->DrawRoundedRectangle(
        D2D1::RoundedRect(
            rightButton,
            22.0f,
            22.0f
        ),
        g_whiteBrush,
        1.5f
    );


    // ========================================================
    // CENTER DIVIDER
    // ========================================================

    g_renderTarget->DrawLine(
        D2D1::Point2F(
            x + MOUSE_WIDTH / 2.0f,
            y + 10.0f
        ),

        D2D1::Point2F(
            x + MOUSE_WIDTH / 2.0f,
            y + MOUSE_BUTTON_HEIGHT
        ),

        g_whiteBrush,
        1.5f
    );


    // ========================================================
    // SCROLL WHEEL
    // ========================================================

    float wheelX =
        x +
        MOUSE_WIDTH / 2.0f -
        MOUSE_WHEEL_WIDTH / 2.0f;


    float wheelY =
        y + 54.0f;


    D2D1_ROUNDED_RECT wheel =
        D2D1::RoundedRect(
            D2D1::RectF(
                wheelX,
                wheelY,
                wheelX + MOUSE_WHEEL_WIDTH,
                wheelY + MOUSE_WHEEL_HEIGHT
            ),
            12.0f,
            12.0f
        );


    if (g_mouse.middlePressed)
    {
        g_renderTarget->FillRoundedRectangle(
            wheel,
            g_mousePressedBrush
        );
    }
    else
    {
        g_renderTarget->FillRoundedRectangle(
            wheel,
            g_mouseWheelBrush
        );
    }


    g_renderTarget->DrawRoundedRectangle(
        wheel,
        g_whiteBrush,
        1.5f
    );


    // Wheel grooves.

    for (int i = 0; i < 5; ++i)
    {
        float grooveY =
            wheelY +
            10.0f +
            static_cast<float>(i) * 9.0f;


        g_renderTarget->DrawLine(
            D2D1::Point2F(
                wheelX + 7.0f,
                grooveY
            ),

            D2D1::Point2F(
                wheelX + MOUSE_WHEEL_WIDTH - 7.0f,
                grooveY
            ),

            g_whiteBrush,
            1.0f
        );
    }


    // ========================================================
    // SIDE BUTTONS
    // ========================================================

    float sideX =
        x - MOUSE_SIDE_WIDTH - 8.0f;


    float sideY1 =
        y + 120.0f;


    float sideY2 =
        y + 205.0f;


    D2D1_ROUNDED_RECT sideBack =
        D2D1::RoundedRect(
            D2D1::RectF(
                sideX,
                sideY1,
                sideX + MOUSE_SIDE_WIDTH,
                sideY1 + MOUSE_SIDE_HEIGHT
            ),
            10.0f,
            10.0f
        );


    D2D1_ROUNDED_RECT sideForward =
        D2D1::RoundedRect(
            D2D1::RectF(
                sideX,
                sideY2,
                sideX + MOUSE_SIDE_WIDTH,
                sideY2 + MOUSE_SIDE_HEIGHT
            ),
            10.0f,
            10.0f
        );


    if (g_mouse.sideBackPressed)
    {
        g_renderTarget->FillRoundedRectangle(
            sideBack,
            g_mousePressedBrush
        );
    }


    if (g_mouse.sideForwardPressed)
    {
        g_renderTarget->FillRoundedRectangle(
            sideForward,
            g_mousePressedBrush
        );
    }


    g_renderTarget->DrawRoundedRectangle(
        sideBack,
        g_whiteBrush,
        1.5f
    );


    g_renderTarget->DrawRoundedRectangle(
        sideForward,
        g_whiteBrush,
        1.5f
    );


    // ========================================================
    // SCROLL ARROWS
    // ========================================================

    float arrowX =
        x + MOUSE_WIDTH + 48.0f;


    float arrowCenterY =
        y + 150.0f;


    bool upActive =
        MouseFlashActive(
            g_mouse.scrollUpTime
        );


    bool downActive =
        MouseFlashActive(
            g_mouse.scrollDownTime
        );


    // UP

    ID2D1Brush* upBrush =
        upActive
        ? g_scrollUpBrush
        : g_mouseLineBrush;


    DrawArrow(
        D2D1::Point2F(
            arrowX,
            arrowCenterY + 18.0f
        ),

        D2D1::Point2F(
            arrowX,
            arrowCenterY - 18.0f
        ),

        upBrush
    );


    // DOWN

    ID2D1Brush* downBrush =
        downActive
        ? g_scrollDownBrush
        : g_mouseLineBrush;


    DrawArrow(
        D2D1::Point2F(
            arrowX,
            arrowCenterY + 55.0f
        ),

        D2D1::Point2F(
            arrowX,
            arrowCenterY + 91.0f
        ),

        downBrush
    );


    // Scroll text.

    DrawTextCentered(
        L"SCROLL UP",
        D2D1::RectF(
            arrowX - 55.0f,
            arrowCenterY - 60.0f,
            arrowX + 55.0f,
            arrowCenterY - 38.0f
        ),
        g_mouseLabelTextFormat,
        upBrush
    );


    DrawTextCentered(
        L"SCROLL DOWN",
        D2D1::RectF(
            arrowX - 65.0f,
            arrowCenterY + 100.0f,
            arrowX + 65.0f,
            arrowCenterY + 122.0f
        ),
        g_mouseLabelTextFormat,
        downBrush
    );


    // ========================================================
    // POINTER LABELS
    // ========================================================

    // LEFT CLICK

    DrawMouseLabelLine(
        D2D1::Point2F(
            x + 35.0f,
            y + 55.0f
        ),

        D2D1::Point2F(
            x - 55.0f,
            y + 35.0f
        ),

        L"LEFT CLICK",

        D2D1::RectF(
            x - 180.0f,
            y + 10.0f,
            x - 60.0f,
            y + 55.0f
        )
    );


    // RIGHT CLICK

    DrawMouseLabelLine(
        D2D1::Point2F(
            x + MOUSE_WIDTH - 35.0f,
            y + 55.0f
        ),

        D2D1::Point2F(
            x + MOUSE_WIDTH + 125.0f,
            y + 35.0f
        ),

        L"RIGHT CLICK",

        D2D1::RectF(
            x + MOUSE_WIDTH + 80.0f,
            y + 10.0f,
            x + MOUSE_WIDTH + 205.0f,
            y + 55.0f
        )
    );


    // MIDDLE CLICK

    DrawMouseLabelLine(
        D2D1::Point2F(
            wheelX + MOUSE_WHEEL_WIDTH / 2.0f,
            wheelY + MOUSE_WHEEL_HEIGHT
        ),

        D2D1::Point2F(
            x + MOUSE_WIDTH / 2.0f,
            y + 205.0f
        ),

        L"MIDDLE CLICK",

        D2D1::RectF(
            x + MOUSE_WIDTH / 2.0f - 65.0f,
            y + 210.0f,
            x + MOUSE_WIDTH / 2.0f + 65.0f,
            y + 235.0f
        )
    );


    // SIDE BUTTON 1

    DrawMouseLabelLine(
        D2D1::Point2F(
            sideX,
            sideY1 + MOUSE_SIDE_HEIGHT / 2.0f
        ),

        D2D1::Point2F(
            x - 105.0f,
            sideY1 + MOUSE_SIDE_HEIGHT / 2.0f
        ),

        L"SIDE BUTTON 1",

        D2D1::RectF(
            x - 250.0f,
            sideY1 + 12.0f,
            x - 110.0f,
            sideY1 + 52.0f
        )
    );


    // SIDE BUTTON 2

    DrawMouseLabelLine(
        D2D1::Point2F(
            sideX,
            sideY2 + MOUSE_SIDE_HEIGHT / 2.0f
        ),

        D2D1::Point2F(
            x - 105.0f,
            sideY2 + MOUSE_SIDE_HEIGHT / 2.0f
        ),

        L"SIDE BUTTON 2",

        D2D1::RectF(
            x - 250.0f,
            sideY2 + 12.0f,
            x - 110.0f,
            sideY2 + 52.0f
        )
    );


    // ========================================================
    // MOUSE TITLE
    // ========================================================

    DrawTextCentered(
        L"GAMING MOUSE",
        D2D1::RectF(
            x - 100.0f,
            y + MOUSE_HEIGHT + 10.0f,
            x + MOUSE_WIDTH + 100.0f,
            y + MOUSE_HEIGHT + 45.0f
        ),
        g_mouseLabelTextFormat,
        g_whiteBrush
    );


    // ========================================================
    // MOUSE STATUS
    // ========================================================

    std::wstring status;

    if (g_mouse.leftPressed)
        status = L"LEFT CLICK";

    else if (g_mouse.rightPressed)
        status = L"RIGHT CLICK";

    else if (g_mouse.middlePressed)
        status = L"MIDDLE CLICK";

    else if (g_mouse.sideBackPressed)
        status = L"SIDE BUTTON 1";

    else if (g_mouse.sideForwardPressed)
        status = L"SIDE BUTTON 2";

    else if (upActive)
        status = L"SCROLLING UP";

    else if (downActive)
        status = L"SCROLLING DOWN";

    else
        status = L"READY";


    DrawTextCentered(
        status,
        D2D1::RectF(
            x - 30.0f,
            y + MOUSE_HEIGHT + 48.0f,
            x + MOUSE_WIDTH + 30.0f,
            y + MOUSE_HEIGHT + 78.0f
        ),
        g_mouseLabelTextFormat,
        g_whiteBrush
    );
}


// ============================================================
// UPDATE SOCIAL BUTTON POSITIONS
// ============================================================

void UpdateButtonPositions()
{
    if (!g_renderTarget ||
        !g_backgroundBrush ||
        !g_whiteBrush ||
        !g_blackBrush)
    {
        return;
    }



    D2D1_SIZE_F size =
        g_renderTarget->GetSize();


    float y =
        size.height -
        BUTTON_HEIGHT -
        BUTTON_MARGIN_BOTTOM;


    for (
        size_t i = 0;
        i < g_buttons.size();
        ++i
        )
    {
        g_buttons[i].x =
            BUTTON_MARGIN_LEFT +
            static_cast<float>(i) *
            (BUTTON_WIDTH +
                BUTTON_SPACING);


        g_buttons[i].y =
            y;


        g_buttons[i].width =
            BUTTON_WIDTH;


        g_buttons[i].height =
            BUTTON_HEIGHT;
    }
}


// ============================================================
// RENDER
// ============================================================

void Render()
{
    if (!g_renderTarget)
    {
        return;
    }


    UpdateButtonPositions();


    g_renderTarget->BeginDraw();


    D2D1_SIZE_F size =
        g_renderTarget->GetSize();


    g_renderTarget->FillRectangle(
        D2D1::RectF(
            0.0f,
            0.0f,
            size.width,
            size.height
        ),
        g_backgroundBrush
    );


    // ========================================================
    // TITLE
    // ========================================================

    DrawTextCentered(
        L"KeyChecker",

        D2D1::RectF(
            0.0f,
            35.0f,
            1150.0f,
            95.0f
        ),

        g_titleTextFormat,

        g_whiteBrush
    );


    // ========================================================
    // KEYBOARD
    // ========================================================

    for (
        const KeyboardKey& key :
        g_keyboard
        )
    {
        DrawKey(key);
    }


    // ========================================================
    // GAMING MOUSE
    // ========================================================

    DrawGamingMouse();


    // ========================================================
    // SOCIAL BUTTONS
    // ========================================================

    DrawSocialButtons();


    // ========================================================
    // COPYRIGHT
    // ========================================================

    DrawCopyright();


    HRESULT hr =
        g_renderTarget->EndDraw();


    if (
        hr ==
        D2DERR_RECREATE_TARGET
        )
    {
        DiscardDeviceResources();
    }


    // Keep scroll indicators updating.

    if (
        MouseFlashActive(
            g_mouse.scrollUpTime
        ) ||
        MouseFlashActive(
            g_mouse.scrollDownTime
        )
        )
    {
        InvalidateRect(
            g_hWnd,
            nullptr,
            FALSE
        );
    }
}


// ============================================================
// KEY STATE
// ============================================================

void SetKeyPressed(
    int virtualKey,
    bool pressed
)
{
    bool changed = false;


    for (
        KeyboardKey& key :
        g_keyboard
        )
    {
        if (
            key.virtualKey ==
            virtualKey
            )
        {
            if (
                key.pressed !=
                pressed
                )
            {
                key.pressed =
                    pressed;

                changed = true;
            }
        }
    }


    if (changed)
    {
        InvalidateRect(
            g_hWnd,
            nullptr,
            FALSE
        );
    }
}


// ============================================================
// BUTTON HIT TEST
// ============================================================

int GetButtonAtPoint(
    float x,
    float y
)
{
    for (
        size_t i = 0;
        i < g_buttons.size();
        ++i
        )
    {
        const SocialButton& button =
            g_buttons[i];


        if (
            x >= button.x &&
            x <=
            button.x +
            button.width &&

            y >= button.y &&
            y <=
            button.y +
            button.height
            )
        {
            return static_cast<int>(
                i
                );
        }
    }


    return -1;
}


// ============================================================
// BUTTON HOVER
// ============================================================

void UpdateButtonHover(
    int mouseX,
    int mouseY
)
{
    bool changed = false;


    for (
        SocialButton& button :
        g_buttons
        )
    {
        bool hovered =
            mouseX >= button.x &&
            mouseX <=
            button.x +
            button.width &&

            mouseY >= button.y &&
            mouseY <=
            button.y +
            button.height;


        if (
            hovered !=
            button.hovered
            )
        {
            button.hovered =
                hovered;

            changed = true;
        }
    }


    if (changed)
    {
        InvalidateRect(
            g_hWnd,
            nullptr,
            FALSE
        );
    }


    bool anyHovered = false;


    for (
        const SocialButton& button :
        g_buttons
        )
    {
        if (button.hovered)
        {
            anyHovered = true;
            break;
        }
    }


    SetCursor(
        LoadCursorW(
            nullptr,
            anyHovered
            ? IDC_HAND
            : IDC_ARROW
        )
    );
}


// ============================================================
// OPEN SOCIAL URL
// ============================================================

void OpenButtonURL(
    int buttonIndex
)
{
    if (
        buttonIndex < 0 ||
        buttonIndex >=
        static_cast<int>(
            g_buttons.size()
            )
        )
    {
        return;
    }


    const std::wstring& url =
        g_buttons[
            buttonIndex
        ].url;


    ShellExecuteW(
        nullptr,
        L"open",
        url.c_str(),
        nullptr,
        nullptr,
        SW_SHOWNORMAL
    );
}


// ============================================================
// MOUSE STATE INVALIDATION
// ============================================================

void UpdateMouseDisplay()
{
    InvalidateRect(
        g_hWnd,
        nullptr,
        FALSE
    );
}


// ============================================================
// WINDOW PROCEDURE
// ============================================================

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {
        // ========================================================
        // CREATE
        // ========================================================

    case WM_CREATE:
    {
        g_hWnd = hwnd;

        CreateKeyboard();
        CreateButtons();

        return 0;
    }


    // ========================================================
    // SIZE
    // ========================================================

    case WM_SIZE:
    {
        if (g_renderTarget)
        {
            UINT width = LOWORD(lParam);
            UINT height = HIWORD(lParam);

            if (width > 0 && height > 0)
            {
                HRESULT hr = g_renderTarget->Resize(
                    D2D1::SizeU(width, height)
                );

                if (hr == D2DERR_RECREATE_TARGET)
                {
                    DiscardDeviceResources();
                }
            }
        }

        InvalidateRect(
            hwnd,
            nullptr,
            FALSE
        );

        return 0;
    }



    // ========================================================
    // MOUSE MOVE
    // ========================================================

    case WM_MOUSEMOVE:
    {
        int mouseX =
            GET_X_LPARAM(lParam);

        int mouseY =
            GET_Y_LPARAM(lParam);


        UpdateButtonHover(
            mouseX,
            mouseY
        );


        TRACKMOUSEEVENT tme{};

        tme.cbSize =
            sizeof(TRACKMOUSEEVENT);

        tme.dwFlags =
            TME_LEAVE;

        tme.hwndTrack =
            hwnd;


        TrackMouseEvent(
            &tme
        );


        return 0;
    }


    // ========================================================
    // MOUSE LEAVE
    // ========================================================

    case WM_MOUSELEAVE:
    {
        bool changed = false;


        for (
            SocialButton& button :
            g_buttons
            )
        {
            if (button.hovered)
            {
                button.hovered =
                    false;

                changed = true;
            }
        }


        if (changed)
        {
            InvalidateRect(
                hwnd,
                nullptr,
                FALSE
            );
        }


        SetCursor(
            LoadCursorW(
                nullptr,
                IDC_ARROW
            )
        );


        return 0;
    }


    // ========================================================
    // LEFT MOUSE BUTTON DOWN
    // ========================================================

    case WM_LBUTTONDOWN:
    {
        g_mouse.leftPressed = true;

        SetCapture(hwnd);

        UpdateMouseDisplay();

        return 0;
    }


    // ========================================================
    // LEFT MOUSE BUTTON UP
    // ========================================================

    case WM_LBUTTONUP:
    {
        g_mouse.leftPressed = false;

        if (GetCapture() == hwnd)
        {
            ReleaseCapture();
        }


        int mouseX =
            GET_X_LPARAM(lParam);

        int mouseY =
            GET_Y_LPARAM(lParam);


        int buttonIndex =
            GetButtonAtPoint(
                static_cast<float>(
                    mouseX
                    ),

                static_cast<float>(
                    mouseY
                    )
            );


        if (buttonIndex >= 0)
        {
            OpenButtonURL(
                buttonIndex
            );
        }


        UpdateMouseDisplay();

        return 0;
    }


    // ========================================================
    // RIGHT MOUSE BUTTON DOWN
    // ========================================================

    case WM_RBUTTONDOWN:
    {
        g_mouse.rightPressed = true;

        SetCapture(hwnd);

        UpdateMouseDisplay();

        return 0;
    }


    // ========================================================
    // RIGHT MOUSE BUTTON UP
    // ========================================================

    case WM_RBUTTONUP:
    {
        g_mouse.rightPressed = false;

        if (GetCapture() == hwnd)
        {
            ReleaseCapture();
        }

        UpdateMouseDisplay();

        return 0;
    }


    // ========================================================
    // MIDDLE / SCROLL WHEEL CLICK DOWN
    // ========================================================

    case WM_MBUTTONDOWN:
    {
        g_mouse.middlePressed = true;

        SetCapture(hwnd);

        UpdateMouseDisplay();

        return 0;
    }


    // ========================================================
    // MIDDLE / SCROLL WHEEL CLICK UP
    // ========================================================

    case WM_MBUTTONUP:
    {
        g_mouse.middlePressed = false;

        if (GetCapture() == hwnd)
        {
            ReleaseCapture();
        }

        UpdateMouseDisplay();

        return 0;
    }


    // ========================================================
    // SIDE MOUSE BUTTONS
    // ========================================================

    case WM_XBUTTONDOWN:
    {
        WORD button =
            GET_XBUTTON_WPARAM(wParam);


        if (
            button ==
            XBUTTON1
            )
        {
            g_mouse.sideBackPressed =
                true;
        }
        else if (
            button ==
            XBUTTON2
            )
        {
            g_mouse.sideForwardPressed =
                true;
        }


        SetCapture(hwnd);

        UpdateMouseDisplay();

        return TRUE;
    }


    // ========================================================
    // SIDE MOUSE BUTTON UP
    // ========================================================

    case WM_XBUTTONUP:
    {
        WORD button =
            GET_XBUTTON_WPARAM(wParam);


        if (
            button ==
            XBUTTON1
            )
        {
            g_mouse.sideBackPressed =
                false;
        }
        else if (
            button ==
            XBUTTON2
            )
        {
            g_mouse.sideForwardPressed =
                false;
        }


        if (GetCapture() == hwnd)
        {
            ReleaseCapture();
        }


        UpdateMouseDisplay();

        return TRUE;
    }


    // ========================================================
    // MOUSE WHEEL
    // ========================================================

    case WM_MOUSEWHEEL:
    {
        short delta =
            GET_WHEEL_DELTA_WPARAM(
                wParam
            );


        if (delta > 0)
        {
            g_mouse.scrollUp = true;
            g_mouse.scrollDown = false;

            g_mouse.scrollUpTime =
                GetTickCount();

            g_mouse.scrollDownTime = 0;
        }
        else if (delta < 0)
        {
            g_mouse.scrollDown = true;
            g_mouse.scrollUp = false;

            g_mouse.scrollDownTime =
                GetTickCount();

            g_mouse.scrollUpTime = 0;
        }


        UpdateMouseDisplay();

        return 0;
    }


    // ========================================================
    // KEY DOWN
    // ========================================================

    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    {
        if (
            (lParam &
                (1 << 30)) == 0
            )
        {
            int virtualKey =
                static_cast<int>(
                    wParam
                    );


            if (
                virtualKey ==
                VK_SHIFT
                )
            {
                if (
                    GetKeyState(
                        VK_LSHIFT
                    ) & 0x8000
                    )
                {
                    SetKeyPressed(
                        VK_LSHIFT,
                        true
                    );
                }


                if (
                    GetKeyState(
                        VK_RSHIFT
                    ) & 0x8000
                    )
                {
                    SetKeyPressed(
                        VK_RSHIFT,
                        true
                    );
                }
            }


            else if (
                virtualKey ==
                VK_CONTROL
                )
            {
                if (
                    GetKeyState(
                        VK_LCONTROL
                    ) & 0x8000
                    )
                {
                    SetKeyPressed(
                        VK_LCONTROL,
                        true
                    );
                }


                if (
                    GetKeyState(
                        VK_RCONTROL
                    ) & 0x8000
                    )
                {
                    SetKeyPressed(
                        VK_RCONTROL,
                        true
                    );
                }
            }


            else if (
                virtualKey ==
                VK_MENU
                )
            {
                if (
                    GetKeyState(
                        VK_LMENU
                    ) & 0x8000
                    )
                {
                    SetKeyPressed(
                        VK_LMENU,
                        true
                    );
                }


                if (
                    GetKeyState(
                        VK_RMENU
                    ) & 0x8000
                    )
                {
                    SetKeyPressed(
                        VK_RMENU,
                        true
                    );
                }
            }


            else
            {
                SetKeyPressed(
                    virtualKey,
                    true
                );
            }
        }


        return 0;
    }


    // ========================================================
    // KEY UP
    // ========================================================

    case WM_KEYUP:
    case WM_SYSKEYUP:
    {
        int virtualKey =
            static_cast<int>(
                wParam
                );


        if (
            virtualKey ==
            VK_SHIFT
            )
        {
            SetKeyPressed(
                VK_LSHIFT,
                false
            );

            SetKeyPressed(
                VK_RSHIFT,
                false
            );
        }

        else if (
            virtualKey ==
            VK_CONTROL
            )
        {
            SetKeyPressed(
                VK_LCONTROL,
                false
            );

            SetKeyPressed(
                VK_RCONTROL,
                false
            );
        }

        else if (
            virtualKey ==
            VK_MENU
            )
        {
            SetKeyPressed(
                VK_LMENU,
                false
            );

            SetKeyPressed(
                VK_RMENU,
                false
            );
        }

        else
        {
            SetKeyPressed(
                virtualKey,
                false
            );
        }


        return 0;
    }


    // ========================================================
    // PAINT
    // ========================================================

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};


        BeginPaint(
            hwnd,
            &ps
        );


        if (
            SUCCEEDED(
                CreateDeviceResources(
                    hwnd
                )
            )
            )
        {
            Render();
        }


        EndPaint(
            hwnd,
            &ps
        );


        return 0;
    }


    // ========================================================
    // DESTROY
    // ========================================================

    case WM_DESTROY:
    {
        PostQuitMessage(0);

        return 0;
    }
    }


    return DefWindowProcW(
        hwnd,
        message,
        wParam,
        lParam
    );
}


// ============================================================
// ENTRY POINT
// ============================================================

int WINAPI wWinMain(
    HINSTANCE hInstance,
    HINSTANCE,
    PWSTR,
    int nCmdShow
)
{
    HRESULT comResult =
        CoInitializeEx(
            nullptr,
            COINIT_APARTMENTTHREADED
        );


    bool comInitialized =
        SUCCEEDED(
            comResult
        );


    LoadCustomFont();


    const wchar_t CLASS_NAME[] =
        L"KeyCheckerWindow";


    WNDCLASSEXW wc{};

    wc.cbSize =
        sizeof(WNDCLASSEXW);

    wc.style =
        CS_HREDRAW |
        CS_VREDRAW;

    wc.lpfnWndProc =
        WindowProc;

    wc.hInstance =
        hInstance;

    wc.hCursor =
        LoadCursorW(
            nullptr,
            IDC_ARROW
        );

    wc.hbrBackground =
        nullptr;

    wc.lpszClassName =
        CLASS_NAME;


    if (
        !RegisterClassExW(&wc)
        )
    {
        MessageBoxW(
            nullptr,
            L"Failed to register the KeyChecker window class.",
            L"KeyChecker Error",
            MB_ICONERROR
        );


        UnloadCustomFont();


        if (comInitialized)
        {
            CoUninitialize();
        }


        return 1;
    }


    // ========================================================
    // LARGER WINDOW FOR MOUSE
    // ========================================================

    HWND hwnd =
        CreateWindowExW(
            0,

            CLASS_NAME,

            L"KeyChecker",

            WS_OVERLAPPEDWINDOW,

            CW_USEDEFAULT,
            CW_USEDEFAULT,

            1550,
            700,

            nullptr,
            nullptr,
            hInstance,
            nullptr
        );


    if (!hwnd)
    {
        MessageBoxW(
            nullptr,
            L"Failed to create the KeyChecker window.",
            L"KeyChecker Error",
            MB_ICONERROR
        );


        UnloadCustomFont();


        if (comInitialized)
        {
            CoUninitialize();
        }


        return 1;
    }


    g_hWnd = hwnd;


    ShowWindow(
        hwnd,
        nCmdShow
    );


    UpdateWindow(
        hwnd
    );


    MSG msg{};


    while (
        GetMessageW(
            &msg,
            nullptr,
            0,
            0
        ) > 0
        )
    {
        TranslateMessage(
            &msg
        );


        DispatchMessageW(
            &msg
        );
    }


    DiscardDeviceResources();

    SafeRelease(g_wicFactory);
    SafeRelease(g_writeFactory);
    SafeRelease(g_d2dFactory);

    UnloadCustomFont();



    if (comInitialized)
    {
        CoUninitialize();
    }


    return static_cast<int>(
        msg.wParam
        );
}
