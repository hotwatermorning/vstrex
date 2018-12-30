/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class VstrexAudioProcessorEditor  : public AudioProcessorEditor
{
public:
    VstrexAudioProcessorEditor (VstrexAudioProcessor&);
    ~VstrexAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void mouseUp(MouseEvent const &ev) override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    VstrexAudioProcessor& processor;
    
    struct Impl;
    std::unique_ptr<Impl> pimpl_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VstrexAudioProcessorEditor)
};
