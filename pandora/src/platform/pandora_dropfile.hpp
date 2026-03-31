#pragma once
#include <vector>
#include <winerror.h>
#include <string>
#include <oleidl.h>
#include <shellapi.h>

namespace Pandora {
namespace Platform {

class DropFile : public IDropTarget {
private:
    HWND m_hwnd = nullptr;

public:
    std::vector<std::string> files;

    ULONG AddRef() { return 1; }
    ULONG Release() { return 0; }

    HRESULT QueryInterface(REFIID riid, void** ppvObject) {
        if (riid == IID_IDropTarget) {
            *ppvObject = this;
            return S_OK;
        }
        *ppvObject = NULL;
        return E_NOINTERFACE;
    }

    HRESULT DragEnter(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
        *pdwEffect &= DROPEFFECT_COPY;
        return S_OK;
    }

    HRESULT DragLeave() { return S_OK; }

    HRESULT DragOver(DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
        *pdwEffect &= DROPEFFECT_COPY;
        return S_OK;
    }

    HRESULT Drop(IDataObject* pDataObj, DWORD grfKeyState, POINTL pt, DWORD* pdwEffect) {
        FORMATETC fmte = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
        STGMEDIUM stgm;

        if (SUCCEEDED(pDataObj->GetData(&fmte, &stgm))) {
            HDROP hdrop = (HDROP)stgm.hGlobal;
            UINT file_count = DragQueryFile(hdrop, 0xFFFFFFFF, NULL, 0);

            for (UINT i = 0; i < file_count; i++) {
                TCHAR szFile[MAX_PATH];
                UINT cch = DragQueryFile(hdrop, i, szFile, MAX_PATH);
                if (cch > 0 && cch < MAX_PATH) {
                    std::wstring wpath(szFile);
                    std::string utf8Path;
                    int bufSize = WideCharToMultiByte(
                        CP_UTF8, 0, wpath.c_str(), -1,
                        NULL, 0, NULL, NULL
                    );

                    if (bufSize > 0) {
                        utf8Path.resize(bufSize);
                        WideCharToMultiByte(
                            CP_UTF8, 0, wpath.c_str(), -1,
                            &utf8Path[0], bufSize, NULL, NULL
                        );
                    }

                    this->files.push_back(utf8Path);
                }
            }

            ReleaseStgMedium(&stgm);
        }

        *pdwEffect &= DROPEFFECT_COPY;
        return S_OK;
    }

    void AttachToWindow(HWND hwnd) {
        m_hwnd = hwnd;
        CoLockObjectExternal(this, TRUE, FALSE);
    }
};

}
}
