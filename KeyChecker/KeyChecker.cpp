// ============================================================
// KeyChecker
// Native Windows C++ Desktop Application
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

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "shell32.lib")




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




HWND g_hWnd = nullptr;




ID2D1Factory* g_d2dFactory = nullptr;

ID2D1HwndRenderTarget* g_renderTarget = nullptr;

ID2D1SolidColorBrush* g_whiteBrush = nullptr;
ID2D1SolidColorBrush* g_blackBrush = nullptr;

ID2D1SolidColorBrush* g_buttonBrush = nullptr;
ID2D1SolidColorBrush* g_buttonHoverBrush = nullptr;

ID2D1LinearGradientBrush* g_backgroundBrush = nullptr;




IDWriteFactory* g_writeFactory = nullptr;

IDWriteTextFormat* g_keyTextFormat = nullptr;
IDWriteTextFormat* g_titleTextFormat = nullptr;
IDWriteTextFormat* g_copyrightTextFormat = nullptr;
IDWriteTextFormat* g_buttonTextFormat = nullptr;




IWICImagingFactory* g_wicFactory = nullptr;




std::vector<KeyboardKey> g_keyboard;
std::vector<SocialButton> g_buttons;




bool g_customFontLoaded = false;

std::wstring g_customFontPath;




template <typename T>
void SafeRelease(T*& object)
{
    if (object != nullptr)
    {
        object->Release();
        object = nullptr;
    }
}




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

    // Backslash is kept in its own correct position.

    AddKey(
        L"\\",
        VK_OEM_5,
        x,
        ROW_QWERTY,
        82.0f
    );




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




    constexpr float NAV_X = 935.0f;
    constexpr float NAV_Y = 190.0f;
    constexpr float NAV_WIDTH = 72.0f;

    AddKey(
        L"INS",
        VK_INSERT,
        NAV_X,
        NAV_Y,
        NAV_WIDTH
    );

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

    // LEFT is explicitly placed to the LEFT
    // of DOWN instead of underneath CTRL.

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

    g_buttons.push_back(
        github
    );


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

    g_buttons.push_back(
        discord
    );


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

    g_buttons.push_back(
        youtube
    );
}



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

        GetClientRect(
            hwnd,
            &rect
        );


        UINT width =
            static_cast<UINT>(
                rect.right -
                rect.left
                );


        UINT height =
            static_cast<UINT>(
                rect.bottom -
                rect.top
                );


        hr =
            g_d2dFactory->CreateHwndRenderTarget(
                D2D1::RenderTargetProperties(),
                D2D1::HwndRenderTargetProperties(
                    hwnd,
                    D2D1::SizeU(
                        width,
                        height
                    )
                ),
                &g_renderTarget
            );


        if (FAILED(hr))
        {
            return hr;
        }
    }




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



    if (!g_backgroundBrush)
    {
        ID2D1GradientStopCollection*
            gradientStops = nullptr;


        D2D1_GRADIENT_STOP stops[3];


        stops[0].position = 0.0f;

        stops[0].color =
            D2D1::ColorF(
                0x300642,
                1.0f
            );


        stops[1].position = 0.5f;

        stops[1].color =
            D2D1::ColorF(
                0x170A3B,
                1.0f
            );


        stops[2].position = 1.0f;

        stops[2].color =
            D2D1::ColorF(
                0x041936,
                1.0f
            );


        hr =
            g_renderTarget->CreateGradientStopCollection(
                stops,
                3,
                D2D1_GAMMA_2_2,
                D2D1_EXTEND_MODE_CLAMP,
                &gradientStops
            );


        if (FAILED(hr))
        {
            return hr;
        }


        RECT rect{};

        GetClientRect(
            hwnd,
            &rect
        );


        float width =
            static_cast<float>(
                rect.right -
                rect.left
                );


        float height =
            static_cast<float>(
                rect.bottom -
                rect.top
                );


        hr =
            g_renderTarget->CreateLinearGradientBrush(
                D2D1::LinearGradientBrushProperties(
                    D2D1::Point2F(
                        0.0f,
                        0.0f
                    ),
                    D2D1::Point2F(
                        width,
                        height
                    )
                ),
                D2D1::BrushProperties(),
                gradientStops,
                &g_backgroundBrush
            );


        SafeRelease(
            gradientStops
        );


        if (FAILED(hr))
        {
            return hr;
        }
    }




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

    SafeRelease(g_keyTextFormat);
    SafeRelease(g_titleTextFormat);
    SafeRelease(g_copyrightTextFormat);
    SafeRelease(g_buttonTextFormat);

    SafeRelease(g_renderTarget);

    SafeRelease(g_wicFactory);

    SafeRelease(g_writeFactory);

    SafeRelease(g_d2dFactory);
}




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
        // White background.

        g_renderTarget->FillRoundedRectangle(
            roundedRect,
            g_whiteBrush
        );


        // White border.

        g_renderTarget->DrawRoundedRectangle(
            roundedRect,
            g_whiteBrush,
            BORDER_THICKNESS
        );


        // Black text.

        DrawTextCentered(
            key.name,
            textRect,
            g_keyTextFormat,
            g_blackBrush
        );
    }
    else
    {
        // No fill = transparent.

        g_renderTarget->DrawRoundedRectangle(
            roundedRect,
            g_whiteBrush,
            BORDER_THICKNESS
        );


        // White text.

        DrawTextCentered(
            key.name,
            textRect,
            g_keyTextFormat,
            g_whiteBrush
        );
    }
}




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




void UpdateButtonPositions()
{
    if (!g_renderTarget)
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




    DrawTextCentered(
        L"KeyChecker",

        D2D1::RectF(
            0.0f,
            35.0f,
            size.width,
            95.0f
        ),

        g_titleTextFormat,

        g_whiteBrush
    );




    for (
        const KeyboardKey& key :
        g_keyboard
        )
    {
        DrawKey(key);
    }



    DrawSocialButtons();




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
}




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




LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
)
{
    switch (message)
    {


    case WM_CREATE:
    {
        g_hWnd = hwnd;

        CreateKeyboard();
        CreateButtons();

        return 0;
    }



    case WM_SIZE:
    {
        if (g_renderTarget)
        {
            UINT width =
                LOWORD(lParam);

            UINT height =
                HIWORD(lParam);


            if (
                width > 0 &&
                height > 0
                )
            {
                g_renderTarget->Resize(
                    D2D1::SizeU(
                        width,
                        height
                    )
                );
            }
        }


        InvalidateRect(
            hwnd,
            nullptr,
            FALSE
        );


        return 0;
    }




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




    case WM_LBUTTONUP:
    {
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

            return 0;
        }


        break;
    }




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




    HWND hwnd =
        CreateWindowExW(
            0,

            CLASS_NAME,

            L"KeyChecker",

            WS_OVERLAPPEDWINDOW,

            CW_USEDEFAULT,
            CW_USEDEFAULT,

            1400,
            650,

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

    UnloadCustomFont();


    if (comInitialized)
    {
        CoUninitialize();
    }


    return static_cast<int>(
        msg.wParam
        );
}
