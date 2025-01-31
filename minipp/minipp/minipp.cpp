#include "minipp.hpp"
#include <fstream>

#define MINIPP_ENABLE_DEBUG_OUTPUT true

#if MINIPP_ENABLE_DEBUG_OUTPUT
#include <iostream>
#define COUT(msg) std::cout << msg << std::endl
#else
#define COUT(msg)
#endif

#define ASSERT(condition) if (!condition) abort();

#define COUT_SYNTAX_ERROR(line, msg) COUT("Error in line " << line << ": " << msg)

minipp::MiniPPFile::Section::~Section()
{
    for (auto& pair : m_values)
        delete pair.second;
    for (auto& pair : m_subSections)
        delete pair.second;
}

minipp::MiniPPFile::Section* minipp::MiniPPFile::Section::GetSubSection(const std::string& key, EResult* result) const noexcept
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
        if (result != nullptr)
            *result = EResult::SectionNotPresent;
        return nullptr;
    }
    if (result != nullptr)
		*result = EResult::Success;
	if (rest.empty())
		return it->second;
	return it->second->GetSubSection(rest, result);
}

minipp::MiniPPFile::Section* minipp::MiniPPFile::Section::SetSubSection(const std::string& name, std::unique_ptr<Section> value, bool allowOverwrite, EResult* result) noexcept
{
    if (m_subSections.find(name) != m_subSections.end())
        if (!allowOverwrite)
        {
			if (result != nullptr)
				*result = EResult::SectionAlreadyPresent;
            return nullptr;
        }
        else
            delete m_subSections[name];

    auto val = value.release();
    m_subSections[name] = val;
	if (result != nullptr)
		*result = EResult::Success;
	return val;
}

minipp::MiniPPFile::StringValue::StringValue(const std::string& str) : m_value(str) {}

minipp::EResult minipp::MiniPPFile::StringValue::Parse(const std::string& str) noexcept
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

minipp::EResult minipp::MiniPPFile::StringValue::ToString(std::string& destination) const noexcept
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

const std::string& minipp::MiniPPFile::StringValue::GetValue() const noexcept
{
    return m_value;
}

minipp::EResult minipp::MiniPPFile::Parse(const std::string& path) noexcept
{
    std::ifstream ifs;
    ifs.open(path);
    if (!ifs.is_open())
        return EResult::FileIOError;

    int64_t lineCounter = 0;

    Section* currentSection = nullptr;

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
            continue;

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
                ubSection->SetSubSection(sectionName, std::make_unique<Section>(), false, &result); // if this fails, the section is already present
                if (result == EResult::SectionAlreadyPresent && i == sectionPath.size() - 1)
                {
                    COUT_SYNTAX_ERROR(lineCounter, "All (sub-) sections may only be defined once.");
                    return EResult::FormatError;
                }
                ubSection = ubSection->GetSubSection(sectionName, &result);
                ASSERT(Tools::IsResultOk(result)); // should never happen
            }
            currentSection = ubSection;
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
        std::string& value = keyValuePair.second;
        char valueFirstChar = value.front();
        char valueLastChar = value.back();
        if (valueFirstChar == '"')
        {
            if (valueLastChar != '"')
            {
                COUT_SYNTAX_ERROR(lineCounter, "Expected '\"' at the end of the value.");
                return EResult::FormatError;
            }
            // is string
            value = value.substr(1, value.size() - 2);
            StringValue strValue;
            auto parseResult = strValue.Parse(value);
            if (!Tools::IsResultOk(parseResult))
            {
                COUT_SYNTAX_ERROR(lineCounter, "Invalid string value.");
                return EResult::FormatError;
            }
            currentSection->SetValue(keyValuePair.first, strValue, false);
            continue;
        }
        else 
        {
            IntValue intValue;
			auto parseResult = intValue.Parse(value);
            if (!Tools::IsResultOk(parseResult))
            {
                COUT_SYNTAX_ERROR(lineCounter, "Invalid integer value.");
                return EResult::FormatError;
            }
			currentSection->SetValue(keyValuePair.first, intValue, false);
            continue;
        }

    }

    return EResult::Success;
}

void minipp::MiniPPFile::Write(const std::string& path) noexcept
{
}

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

bool minipp::MiniPPFile::Tools::IsResultOk(EResult result) noexcept
{
    return static_cast<int32_t>(result) > 0;
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

minipp::MiniPPFile::IntValue::IntValue(int64_t value) : m_value(value) {}

minipp::EResult minipp::MiniPPFile::IntValue::Parse(const std::string& str) noexcept
{
	std::string sanitizedValue = str;
	Tools::RemoveAll(sanitizedValue, '_');
	if (sanitizedValue.empty())
		return EResult::FormatError;

    char lastCharacter = str.back();
	auto rest = str.substr(0, str.size() - 1);
    if (lastCharacter == 'h')
		m_value = std::stoll(rest, nullptr, 16);
	else if (lastCharacter == 'b')
		m_value = std::stoll(rest, nullptr, 2);
    else
    {
		if (!Tools::IsIntegerDecimal(sanitizedValue))
			return EResult::FormatError;
        m_value = std::stoll(sanitizedValue);
    }
    return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::IntValue::ToString(std::string& destination) const noexcept
{
	destination = std::to_string(m_value);
	return EResult::Success;
}

int64_t minipp::MiniPPFile::IntValue::GetValue() const noexcept
{
    return m_value;
}
