#pragma once
#include <cstdint>
#include <unordered_map>
#include <string>
#include <memory>
#include <utility>
#include <vector>

#define MINIPP_ENABLE_DEBUG_OUTPUT true

// define MINIPP_IMPLEMENTATION in one single translation unit before including this header!

namespace minipp
{
    enum class EResult
    {
        /* Errors */
        KeyNotPresent               = -1,
        KeyAlreadyPresent           = -2,
        SectionNotPresent           = -3,
        SectionAlreadyPresent       = -4,
        FileIOError                 = -5,
        InvalidDataType             = -6,
        FormatError                 = -7,
		ArrayDataTypeInconsitency   = -8,

        /* OK Codes */
        Success                     = +1,
        ValueOverwritten            = +2
    };

	enum class EIntStyle
	{
		Decimal,
		Hexadecimal,
		Binary
	};

    class MiniPPFile
    {
    public:
        class Value
        {
            friend class MiniPPFile;

        protected:
            std::vector<std::string> m_comments;

        public:
            virtual EResult Parse(const std::string& str) noexcept = 0;
            virtual EResult ToString(std::string& destination) const noexcept = 0;
            virtual ~Value() = default;
			std::vector<std::string>& GetComments() noexcept { return m_comments; }
			const std::vector<std::string>& GetComments() const noexcept { return m_comments; }
        };

        class Values
        {
        public:
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
				EIntStyle m_style = EIntStyle::Decimal;

            public:
                IntValue() = default;
                IntValue(int64_t value);
                EResult Parse(const std::string& str) noexcept override;
                EResult ToString(std::string& destination) const noexcept override;
                int64_t GetValue() const noexcept;
            };

            class BooleanValue : public Value
            {
            private:
                bool m_value = false;

            public:
                BooleanValue() = default;
                BooleanValue(bool value);
                EResult Parse(const std::string& str) noexcept override;
                EResult ToString(std::string& destination) const noexcept override;
                bool GetValue() const noexcept;
            };

            class FloatValue : public Value
            {
            private:
                double m_value = 0.0f;

            public:
                FloatValue() = default;
                FloatValue(double value);
                EResult Parse(const std::string& str) noexcept override;
                EResult ToString(std::string& destination) const noexcept override;
                double GetValue() const noexcept;
            };

            class ArrayValue : public Value
            {
            private:
                std::vector<std::unique_ptr<Value>> m_values;

            public:
                ArrayValue() = default;
                ArrayValue(const ArrayValue&) = delete;
                ArrayValue& operator=(const ArrayValue&) = delete;

            public:
                EResult Parse(const std::string& str) noexcept override;
                EResult ToString(std::string& destination) const noexcept override;
                std::vector<std::unique_ptr<Value>>& GetValues() noexcept { return m_values; }
                const std::vector<std::unique_ptr<Value>>& GetValues() const noexcept { return m_values; }

                Value* operator[](size_t index) noexcept
                {
                    if (index >= m_values.size())
                        return nullptr;
                    return m_values[index].get();
                }
            };
        };
       
        class Section
        {
			friend class MiniPPFile;

        private:
            std::unordered_map<std::string, Value*> m_values;
            std::unordered_map<std::string, Section*> m_subSections;
            std::vector<std::string> m_comments;

        public:
			std::vector<std::string>& GetComments() noexcept { return m_comments; }
			const std::vector<std::string>& GetComments() const noexcept { return m_comments; }

        public:
            Section() = default;
            ~Section();
            Section(const Section&) = delete;
			Section& operator=(const Section&) = delete;

        public:
            template<typename T>
            EResult GetValue(const std::string& key, T** target) noexcept
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
            EResult SetValue(const std::string& name, std::unique_ptr<T> value, bool allowOverwrite = false) noexcept
            {
                if (m_values.find(name) != m_values.end())
                    if (!allowOverwrite)
                        return EResult::KeyAlreadyPresent;
                    else
                        delete m_values[name];

                m_values[name] = value.release();
                return EResult::Success;
            }

