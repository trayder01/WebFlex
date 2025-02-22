#ifndef WBROWSER_HPP
#define WBROWSER_HPP

#include "geometry/point.hpp"
#include "geometry/size.hpp"
#include "core/js_types.hpp"
#include "core/traits.hpp"
#include "core/signal.hpp"
#include "byte_array.hpp"
#include "defines.hpp"
#include "js_args.hpp"
#include "color.hpp"
#include "icon.hpp"
#include "url.hpp"
#include "cookie_store.hpp"

#include <future>
#include <unordered_set>

namespace webflex
{
    namespace impl
    {
        class BrowserImpl;
    }

    class Navigation;
    class Pdf;
    class JsAccessible;
    class CookieStore;

    class Browser
    {
    public:
        Browser();
 
        ~Browser();

        Browser(const Browser&) = delete;
        Browser& operator=(const Browser&) = delete;

        Browser(Browser&&) = delete;
        Browser& operator=(Browser&&) = delete;

        static std::shared_ptr<Browser> create();
        
        static std::shared_ptr<Browser> create(const std::string &title);
        
        static std::shared_ptr<Browser> create(const std::string &title, const geometry::Size &size);
        
        static std::shared_ptr<Browser> create(const std::string &title, const geometry::Size &size, const geometry::Point &point);

        static void registerScheme(const std::string &name);

        void setBackgroundColor(const Color& color);
        
        Color backgroundColor() const;

        void setTitle(const std::string &title);
        
        std::string title() const;

        template <typename T>
        void embed(const std::string& name, std::shared_ptr<T> object) 
        {
            static_assert(std::is_base_of_v<JsAccessible, T>, "T must be derived from JsAccessible");
            embedImpl(object, name.c_str());
        }

        void setIcon(const webflex::Icon &icon);
        
        Icon icon() const;

        void resize(int w, int h);

        void resize(const geometry::Size &size);

        void execute(const std::string &script);

        template <typename... _Args>
        void execute(const std::string &script, const utils::args_pack<_Args...> &args)
        {
            execute(utils::format_script(script, args.args));
        }

        template <typename ReturnType, typename... _Args>
        std::future<ReturnType> evaluate(const std::string &script, const utils::args_pack<_Args...> &args)
        {
            return evaluate<ReturnType>(utils::format_script(script, args.args));
        }

        template <typename ReturnType>
        std::future<ReturnType> evaluate(const std::string &script) {
            auto promise = std::make_shared<std::promise<ReturnType>>();
            auto future = promise->get_future();

            evaluateImpl(script, [this, promise](const core::js_any_t &result) {
                auto setException = [&]() {
                    promise->set_exception(std::make_exception_ptr(std::invalid_argument("Unsupported type")));
                };

                if constexpr (std::is_same_v<ReturnType, JsValue>) {
                    if (auto base = std::get_if<core::js_value_t>(&result)) {
                        if (!setJsValue(*base, promise)) setException();
                    }
                    return;
                }
                else if constexpr (std::is_same_v<ReturnType, JsArray>) {
                    if (auto list = std::get_if<core::js_list_t>(&result)) {
                        JsArray array;
                        for (const auto &item : *list) {
                            if (!setJsValue(item, array)) {
                                setException();
                                return;
                            }
                        }
                        promise->set_value(array);
                    } else {
                        setException();
                    }
                    return;
                }
                else if constexpr (std::is_same_v<ReturnType, JsMap>) {
                    if (auto dict = std::get_if<core::js_dict_t>(&result)) 
                    {
                        JsMap map;
                        for (const auto &[key, variant] : *dict) {
                            if (!setJsValueMap(variant, map, key)) 
                            {
                                setException();
                                return;
                            }
                        }

                        promise->set_value(map);                        
                    }
                    return;
                }
                else {
                    if (auto base = std::get_if<core::js_value_t>(&result)) {
                        if constexpr (std::is_same_v<ReturnType, std::string>) {
                            if (auto value = std::get_if<std::string>(base)) {
                                promise->set_value(*value);
                            }
                        }
                        else if constexpr(std::is_same_v<ReturnType, bool>) {
                            if (auto value = std::get_if<bool>(base)) {
                                promise->set_value(*value);
                            }
                        }
                        else if constexpr (std::is_same_v<ReturnType, double>) {
                            if (auto value = std::get_if<double>(base)) {
                                promise->set_value(*value);
                            }
                        }
                    } else if (auto list = std::get_if<core::js_list_t>(&result)) {
                        if constexpr (std::is_same_v<ReturnType, std::vector<core::js_value_t>>) {
                            promise->set_value(*list);
                        } else {
                            setException();
                        }
                    } else if (auto dict = std::get_if<core::js_dict_t>(&result)) {
                        if constexpr (std::is_same_v<ReturnType, std::unordered_map<std::string, core::js_value_t>>) {
                            promise->set_value(*dict);
                        } else {
                            setException();
                        }
                    } else {
                        setException();
                    }
                }
            });

            return future;
        }

