#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <vector>

namespace minipp
{
    enum class EResult
    {
        /* Errors */
        KeyNotPresent = -1,
        KeyAlreadyPresent = -2,
        SectionNotPresent = -3,
        SectionAlreadyPresent = -4,
        FileIOError = -5,
        InvalidDataType = -6,
        FormatError = -7,

        /* OK Codes*/
        Success = 1,
        ValueOverwritten,
        ValueOverwrittenAndDataTypeChanged
    };

    class MiniPPFile
    {
    private:
        class Tools
        {
        public:
            static bool StringStartsWith(const std::string& str, const std::string& beg);
            static bool StringEndsWith(const std::string& str, const std::string& end);
            static void StringTrim(std::string& str);
            static bool IsNameValid(const std::string& name) noexcept;
            static int64_t FirstIndexOf(const std::string& str, char c) noexcept;
            static int64_t LastIndexOf(const std::string& str, char c) noexcept;
            static std::pair<std::string, std::string> SplitInTwo(const std::string& str, int64_t firstLength) noexcept;
            static std::vector<std::string> SplitByDelimiter(const std::string& str, char delimiter) noexcept;
            static bool IsResultOk(EResult result) noexcept;
            static void RemoveAll(std::string& str, char old);
			static bool IsIntegerDecimal(const std::string& str) noexcept;
        };

    public:
        class Value
        {
        public:
            virtual EResult Parse(const std::string& str) noexcept = 0;
            virtual EResult ToString(std::string& destination) const noexcept = 0;
        };

        class StringValue : public Value
        {
        private:
            std::string m_value;

        public:
            StringValue() = default;
            StringValue(const std::string& str);
            EResult Parse(const std::string& str) noexcept override;
            EResult ToString(std::string& destination) const noexcept override;
            const std::string& GetValue() const noexcept;
        };

        class IntValue : public Value
        {
        private:
			int64_t m_value = 0;

		public:
			IntValue() = default;
			IntValue(int64_t value);
			EResult Parse(const std::string& str) noexcept override;
			EResult ToString(std::string& destination) const noexcept override;
			int64_t GetValue() const noexcept;
        };

        class Section
        {
        private:
            std::unordered_map<std::string, Value*> m_values;
            std::unordered_map<std::string, Section*> m_subSections;

        public:
            Section() = default;
            ~Section();
            Section(const Section&) = delete;
			Section& operator=(const Section&) = delete;

        public:
            template<typename T>
            EResult GetValue(T** target, const std::string& key) noexcept
            {
                auto it = m_values.find(key);
                if (it == m_values.end())
                    return EResult::KeyNotPresent;

                auto val = dynamic_cast<T*>(it->second);
                if (val == nullptr)
                    return EResult::InvalidDataType;

                *target = val;
                return EResult::Success;
            }

            template<typename T>
            EResult SetValue(const std::string& name, const T& value, bool allowOverwrite = false) noexcept
            {
                if (m_values.find(name) != m_values.end())
                    if (!allowOverwrite)
                        return EResult::KeyAlreadyPresent;
                    else
                        delete m_values[name];

                m_values[name] = new T(value);
                return EResult::Success;
            }

        public:
            Section* GetSubSection(const std::string& key, EResult* result = nullptr) const noexcept;
            Section* SetSubSection(const std::string& name, std::unique_ptr<Section> value, bool allowOverwrite = false, EResult* result = nullptr) noexcept;
        };

    public:
        MiniPPFile() = default;

    private:
        Section m_rootSection{};

    public:
        EResult Parse(const std::string& path) noexcept;
        void Write(const std::string& path) noexcept;

    public:
        const Section& GetRoot() const noexcept { return m_rootSection; }
        Section& GetRoot() noexcept { return m_rootSection; }
    };
}