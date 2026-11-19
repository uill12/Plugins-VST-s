#define NOMINMAX
#include <windows.h>
#include <clap/clap.h>
#include <cstdio>
#include <atomic>
#include <string>

static std::atomic_bool callbackPending{false};
static const clap_host_gui_t guiHost{
    [](const clap_host_t*) {},
    [](const clap_host_t*, uint32_t, uint32_t) { return false; },
    [](const clap_host_t*) { return true; },
    [](const clap_host_t*) { return true; },
    [](const clap_host_t*, bool) {}
};

int main(int argc,char** argv)
{
    if(argc!=2) return 1;
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    auto window=CreateWindowExW(0,L"STATIC",L"Albedo viewport fixture",WS_POPUP|WS_VISIBLE,
                                0,0,900,600,nullptr,nullptr,GetModuleHandleW(nullptr),nullptr);
    if(!window) return 2;
    const auto dll=LoadLibraryA(argv[1]);
    if(!dll) return 3;
    const auto* entry=reinterpret_cast<const clap_plugin_entry_t*>(GetProcAddress(dll,"clap_entry"));
    if(!entry || !entry->init(argv[1])) return 4;
    const auto* factory=static_cast<const clap_plugin_factory_t*>(entry->get_factory(CLAP_PLUGIN_FACTORY_ID));
    if(!factory) return 5;
    clap_host_t host{};
    host.clap_version=CLAP_VERSION;
    host.name="Albedo QA"; host.vendor="Albedo"; host.url="https://example.org";host.version="1";
    host.get_extension=[](const clap_host_t*,const char* id)->const void* {return std::string(id)==CLAP_EXT_GUI?&guiHost:nullptr;};
    host.request_restart=host.request_process=[](const clap_host_t*){};
    host.request_callback=[](const clap_host_t*){callbackPending=true;};
    const auto* plugin=factory->create_plugin(factory,&host,factory->get_plugin_descriptor(factory,0)->id);
    if(!plugin || !plugin->init(plugin)) return 6;
    const auto* gui=static_cast<const clap_plugin_gui_t*>(plugin->get_extension(plugin,CLAP_EXT_GUI));
    if(!gui || !gui->create(plugin,CLAP_WINDOW_API_WIN32,false)) return 7;
    clap_window_t parent{}; parent.api=CLAP_WINDOW_API_WIN32;parent.win32=window;
    uint32_t w=1200,h=800;
    if(!gui->set_parent(plugin,&parent) || !gui->adjust_size(plugin,&w,&h)
        || !gui->set_size(plugin,w,h) || !gui->show(plugin)) return 8;
    SetPropW(window,L"AlbedoFixtureMaxWidth",reinterpret_cast<HANDLE>(static_cast<uintptr_t>(w)));
    // The embedder intentionally grants fewer physical pixels than the editor requests.
    std::printf("HWND %llu\n",static_cast<unsigned long long>(reinterpret_cast<uintptr_t>(window)));
    std::fflush(stdout);
    MSG msg{};
    for(;;) {
        while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)) {
            if(msg.message==WM_QUIT) return 0;
            TranslateMessage(&msg);DispatchMessageW(&msg);
        }
        if(callbackPending.exchange(false)) plugin->on_main_thread(plugin);
        MsgWaitForMultipleObjects(0,nullptr,FALSE,10,QS_ALLINPUT);
    }
}
