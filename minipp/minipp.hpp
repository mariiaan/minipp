#pragma once

// This is a single-header reference implementation for the MINI config file format.
// The MINI format is a simple, human-readable configuration file format.
// #define MINIPP_IMPLEMENTATION in a single translation unit before including this header!

/*
	Copyright (c) 2025 mariiaan
	Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files 
	(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
	publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
	subject to the following conditions:
	The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
	INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
	IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
	DEALINGS IN THE SOFTWARE.
*/

// enabled helpful debug messages via std::cout (for parsing and writing)
#define MINIPP_ENABLE_DEBUG_OUTPUT true

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
		KeyNotPresent					= -1,
		KeyAlreadyPresent				= -2,
		SectionNotPresent				= -3,
		SectionAlreadyPresent			= -4,
		FileIOError						= -5,
		InvalidDataType					= -6,
		FormatError						= -7,
		ArrayDataTypeInconsistency		= -8,
		BadEscapeSequence				= -9,
		UnknownEscapeSequence			= -10,
		UnescapedStringValue			= -11,
		ValueEmpty						= -12,
		IntegerValueInvalid				= -13,
		IntegerValueOutOfRange			= -14,
		IntegerStyleInvalid				= -15,
		FloatValueInvalid				= -16,
		BooleanValueInvalid				= -17,
		ArrayNotEnclosed				= -18,
		ArrayBracketsInbalanced			= -19,
		InvalidName						= -20,
		SectionExpectedClosingBracket	= -21,
		EmptySectionName				= -22,
		KeyValuePairNotInSection		= -23,
		ExpectedKeyValuePair			= -24,
		KeyEmpty						= -25,
		MissingQuote					= -26,

		/* OK Codes */
		Success							= +1,
		ValueOverwritten				= +2
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

		public:
			static std::unique_ptr<Value> ParseValue(std::string value, EResult* result = nullptr);
		};

		class Values
		{
		public:
			class StringValue : public Value
			{
			public:
				using BaseType = std::string;

			private:
				BaseType m_value;

			public:
				StringValue() = default;
				StringValue(const BaseType& str) : m_value(str) {};
				EResult Parse(const std::string& str) noexcept override;
				EResult ToString(std::string& destination) const noexcept override;
				const BaseType& GetValue() const noexcept { return m_value; }
			};

			class IntValue : public Value
			{
			public:
				using BaseType = int64_t;

			private:
				BaseType m_value = 0;
				EIntStyle m_style = EIntStyle::Decimal;

			public:
				IntValue() = default;
				IntValue(BaseType value) : m_value(value) {};
				EResult Parse(const std::string& str) noexcept override;
				EResult ToString(std::string& destination) const noexcept override;
				BaseType GetValue() const noexcept { return m_value; }
			};

			class BooleanValue : public Value
			{
			public:
				using BaseType = bool;

			private:
				BaseType m_value = false;

			public:
				BooleanValue() = default;
				BooleanValue(BaseType value) : m_value(value) {};
				EResult Parse(const std::string& str) noexcept override;
				EResult ToString(std::string& destination) const noexcept override;
				BaseType GetValue() const noexcept { return m_value; }
			};

			class FloatValue : public Value
			{
			public:
				using BaseType = double;

			private:
				BaseType m_value = 0.0f;

			public:
				FloatValue() = default;
				FloatValue(BaseType value) : m_value(value) {};
				EResult Parse(const std::string& str) noexcept override;
				EResult ToString(std::string& destination) const noexcept override;
				BaseType GetValue() const noexcept { return m_value; }
			};

			class ArrayValue : public Value
			{
			public:
				using BaseType = std::vector<Value*>;

			private:
				BaseType m_values;

			public:
				ArrayValue() = default;
				ArrayValue(const ArrayValue&) = delete;
				ArrayValue& operator=(const ArrayValue&) = delete;
				virtual ~ArrayValue();

			public:
				EResult Parse(const std::string& str) noexcept override;
				EResult ToString(std::string& destination) const noexcept override;
				BaseType& GetValue() noexcept { return m_values; }
				const BaseType& GetValue() const noexcept { return m_values; }

				Value* operator[](size_t index) noexcept
				{
					if (index >= m_values.size())
						return nullptr;
					return m_values[index];
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
			std::unordered_map<std::string, Value*>& GetValues() noexcept { return m_values; }
			const std::unordered_map<std::string, Value*>& GetValues() const noexcept { return m_values; }
			std::unordered_map<std::string, Section*>& GetSubSections() noexcept { return m_subSections; }
			const std::unordered_map<std::string, Section*>& GetSubSections() const noexcept { return m_subSections; }

		public:
			Section() = default;
			~Section();
			Section(const Section&) = delete;
			Section& operator=(const Section&) = delete;

		public:
			template<typename ValueDataType>
			EResult GetValue(const std::string& key, ValueDataType** target) noexcept
			{
				static_assert(std::is_base_of<Value, ValueDataType>::value, "ValueDataType must be a subclass of Value");

				int64_t firstSeparatorIndex = Tools::FirstIndexOf(key, '.');
				if (firstSeparatorIndex != -1)
				{
					std::string thisKey = key.substr(0, firstSeparatorIndex);
					std::string rest = key.substr(firstSeparatorIndex + 1);

					auto it = m_subSections.find(thisKey);
					if (it == m_subSections.end())
						return EResult::SectionNotPresent;

					return it->second->GetValue(rest, target);
				}

				auto it = m_values.find(key);
				if (it == m_values.end())
					return EResult::KeyNotPresent;

				auto val = dynamic_cast<ValueDataType*>(it->second);
				if (val == nullptr)
					return EResult::InvalidDataType;

				*target = val;
				return EResult::Success;
			}

			template<typename ValueDataType>
			EResult SetValue(const std::string& name, std::unique_ptr<ValueDataType> value, bool allowOverwrite = false) noexcept
			{
				static_assert(std::is_base_of<Value, ValueDataType>::value, "ValueDataType must be a subclass of Value");
				bool overwritten = false;

				if (m_values.find(name) != m_values.end())
					if (!allowOverwrite)
						return EResult::KeyAlreadyPresent;
					else
					{
						delete m_values[name];
						overwritten = true;
					}

				m_values[name] = value.release();
				return overwritten ? EResult::ValueOverwritten : EResult::Success;
			}

			template<typename ValueDataType>
			typename ValueDataType::BaseType GetValueOrDefault(const std::string& key, 
				const typename ValueDataType::BaseType& defaultValue = typename ValueDataType::BaseType{})
			{
				static_assert(std::is_base_of<Value, ValueDataType>::value, "ValueDataType must be a subclass of Value");

				ValueDataType* value = nullptr;
				if (GetValue(key, &value) != EResult::Success)
					return defaultValue;
				return value->GetValue();
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
		static minipp::EResult WriteSection(const Section* section, std::ofstream& ofs, std::string partTreeName) noexcept;

	public:
		EResult Parse(const std::string& path, bool additional = false) noexcept;
		EResult Write(const std::string& path) const noexcept;

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
#include <bitset>

#if MINIPP_ENABLE_DEBUG_OUTPUT
#include <iostream>
	#define PP_COUT(msg) std::cout << "[minipp] " << msg << std::endl
#else
	#define PP_COUT(msg)
#endif

#define PP_COUT_SYNTAX_ERROR_LINE(line, msg) PP_COUT(line << ": " << msg)
#define PP_COUT_SYNTAX_ERROR(msg) PP_COUT("Syntax error: " << msg)

std::unique_ptr<minipp::MiniPPFile::Value> minipp::MiniPPFile::Value::ParseValue(std::string value, EResult* result)
{
#define RETURN_NULLPTR_WITH_RESULT(r) { if (result != nullptr) *result = r; return nullptr; }

	char valueFirstChar = value.front();
	char valueLastChar = value.back();
	if (valueFirstChar == '"')
	{
		if (valueLastChar != '"')
			RETURN_NULLPTR_WITH_RESULT(EResult::MissingQuote);

		// is string
		value = value.substr(1, value.size() - 2);
		auto strValue = std::make_unique<Values::StringValue>();
		auto parseResult = strValue->Parse(value);
		if (!IsResultOk(parseResult))
			RETURN_NULLPTR_WITH_RESULT(parseResult);

		return strValue;
	}
	else if (valueLastChar == 'e')
	{
		auto boolValue = std::make_unique<Values::BooleanValue>();
		auto parseResult = boolValue->Parse(value);
		if (!IsResultOk(parseResult))
			RETURN_NULLPTR_WITH_RESULT(parseResult);

		return boolValue;
	}
	else if (valueLastChar == 'f')
	{
		auto floatValue = std::make_unique<Values::FloatValue>();
		auto parseResult = floatValue->Parse(value);
		if (!IsResultOk(parseResult))
			RETURN_NULLPTR_WITH_RESULT(parseResult);

		return floatValue;
	}
	else if (valueLastChar == ']')
	{
		auto arrayValue = std::make_unique<Values::ArrayValue>();
		auto parseResult = arrayValue->Parse(value);
		if (!IsResultOk(parseResult))
			RETURN_NULLPTR_WITH_RESULT(parseResult);

		return arrayValue;
	}
	else
	{
		auto intValue = std::make_unique<Values::IntValue>();
		auto parseResult = intValue->Parse(value);
		if (!IsResultOk(parseResult))
			RETURN_NULLPTR_WITH_RESULT(parseResult);

		return intValue;
	}
}

#pragma region Value Types

minipp::EResult minipp::MiniPPFile::Values::StringValue::Parse(const std::string& str) noexcept
{
	m_value = "";

	for (size_t i = 0; i < str.size(); ++i)
	{
		if (str[i] == '\\')
		{
			if (i + 1 >= str.size())
			{
				PP_COUT_SYNTAX_ERROR("'\\' at end of string");
				return EResult::BadEscapeSequence;
			}

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
				PP_COUT_SYNTAX_ERROR("Unknown escape sequence '\\" << str[i + 1] << "'");
				return EResult::UnknownEscapeSequence;
			}
			++i;
		}
		else if (str[i] == '"')
			return EResult::UnescapedStringValue;
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
		default: break;
		}
	}

	destination = "\"" + sanitizedValue + "\"";
	return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::IntValue::Parse(const std::string& str) noexcept
{
	std::string sanitizedValue = str;
	Tools::RemoveAll(sanitizedValue, '_');
	if (sanitizedValue.empty())
	{
		PP_COUT_SYNTAX_ERROR("Empty string for integer value.");
		return EResult::FormatError;
	}

	char lastCharacter = str.back();
	auto rest = str.substr(0, str.size() - 1);
	try
	{
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
			{
				PP_COUT_SYNTAX_ERROR("Invalid decimal integer value: " << sanitizedValue);
				return EResult::IntegerValueInvalid;
			}
			m_value = std::stoll(sanitizedValue);
			m_style = EIntStyle::Decimal;
		}
	}
	catch (const std::invalid_argument&)
	{
		PP_COUT_SYNTAX_ERROR("Invalid integer value: " << sanitizedValue);
		return EResult::IntegerValueInvalid;
	}
	catch (const std::out_of_range&)
	{
		PP_COUT_SYNTAX_ERROR("Integer value out of range: " << sanitizedValue);
		return EResult::IntegerValueOutOfRange;
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
		for (i = 0; i < destination.size(); ++i) // cut of leading zeros
			if (destination[i] == '1')
			{
				destination = destination.substr(i);
				break;
			}
		if (i == destination.size()) // we dont wan't 64 zeros for a 0 value
			destination = "0b";
		break;
	}
	default:
		PP_COUT_SYNTAX_ERROR("Invalid integer style.");
		return EResult::IntegerStyleInvalid;
	}

	return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::BooleanValue::Parse(const std::string& str) noexcept
{
	if (str == "true")
		m_value = true;
	else if (str == "false")
		m_value = false;
	else
	{
		PP_COUT_SYNTAX_ERROR("Invalid boolean value: " << str << " (may only contain lowercase true and false)");
		return EResult::BooleanValueInvalid;
	}
	return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::BooleanValue::ToString(std::string& destination) const noexcept
{
	destination = m_value ? "true" : "false";
	return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::FloatValue::Parse(const std::string& str) noexcept
{
	try
	{
		m_value = std::stod(str);
	}
	catch (...)
	{
		PP_COUT_SYNTAX_ERROR("Invalid float value: " << str);
		return EResult::FloatValueInvalid;
	}

	return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Values::FloatValue::ToString(std::string& destination) const noexcept
{
	destination = std::to_string(m_value);
	return EResult::Success;
}

minipp::MiniPPFile::Values::ArrayValue::~ArrayValue()
{
	for (auto& val : m_values)
		delete val;
}

minipp::EResult minipp::MiniPPFile::Values::ArrayValue::Parse(const std::string& str) noexcept
{
	if (str.front() != '[' || str.back() != ']')
	{
		PP_COUT_SYNTAX_ERROR("Array value must be enclosed in [] brackets.");
		return EResult::FormatError;
	}

	int64_t bracketCounter = 0;
	bool isInString = false; // we may encounter array value separators "," inside strings; we need to ignore those

	std::vector<std::string> elements;
	std::string currentElement;

	for (size_t i = 0; i < str.size(); ++i)
	{
		char c = str[i];

		if (isInString)
		{
			if (c == '\\')
			{
				if (i + 1 >= str.size())
				{
					PP_COUT_SYNTAX_ERROR("Bad escape sequence: '\\' at end of string");
					return EResult::BadEscapeSequence;
				}
				currentElement += c;
				currentElement += str[i + 1];
				++i;
			}
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
				{
					PP_COUT_SYNTAX_ERROR("Array brackets are not balanced. (One ] too much or encountered too early)");
					return EResult::ArrayBracketsInbalanced;
				}
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

	if (bracketCounter != 0) // will always be a positive value because negative values are caught earlier
	{
		PP_COUT_SYNTAX_ERROR("Array brackets are not balanced. (Missing " << bracketCounter << " closing brackets)");
		return EResult::ArrayBracketsInbalanced;
	}

	if (!currentElement.empty())
		elements.push_back(currentElement);
	size_t lastTypeIdHash = 0;
	bool hasTypeHash = false;

	EResult result;
	for (auto& elem : elements)
	{
		auto parsed = ParseValue(elem, &result);
		if (parsed == nullptr)
			return result;

		if (!hasTypeHash)
		{
			lastTypeIdHash = typeid(*parsed).hash_code();
			hasTypeHash = true;
		}
		else if (typeid(*parsed).hash_code() != lastTypeIdHash)
			return EResult::ArrayDataTypeInconsistency;

		m_values.push_back(parsed.release());
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
			return EResult::ArrayDataTypeInconsistency;

		ss << buf << ", ";
	}
	std::string valueString = ss.str();
	if (!valueString.empty()) // remove the last ", " if there are any elements
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
		PP_COUT_SYNTAX_ERROR("Sub-Section not found: " << thisKey);
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

minipp::EResult minipp::MiniPPFile::WriteSection(const Section* section, std::ofstream& ofs, std::string partTreeName) noexcept
{
	if (section->m_values.size() > 0)
	{
		std::string valueString;
		for (const auto& pair : section->m_values)
		{
			if (!Tools::IsNameValid(pair.first))
			{
				PP_COUT_SYNTAX_ERROR("Invalid name for key: " << pair.first);
				return EResult::InvalidName;
			}

			for (const auto& comment : pair.second->m_comments)
				ofs << comment << std::endl;

			ofs << pair.first << " = ";
			auto result = pair.second->ToString(valueString);
			if (!IsResultOk(result))
				return result;
			ofs << valueString << std::endl;
		}
		ofs << std::endl;
	}

	if (!partTreeName.empty())
		partTreeName += ".";

	for (const auto& pair : section->m_subSections)
	{
		if (!Tools::IsNameValid(pair.first))
		{
			PP_COUT_SYNTAX_ERROR("Invalid name for section: " << pair.first);
			return EResult::InvalidName;
		}

		for (const auto& comment : pair.second->m_comments)
			ofs << comment << std::endl;

		ofs << "[" << partTreeName << pair.first << "]" << std::endl;
		auto result = WriteSection(pair.second, ofs, partTreeName + pair.first);
		if (!IsResultOk(result))
			return result;
	}

	return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Parse(const std::string& path, bool additional) noexcept
{
#define PP_COUT_HERE() PP_COUT_SYNTAX_ERROR_LINE(lineCounter, currentLine << " <- HERE");
	if (!additional)
	{
		m_rootSection.m_comments.clear();
		m_rootSection.m_values.clear();
		m_rootSection.m_subSections.clear();
	}

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
				PP_COUT_SYNTAX_ERROR("Expected ']' at the end of the line.");
				PP_COUT_HERE();
				return EResult::SectionExpectedClosingBracket;
			}
			std::string sectionPathStr = currentLine.substr(1, currentLine.size() - 2);
			Tools::StringTrim(sectionPathStr);
			if (sectionPathStr.empty())
			{
				PP_COUT_SYNTAX_ERROR("Expected section path. Found empty section begin notation.");
				PP_COUT_HERE();
				return EResult::EmptySectionName;
			}
			// Create section tree
			Section* ubSection = &m_rootSection;

			std::vector<std::string> sectionPath = Tools::SplitByDelimiter(sectionPathStr, '.');
			EResult result;
			for (size_t i = 0; i < sectionPath.size(); ++i)
			{
				const std::string& sectionName = sectionPath[i];
				if (!Tools::IsNameValid(sectionName))
				{
					PP_COUT_SYNTAX_ERROR("Invalid section name. (\"" << sectionName << "\") May only contain [a - z][A - Z][0 - 9] and _.");
					PP_COUT_HERE();
					return EResult::InvalidName;
				}

				result = ubSection->SetSubSection(sectionName, std::make_unique<Section>(), false);
				if (result == EResult::SectionAlreadyPresent && i == sectionPath.size() - 1)
				{
					PP_COUT_SYNTAX_ERROR("All (sub-) sections may only be defined once.");
					PP_COUT_HERE();
					return EResult::SectionAlreadyPresent;
				}
				ubSection->GetSubSection(sectionName, &ubSection);
			}
			currentSection = ubSection;
			currentSection->m_comments = commentBuffer;
			commentBuffer.clear();
			continue;
		}
		if (currentSection == nullptr)
		{
			PP_COUT_SYNTAX_ERROR("Expected section begin before key-value pair.");
			PP_COUT_HERE();
			return EResult::KeyValuePairNotInSection;
		}

		int64_t keyValueDelimiterIndex = Tools::FirstIndexOf(currentLine, '=');
		if (keyValueDelimiterIndex == -1)
		{
			PP_COUT_SYNTAX_ERROR("Expected '=' in line.");
			PP_COUT_HERE();
			return EResult::ExpectedKeyValuePair;
		}

		auto keyValuePair = Tools::SplitInTwo(currentLine, keyValueDelimiterIndex);
		Tools::StringTrim(keyValuePair.first);
		Tools::StringTrim(keyValuePair.second);

		if (keyValuePair.first.empty())
		{
			PP_COUT_SYNTAX_ERROR("Expected key in line.");
			PP_COUT_HERE();
			return EResult::KeyEmpty;
		}
		if (!Tools::IsNameValid(keyValuePair.first))
		{
			PP_COUT_SYNTAX_ERROR("Invalid key name. (\"" << keyValuePair.first << "\") May only contain [a - z][A - Z][0 - 9] and _.");
			PP_COUT_HERE();
			return EResult::InvalidName;
		}

		if (keyValuePair.second.empty())
		{
			PP_COUT_SYNTAX_ERROR( "Empty keys are not allowed");
			PP_COUT_HERE();
			return EResult::ValueEmpty;
		}
		EResult parseResult;
		auto parsedValue = Value::ParseValue(keyValuePair.second, &parseResult);
		if (parsedValue == nullptr)
		{
			PP_COUT_HERE();
			return parseResult;
		}
		parsedValue->m_comments = commentBuffer;
		commentBuffer.clear();

		auto valueSetResult = currentSection->SetValue(keyValuePair.first, std::move(parsedValue), false);
		if (valueSetResult != EResult::Success)
		{
			PP_COUT_SYNTAX_ERROR("Key already present: " << keyValuePair.first);
			PP_COUT_HERE();
			return valueSetResult;
		}
	}

	return EResult::Success;
}

minipp::EResult minipp::MiniPPFile::Write(const std::string& path) const noexcept
{
	std::ofstream ofs;
	ofs.open(path);
	if (!ofs.is_open())
		return EResult::FileIOError;

	return WriteSection(&m_rootSection, ofs, "");
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