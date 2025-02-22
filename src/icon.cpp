#include "webflex/icon.hpp"
#include <fstream>
#include <iostream>
#include <QImage>
#include <QBuffer>

namespace webflex
{
    Icon::Icon() : m_icon_data{} {}

    Icon::Icon(const std::filesystem::path &path) : Icon()
    {
        loadFromFile(path);
    }

    Icon::Icon(uint8_t *data, size_t size) : Icon()
    {
        loadFromMemory(data, size);
    }

    Icon::Icon(const Icon &other) : m_icon_data(other.m_icon_data)
    {
    }

    Icon &Icon::operator=(const Icon &other)
    {
        if (this != &other)
        {
            m_icon_data = other.m_icon_data;
        }

        return *this;
    }

    Icon::Icon(Icon &&other) : m_icon_data(std::move(other.m_icon_data))
    {
    }

    Icon &Icon::operator=(Icon &&other)
    {
        if (this != &other)
        {
            m_icon_data = std::move(other.m_icon_data);
        }

        return *this;
    }

    bool Icon::operator==(const Icon &other)
    {
        return other.m_icon_data == m_icon_data;
    }

    bool Icon::operator!=(const Icon &other)
    {
        return !(*this == other);
    }

    bool Icon::loadFromFile(const std::filesystem::path &path)
    {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return false;
        }

        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        m_icon_data.resize(fileSize);
        if (!file.read(reinterpret_cast<char *>(m_icon_data.data()), fileSize))
        {
            std::cerr << "Failed to read file: " << path << std::endl;
            return false;
        }

        return true;
    }

    bool Icon::saveToFile(const std::filesystem::path &path) const
    {
        if (m_icon_data.empty())
        {
            std::cerr << "No data to save." << std::endl;
            return false;
        }

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Failed to open file: " << path << std::endl;
            return false;
        }

        file.write(reinterpret_cast<const char *>(m_icon_data.data()), m_icon_data.size());

        bool good = file.good();

        file.close();

        return good;
    }

    bool Icon::loadFromMemory(uint8_t *data, size_t size)
    {
        if (!data || size == 0)
        {
            std::cerr << "Invalid memory data or size" << std::endl;
            return false;
        }

        m_icon_data.assign(data, data + size);
        return true;
    }

    void Icon::clear()
    {
        m_icon_data.clear();
    }

    bool Icon::isEmpty() const
    {
        return m_icon_data.empty();
    }

    const std::vector<uint8_t> &Icon::data() const
    {
        return m_icon_data;
    }

    void Icon::resize(int w, int h)
    {
        if (m_icon_data.empty())
        {
            std::cerr << "No Icon data to resize." << std::endl;
            return;
        }

        QImage image;
        if (!image.loadFromData(m_icon_data.data(), static_cast<int>(m_icon_data.size())))
        {
            std::cerr << "Failed to load image from data." << std::endl;
            return;
        }

        QImage resizedImage = image.scaled(m_width, m_height, Qt::KeepAspectRatio, Qt::SmoothTransformation);

        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        buffer.open(QIODevice::WriteOnly);
        resizedImage.save(&buffer, "PNG");

        m_icon_data = std::vector<uint8_t>(byteArray.begin(), byteArray.end());
    }

    void Icon::resize(const geometry::Size& size) 
    {
        resize(size.width(), size.height());
    }

    geometry::Size Icon::size() const 
    {
        return geometry::Size{m_width, m_height};
    }
}