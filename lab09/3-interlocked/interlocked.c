/* do not use UNICODE */
#undef _UNICODE
#undef UNICODE

#include <windows.h>
#include <stdio.h>

#include "../utils.h"

#define BUF_SIZE	128
#define LIMIT		10
#define NO_THREADS	23
#define NO_ROUNDS	1000001


LONG count;
HANDLE hMutex;


DWORD WINAPI thread_function(LPVOID arg)
{
	for (int i = 0; i < NO_ROUNDS; ++i)
    {
        InterlockedIncrement(&count);
		InterlockedCompareExchange(&count, 0, LIMIT);
	}

	return 0;
}

DWORD WINAPI thread_function_mutex(LPVOID arg)
{
	for (int i = 0; i < NO_ROUNDS; ++i)
    {
		DWORD dwRet = WaitForSingleObject(hMutex, INFINITE);
		DIE(dwRet == WAIT_FAILED, "WaitForSingleObject(mutex) failed");

		count++;
		if (count == LIMIT)
			count = 0;

		DIE(!ReleaseMutex(hMutex), "ReleaseMutex(mutex) failed");
	}

	return 0;
}

int main(void)
{
	HANDLE hThread[NO_THREADS];
	DWORD dwRet, i, start_time, end_time;

	hMutex = CreateMutex(
		NULL,  /* default security attributes */
		FALSE, /* initially not owned */
		NULL  /* unnamed mutex */
	);
	DIE(hMutex == INVALID_HANDLE_VALUE, "CreateMutex");

	start_time = GetTickCount();

		for (i = 0; i < NO_THREADS; i++) {
			hThread[i] = CreateThread(NULL, 0,
					thread_function, NULL, 0, NULL);
			DIE(hThread[i] == NULL, "CreateThread");
		}

		dwRet = WaitForMultipleObjects(NO_THREADS,
				hThread, TRUE, INFINITE);
		DIE(dwRet == WAIT_FAILED, "WaitForMultipleObjects");

	end_time = GetTickCount();
	printf("Count final value = %ld\n", count);
	printf("Program runned for %d ms\n", end_time - start_time);

	for (i = 0; i < NO_THREADS; i++) {
		dwRet = CloseHandle(hThread[i]);
		DIE(dwRet == FALSE, "CloseHandle");
	}
	dwRet = CloseHandle(hMutex);
	DIE(dwRet == FALSE, "CloseHandle");

	return 0;
}