        public:
            EResult GetSubSection(const std::string& key, Section** destination) const noexcept;
            EResult SetSubSection(const std::string& name, std::unique_ptr<Section> value, bool allowOverwrite = false) noexcept;
        };

    public:
        MiniPPFile() = default;

    private:
        Section m_rootSection{};

    private:
        static std::unique_ptr<Value> ParseValue(std::string value);
        static minipp::EResult WriteSection(const Section* section, std::ofstream& ofs) noexcept;

    public:
        EResult Parse(const std::string& path) noexcept;
        void Write(const std::string& path) const noexcept;

    public:
        const Section& GetRoot() const noexcept { return m_rootSection; }
        Section& GetRoot() noexcept { return m_rootSection; }

    public:
		static bool IsResultOk(EResult result) noexcept;

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
            static void RemoveAll(std::string& str, char old);
            static bool IsIntegerDecimal(const std::string& str) noexcept;
        };
    };
}

#ifdef MINIPP_IMPLEMENTATION

#include <fstream>
#include <typeinfo>
#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <bitset>

#if MINIPP_ENABLE_DEBUG_OUTPUT
#include <iostream>
#define COUT(msg) std::cout << "[minipp] " << msg << std::endl
#else
#define COUT(msg)
#endif

#define ASSERT(condition) if (!condition) abort();

#define COUT_SYNTAX_ERROR(line, msg) COUT("Error in line " << line << ": " << msg)

#pragma region Value Types

minipp::MiniPPFile::Values::StringValue::StringValue(const std::string& str) : m_value(str) {}

minipp::EResult minipp::MiniPPFile::Values::StringValue::Parse(const std::string& str) noexcept
{
    m_value = "";
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\\')
        {
            if (i + 1 >= str.size())
                return EResult::FormatError;

            switch (str[i + 1])
            {
            case '\"':
                m_value.push_back('\"');
                break;
            case 'n':
                m_value.push_back('\n');
                break;
            case 't':
                m_value.push_back('\t');
                break;
            case 'r':
                m_value.push_back('\r');
                break;
            case '\\':
                m_value.push_back('\\');
                break;
            default:
                return EResult::FormatError;
            }
            ++i;
        }
        else if (str[i] == '"')
            return EResult::FormatError;
        else
            m_value.push_back(str[i]);
    }
    return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::StringValue::ToString(std::string& destination) const noexcept
{
    std::string sanitizedValue = m_value;
    for (size_t i = 0; i < sanitizedValue.size(); ++i)
    {
        switch (sanitizedValue[i])
        {
        case '\n':
            sanitizedValue.replace(i, 1, "\\n");
            i++;
            break;
        case '\t':
            sanitizedValue.replace(i, 1, "\\t");
            i++;
            break;
        case '\r':
            sanitizedValue.replace(i, 1, "\\r");
            i++;
            break;
        case '\\':
            sanitizedValue.insert(i, 1, '\\');
            i++;
            break;
        case '\"':
            sanitizedValue.insert(i, 1, '\\');
            i++;
            break;
        }
    }
    destination = "\"" + sanitizedValue + "\"";
    return EResult::Success;
}

const std::string& minipp::MiniPPFile::Values::StringValue::GetValue() const noexcept
{
    return m_value;
}

minipp::MiniPPFile::Values::IntValue::IntValue(int64_t value) : m_value(value) {}

