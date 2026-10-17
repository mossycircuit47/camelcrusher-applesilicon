#include "PluginProcessor.h"
#include "PluginEditor.h"
namespace {
juce::String display(int i,float v){
 if(crusher::boolean(i))return v>=.5f?"On":"Off";
 if(i==4)return juce::String(int(camelphat_recovered::MMEngine::hertz(v,0,0)/camelphat_recovered::floatBits(0x3b14f209)))+" Hz";
 if(i==7)return juce::String(int(20.f*std::log10(1.f-v*.99f)))+" dB";
 if(i==11)return v==0?"-inf dB":juce::String(int(20.*std::log10(std::pow(double(v),2.736999988555908))))+" dB";
 return juce::String(int(v*100.f))+" %";
}
}
juce::AudioProcessorValueTreeState::ParameterLayout CrusherProcessor::layout(){juce::AudioProcessorValueTreeState::ParameterLayout result;
 for(int i=0;i<17;++i){auto attrs=juce::AudioParameterFloatAttributes().withStringFromValueFunction([i](float v,int){return display(i,v);});
 result.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID(crusher::names[size_t(i)],1),crusher::names[size_t(i)],juce::NormalisableRange<float>(0,1,crusher::boolean(i)?1.f:0.f),crusher::presets[0].values[size_t(i)],attrs));}return result;
}
CrusherProcessor::CrusherProcessor():AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true).withOutput("Output",juce::AudioChannelSet::stereo(),true)),state(*this,nullptr,"CamelCrusherState",layout()){
 for(int i=0;i<17;++i){values[size_t(i)]=state.getRawParameterValue(crusher::names[size_t(i)]);midiCC[size_t(i)].store(-1);}
 for(auto& p:crusher::presets)programs.push_back({p.name,p.values});
}
bool CrusherProcessor::isBusesLayoutSupported(const BusesLayout& b)const{return b.getMainInputChannelSet()==juce::AudioChannelSet::stereo()&&b.getMainOutputChannelSet()==juce::AudioChannelSet::stereo();}
void CrusherProcessor::prepareToPlay(double sr,int){for(int i=0;i<17;++i)last[size_t(i)]=values[size_t(i)]->load();engine.prepare(float(sr),last);}
void CrusherProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer& midi){juce::ScopedNoDenormals noDenormals;
 for(int i=0;i<17;++i){float v=values[size_t(i)]->load();if(v!=last[size_t(i)]){engine.set(i,v);last[size_t(i)]=v;}}
 std::array<crusher::Event,1024> events{};int count=0;
 for(auto metadata:midi){auto msg=metadata.getMessage();if(!msg.isController())continue;
  int cc=msg.getControllerNumber(),target=learnTarget.exchange(-1);if(target>=0)midiCC[size_t(target)].store(cc);
  for(int i=0;i<12;++i)if(midiCC[size_t(i)].load()==cc&&count<int(events.size())){float v=float(msg.getControllerValue())/127.f;state.getParameter(crusher::names[size_t(i)])->setValueNotifyingHost(v);events[size_t(count++)]={metadata.samplePosition,i,v};last[size_t(i)]=v;}
 }
 engine.process(b.getReadPointer(0),b.getReadPointer(1),b.getWritePointer(0),b.getWritePointer(1),b.getNumSamples(),events.data(),count);midi.clear();
}

