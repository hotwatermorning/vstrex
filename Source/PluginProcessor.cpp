/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VstrexAudioProcessor::VstrexAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    addParameter (rotation_speed_ = new AudioParameterFloat("rotation_speed", // parameter ID
                                                            "Rotation Speed", // parameter name
                                                            0.0f,   // minimum value
                                                            3.0f,   // maximum value
                                                            0.125f)); // default value
    
    addParameter (running_speed_ = new AudioParameterFloat ("running_speed", // parameter ID
                                                            "Running Speed", // parameter name
                                                            0.0f,   // minimum value
                                                            3.0f,   // maximum value
                                                            0.5f)); // default value
    
    lpf_.resize(2);
    rms_ = 0;
    tempo_ = 120.0;
}

VstrexAudioProcessor::~VstrexAudioProcessor()
{
}

//==============================================================================
const String VstrexAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VstrexAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VstrexAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VstrexAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VstrexAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VstrexAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VstrexAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VstrexAudioProcessor::setCurrentProgram (int index)
{
}

const String VstrexAudioProcessor::getProgramName (int index)
{
    return {};
}

void VstrexAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void VstrexAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    rms_meter_ = std::make_unique<RMSMeter>(sampleRate, 30);
    for(int ch = 0; ch < lpf_.size(); ++ch) {
        lpf_[ch].setCoefficients(IIRCoefficients::makeLowPass(sampleRate, 500));
    }
}

void VstrexAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VstrexAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VstrexAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for(int i = 0; i < buffer.getNumSamples(); ++i) {
        double sum = 0;
        for(int ch = 0; ch < totalNumInputChannels; ++ch) {
            auto tmp = buffer.getReadPointer(ch)[i];
            sum += lpf_[ch].processSingleSampleRaw(tmp);
        }
        rms_meter_->PushSample(sum);
    }
    
    auto sec = buffer.getNumSamples() / getSampleRate();
    auto decay = std::max<double>(0, rms_.load() - sec * 2.5);
    rms_.store(jlimit(decay, 1.0, rms_meter_->GetRMS() * 1.1));
    
    auto ph = getPlayHead();
    if(ph) {
        AudioPlayHead::CurrentPositionInfo info;
        ph->getCurrentPosition(info);
        tempo_.store(info.bpm);
    }
}

//==============================================================================
bool VstrexAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* VstrexAudioProcessor::createEditor()
{
    return new VstrexAudioProcessorEditor (*this);
}

//==============================================================================
void VstrexAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VstrexAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VstrexAudioProcessor();
}
