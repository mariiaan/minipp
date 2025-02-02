# minipp

A lightweight, header-only C++14 parser and writer for the [mini config format](https://github.com/ToyB-Chan/mini-file-format).

## Description
minipp provides a convenient way to parse and write configuration files in the "mini" format. This format is characterized by its human-readable structure and nested sections, making it ideal for application configurations.

## Features

- **Parsing**: Extract values from existing mini configuration files.
  - Supports strings, integers, floats, booleans, arrays, and nested sections.
  
- **Writing**: Generate new mini configuration files.
  - Create complex structures with nested sections and various data types.
  
- **Single Header**: minipp is a single-header library, meaning you only need to include one file.

### Example Usage

```cpp
#define MINIPP_IMPLEMENTATION
#include "minipp.hpp"

using namespace minipp;

int main()
{
  // Make room for a result variable. 
  EResult result;

  MiniPPFile file;
  result = file.Parse("test.mini");
  auto& root = file.GetRoot();

  MiniPPFile::Section* gameSection = nullptr;
  result = root.GetSubSection("game", &gameSection);
  // "Easy" API
  int64_t test = gameSection.GetValueOrDefault<MiniPPFile::Values::IntValue>("year", 1999);

  // Verbose API
  MiniPPFile::Values::StringValue* nameValue = nullptr;
  result = gameSection->GetValue("name", &nameValue);
  MiniPPFile::Values::IntValue* yearValue = nullptr;
  result = gameSection->GetValue("year", &yearValue);

  // Get a sub-section (section of a section (stated in the MINI file with "[game.window]")
  MiniPPFile::Section* windowSection = nullptr;
  result = gameSection->GetSubSection("window", &windowSection);

  MiniPPFile::Section* windowPlatformSection = nullptr;
  // Get a sub-section by using the dot operator
  result = gameSection->GetSubSection("window.platform", &windowPlatformSection);

  MiniPPFile::Values::ArrayValue* pointsValue = nullptr;
  // Retrieve an array by using the relative value path (the game section is a child of the root section)
  result = root.GetValue("game.window.platform.targets", &pointsValue);

  // Modify the "targets" array by adding a new value
  pointsValue->GetValues().push_back(std::make_unique<MiniPPFile::Values::StringValue>("haiku"));

  // Serialize the config
  result = file.Write("test_out.mini");

  return 0;
}
```

## Getting Started

1. **Include minipp**: Add the following line to your file:

   ```cpp
   #define MINIPP_IMPLEMENTATION
   #include "minipp.hpp"
   ```
   Note, that you must define MINIPP_IMPLEMENTATION exactly once in your project!

2. **Create a `MiniPPFile` object**:

   ```cpp
   MiniPPFile file;
   EResult result = file.Parse("your-file.mini");
   ```

3. **Parse and access values**:

   ```cpp
    // Access sections
    MiniPPFile::Section* gameSection = nullptr;
    EResult result = root.GetSubSection("game", &gameSection);

    // Access values
    MiniPPFile::Values::StringValue* nameValue = nullptr;
    result = gameSection->GetValue("name", &nameValue);
   
    // Navigate nested sections

    MiniPPFile::Section* windowSection = nullptr;
    result = gameSection->GetSubSection("window", &windowSection);

    // .. OR ..

    MiniPPFile::Section* windowSection = nullptr;
    result = root->GetSubSection("game.window", &windowSection);
   
   ```
   Note, that the root node cannot have values!
   
   Also, arrays are contained in an ArrayValue (container of multiple different Values (may also include other arrays))

   ```cpp
    // Handle arrays
    MiniPPFile::Values::ArrayValue* pointsValue = nullptr;
    result = root.GetValue("game.window.platform.points", &pointsValue);

    // Modify the array values (or read them)
    pointsValue->GetValues().push_back(std::make_unique<MiniPPFile::Values::StringValue>("haiku"));

   ```
5. **Write new files**:

   ```cpp
    result = file.Write("new-file.mini");
   ```

## Example

An example mini file is contained in this [repository](minipp/test.mini). The full mini file format specilization can be found [here](https://github.com/ToyB-Chan/mini-file-format).

## Installation

1. Copy the contents of [minipp.hpp](minipp/minipp.hpp) to a new file in your project.
2. #Include "minipp.hpp" after defining MINIPP_IMPLEMENTATION in one single cpp file.
3. #Include "minipp.hpp" in any other desired files without the IMPLEMENTATION define!

## License

This code is licensed under the MIT License. See the [LICENSE](LICENSE) file for more details.
