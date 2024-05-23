#include <iostream>
#include <windows.h>
#include <string>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Command to be executed
    const char* cmd = "cmd.exe /C bcdedit /enum {current}";

    // Set up the security attributes struct
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    // Create the pipe for the process's STDOUT
    HANDLE hStdOutRead, hStdOutWrite;
    if (!CreatePipe(&hStdOutRead, &hStdOutWrite, &sa, 0)) {
        std::cerr << "Ошибка при создании pipe." << std::endl;
        return 1;
    }

    // Ensure the read handle to the pipe for STDOUT is not inherited
    SetHandleInformation(hStdOutRead, HANDLE_FLAG_INHERIT, 0);

    // Set up the process startup info struct
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    si.hStdOutput = hStdOutWrite;
    si.hStdError = hStdOutWrite;
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    // Set up the process info struct
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    // Create the process
    if (!CreateProcessA(NULL, (LPSTR)cmd, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        std::cerr << "Ошибка при создании процесса." << std::endl;
        CloseHandle(hStdOutRead);
        CloseHandle(hStdOutWrite);
        return 1;
    }

    // Close the write end of the pipe before reading from the read end of the pipe
    CloseHandle(hStdOutWrite);

    // Read output from the child process
    char buffer[128];
    DWORD bytesRead;
    std::string result = "";
    while (ReadFile(hStdOutRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        result += buffer;
    }

    // Close the read end of the pipe
    CloseHandle(hStdOutRead);

    // Wait until child process exits
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // Check if "testsigning" is enabled
    bool testsigning_enabled = (result.find("testsigning") != std::string::npos && result.find("Yes") != std::string::npos);

    // Return the result
    return testsigning_enabled;
}
