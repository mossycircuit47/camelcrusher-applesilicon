#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"
#include "FactoryPresets.h"
#include "LegacyPresets.h"
class CrusherProcessor final:public juce::AudioProcessor {
public:
 CrusherProcessor();
 juce::AudioProcessorValueTreeState state;
 void prepareToPlay(double,int)override;void releaseResources()override{};
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&)override;
 bool isBusesLayoutSupported(const BusesLayout&)const override;
 juce::AudioProcessorEditor* createEditor()override;bool hasEditor()const override{return true;}
 const juce::String getName()const override{return "CamelCrusher Rebuilt";}
 bool acceptsMidi()const override{return true;}bool producesMidi()const override{return false;}
 double getTailLengthSeconds()const override{return 0;}
 int getNumPrograms()override{return int(programs.size());}int getCurrentProgram()override{return current.load();}
 void setCurrentProgram(int)override;const juce::String getProgramName(int)override;void changeProgramName(int,const juce::String&)override;
 void getStateInformation(juce::MemoryBlock&)override;void setStateInformation(const void*,int)override;
 bool importLegacy(const juce::File&);bool saveLegacy(const juce::File&,bool bank);
 void clearPreset();void randomize();void factory(int);
 void clearMapping(int i){midiCC[size_t(i)].store(-1);}
 void learn(int i){learnTarget.store(i);}int mapping(int i)const{return midiCC[size_t(i)].load();}
private:
 static juce::AudioProcessorValueTreeState::ParameterLayout layout();
 std::array<std::atomic<float>*,17> values{};std::array<float,17> last{};
 std::array<std::atomic<int>,17> midiCC{};std::atomic<int> learnTarget{-1},current{0};
 std::vector<crusher::LegacyPreset> programs;
 crusher::Engine engine;
 void storeProgram();void activate(int);
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrusherProcessor)
};