void CrusherProcessor::storeProgram(){int k=current.load();for(int i=0;i<17;++i)programs[size_t(k)].values[size_t(i)]=values[size_t(i)]->load();}
void CrusherProcessor::activate(int k){current.store(k);for(int i=0;i<17;++i)state.getParameter(crusher::names[size_t(i)])->setValueNotifyingHost(programs[size_t(k)].values[size_t(i)]);}
void CrusherProcessor::setCurrentProgram(int k){if(k<0||k>=int(programs.size()))return;storeProgram();activate(k);}
const juce::String CrusherProcessor::getProgramName(int k){return k>=0&&k<int(programs.size())?juce::String(programs[size_t(k)].name):juce::String();}
void CrusherProcessor::changeProgramName(int k,const juce::String& n){if(k>=0&&k<int(programs.size()))programs[size_t(k)].name=n.toStdString();}
void CrusherProcessor::factory(int k){if(k<0||k>=20)return;programs.clear();for(auto& p:crusher::presets)programs.push_back({p.name,p.values});activate(k);}
void CrusherProcessor::clearPreset(){std::array<float,17> v{};v[4]=.5f;v[9]=v[10]=v[11]=1;for(int i=0;i<17;++i)state.getParameter(crusher::names[size_t(i)])->setValueNotifyingHost(v[size_t(i)]);changeProgramName(current.load(),"blank");}
void CrusherProcessor::randomize(){auto& rng=juce::Random::getSystemRandom();auto uniform=[&](){return rng.nextFloat();};auto gaussian=[&](float d){double x,y,r;do{x=uniform()*2.-1.;y=uniform()*2.-1.;r=x*x+y*y;}while(r<=0||r>1);return juce::jlimit(0.f,1.f,float(x*std::sqrt(-2.*std::log(r)/r)*d));};auto set=[&](int i,float v){state.getParameter(crusher::names[size_t(i)])->setValueNotifyingHost(v);};
 if(values[0]->load()>.5f){set(1,gaussian(.3f));set(2,gaussian(1));}if(values[3]->load()>.5f){set(4,uniform());set(5,uniform());}if(values[6]->load()>.5f){set(7,uniform());set(8,uniform()<.5f?1.f:0.f);}
}
void CrusherProcessor::getStateInformation(juce::MemoryBlock& block){storeProgram();auto tree=state.copyState();auto old=tree.getChildWithName("Programs");if(old.isValid())tree.removeChild(old,nullptr);tree.setProperty("program",current.load(),nullptr);for(int i=0;i<17;++i)tree.setProperty("cc"+juce::String(i),midiCC[size_t(i)].load(),nullptr);juce::ValueTree bank("Programs");for(auto& p:programs){juce::ValueTree item("Preset");item.setProperty("name",juce::String(p.name),nullptr);for(int i=0;i<17;++i)item.setProperty("p"+juce::String(i),p.values[size_t(i)],nullptr);bank.addChild(item,-1,nullptr);}tree.addChild(bank,-1,nullptr);auto xml=tree.createXml();copyXmlToBinary(*xml,block);}
void CrusherProcessor::setStateInformation(const void* data,int size){auto xml=getXmlFromBinary(data,size);if(!xml)return;auto tree=juce::ValueTree::fromXml(*xml);if(!tree.hasType("CamelCrusherState"))return;
 std::vector<crusher::LegacyPreset> bank;for(auto item:tree.getChildWithName("Programs")){crusher::LegacyPreset p;p.name=item["name"].toString().toStdString();for(int i=0;i<17;++i){float v=float(item["p"+juce::String(i)]);if(!std::isfinite(v)||v<0||v>1)return;p.values[size_t(i)]=v;}bank.push_back(p);}
 if(bank.empty()||bank.size()>1024)return;int k=int(tree.getProperty("program",0));if(k<0||k>=int(bank.size()))return;
 programs=std::move(bank);current.store(k);for(int i=0;i<17;++i){int cc=int(tree.getProperty("cc"+juce::String(i),-1));midiCC[size_t(i)].store(juce::jlimit(-1,127,cc));}state.replaceState(tree);
}
bool CrusherProcessor::importLegacy(const juce::File& file){juce::MemoryBlock b;if(!file.loadFileAsData(b))return false;std::vector<crusher::LegacyPreset> decoded;if(!crusher::decodeLegacy(b.getData(),b.getSize(),decoded))return false;if(file.hasFileExtension("fxp")){if(decoded.size()!=1)return false;programs[size_t(current.load())]=decoded[0];activate(current.load());}else{programs=std::move(decoded);activate(0);}return true;}
bool CrusherProcessor::saveLegacy(const juce::File& file,bool bank){storeProgram();auto b=crusher::encodeLegacy(bank?programs:std::vector<crusher::LegacyPreset>{programs[size_t(current.load())]},bank);return !b.empty()&&file.replaceWithData(b.data(),b.size());}
juce::AudioProcessorEditor* CrusherProcessor::createEditor(){return new CrusherEditor(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new CrusherProcessor;}
