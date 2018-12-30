#pragma once

#include <atomic>

#include "../JuceLibraryCode/JuceHeader.h"
#include "./RMS.h"

//==============================================================================
/**
*/
class VstrexAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    VstrexAudioProcessor();
    ~VstrexAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    double getRMS() const { return rms_.load(); };
    AudioParameterFloat *rotation_speed_;
    AudioParameterFloat *running_speed_;
    std::unique_ptr<RMSMeter> rms_meter_;
    std::atomic<double> rms_;
    std::atomic<double> tempo_;
    std::vector<IIRFilter> lpf_;

private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VstrexAudioProcessor)
};
