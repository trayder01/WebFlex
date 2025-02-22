#ifndef WJSARGS_HPP
#define WJSARGS_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include <cstdint>
#include <optional>
#include <stdexcept>

#include "core/js_types.hpp"

namespace webflex
{
    class JsValue
    {
    public:
        JsValue();

        explicit JsValue(const std::string &value);
        explicit JsValue(double value);
        explicit JsValue(bool value);

        JsValue(const JsValue &other);
        JsValue &operator=(const JsValue &other);
        JsValue(JsValue &&other) noexcept;

        JsValue &operator=(JsValue &&other) noexcept;
        bool operator==(const JsValue &other) const;
        bool operator!=(const JsValue &other) const;

        bool isString() const;
        bool isBool() const;
        bool isNumber() const;

        std::optional<std::string> asString() const;
        std::optional<bool> asBool() const;
        std::optional<double> asNumber() const;

        operator bool() const
        {
            return isEmpty();
        }

        bool isEmpty() const
        {
            return isString() || isBool() || isNumber();
        }

    private:
        core::js_value_t m_value;
    };

    class JsArray
    {
    public:
        JsArray();
        explicit JsArray(size_t reserve);
        explicit JsArray(const std::initializer_list<JsValue> &list);

        bool operator==(const JsArray &other) const;
        bool operator!=(const JsArray &other) const;

        void append(const JsValue &value);
        void insert(size_t index, const JsValue &value);

        JsValue at(size_t index);
        const JsValue &at(size_t index) const;

        JsValue operator[](size_t index);
        const JsValue &operator[](size_t index) const;

        bool isEmpty() const { return m_array.empty(); }

        std::vector<webflex::JsValue>::const_iterator begin() const { return m_array.begin(); }
        std::vector<webflex::JsValue>::const_iterator end() const { return m_array.end(); }

        std::vector<webflex::JsValue>::iterator begin() { return m_array.begin(); }
        std::vector<webflex::JsValue>::iterator end() { return m_array.end(); }

        size_t size() const { return m_array.size(); }

    private:
        std::vector<JsValue> m_array;
    };

    class JsMap
    {
    public:
        JsMap();

        JsMap(const JsMap &other);
        JsMap &operator=(const JsMap &other);

        JsMap(JsMap &&other) noexcept;
        JsMap &operator=(JsMap &&other) noexcept;

        bool operator==(const JsMap &other) const;
        bool operator!=(const JsMap &other) const;

        void insert(const std::string &key, const JsValue &value);

        void erase(const std::string &key);

        JsValue get(const std::string &key) const;

        JsValue operator[](const std::string &key) const;

        bool contains(const std::string &key) const;

        std::unordered_map<std::string, webflex::JsValue>::const_iterator begin() const { return m_map.begin(); }

        std::unordered_map<std::string, webflex::JsValue>::const_iterator end() const { return m_map.end(); }

        std::unordered_map<std::string, webflex::JsValue>::iterator begin() { return m_map.begin(); }

        std::unordered_map<std::string, webflex::JsValue>::iterator end() { return m_map.end(); }

        size_t size() const { return m_map.size(); }
        bool isEmpty() const { return m_map.empty(); }

    private:
        std::unordered_map<std::string, JsValue> m_map;
    };

    class JsArguments
    {
    public:
        JsArguments() = default;

        JsArguments(const JsArguments &other)
            : m_values(other.m_values), m_arrays(other.m_arrays), m_maps(other.m_maps) {}

        JsArguments(JsArguments &&other) noexcept
            : m_values(std::move(other.m_values)),
              m_arrays(std::move(other.m_arrays)),
              m_maps(std::move(other.m_maps)) {}

        JsArguments &operator=(const JsArguments &other)
        {
            if (this != &other)
            {
                m_values = other.m_values;
                m_arrays = other.m_arrays;
                m_maps = other.m_maps;
            }
            return *this;
        }

        JsArguments &operator=(JsArguments &&other) noexcept
        {
            if (this != &other)
            {
                m_values = std::move(other.m_values);
                m_arrays = std::move(other.m_arrays);
                m_maps = std::move(other.m_maps);
            }
            return *this;
        }

        template <typename T>
        std::optional<T> as(size_t index) const
        {
            if (index < m_values.size())
            {
                if constexpr (std::is_same_v<T, JsValue>)
                {
                    return m_values[index];
                }
                else if constexpr (std::is_same_v<T, JsArray>)
                {
                    return m_arrays[index];
                }
                else if constexpr (std::is_same_v<T, JsMap>)
                {
                    return m_maps[index];
                }
            }
            return std::nullopt;
        }

        template <typename T>
        void add(const T &value)
        {
            if constexpr (std::is_same_v<T, JsValue>)
            {
                m_values.push_back(value);
            }
            else if constexpr (std::is_same_v<T, JsArray>)
            {
                m_arrays.push_back(value);
            }
            else if constexpr (std::is_same_v<T, JsMap>)
            {
                m_maps.push_back(value);
            }
        }

        const std::vector<JsMap> &maps() const { return m_maps; }

        const std::vector<JsArray> &arrays() const { return m_arrays; }

        const std::vector<JsValue> &values() const { return m_values; }

    private:
        std::vector<JsValue> m_values;
        std::vector<JsArray> m_arrays;
        std::vector<JsMap> m_maps;
    };

    namespace utils
    {
        template <typename T>
        inline T extract_value(const webflex::JsArguments &js_args, size_t index)
        {
            if constexpr (std::is_same_v<T, JsValue>)
            {
                return js_args.values().at(index);
            }
            else if constexpr (std::is_same_v<T, JsArray>)
            {
                return js_args.arrays().at(index);
            }
            else if constexpr (std::is_same_v<T, JsMap>)
            {
                return js_args.maps().at(index);
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                auto value = js_args.values().at(index);
                return value.asNumber().value_or(0.0);
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                auto value = js_args.values().at(index);
                return value.asString().value_or("");
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                auto value = js_args.values().at(index);
                return value.asBool().value_or(false);
            }
            else if constexpr (std::is_same_v<T, int>)
            {
                auto value = js_args.values().at(index);
                return value.asNumber().value_or(0.0);
            }

            return T{};
        }
    }
}

#endif
