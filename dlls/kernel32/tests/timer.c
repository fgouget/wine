/* WTBS Dummy patch
 * Unit test suite for timer functions
 *
 * Copyright 2004 Mike McCormack
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include "wine/test.h"
#include "winbase.h"
#include "winternl.h"
#include "winuser.h"


static void test_timer(void)
{
    HANDLE handle;
    BOOL r;
    LARGE_INTEGER due;

    /* try once with a positive number */
    handle = CreateWaitableTimerA( NULL, 0, NULL );
    ok( handle != NULL, "failed to create waitable timer with no name\n" );

    due.QuadPart = 10000;
    r = SetWaitableTimer( handle, &due, 0x1f4, NULL, NULL, FALSE );
    ok( r, "failed to set timer\n");

    CloseHandle( handle );

    /* try once with a negative number */
    handle = CreateWaitableTimerA( NULL, 0, NULL );
    ok( handle != NULL, "failed to create waitable timer with no name\n" );

    due.QuadPart = -10000;
    r = SetWaitableTimer( handle, &due, 0x1f4, NULL, NULL, FALSE );
    ok( r, "failed to set timer\n");

    CloseHandle( handle );
}

#define TICKSPERSEC       10000000

static BOOL adjust_system_time(int sec)
{
    ULARGE_INTEGER uli;
    SYSTEMTIME st;
    FILETIME ft;

    GetSystemTimeAsFileTime(&ft);
    uli.u.LowPart = ft.dwLowDateTime;
    uli.u.HighPart = ft.dwHighDateTime;
    uli.QuadPart += (LONGLONG)sec * TICKSPERSEC;
    ft.dwLowDateTime = uli.u.LowPart;
    ft.dwHighDateTime = uli.u.HighPart;
    if (!FileTimeToSystemTime(&ft, &st))
        return FALSE;
    return SetSystemTime(&st);
}

static DWORD WINAPI thread_WaitForSingleObject(void *arg)
{
    HANDLE event;
    DWORD t, r;

    event = CreateEventW(NULL, FALSE, FALSE, NULL);
    ok(event != NULL, "CreateEvent failed\n");
    t = GetTickCount();
    r = WaitForSingleObject(event, 3000);
    ok(r == WAIT_TIMEOUT, "WiatForSingleObject returned %x\n", r);
    CloseHandle(event);
    t = GetTickCount() - t;
    ok(t > 2000, "t = %d\n", t);
    return 0;
}

static DWORD WINAPI thread_Sleep(void *arg)
{
    DWORD t = GetTickCount();

    Sleep(3000);
    t = GetTickCount() - t;
    ok(t > 2000, "t = %d\n", t);
    return 0;
}

static DWORD WINAPI thread_SleepEx(void *arg)
{
    DWORD t = GetTickCount();

    SleepEx(3000, TRUE);
    t = GetTickCount() - t;
    ok(t > 2000, "t = %d\n", t);
    return 0;
}

static DWORD WINAPI thread_WaitableTimer_rel(void *arg)
{
    LARGE_INTEGER li;
    HANDLE timer;
    DWORD t, r;

    li.QuadPart = -3 * TICKSPERSEC;

    timer = CreateWaitableTimerA(NULL, TRUE, NULL);
    ok(timer != NULL, "CreateWaitableTimer failed\n");

    t = GetTickCount();
    r = SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
    ok(r, "SetWaitableTimer failed\n");

    r = WaitForSingleObject(timer, INFINITE);
    ok(r == WAIT_OBJECT_0, "WaitForSingleObject returned %d\n", r);
    CloseHandle(timer);
    t = GetTickCount() - t;
    ok(t > 2000, "t = %d\n", t);
    return 0;
}

static DWORD WINAPI thread_WaitableTimer_abs(void *arg)
{
    LARGE_INTEGER li;
    HANDLE timer;
    FILETIME ft;
    DWORD t, r;

    GetSystemTimeAsFileTime(&ft);
    li.u.LowPart = ft.dwLowDateTime;
    li.u.HighPart = ft.dwHighDateTime;
    li.QuadPart += 3 * TICKSPERSEC;

    timer = CreateWaitableTimerA(NULL, TRUE, NULL);
    ok(timer != NULL, "CreateWaitableTimer failed\n");

    t = GetTickCount();
    r = SetWaitableTimer(timer, &li, 0, NULL, NULL, FALSE);
    ok(r, "SetWaitableTimer failed\n");

    r = WaitForSingleObject(timer, INFINITE);
    ok(r == WAIT_OBJECT_0, "WaitForSingleObject returned %d\n", r);
    CloseHandle(timer);
    t = GetTickCount() - t;
    ok(t < 2000, "t = %d\n", t);
    return 0;
}

