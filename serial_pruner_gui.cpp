#include <windows.h>
#include <string>
#include <vector>
#include <regex>
#include <unordered_set>
#include <algorithm>

static const wchar_t* kWindowClass = L"SerialPrunerWnd";
static const wchar_t* kWindowTitle = L"Serial Number Pruner";

static HWND g_inputEdit = nullptr;
static HWND g_outputEdit = nullptr;
static HWND g_checkPreserve = nullptr;
static HWND g_button = nullptr;

static std::wstring trim(const std::wstring& s) {
    size_t start = s.find_first_not_of(L" \t\r\n");
    if (start == std::wstring::npos) return L"";
    size_t end = s.find_last_not_of(L" \t\r\n");
    return s.substr(start, end - start + 1);
}

static std::wstring extract_useful_number(const std::wstring& s) {
    // Match SNxxxx or SNOxxxx (case-insensitive)
    static const std::wregex re(L"SN?O?([A-Za-z0-9]+)", std::regex_constants::icase);
    std::wsmatch m;
    if (std::regex_search(s, m, re) && m.size() > 1) {
        return m[1].str();
    }
    return L"";
}

static std::vector<std::wstring> split_tokens(const std::wstring& text) {
    std::vector<std::wstring> tokens;
    static const std::wregex re(L"[;,\n]+", std::regex_constants::icase);
    std::wsregex_token_iterator it(text.begin(), text.end(), re, -1);
    std::wsregex_token_iterator end;
    for (; it != end; ++it) {
        tokens.push_back(it->str());
    }
    return tokens;
}

static void on_prune() {
    int len = GetWindowTextLengthW(g_inputEdit);
    std::wstring input;
    input.resize(len);
    GetWindowTextW(g_inputEdit, &input[0], len + 1);

    auto raw_tokens = split_tokens(input);
    std::vector<std::wstring> cleaned;
    cleaned.reserve(raw_tokens.size());

    for (const auto& t : raw_tokens) {
        std::wstring token = trim(t);
        if (token.empty()) continue;
        std::wstring extracted = extract_useful_number(token);
        if (!extracted.empty()) cleaned.push_back(extracted);
    }

    bool preserve = (SendMessageW(g_checkPreserve, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (preserve) {
        std::unordered_set<std::wstring> seen;
        std::vector<std::wstring> unique;
        unique.reserve(cleaned.size());
        for (const auto& s : cleaned) {
            if (seen.insert(s).second) unique.push_back(s);
        }
        cleaned.swap(unique);
    } else {
        std::sort(cleaned.begin(), cleaned.end());
        cleaned.erase(std::unique(cleaned.begin(), cleaned.end()), cleaned.end());
    }

    std::wstring output;
    for (size_t i = 0; i < cleaned.size(); ++i) {
        output += cleaned[i];
        if (i + 1 < cleaned.size()) output += L"\r\n";
    }

    SetWindowTextW(g_outputEdit, output.c_str());
}

static void create_controls(HWND hwnd) {
    CreateWindowW(L"STATIC", L"Paste Serial Numbers:",
        WS_CHILD | WS_VISIBLE,
        10, 10, 560, 20, hwnd, nullptr, nullptr, nullptr);

    g_inputEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL,
        10, 35, 560, 120, hwnd, nullptr, nullptr, nullptr);

    g_checkPreserve = CreateWindowW(L"BUTTON", L"Preserve input order (no alphabetical sort)",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        10, 165, 360, 24, hwnd, nullptr, nullptr, nullptr);

    g_button = CreateWindowW(L"BUTTON", L"Prune and Sort",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        400, 162, 170, 28, hwnd, (HMENU)1, nullptr, nullptr);

    CreateWindowW(L"STATIC", L"Cleaned Serial Numbers:",
        WS_CHILD | WS_VISIBLE,
        10, 200, 560, 20, hwnd, nullptr, nullptr, nullptr);

    g_outputEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
        10, 225, 560, 120, hwnd, nullptr, nullptr, nullptr);
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        create_controls(hwnd);
        return 0;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            on_prune();
            return 0;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = kWindowClass;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassW(&wc)) return 1;

    HWND hwnd = CreateWindowExW(
        0, kWindowClass, kWindowTitle,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 410,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd) return 1;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}