        template <typename ReturnType>
        void evaluate(const std::string &script, const std::function<void(ReturnType)>& callback)
        {
            evaluateImpl(script, [this, callback = std::move(callback)](const core::js_any_t &result)
            {
                auto reportError = [&](){};

                if constexpr (std::is_same_v<ReturnType, JsValue>) {
                    if (auto base = std::get_if<core::js_value_t>(&result)) {
                        if (auto string = std::get_if<std::string>(base)) {
                            callback(JsValue(*string));
                        } else if (auto boolean = std::get_if<bool>(base)) {
                            callback(JsValue(*boolean));
                        } else if (auto number = std::get_if<double>(base)) {
                            callback(JsValue(*number));
                        }
                        else if (auto number = std::get_if<int>(base)) {
                            callback(JsValue((double)*number));
                        } 
                        else {
                            reportError();
                        }
                    } else {
                        reportError();
                    }
                    return;
                }

                if constexpr (std::is_same_v<ReturnType, JsArray>) {
                    if (auto list = std::get_if<core::js_list_t>(&result)) {
                        JsArray array;
                        for (const auto &item : *list) {
                            if (auto string = std::get_if<std::string>(&item)) {
                                array.append(JsValue(*string));
                            } else if (auto boolean = std::get_if<bool>(&item)) {
                                array.append(JsValue(*boolean));
                            } else if (auto number = std::get_if<double>(&item)) {
                                array.append(JsValue(*number));
                            }
                            else if (auto number = std::get_if<int>(&item)) {
                                array.append(JsValue((double)*number));
                            }  
                            else {
                                reportError();
                                return;
                            }
                        }
                        callback(array);
                    } else {
                        reportError();
                    }
                    return;
                }

                if constexpr (std::is_same_v<ReturnType, JsMap>) {
                    if (auto dict = std::get_if<core::js_dict_t>(&result)) {
                        JsMap map;
                        for (const auto &[key, variant] : *dict) 
                        {
                            if (auto string = std::get_if<std::string>(&variant)) {
                                map.insert(key, JsValue(*string));
                            } else if (auto boolean = std::get_if<bool>(&variant)) {
                                map.insert(key, JsValue(*boolean));
                            } else if (auto number = std::get_if<double>(&variant)) {
                                map.insert(key, JsValue(*number));
                            }
                            else if (auto number = std::get_if<int>(&variant)) {
                                map.insert(key, JsValue((double)*number));
                            } 
                            else {
                                reportError();
                                return;
                            }
                        }
                        callback(map);
                    } else {
                        reportError();
                    }
                    return;
                }
                else {
                    if (auto base = std::get_if<core::js_value_t>(&result)) {
                        if (auto string = std::get_if<std::string>(base)) {
                            if constexpr (std::is_same_v<ReturnType, std::string>) {
                                callback(*string);
                            } else {
                                reportError();
                            }
                        } else if (auto boolean = std::get_if<bool>(base)) {
                            if constexpr (std::is_same_v<ReturnType, bool>) {
                                callback(*boolean);
                            } else {
                                reportError();
                            }
                        } else if (auto number = std::get_if<double>(base)) {
                            if constexpr (std::is_same_v<ReturnType, double>) {
                                callback(*number);
                            } else {
                                reportError();
                            }
                        }
                        else if (auto number = std::get_if<int>(base)) {
                            if constexpr (std::is_same_v<ReturnType, int>) {
                                callback(*number);
                            } else {
                                reportError();
                            }
                        } 
                        else {
                            reportError();
                        }
                    } else if (auto list = std::get_if<core::js_list_t>(&result)) {
                        if constexpr (std::is_same_v<ReturnType, std::vector<core::js_value_t>>) {
                            callback(*list);
                        } else {
                            reportError();
                        }
                    } else if (auto dict = std::get_if<core::js_dict_t>(&result)) {
                        if constexpr (std::is_same_v<ReturnType, std::unordered_map<std::string, core::js_value_t>>) {
                            callback(*dict);
                        } else {
                            reportError();
                        }
                    } else {
                        reportError();
                    }
                }
            });
        }

