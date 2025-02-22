#pragma once

#include <QIcon>
#include <QByteArray>
#include <QImage>
#include <QDebug>
#include <QBuffer>
#include <QVariant>

#include "../icon.hpp"
#include "../js_args.hpp"

namespace webflex::utils {
    inline QIcon convertIcon(const Icon& icon) 
    {
        auto icon_data = icon.data();
        QByteArray data = {reinterpret_cast<const char*>(icon_data.data()), static_cast<qsizetype>(icon_data.size())};   

        if (data.isEmpty()) 
        {
            qWarning() << "Failed data is empty.";
            return QIcon{};
        }

        QImage image;

        if (!image.loadFromData(data)) 
        {
            qWarning() << "Failed to load image from data. Cannot create WIcon.";
            return QIcon{};
        }

        return QIcon(QPixmap::fromImage(image));
    }

    inline Icon convertIcon(const QIcon& icon) 
    {
        Icon wIcon;
        QPixmap pixmap = icon.pixmap(64, 64); 
        QImage image = pixmap.toImage();

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        if (image.save(&buffer, "PNG")) {
            std::vector<uint8_t> iconData(byteArray.begin(), byteArray.end());
            wIcon.loadFromMemory(iconData.data(), iconData.size());
        }

        return wIcon;
    }

    inline webflex::core::js_any_t convertQvariantToStdVariant(const QVariant& arg) 
    {
        using namespace webflex::core;
        
        if (!arg.isValid() || arg.isNull()) 
        {
            return std::monostate{}; 
        }

        switch (arg.typeId()) 
        {
            case QVariant::Bool: {
                return arg.toBool();
            }
            case QVariant::Int: {
                return static_cast<double>(arg.toInt());
            }
            case QVariant::UInt: {
                return static_cast<double>(arg.toUInt());
            }
            case QVariant::LongLong: {
                return static_cast<double>(arg.toLongLong());
            }
            case QVariant::ULongLong: {
                return static_cast<double>(arg.toULongLong());
            }
            case QVariant::Double: {
                return arg.toDouble();
            }
            case QVariant::String: {
                return arg.toString().toStdString();
            }
            case QVariant::List: 
            {
                js_list_t list;
                for (const auto& item : arg.toList()) 
                {
                    list.push_back(std::get<std::variant<std::monostate, int, double, bool, std::string>>(convertQvariantToStdVariant(item)));
                }
                return list;
            }
            case QVariant::Map:
            case QVariant::Hash: 
            {
                js_dict_t dict;
                auto map = arg.toMap();
                for (auto it = map.begin(); it != map.end(); ++it) 
                {
                    dict[it.key().toStdString()] = std::get<std::variant<std::monostate, int, double, bool, std::string>>(convertQvariantToStdVariant(it.value()));
                }
                return dict;
            }
            default: {
                break;
            }
        }

        return std::monostate{}; 
    }

    inline QVariant fromStdVariantToQvariant(const webflex::core::js_any_t& any) 
    {
        if (auto base = std::get_if<webflex::core::js_value_t>(&any)) 
        {
            if (auto string = std::get_if<std::string>(base)) 
            {
                return QString::fromStdString(*string);
            }
            else if (auto double_ = std::get_if<double>(base)) 
            {
                return *double_;
            }
            else if (auto bool_ = std::get_if<bool>(base)) 
            {
                return *bool_;
            }
            else if (auto int_ = std::get_if<int>(base)) 
            {
                return *int_;
            }
        }
        else if (auto vector = std::get_if<webflex::core::js_list_t>(&any)) 
        {
            QVariantList qList;
            for (const auto& item : *vector)
            {
                qList.append(fromStdVariantToQvariant(item));
            }
            return qList;
        }
        else if (auto dict = std::get_if<webflex::core::js_dict_t>(&any)) 
        {
            QVariantMap qMap;
            for (const auto& [key, value] : *dict)
            {
                qMap[QString::fromStdString(key)] = fromStdVariantToQvariant(value);
            }
            return qMap;
        }
        
        return QVariant{}; 
    }

    inline void fillArguments(JsArguments& arguments, const QVariant& variant) 
    {
        auto value = utils::convertQvariantToStdVariant(variant);

        if (auto base_type = std::get_if<core::js_value_t>(&value)) 
        {
            if (auto value = std::get_if<std::string>(base_type)) 
            {
                arguments.add(JsValue(*value));
            }
            else if (auto value = std::get_if<bool>(base_type)) 
            {
                arguments.add(JsValue(*value));
            }
            else if (auto value = std::get_if<double>(base_type)) 
            {
                arguments.add(JsValue(*value));
            }
            else if (auto value = std::get_if<int>(base_type)) 
            {
                arguments.add(JsValue((double)*value));
            }
        }
        else if (auto list_type = std::get_if<core::js_list_t>(&value)) 
        {
            JsArray array(list_type->size());

            for (const auto& val : *list_type) 
            {
                if (auto value = std::get_if<std::string>(&val)) 
                {
                    array.append(JsValue(*value));
                }
                else if (auto value = std::get_if<bool>(&val)) 
                {
                    array.append(JsValue(*value));
                }
                else if (auto value = std::get_if<double>(&val)) 
                {
                    array.append(JsValue(*value));
                }
                else if (auto value = std::get_if<int>(&val)) 
                {
                    arguments.add(JsValue((double)*value));
                }
            }

            arguments.add(array);
        }
        else if (auto dict_type = std::get_if<core::js_dict_t>(&value)) 
        {
            JsMap map;

            for (const auto& val : *dict_type) 
            {
                if (auto value = std::get_if<std::string>(&val.second)) 
                {
                    map.insert(val.first, JsValue(*value));
                }
                else if (auto value = std::get_if<bool>(&val.second)) 
                {
                    map.insert(val.first, JsValue(*value));
                }
                else if (auto value = std::get_if<double>(&val.second)) 
                {
                    map.insert(val.first, JsValue(*value));
                }
                else if (auto value = std::get_if<int>(&val.second)) 
                {
                    arguments.add(JsValue((double)*value));
                }
            }

            arguments.add(map);
        }   
    }

    inline std::string systemTheme()
    {
#if __linux__
        const char *command = R"(gsettings get org.gnome.desktop.interface color-scheme)";
        char buffer[128];
        std::string result;

        FILE *pipe = popen(command, "r");
        if (!pipe)
        {
            
            return "unknown";
        }

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }

        pclose(pipe);

        return result;
#elif _WIN32 || _WIN64
            
        try
        {
            HKEY hKey;
            DWORD value;
            DWORD valueSize = sizeof(value);
            if (RegOpenKeyEx(HKEY_CURRENT_USER, R"(Software\Microsoft\Windows\CurrentVersion\Themes\Personalize)", 0, KEY_READ, &hKey) == ERROR_SUCCESS)
            {
                if (RegQueryValueEx(hKey, L"AppsUseLightTheme", nullptr, nullptr, reinterpret_cast<LPBYTE>(&value), &valueSize) == ERROR_SUCCESS)
                {
                    RegCloseKey(hKey);
                    return value == 0 ? "dark" : "light";
                }
                RegCloseKey(hKey);
            }
        }
        catch (...)
        {
            
        }
        return "unknown";

#elif __APPLE__
            
        const char *command = R"(defaults read -g AppleInterfaceStyle)";
        char buffer[128];
        std::string result;

        FILE *pipe = popen(command, "r");
        if (!pipe)
        {
            
            return "unknown";
        }

        while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        {
            result += buffer;
        }

        pclose(pipe);

        return result;
#endif
    } 
}