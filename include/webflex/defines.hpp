#pragma once

#include <cassert>
#include <string>

#define W_ASSERT(cond) assert(cond)
#define W_STATIC_ASSERT(cond) static_assert(cond)
#define W_THROW(arg) throw arg
#define W_STRINGIFY(str) #str

namespace webflex
{
    enum class ExecutionTime
    {
        Deferred,
        DocumentReady,
        DocumentCreation
    };
    
    enum class InjectWorld
    {
        Main,
        Application,
        User
    };

    struct InjectOptions
    {
        std::string code;
        ExecutionTime time = ExecutionTime::Deferred;
        InjectWorld world = InjectWorld::Application;
        bool permanent = false;
    };

    enum class ThemeMode
    {
        Dark,
        Light,
        System
    };

    enum class JsConsoleMessageLevel 
    {
        Info,
        Warning,
        Error
    };

    enum class Launch {
        Async,
        Sync
    };

    enum class WebEvent 
    {
        DomReady,
        Navigated,
        FaviconChanged,
        TitleChanged,
        LoadProgress,
        Scrolled,
        LoadStarted,
        ZoomChanged,
        ClearHttpCache,
        CookieAdded,
        CookieRemoved
    };

    enum class WindowEvent 
    {
        Close,
        Closed,
        Resize,
        Maximized,
        Minimized
    };

    enum class ShowMode 
    {
        Normal,
        FullScreen,
        Maximized,
        Minimized
    };
}