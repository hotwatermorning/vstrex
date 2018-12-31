#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

inline File GetPluginDir()
{
    return File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
}

inline File GetModelPath()
{
#if defined(_MSC_VER)
	return GetPluginDir().getChildFile("../../Resources/model/tyrannosaurus_anim_XNA.X");
#else
	return GetPluginDir().getChildFile("../Resources/model/tyrannosaurus_anim_XNA.X");
#endif
}

inline File GetLogoPath()
{
#if defined(_MSC_VER)
	return GetPluginDir().getChildFile("../../Resources/image/logo.png");
#else
	return GetPluginDir().getChildFile("../Resources/image/logo.png");
#endif
}