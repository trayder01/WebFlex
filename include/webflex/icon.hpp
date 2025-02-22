#ifndef WICON_HPP
#define WICON_HPP

#include <filesystem>
#include <vector>
#include "geometry/size.hpp"

namespace webflex {
    class Icon 
    {
    public:
        Icon();
        Icon(const std::filesystem::path& path);
        Icon(uint8_t* data, size_t size);

        Icon(const Icon& other);
        Icon& operator=(const Icon& other);

        Icon(Icon&& other);
        Icon& operator=(Icon&& other);

        bool operator==(const Icon& other);
        bool operator!=(const Icon& other);

        bool saveToFile(const std::filesystem::path& path) const;

        bool loadFromFile(const std::filesystem::path& path);
        bool loadFromMemory(uint8_t* data, size_t size);

        const std::vector<uint8_t>& data() const;

        void clear();

        bool isEmpty() const;

        void resize(int w, int h);
        void resize(const geometry::Size& size);

        geometry::Size size() const;

    private:
        std::vector<uint8_t> m_icon_data;
        int m_width;
        int m_height;
    };
}

#endif // WICON_HPP