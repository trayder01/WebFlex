#ifndef WBYTEARRAY_HPP
#define WBYTEARRAY_HPP

#include <memory>
#include <string>
#include <vector>

namespace webflex {
    namespace impl {
        struct ByteArrayImpl;
    }

    class ByteArray
    {
    public:
        ByteArray();
        explicit ByteArray(const std::string &data);
        explicit ByteArray(const std::vector<uint8_t> &data);

        ~ByteArray();

        ByteArray(const ByteArray &other);
        ByteArray(ByteArray &&other) noexcept;

        ByteArray &operator=(const ByteArray &other);
        ByteArray &operator=(ByteArray &&other) noexcept;

        [[nodiscard]] std::string toString() const;
        [[nodiscard]] std::vector<uint8_t> data() const;
        [[nodiscard]] std::size_t size() const;

        void append(const std::string &data);
        void append(uint64_t n, char ch);
        void append(const char *s, uint64_t len);
        void append(const char* s);
        void append(char ch);
        void clear();

        char& back() const;

        char& front() const;

    private:
        std::unique_ptr<impl::ByteArrayImpl> impl_;
    };
}

#endif // WBYTEARRAY_HPP