static DWORD WINAPI thread_WaitableTimer_period(void *arg)
{
    LARGE_INTEGER li;
    HANDLE timer;
    DWORD t, r;

    li.QuadPart = -1;

    timer = CreateWaitableTimerA(NULL, FALSE, NULL);
    ok(timer != NULL, "CreateWaitableTimer failed\n");

    t = GetTickCount();
    r = SetWaitableTimer(timer, &li, 3000, NULL, NULL, FALSE);
    ok(r, "SetWaitableTimer failed\n");

    r = WaitForSingleObject(timer, INFINITE);
    ok(r == WAIT_OBJECT_0, "WaitForSingleObject returned %d\n", r);

    r = WaitForSingleObject(timer, INFINITE);
    ok(r == WAIT_OBJECT_0, "WaitForSingleObject returned %d\n", r);
    CloseHandle(timer);
    t = GetTickCount() - t;
    ok(t > 2000, "t = %d\n", t);
    return 0;
}

static DWORD WINAPI thread_SetTimer(void *arg)
{
    DWORD t = GetTickCount();
    UINT_PTR timer;
    MSG msg;

    timer = SetTimer(NULL, 0, 3000, NULL);
    ok(timer, "SetTimer failed (%d)\n", GetLastError());

    while (GetMessageW(&msg, NULL, 0, 0))
    {
        DispatchMessageW(&msg);
        if (msg.message == WM_TIMER) break;
    }

    t = GetTickCount() - t;
    ok(t > 2000, "t = %d\n", t);
    KillTimer(NULL, timer);
    return 0;
}

static void test_timeouts(void)
{
    HANDLE threads[7];
    DWORD i;

    if (!adjust_system_time(1))
    {
        skip("can't adjust system clock (%d)\n", GetLastError());
        return;
    }


    threads[0] = CreateThread(NULL, 0, thread_WaitForSingleObject, NULL, 0, NULL);
    threads[1] = CreateThread(NULL, 0, thread_Sleep, NULL, 0, NULL);
    threads[2] = CreateThread(NULL, 0, thread_SleepEx, NULL, 0, NULL);
    threads[3] = CreateThread(NULL, 0, thread_WaitableTimer_rel, NULL, 0, NULL);
    threads[4] = CreateThread(NULL, 0, thread_WaitableTimer_abs, NULL, 0, NULL);
    threads[5] = CreateThread(NULL, 0, thread_WaitableTimer_period, NULL, 0, NULL);
    threads[6] = CreateThread(NULL, 0, thread_SetTimer, NULL, 0, NULL);
    for(i=0; i<ARRAY_SIZE(threads); i++)
        ok(threads[i] != NULL, "CreateThread failed\n");

    Sleep(500);
    adjust_system_time(10);

    for (i=0; i<ARRAY_SIZE(threads); i++)
    {
        WaitForSingleObject(threads[i], INFINITE);
        CloseHandle(threads[i]);
    }
    adjust_system_time(-11);
}

START_TEST(timer)
{
    int argc;
    char **argv;

    argc = winetest_get_mainargs( &argv );
    if (argc <= 2)
    {
        PROCESS_INFORMATION pi;
        STARTUPINFOA si = { 0 };
        char cmdline[MAX_PATH];
        BOOL ret;

        trace("WTBS Test handling of CreateProcess() failure\n");
        strcpy(cmdline, "\"t:\\NotADir\\NotAFile.exe\"");
        ret = CreateProcessA(cmdline, cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        ok(!ret, "CreateProcess unexpectedly succeeded\n");
        wait_child_process( pi.hProcess );

        sprintf(cmdline, "\"%s\" %s propagate", argv[0], argv[1]);
        ret = CreateProcessA(argv[0], cmdline, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        ok(ret, "Could not create process: %u\n", GetLastError());
        winetest_wait_child_process( pi.hProcess );
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else if (strcmp(argv[2], "propagate") == 0)
    {
        trace("WTBS Test the propagation of child process errors\n");
        ok(1, "WTBS A successful test\n");
        ok(0, "WTBS A test failure\n");
        todo_if(0) ok(1, "WTBS A not-todo successful test\n");
        todo_if(0) ok(0, "WTBS A not-todo test failure\n");
        todo_if(1) ok(0, "WTBS A todo test failing as expected\n");
        todo_if(1) ok(1, "WTBS A todo test unexpectedly succeeding\n");
        skip("WTBS A plain skip\n");
        win_skip("WTBS A Windows-specific skip\n");
    }
    if (0)
    test_timer();
    test_timeouts();
}
