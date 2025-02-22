#include "webflex/js_args.hpp"
#include "webflex/defines.hpp"

namespace webflex
{
    JsValue::JsValue() : m_value{std::monostate{}} {}

    JsValue::JsValue(const std::string& value) : m_value{value}
    {
    }

    JsValue::JsValue(double value) : m_value{value}
    {
    }

    JsValue::JsValue(bool value) : m_value{value}
    {
    }

    JsValue::JsValue(const JsValue& other) : m_value(other.m_value) {}

    JsValue& JsValue::operator=(const JsValue& other)
    {
        if (this != &other) {
            m_value = other.m_value;
        }
        return *this;
    }

    JsValue::JsValue(JsValue&& other) noexcept : m_value(std::move(other.m_value)) {}

    JsValue& JsValue::operator=(JsValue&& other) noexcept
    {
        if (this != &other) {
            m_value = std::move(other.m_value);
        }
        return *this;
    }

    bool JsValue::operator==(const JsValue& other) const
    {
        return m_value == other.m_value;
    }

    bool JsValue::operator!=(const JsValue& other) const
    {
        return m_value != other.m_value;
    }

    bool JsValue::isString() const 
    {
        return std::holds_alternative<std::string>(m_value);
    }

    bool JsValue::isBool() const 
    {
        return std::holds_alternative<bool>(m_value);
    }
    
    bool JsValue::isNumber() const 
    {
        return std::holds_alternative<double>(m_value);
    }

    std::optional<std::string> JsValue::asString() const 
    {
        if (auto found = std::get_if<std::string>(&m_value)) 
        {
            return std::make_optional(*found);
        }
        return std::nullopt;
    }

    std::optional<bool> JsValue::asBool() const 
    {
        if (auto found = std::get_if<bool>(&m_value)) 
        {
            return std::make_optional(*found);
        }
        return std::nullopt;
    }
    
    std::optional<double> JsValue::asNumber() const 
    {
        if (auto found = std::get_if<double>(&m_value)) 
        {
            return std::make_optional(*found);
        }
        return std::nullopt;
    }

// ----------------------------------------------------------------------------- 

    JsArray::JsArray() : m_array{}
    {
    }

    JsArray::JsArray(size_t reserve) : JsArray()
    {
        m_array.reserve(reserve);
    }
    
    JsArray::JsArray(const std::initializer_list<JsValue>& list) : m_array(list)
    {
    }

    bool JsArray::operator==(const JsArray& other) const 
    {
        return m_array == other.m_array;
    }
    
    bool JsArray::operator!=(const JsArray& other) const 
    {
        return !(other == *this);
    }

    void JsArray::append(const JsValue& value) 
    {
        m_array.push_back(value);
    }

    void JsArray::insert(size_t index, const JsValue& value) 
    {
        if (index >= size()) {
            W_THROW(std::out_of_range("Index out of range"));
        }
        m_array.insert(m_array.begin() + index, value);
    }

    const JsValue& JsArray::at(size_t index) const 
    {
        if (index >= m_array.size()) {
            W_THROW(std::out_of_range("Index out of range"));
        }
        return m_array[index];
    }

    JsValue JsArray::at(size_t index) 
    {
        if (index >= m_array.size()) {
            W_THROW(std::out_of_range("Index out of range"));
        }
        return m_array[index];
    }

    JsValue JsArray::operator[](size_t index) 
    {
        return at(index);
    }

    const JsValue& JsArray::operator[](size_t index) const 
    {
        return at(index);
    }

// ----------------------------------------------------------------------------- 

    JsMap::JsMap() : m_map{} {}

    JsMap::JsMap(const JsMap& other) : m_map(other.m_map) {}

    JsMap& JsMap::operator=(const JsMap& other) {
        if (this != &other) {
            m_map = other.m_map;
        }
        return *this;
    }

    JsMap::JsMap(JsMap&& other) noexcept : m_map(std::move(other.m_map)) {}

    JsMap& JsMap::operator=(JsMap&& other) noexcept {
        if (this != &other) {
            m_map = std::move(other.m_map);
        }
        return *this;
    }

    bool JsMap::operator==(const JsMap& other) const {
        return m_map == other.m_map;
    }

    bool JsMap::operator!=(const JsMap& other) const {
        return m_map != other.m_map;
    }

    void JsMap::insert(const std::string& key, const JsValue& value) 
    {
        m_map[key] = value;
    }

    void JsMap::erase(const std::string& key) 
    {
        m_map.erase(key);   
    }
    
    JsValue JsMap::get(const std::string& key) const 
    {
        if (contains(key)) 
        {
            return m_map.at(key);
        }

        return JsValue{};
    }

    JsValue JsMap::operator[](const std::string& key) const
    {
        return get(key);
    }

    bool JsMap::contains(const std::string& key) const 
    {
        return m_map.find(key) != m_map.end();
    }
}
