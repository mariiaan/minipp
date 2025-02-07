#define MINIPP_IMPLEMENTATION
#include "minipp.hpp"

using namespace minipp;

int main()
{
	EResult result;

	MiniPPFile file;
	result = file.Parse("test.mini");

 	auto& root = file.GetRoot();

	MiniPPFile::Section* gameSection = nullptr;
	result = root.GetSubSection("game", &gameSection);
	MiniPPFile::Values::StringValue* nameValue = nullptr;
	result = gameSection->GetValue("name", &nameValue);
	MiniPPFile::Values::IntValue* yearValue = nullptr;
	result = gameSection->GetValue("year", &yearValue);
	MiniPPFile::Values::FloatValue* completionPercentage = nullptr;
	result = gameSection->GetValue("completionPercentage", &completionPercentage);
	MiniPPFile::Values::BooleanValue* isCompleted = nullptr;
	result = gameSection->GetValue("is_completed", &isCompleted);

	MiniPPFile::Section* windowSection = nullptr;
	result = gameSection->GetSubSection("window", &windowSection);
	MiniPPFile::Values::ArrayValue* dimensionsValue = nullptr;
	result = windowSection->GetValue("dimensions", &dimensionsValue);
	MiniPPFile::Values::IntValue* closeFlags = nullptr;
	result = windowSection->GetValue("close_flags", &closeFlags);
	MiniPPFile::Values::StringValue* hexTest = nullptr;
	result = windowSection->GetValue("hex_test", &hexTest);

	MiniPPFile::Section* windowPlatformSection = nullptr;
	result = gameSection->GetSubSection("window.platform", &windowPlatformSection);
	MiniPPFile::Values::ArrayValue* targetsValue = nullptr;
	result = windowPlatformSection->GetValue("targets", &targetsValue);
	MiniPPFile::Values::ArrayValue* pointsValue = nullptr;
	//result = windowPlatformSection->GetValue("points", &pointsValue);
	result = root.GetValue("game.window.platform.points", &pointsValue);

	result = file.Write("test_out.mini");

	int64_t test = root.GetValueOrDefault<MiniPPFile::Values::IntValue>("game.year", 1999);
	return 0;
}