#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"
#include <string>
#include "tchar.h"
#include <tlhelp32.h>
#include <iostream>
using namespace std;

#define IDC_CHECK3 (WM_USER + 1)

HINSTANCE hInstance;

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CreateProcessByName(const TCHAR* processName);
void PrintProcessInfo(HWND hList) {
	int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (index != LB_ERR) {
		TCHAR processName[MAX_PATH];
		SendMessage(hList, LB_GETTEXT, index, (LPARAM)processName);

		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);

		if (Process32First(hSnapShot, &pe32)) {
			do {
				if (_tcscmp(pe32.szExeFile, processName) == 0) {
					TCHAR buffer[512];
					wsprintf(buffer, _T("ID:  %4u\nThreads: %5u\nPriority: %2u\nName: %s"),
						pe32.th32ProcessID, pe32.cntThreads, pe32.pcPriClassBase, pe32.szExeFile);
					MessageBox(NULL, buffer, _T("Process Information"), MB_OK | MB_ICONINFORMATION);
					break;
				}
			} while (Process32Next(hSnapShot, &pe32));
		}
		CloseHandle(hSnapShot);
	}
}

void ProcessList(HWND hList) {
	SendMessage(hList, LB_RESETCONTENT, 0, 0);

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hSnapShot, &pe32)) {
		SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);

		while (Process32Next(hSnapShot, &pe32)) {
			SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)pe32.szExeFile);
		}
	}
	CloseHandle(hSnapShot);
}
void KillProcess(HWND hList) {
	int index = SendMessage(hList, LB_GETCURSEL, 0, 0);
	if (index != LB_ERR) {
		TCHAR processName[MAX_PATH];
		SendMessage(hList, LB_GETTEXT, index, (LPARAM)processName);

		HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 pe32;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hSnapShot, &pe32)) {
			do {
				if (_tcscmp(pe32.szExeFile, processName) == 0) {
					HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
					TerminateProcess(hProcess, 0);
					CloseHandle(hProcess);
					break;
				}
			} while (Process32Next(hSnapShot, &pe32));
		}
		CloseHandle(hSnapShot);
		ProcessList(hList);
	}
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
	::hInstance = hInstance;
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProc);
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND hList;
	static HWND hEdit;
	static TCHAR buffer[256];
	switch (uMsg) {
	case WM_INITDIALOG: {
		hList = GetDlgItem(hwnd, IDC_LIST1);
		hEdit = GetDlgItem(hwnd, IDC_EDIT1);
		return TRUE;
	}
	case WM_CLOSE: {
		EndDialog(hwnd, 0);
		return TRUE;
	}

	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_BUTTON1: {
			ProcessList(hList);
			return TRUE;
		}
		case IDC_BUTTON2: {
			GetWindowText(hEdit, buffer, 256);
			if (MessageBox(hwnd, L"Вы уверены, что хотите создать процесс с этим именем?", L"Подтверждение", MB_YESNO) == IDYES) {
				CreateProcessByName(buffer);
			}
			return TRUE;
		}
		case IDC_BUTTON3: {
			KillProcess(hList);
			return TRUE;
		}
		case IDC_BUTTON4: {
			PrintProcessInfo(hList);
			return TRUE;
		}

		}
		break;
	}
	}

	return FALSE;
}
void CreateProcessByName(const TCHAR* processName) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(NULL,
		(LPTSTR)processName,
		NULL,
		NULL,
		FALSE,
		0,
		NULL,
		NULL,
		&si,
		&pi)
		) {
		MessageBox(NULL, L"CreateProcess failed", L"Error", MB_OK);
		return;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}