        template <typename ReturnType, typename... _Args>
        void evaluate(const std::string &script, const utils::args_pack<_Args...> &args, const std::function<void(ReturnType)>& callback)
        {
            evaluateImpl(utils::format_script(script, args.args), callback);
        }

        template <typename Function>
        void expose(const std::string &name, Function &&function, Launch police = Launch::Sync)
        {
            using args_tuple  = typename utils::function_traits<Function>::arg_types;
            using return_type = typename utils::function_traits<Function>::return_type;

            auto callback = [function = std::forward<Function>(function)](const webflex::JsArguments &js_args)
            {
                constexpr size_t args_count = std::tuple_size_v<args_tuple>;

                return utils::unpack_and_call(function,
                    [&]<std::size_t... I>(std::index_sequence<I...>)
                    {
                        return std::tuple<std::tuple_element_t<I, args_tuple>...>(
                            (utils::extract_value<std::tuple_element_t<I, args_tuple>>(js_args, I))...
                        );
                    }(std::make_index_sequence<args_count>{})
                );
            };

            if constexpr (!std::is_void_v<return_type>) 
            {
                exposeImplWithReturn(name, std::move(callback), police);
            }
            else 
            {
                exposeImplNoReturn(name, std::move(callback), police);
            }
        }

        void unexpose(const std::string &name);

        template <WindowEvent event>
        auto& on() 
        {
            if constexpr (event == WindowEvent::Close) 
            {
                return m_close_signal;
            }
            else if constexpr (event == WindowEvent::Closed) 
            {
                return m_closed_signal;
            }
            else if constexpr (event == WindowEvent::Resize) 
            {
                return m_resized_signal;
            }
            else if constexpr (event == WindowEvent::Maximized) 
            {
                return m_maximized_signal;
            }
            else if constexpr (event == WindowEvent::Minimized) 
            {
                return m_minimized_signal;
            }
            else {
                static_assert(sizeof(event) == 0, "Unknown WindowEvent type");
                return *this;
            }
        }

        template <WebEvent event>
        auto& on() 
        {
            if constexpr (event == WebEvent::DomReady) {
                return m_dom_ready_signal;
            } 
            else if constexpr (event == WebEvent::Navigated) {
                return m_navigated_signal;
            } 
            else if constexpr (event == WebEvent::FaviconChanged) {
                return m_favicon_changed_signal;
            } 
            else if constexpr (event == WebEvent::TitleChanged) {
                return m_title_changed_signal;
            } 
            else if constexpr (event == WebEvent::LoadProgress) {
                return m_load_progress_signal;
            } 
            else if constexpr (event == WebEvent::Scrolled) {
                return m_scrolled_signal;
            } 
            else if constexpr (event == WebEvent::LoadStarted) {
                return m_load_started_signal;
            } 
            else if constexpr (event == WebEvent::ZoomChanged) {
                return m_zoom_changed_signal;
            }
            else if constexpr (event == WebEvent::ClearHttpCache) {
                return m_clear_http_cache_signal;
            } 
            else if constexpr (event == WebEvent::CookieAdded) 
            {
                return m_cookie_added_signal;
            }
            else if constexpr (event == WebEvent::CookieRemoved) 
            {
                return m_cookie_removed_signal;
            }
            else {
                static_assert(sizeof(event) == 0, "Unknown WebEvent type");
                return *this;
            }
        }

        void clearHttpCache();

        void inject(InjectOptions options);

        std::shared_ptr<webflex::Navigation> &navigation();
        
        std::shared_ptr<webflex::Pdf> &pdf();

        std::shared_ptr<CookieStore>& cookies();

        void move(int x, int y);

        void move(const geometry::Point &point);

        geometry::Size size() const;

        geometry::Point position() const;

        void reload();
        
        void stop();

        void setTheme(ThemeMode mode);
        
        ThemeMode theme() const;

        int width() const;

        int height() const;

        int x() const;

        int y() const;

        void show(ShowMode mode = ShowMode::Normal);

        void hide();

        void close();

        bool isHidden();

        void loadHTML(const std::string &html);
        
        void loadFile(const std::string& path);

        bool isVisible() const;

        void setVisible(bool state);

        void setMaximumSize(const geometry::Size &size);
        
        void setMaximumSize(int w, int h);
        
        geometry::Size maximumSize() const;

        void setMinimumSize(const geometry::Size &size);
        
        void setMinimumSize(int w, int h);
        
        geometry::Size minimumSize() const;

        void setMaximumWidth(int w);
        
        void setMaximumHeight(int h);

        void setMinimumWidth(int w);
        
        void setMinimumHeight(int h);