minipp::EResult minipp::MiniPPFile::Values::IntValue::Parse(const std::string& str) noexcept
{
    std::string sanitizedValue = str;
    Tools::RemoveAll(sanitizedValue, '_');
    if (sanitizedValue.empty())
        return EResult::FormatError;

    char lastCharacter = str.back();
    auto rest = str.substr(0, str.size() - 1);
    if (lastCharacter == 'h')
    {
        m_value = std::stoll(rest, nullptr, 16);
        m_style = EIntStyle::Hexadecimal;
    }
    else if (lastCharacter == 'b')
    {
        m_value = std::stoll(rest, nullptr, 2);
		m_style = EIntStyle::Binary;
    }
    else
    {
        if (!Tools::IsIntegerDecimal(sanitizedValue))
            return EResult::FormatError;
        m_value = std::stoll(sanitizedValue);
		m_style = EIntStyle::Decimal;
    }
    return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::IntValue::ToString(std::string& destination) const noexcept
{
    switch (m_style)
    {
    case EIntStyle::Decimal:
    {
        destination = std::to_string(m_value);
        break;
    }
    case EIntStyle::Hexadecimal:
    {
        std::stringstream ss;
		ss << std::hex << m_value << "h";
        destination = ss.str();
        break;
    }
	case EIntStyle::Binary:
	{
		std::bitset<64> bs(m_value);
        destination = bs.to_string() + "b";
        size_t i;
		for (i = 0; i < destination.size(); ++i)
			if (destination[i] == '1')
			{
				destination = destination.substr(i);
				break;
			}
		if (i == destination.size())
			destination = "0b";
		break;
    }
    default:
		return EResult::FormatError;
	}

    return EResult::Success;
}

int64_t minipp::MiniPPFile::Values::IntValue::GetValue() const noexcept
{
    return m_value;
}

minipp::MiniPPFile::Values::BooleanValue::BooleanValue(bool value) : m_value(value) {}

minipp::EResult minipp::MiniPPFile::Values::BooleanValue::Parse(const std::string& str) noexcept
{
    if (str == "true")
        m_value = true;
    else if (str == "false")
        m_value = false;
    else
        return EResult::FormatError;
    return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::BooleanValue::ToString(std::string& destination) const noexcept
{
    destination = m_value ? "true" : "false";
    return EResult::Success;
}

bool minipp::MiniPPFile::Values::BooleanValue::GetValue() const noexcept
{
    return m_value;
}

minipp::MiniPPFile::Values::FloatValue::FloatValue(double value) : m_value(value) {}

minipp::EResult minipp::MiniPPFile::Values::FloatValue::Parse(const std::string& str) noexcept
{
    m_value = std::stod(str);
    return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::FloatValue::ToString(std::string& destination) const noexcept
{
    destination = std::to_string(m_value);
    return EResult::Success;
}

double minipp::MiniPPFile::Values::FloatValue::GetValue() const noexcept
{
    return m_value;
}

minipp::EResult minipp::MiniPPFile::Values::ArrayValue::Parse(const std::string& str) noexcept
{
    std::string nValue = str;
    if (str.front() != '[' || str.back() != ']')
        return EResult::FormatError;
    nValue = str.substr(1, str.size() - 2);
    if (nValue.empty())
        return EResult::Success;

    int64_t bracketCounter = 0;
    bool isInString = false;

    std::vector<std::string> elements;
    std::string currentElement;

    for (size_t i = 0; i < str.size(); ++i)
    {
        char c = str[i];

        if (isInString)
        {
            if (c == '\\')
                ++i;
            else if (c == '"')
            {
                isInString = false;
                currentElement += c;
            }
            else
                currentElement += c;
        }
        else if (c == '\\')
            ++i;
        else if (c == '"')
        {
            currentElement += c;
            isInString = true;
        }
        else
        {
            if (c == '[')
            {
                if (++bracketCounter > 1)
                    currentElement += c;
            }
            else if (c == ']')
            {
                --bracketCounter;
                if (bracketCounter < 0)
                    return EResult::FormatError;
                else if (bracketCounter >= 1)
                    currentElement += c;
            }
            else if (c == ',' && bracketCounter == 1)
            {
                elements.push_back(currentElement);
                currentElement = "";
            }
            else if (c != ' ' && c != '\t')
                currentElement += c;
        }
    }

    if (bracketCounter != 0)
        return EResult::FormatError;

    if (!currentElement.empty())
        elements.push_back(currentElement);
    size_t lastTypeIdHash = 0;
    bool hasTypeHash = false;

    for (auto& elem : elements)
    {
        auto parsed = ParseValue(elem);
        if (parsed == nullptr)
            return EResult::FormatError;

        if (!hasTypeHash)
        {
            lastTypeIdHash = typeid(*parsed).hash_code();
            hasTypeHash = true;
        }
        else if (typeid(*parsed).hash_code() != lastTypeIdHash)
            return EResult::FormatError;

        m_values.push_back(std::move(parsed));
    }
    return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::ArrayValue::ToString(std::string& destination) const noexcept
{
    std::string buf;
    size_t lastTypeIdHash = 0;
    bool hasTypeHash = false;

    std::stringstream ss;
    for (const auto& val : m_values)
    {
        EResult result = val->ToString(buf);
        if (!IsResultOk(result))
            return result;

        if (!hasTypeHash)
        {
            lastTypeIdHash = typeid(*val).hash_code();
            hasTypeHash = true;
        }
        else if (typeid(*val).hash_code() != lastTypeIdHash)
            return EResult::ArrayDataTypeInconsitency;

        ss << buf << ", ";
    }
    std::string valueString = ss.str();
    if (!valueString.empty())
        valueString = valueString.substr(0, valueString.size() - 2);
    destination = "[" + valueString + "]";
    return EResult::Success;
}

#pragma endregion

minipp::MiniPPFile::Section::~Section()
{
    for (auto& pair : m_values)
        delete pair.second;
    for (auto& pair : m_subSections)
        delete pair.second;
}

minipp::EResult minipp::MiniPPFile::Section::GetSubSection(const std::string& key, Section** destination) const noexcept
{
    auto pathIndex = key.find('.');
    std::string thisKey = key;
    std::string rest;
    if (pathIndex != std::string::npos)
    {
        thisKey = key.substr(0, pathIndex);
        rest = key.substr(pathIndex + 1);
    }

    auto it = m_subSections.find(thisKey);
    if (it == m_subSections.end())
    {
        COUT("Sub-Section not found: " << thisKey);
        return EResult::SectionNotPresent;
    }
    if (rest.empty())
    {
        *destination = it->second;
        return EResult::Success;
    }

    return it->second->GetSubSection(rest, destination);
}

minipp::EResult minipp::MiniPPFile::Section::SetSubSection(const std::string& name, std::unique_ptr<Section> value, bool allowOverwrite) noexcept
{
    if (m_subSections.find(name) != m_subSections.end())
        if (!allowOverwrite)
            return EResult::SectionAlreadyPresent;
        else
            delete m_subSections[name];

    m_subSections[name] = value.release();
    return EResult::Success;
}

std::unique_ptr<minipp::MiniPPFile::Value> minipp::MiniPPFile::ParseValue(std::string value)
{
    char valueFirstChar = value.front();
    char valueLastChar = value.back();
    if (valueFirstChar == '"')
    {
        if (valueLastChar != '"')
            return nullptr;

        // is string
        value = value.substr(1, value.size() - 2);
        auto strValue = std::make_unique<Values::StringValue>();
        auto parseResult = strValue->Parse(value);
        if (!IsResultOk(parseResult))
            return nullptr;

        return strValue;
    }
    else if (valueLastChar == 'e')
    {
        auto boolValue = std::make_unique<Values::BooleanValue>();
        auto parseResult = boolValue->Parse(value);
        if (!IsResultOk(parseResult))
            return nullptr;

        return boolValue;
    }
    else if (valueLastChar == 'f')
    {
        auto floatValue = std::make_unique<Values::FloatValue>();
        auto parseResult = floatValue->Parse(value);
        if (!IsResultOk(parseResult))
            return nullptr;

        return floatValue;
    }
    else if (valueLastChar == ']')
    {
        auto arrayValue = std::make_unique<Values::ArrayValue>();
        auto parseResult = arrayValue->Parse(value);
        if (!IsResultOk(parseResult))
            return nullptr;

        return arrayValue;
    }
    else
    {
        auto intValue = std::make_unique<Values::IntValue>();
        auto parseResult = intValue->Parse(value);
        if (!IsResultOk(parseResult))
            return nullptr;

        return intValue;
    }
}

minipp::EResult minipp::MiniPPFile::WriteSection(const Section* section, std::ofstream& ofs) noexcept
{
    if (section->m_values.size() > 0)
    {
        for (const auto& pair : section->m_values)
        {
            if (!Tools::IsNameValid(pair.first))
            {
                COUT("Invalid name for key: " << pair.first);
                return EResult::FormatError;
            }

			for (const auto& comment : pair.second->m_comments)
				ofs << comment << std::endl;

            ofs << pair.first << " = ";
            std::string valueString;
            auto result = pair.second->ToString(valueString);
			if (!IsResultOk(result))
				return result;
            ofs << valueString << std::endl;
        }
        ofs << std::endl;
    }

    for (const auto& pair : section->m_subSections)
    {
        if (!Tools::IsNameValid(pair.first))
        {
            COUT("Invalid name for section: " << pair.first);
            return EResult::FormatError;
        }

		for (const auto& comment : pair.second->m_comments)
			ofs << comment << std::endl;

        ofs << "[" << pair.first << "]" << std::endl;
        auto result = WriteSection(pair.second, ofs);
        if (!IsResultOk(result))
            return result;
    }

    return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Parse(const std::string& path) noexcept
{
    std::ifstream ifs;
    ifs.open(path);
    if (!ifs.is_open())
        return EResult::FileIOError;

    int64_t lineCounter = 0;
    Section* currentSection = nullptr;

    std::vector<std::string> commentBuffer;

    std::string currentLine;
    while (std::getline(ifs, currentLine))
    {
        ++lineCounter;
        Tools::StringTrim(currentLine);
        if (currentLine.empty())
            continue;
        char firstChar = currentLine[0];
        char lastChar = currentLine[currentLine.size() - 1];

        if (firstChar == '#')
        {
			commentBuffer.push_back(currentLine);
            continue;
        }

        if (firstChar == '[')
        {
            if (lastChar != ']')
            {
                COUT_SYNTAX_ERROR(lineCounter, "Expected ']' at the end of the line.");
                return EResult::FormatError;
            }
            std::string sectionName = currentLine.substr(1, currentLine.size() - 2);
            Tools::StringTrim(sectionName);
            if (sectionName.empty())
            {
                COUT_SYNTAX_ERROR(lineCounter, "Expected section path. Found empty section begin notation.");
                return EResult::FormatError;
            }
            // Create section tree
            Section* ubSection = &m_rootSection;

            std::vector<std::string> sectionPath = Tools::SplitByDelimiter(sectionName, '.');
            for (size_t i = 0; i < sectionPath.size(); ++i)
            {
                const std::string& sectionName = sectionPath[i];
                if (!Tools::IsNameValid(sectionName))
                {
                    COUT_SYNTAX_ERROR(lineCounter, "Invalid section name. (\"" << sectionName << "\") May only contain [a - z][A - Z][0 - 9] and _.");
                    return EResult::FormatError;
                }

                EResult result;
                result = ubSection->SetSubSection(sectionName, std::make_unique<Section>(), false);
                if (result == EResult::SectionAlreadyPresent && i == sectionPath.size() - 1)
                {
                    COUT_SYNTAX_ERROR(lineCounter, "All (sub-) sections may only be defined once.");
                    return EResult::FormatError;
                }
                result = ubSection->GetSubSection(sectionName, &ubSection);
                ASSERT(IsResultOk(result)); // should never happen
            }
            currentSection = ubSection;
			currentSection->m_comments = commentBuffer;
            commentBuffer.clear();
            continue;
        }
        if (currentSection == nullptr)
        {
            COUT_SYNTAX_ERROR(lineCounter, "Expected section begin before key-value pair.");
            return EResult::FormatError;
        }

        int64_t keyValueDelimiterIndex = Tools::FirstIndexOf(currentLine, '=');
        if (keyValueDelimiterIndex == -1)
        {
            COUT_SYNTAX_ERROR(lineCounter, "Expected '=' in line.");
            return EResult::FormatError;
        }

        auto keyValuePair = Tools::SplitInTwo(currentLine, keyValueDelimiterIndex);
        Tools::StringTrim(keyValuePair.first);
        Tools::StringTrim(keyValuePair.second);
        if (keyValuePair.first.empty())
        {
            COUT_SYNTAX_ERROR(lineCounter, "Expected key in line.");
            return EResult::FormatError;
        }
        if (keyValuePair.second.empty())
        {
            COUT_SYNTAX_ERROR(lineCounter, "Empty keys are not allowed");
            return EResult::FormatError;
        }
        auto parsedValue = ParseValue(keyValuePair.second);
        if (parsedValue == nullptr)
        {
            COUT_SYNTAX_ERROR(lineCounter, "Invalid value");
            return EResult::FormatError;
        }
		parsedValue->m_comments = commentBuffer;
        commentBuffer.clear();

        currentSection->SetValue(keyValuePair.first, std::move(parsedValue), false);
    }

    return EResult::Success;
}

void minipp::MiniPPFile::Write(const std::string& path) const noexcept
{
    std::ofstream ofs;
    ofs.open(path);
    if (!ofs.is_open())
        return;

    WriteSection(&m_rootSection, ofs);
}

bool minipp::MiniPPFile::IsResultOk(EResult result) noexcept
{
    return static_cast<int32_t>(result) > 0;
}

#pragma region Tools
bool minipp::MiniPPFile::Tools::StringStartsWith(const std::string& str, const std::string& beg)
{
    if (str.size() < beg.size())
        return false;

    for (size_t i = 0; i < beg.size(); ++i)
        if (str[i] != beg[i])
            return false;
    return true;
}

bool minipp::MiniPPFile::Tools::StringEndsWith(const std::string& str, const std::string& end)
{
    if (str.size() < end.size())
        return false;

    for (size_t i = 0; i < end.size(); ++i)
        if (str[str.size() - i - 1] != end[end.size() - i - 1])
            return false;
    return true;
}

void minipp::MiniPPFile::Tools::StringTrim(std::string& str)
{
    if (str.empty())
        return;

    size_t start = 0;
    size_t end = str.size() - 1;
    while (str[start] == ' ' || str[start] == '\t')
        start++;
    while (str[end] == ' ' || str[end] == '\t')
        end--;

    str = str.substr(start, end - start + 1);
}

bool minipp::MiniPPFile::Tools::IsNameValid(const std::string& name) noexcept
{
    for (const char c : name)
    {
        if (
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_'))
            continue;
        return false;
    }
    return true;
}

int64_t minipp::MiniPPFile::Tools::FirstIndexOf(const std::string& str, char c) noexcept
{
    int64_t index = 0;
    for (const char ch : str)
    {
        if (ch == c)
            return index;
        ++index;
    }
    return -1;
}

int64_t minipp::MiniPPFile::Tools::LastIndexOf(const std::string& str, char c) noexcept
{
    int64_t index = str.size() - 1;
    for (int64_t i = str.size() - 1; i >= 0; --i)
    {
        if (str[i] == c)
            return index;
        --index;
    }
    return -1;
}

std::pair<std::string, std::string> minipp::MiniPPFile::Tools::SplitInTwo(const std::string& str, int64_t firstLength) noexcept
{
    return std::make_pair(str.substr(0, firstLength), str.substr(firstLength + 1));
}

std::vector<std::string> minipp::MiniPPFile::Tools::SplitByDelimiter(const std::string& str, char delimiter) noexcept
{
    std::vector<std::string> elements;
    std::string tmp;

    for (const char c : str)
    {
        if (c == delimiter)
        {
            elements.push_back(tmp);
            tmp.clear();
        }
        else
            tmp.push_back(c);
    }

    if (!tmp.empty())
        elements.push_back(tmp);
    return elements;
}

void minipp::MiniPPFile::Tools::RemoveAll(std::string& str, char old)
{
    str.erase(std::remove(str.begin(), str.end(), old), str.end());
}

bool minipp::MiniPPFile::Tools::IsIntegerDecimal(const std::string& str) noexcept
{
    for (size_t i = 0; i < str.size(); ++i)
        if (str[i] < '0' || str[i] > '9')
            return false;
    return true;
}

#pragma endregion
#endif // MINIPP_IMPLEMENTATION