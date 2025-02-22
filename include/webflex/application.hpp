#ifndef WAPPLICATION_HPP
#define WAPPLICATION_HPP

#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <optional>

namespace webflex 
{
    namespace impl { struct ApplicationImpl; }

    struct AppOptions {
        std::string id;
        std::optional<int> argc;
        std::optional<char**> argv;
        size_t thread_count = 0;
        std::vector<std::string> browser_flag;
    };

    enum class PathKey 
    {
        CookiePath,
        CachePath,
        ConfigPath,
        ApplicationDir,
        DownloadsPath
    };

    class Application 
    {
    public:
        Application(AppOptions options);
        ~Application();

        static std::unique_ptr<Application> init(AppOptions options);

        static const std::string& getPath(PathKey key);

        static void setPath(PathKey key, const std::string& path);

        static size_t threadCount() { return m_thread_count; }
        
        static bool threadSafe();
        
        std::string id() const;

        int run() const;

        void quit() const;

    private:
        std::unique_ptr<impl::ApplicationImpl> impl_;
        int argc_ = 1;
        std::vector<char*> argv_;
        std::string id_;
        static size_t m_thread_count;
        static std::unordered_map<PathKey, std::string> m_paths;
    };
}

#endif // WAPPLICATION_HPP