        void htmlText(const std::function<void(const std::string &)> &result, bool wait_dom_ready = true);

        void plainText(const std::function<void(const std::string &)> &result, bool wait_dom_ready = true);

        void clearScripts();
        
    private:
        void exposeImplWithReturn(const std::string &name, const std::function<core::js_any_t(const JsArguments &args)> &callback, Launch police);
        void exposeImplNoReturn(const std::string &name, const std::function<void(const JsArguments &args)> &callback, Launch police);
        void evaluateImpl(const std::string& script, const std::function<void(const core::js_any_t &)> &resultCallback);
        void embedImpl(std::shared_ptr<JsAccessible> object, const std::string_view& className);

    private:
        template <typename T>
        bool setJsValue(const core::js_value_t &base, std::shared_ptr<std::promise<T>> promise) {
            if (auto value = std::get_if<std::string>(&base)) {
                promise->set_value(T(JsValue(*value)));
                return true;
            }
            if (auto value = std::get_if<bool>(&base)) {
                promise->set_value(T(JsValue(*value)));
                return true;
            }
            if (auto value = std::get_if<double>(&base)) {
                promise->set_value(T(JsValue(*value)));
                return true;
            }
            if (auto value = std::get_if<int>(&base)) {
                promise->set_value(T(JsValue((double)*value)));
                return true;
            }
            return false;
        }

        bool setJsValueMap(const core::js_value_t &base, JsMap &map, const std::string &key) 
        {
            if (auto value = std::get_if<std::string>(&base)) {
                map.insert(key, JsValue(*value));
                return true;
            }
            if (auto value = std::get_if<bool>(&base)) {
                map.insert(key, JsValue(*value));
                return true;
            }
            if (auto value = std::get_if<double>(&base)) {
                map.insert(key, JsValue(*value));
                return true;
            }
            if (auto value = std::get_if<int>(&base)) {
                map.insert(key, JsValue((double)*value));
                return true;
            }
            return false;   
        }
        
        bool setJsValue(const core::js_value_t &base, JsArray &array) {
            if (auto value = std::get_if<std::string>(&base)) {
                array.append(JsValue(*value));
                return true;
            }
            if (auto value = std::get_if<bool>(&base)) {
                array.append(JsValue(*value));
                return true;
            }
            if (auto value = std::get_if<double>(&base)) {
                array.append(JsValue(*value));
                return true;
            }
            if (auto value = std::get_if<int>(&base)) {
                array.append(JsValue((double)*value));
                return true;
            }
            return false;
        }

        void initConnections();

        friend class Navigation;
        friend class Settings;
        friend class Pdf;
        friend class JsArguments;
        friend class CookieStore;

    private:
        std::unique_ptr<impl::BrowserImpl> m_impl;

        webflex::Icon m_icon;

        webflex::Url m_url;

        bool m_dom_ready = false;

        ThemeMode m_theme_mode;

        geometry::Size m_maximum_size;

        geometry::Size m_minimum_size;

        std::vector<std::pair<bool, std::string>> m_scripts_permanent;

        std::vector<std::pair<std::string, std::function<void(const core::js_any_t &)>>> m_pending_scripts;

        std::shared_ptr<webflex::Navigation> m_navigation;

        std::shared_ptr<CookieStore> m_cookies;

        std::shared_ptr<webflex::Pdf> m_pdf;

        static std::unordered_set<std::string> m_register_schemes;

        std::unordered_set<std::string> m_script_set;

        Color m_background_color;

        std::unordered_set<std::string_view> m_embed_classes;

        std::unordered_map<std::string, ByteArray> m_read_data;

    private:
        sigslot::signal<> m_close_signal;
        sigslot::signal<> m_closed_signal;
        sigslot::signal<bool> m_maximized_signal;
        sigslot::signal<bool> m_minimized_signal;
        sigslot::signal<int, int> m_resized_signal;

        sigslot::signal<bool> m_dom_ready_signal;
        sigslot::signal<Icon> m_favicon_changed_signal;
        sigslot::signal<std::string> m_title_changed_signal;
        sigslot::signal<int> m_load_progress_signal;
        sigslot::signal<double, double> m_scrolled_signal;
        sigslot::signal<> m_load_started_signal;
        sigslot::signal<double> m_zoom_changed_signal;
        sigslot::signal<Url> m_navigated_signal;
        sigslot::signal<> m_clear_http_cache_signal;
        sigslot::signal<Cookie> m_cookie_added_signal;
        sigslot::signal<Cookie> m_cookie_removed_signal;
    };
}

#endif