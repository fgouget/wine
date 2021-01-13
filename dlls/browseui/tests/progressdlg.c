/* Unit tests for progressdialog object
 *
 * Copyright 2012 Detlef Riekenberg
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

#define COBJMACROS

#include <stdarg.h>
#include <shlobj.h>

#include "wine/test.h"


static void test_IProgressDialog_QueryInterface(void)
{
    IProgressDialog *dlg;
    IProgressDialog *dlg2;
    IOleWindow *olewindow;
    IUnknown *unk;
    HRESULT hr;

    hr = CoCreateInstance(&CLSID_ProgressDialog, NULL, CLSCTX_INPROC_SERVER, &IID_IProgressDialog, (void*)&dlg);
    if (FAILED(hr)) {
        win_skip("CoCreateInstance for IProgressDialog returned 0x%x\n", hr);
        return;
    }

    hr = IProgressDialog_QueryInterface(dlg, &IID_IUnknown, NULL);
    ok(hr == E_POINTER, "got 0x%x (expected E_POINTER)\n", hr);

    hr = IProgressDialog_QueryInterface(dlg, &IID_IUnknown, (void**)&unk);
    ok(hr == S_OK, "QueryInterface (IUnknown) returned 0x%x\n", hr);
    if (SUCCEEDED(hr)) {
        IUnknown_Release(unk);
    }

    hr = IProgressDialog_QueryInterface(dlg, &IID_IOleWindow, (void**)&olewindow);
    ok(hr == S_OK, "QueryInterface (IOleWindow) returned 0x%x\n", hr);
    if (SUCCEEDED(hr)) {
        hr = IOleWindow_QueryInterface(olewindow, &IID_IProgressDialog, (void**)&dlg2);
        ok(hr == S_OK, "QueryInterface (IProgressDialog) returned 0x%x\n", hr);
        if (SUCCEEDED(hr)) {
            IProgressDialog_Release(dlg2);
        }
        IOleWindow_Release(olewindow);
    }
    IProgressDialog_Release(dlg);
}


START_TEST(progressdlg)
{
    if (1)
    {
        int i;
        for (i = 0; i < 100; i++)
        {
            trace("WTBS trace %d\n", i);
            todo_if(1) ok(0, "WTBS todo %d\n", i);
            skip("WTBS skip %d\n", i);
        }
        trace("WTBS another trace\n");
        todo_if(1) ok(0, "WTBS another todo\n");
        skip("WTBS another skip\n");
        return;
    }
    CoInitialize(NULL);

    test_IProgressDialog_QueryInterface();

    CoUninitialize();

    /* Simulate subtest traces and test failures */
    subtest("dummy");
    printf("dummy.c:12: WTBS A sample trace\n");
    printf("dummy.c:13: Test failed: WTBS A test failure\n");
    printf("dummy.c:14: Test marked todo: WTBS A todo test failing as expected\n");
    printf("dummy.c:15: Test succeeded inside todo block: WTBS A todo test unexpectedly succeeding\n");
    printf("dummy.c:16: Tests skipped: WTBS A plain skip\n");

    printf("Garble garbledummy.c:17: Test failed: WTBS A garbled failure\n");

    winetest_add_failures(3);
}
