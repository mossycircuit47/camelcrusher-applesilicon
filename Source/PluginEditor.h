#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <map>
class CrusherEditor final:public juce::AudioProcessorEditor,private juce::Timer {
public:
 explicit CrusherEditor(CrusherProcessor&);
 void paint(juce::Graphics&)override;
 void mouseDown(const juce::MouseEvent&)override;void mouseDrag(const juce::MouseEvent&)override;void mouseUp(const juce::MouseEvent&)override;
 void mouseDoubleClick(const juce::MouseEvent&)override;void mouseWheelMove(const juce::MouseEvent&,const juce::MouseWheelDetails&)override;
private:
 struct Control {juce::String name,type,image;int id=-1,x=0,y=0;juce::Rectangle<int> bounds;};
 CrusherProcessor& processor;std::vector<Control> controls;std::map<juce::String,juce::Image> images;
 juce::String glyphs;std::vector<int> widths;int dragging=-1,pressed=-1;float start=0;juce::Point<float> origin;
 juce::String display;double displayUntil=0;bool circular=false;
 std::unique_ptr<juce::FileChooser> chooser;
 juce::RangedAudioParameter* parameter(int)const;void set(int,float);void menu(const Control&);void chooseFile(bool,bool);
 void text(juce::Graphics&,juce::String,int,int);void timerCallback()override{if(juce::Time::getMillisecondCounterHiRes()>displayUntil)display.clear();repaint();}
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CrusherEditor)
};
