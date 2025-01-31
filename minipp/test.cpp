#include "minipp/minipp.hpp"

int main()
{
	minipp::MiniPPFile file;
	auto result = file.Parse("test.mini");

	auto& section = file.GetRoot();
	
	minipp::MiniPPFile::Section* profileSection = section.GetSubSection("profile", &result);

	minipp::MiniPPFile::IntValue* value;
	auto result1 = profileSection->GetValue(&value, "window_width");

	auto result2 = value->GetValue();

	auto marianProfile = section.GetSubSection("profile.marian", &result);
	minipp::MiniPPFile::StringValue* dockOutlinerType;
	marianProfile->GetValue(&dockOutlinerType, "dock_outliner");

	return 0;
}