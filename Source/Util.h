#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

inline File GetPluginDir()
{
    return File::getSpecialLocation(juce::File::currentExecutableFile).getParentDirectory();
}
