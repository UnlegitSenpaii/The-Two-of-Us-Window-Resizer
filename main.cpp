#include <algorithm>
#include <iostream>
#include <string>
#include <Windows.h>

constexpr const char* targetWindow = "The Two of Us";
static HWND targetHandle = nullptr;

extern BOOL CALLBACK ParseWindow(HWND hwnd, LPARAM param);
extern bool HandleUserInput();

int main() {
    //set our console title
    SetConsoleTitleA("TToU Window Resizer");

    //search for the "the two of us" window
    while (!targetHandle) {
        std::cout << "Waiting for " << targetWindow << "..\r";
        EnumWindows(ParseWindow, NULL);
        Sleep(500);
    }

    //just to make sure
    if (!targetHandle)
        return 1;

    //handle user input, retry if invalid input
    while (!HandleUserInput()) {
        system("cls");
        Sleep(100);
    }

    system("timeout 3");
}

BOOL CALLBACK ParseWindow(HWND hwnd, LPARAM param) {

    //get length of the title
    const int size = GetWindowTextLengthA(hwnd) + 1;

    //allocate memory to the buffer
    auto buffer = static_cast<PSTR>(VirtualAlloc(nullptr, static_cast<DWORD>(size), MEM_COMMIT, PAGE_READWRITE));

    if (!buffer)
        return TRUE;

    GetWindowTextA(hwnd, buffer, size);

    if (std::string(buffer, size).find(targetWindow) == std::string::npos) //check if window exists
        return TRUE;

    if (std::string(buffer, size).find("Steam") != std::string::npos) //ignore steam windows
        return TRUE;

    std::cout << "Found window: " << buffer << std::endl;

    targetHandle = hwnd; //assign to global handle
    return TRUE;
}

bool HandleUserInput() {
    std::cout << "What mode do you want the game to run in?\n" << "\t[0] - Maximized Window\n" <<
        "\t[1] - Borderless Window\n" << std::endl;

    //get user input
    std::string userChoise;
    std::cin >> userChoise;

    //check if user input only contains numbers
    if (!std::all_of(userChoise.begin(), userChoise.end(), isdigit))
        return false;

    //this is not good practise at all! 
    switch (std::stoi(userChoise)) {
    case 0: {
        if (!PostMessage(targetHandle, WM_SYSCOMMAND, SC_MAXIMIZE, 0))
            std::cout << "Failed to send WM_SYSCOMMAND!\n" << std::endl;
        return true;
    }
    case 1: {
        RECT desktopResolution;
        GetWindowRect(GetDesktopWindow(), &desktopResolution); //fuck it, we don't need no null check for that
        SetWindowLongPtrW(targetHandle, GWL_STYLE, WS_POPUP | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);
        SetWindowPos(
            targetHandle, nullptr, 0, 0, desktopResolution.right, desktopResolution.bottom,
            SWP_FRAMECHANGED | SWP_NOZORDER
        );
        ShowWindow(targetHandle, SW_SHOW);
        return true;
    }
    //todo: case 2:
    default: {
        return false;
    }
    }
}
