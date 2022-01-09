#include <algorithm>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <Windows.h>

constexpr const char* targetWindow = "The Two of Us";
static HWND targetHandle = nullptr;

extern BOOL CALLBACK ParseWindow(HWND hwnd, LPARAM param);
extern bool HandleUserInput();

int main() {
    SetConsoleTitleA("TToU Window Resizer");

    while (!targetHandle) {
        std::cout << "Waiting for " << targetWindow << "..\r";
        EnumWindows(ParseWindow, NULL);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    if (!targetHandle)
        return 1;

    while (!HandleUserInput()) {
        system("cls");
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    system("timeout 3");
}

BOOL CALLBACK ParseWindow(HWND hwnd, LPARAM param) {

    const int size = GetWindowTextLengthA(hwnd) + 1;

    auto buffer = static_cast<PSTR>(VirtualAlloc(nullptr, static_cast<DWORD>(size), MEM_COMMIT, PAGE_READWRITE));

    if (!buffer)
        return TRUE;

    GetWindowTextA(hwnd, buffer, size);

    if (std::string(buffer, size).find(targetWindow) == std::string::npos) //check if window exists
        return TRUE;

    if (std::string(buffer, size).find("Steam") != std::string::npos) //ignore steam windows
        return TRUE;

    std::cout << "Found window: " << buffer << std::endl;

    targetHandle = hwnd;
    return TRUE;
}

void DoBorderlessWindow(const HWND target) {
    RECT desktopResolution;
    GetWindowRect(GetDesktopWindow(), &desktopResolution); //fuck it, we don't need no null check for that
    SetWindowLongPtrW(target, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
    SetWindowPos(
        target, nullptr, 0, 0, desktopResolution.right, desktopResolution.bottom, SWP_FRAMECHANGED | SWP_NOZORDER
    );
    ShowWindow(target, SW_SHOW);
}

bool ClampMouseToWindow(const HWND target) {
    POINT cursorPos{};
    GetCursorPos(&cursorPos);

    RECT gameRes{};
    GetWindowRect(targetHandle, &gameRes);

    //this will make the clamp bite earlier increases accuracy -> i prefer not tabbing out instead of mouse movement space, since you dont need the second in the game much
    gameRes.right = std::lround(gameRes.right / 1.15);
    gameRes.bottom = std::lround(gameRes.bottom / 1.15);
    gameRes.left = std::lround(gameRes.right * 0.15);
    gameRes.top = std::lround(gameRes.bottom * 0.15);

    bool didClamp = false;

    //clamp cursor pos;
    if (cursorPos.x > gameRes.right) {
        cursorPos.x = gameRes.right;
        didClamp = true;
    }

    if (cursorPos.y > gameRes.bottom) {
        cursorPos.y = gameRes.bottom;
        didClamp = true;
    }

    if (cursorPos.x < gameRes.left) {
        cursorPos.x = gameRes.left;
        didClamp = true;
    }

    if (cursorPos.y < gameRes.top) {
        cursorPos.y = gameRes.top;
        didClamp = true;
    }
    SetCursorPos(cursorPos.x, cursorPos.y);

    return didClamp;
}

bool HandleUserInput() {
    std::cout << "What mode do you want the game to run in?\n" << "\t[0] - Maximized Window\n" <<
        "\t[1] - Borderless Window\n" << "\t[2] - Borderless Window with Mouse Lock\n" << std::endl;

    //get user input
    std::string userChoise;
    std::cin >> userChoise;

    //check if user input only contains numbers
    if (!std::all_of(userChoise.begin(), userChoise.end(), isdigit))
        return false;

    switch (std::stoi(userChoise)) {
    case 0: {
        if (!PostMessage(targetHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0))
            std::cout << "Failed to send WM_SYSCOMMAND!\n" << std::endl;
        return true;
    }
    case 1: {
        DoBorderlessWindow(targetHandle);
        return true;
    }
    case 2: {
        DoBorderlessWindow(targetHandle);
        std::cout << "Clamping mouse to game resolution * 0.15. [F8 - TO STOP]\n";
        while (true) {
            //abort key
            if (GetAsyncKeyState(VK_F8)) {
                std::cout << "\t - Stopping mouse clamp." << std::endl;
                return true;
            }

            //wait for game to be active 
            if (GetForegroundWindow() != targetHandle) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (!ClampMouseToWindow(targetHandle))
                std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
        break;
    }
    default: {
        return false;
    }
    }
}
