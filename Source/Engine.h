#pragma once
#include <array>
#include <algorithm>
#include "RecoveredDistortion.h"
#include "RecoveredMMEngine.h"
namespace crusher {
inline constexpr std::array<const char*,17> names{{"DistOn","DistMech","DistTube","MmFilterOn","MmFilterCutoff","MmFilterRes","CompressOn","CompressAmount","CompressMode","MasterOn","MasterMix","MasterVolume","Unused1","Unused2","Unused3","Unused4","Unused5"}};
inline bool boolean(int i){return i==0||i==3||i==6||i==8||i==9||i>=12;}
struct Parameter {
 float value=0,target=0,start=0;int elapsed=0,frames=441;
 void set(float v){if(v==target)return;start=value;target=v;elapsed=0;}
 float advance(int n){if(value!=target){elapsed+=n;float t=std::min(1.f,float(elapsed)/float(frames));value=(1.f-t)*start+t*target;}return value;}
};
struct Event {int sample,id;float value;};
class Engine {
public:
 std::array<Parameter,17> p;std::array<float,17> normalized{};
 camelphat_recovered::Distortion distortion;
 camelphat_recovered::MMEngine filter;
 camelphat_recovered::CACompressor compressor;
 float gain=0,mix=0;
 void set(int i,float v){if(boolean(i))v=float(v>=.5f);normalized[size_t(i)]=v;p[size_t(i)].set(i==7?v*.99f:v);}
 void prepare(float sr,const std::array<float,17>& values){
  for(int i=0;i<17;++i){p[size_t(i)]={};p[size_t(i)].frames=int(.01f*sr);set(i,values[size_t(i)]);p[size_t(i)].value=p[size_t(i)].target;}
  filter.reset(sr);compressor=camelphat_recovered::CACompressor{};compressor.follower.attack=.03f;compressor.follower.release=.08f;compressor.setSampleRate(sr);compressor.clear();gain=mix=0;
 }
 void process(const float* inL,const float* inR,float* outL,float* outR,int n,const Event* events=nullptr,int eventCount=0){
  if(normalized[9]<.5f){std::copy_n(inL,n,outL);std::copy_n(inR,n,outR);return;}
  bool mech=false,tube=false;
  if(p[0].advance(n)>.5f){mech=p[1].advance(n)>0;tube=p[2].advance(n)>0;}
  bool filtering=p[3].advance(n)>.5f,compressing=p[6].advance(n)>.5f;
  compressor.setEnvelopeMode(normalized[8]<.5f);
  int eventIndex=0;
  for(int offset=0;offset<n;offset+=64){int count=std::min(64,n-offset);while(eventIndex<eventCount&&events[eventIndex].sample<=offset+64){const auto& e=events[eventIndex++];set(e.id,e.value);if(e.id==8)compressor.setEnvelopeMode(e.value<.5f);}
   float l[64],r[64];std::copy_n(inL+offset,count,l);std::copy_n(inR+offset,count,r);
   if(mech)distortion.processMech(l,r,count,p[1].advance(count));
   if(filtering){float cut=p[4].advance(count),res=p[5].advance(count);filter.process(0,cut,res,0,1,0,0,l,r,count);}
   if(tube)distortion.processTube(l,r,count,p[2].advance(count));
   if(compressing)compressor.process(l,r,count,std::min(.99f,p[7].advance(count)),.00025f);
   float nextMix=p[10].advance(count),nextGain=float(std::pow(double(p[11].advance(count)),2.736999988555908));
   float mixStep=(nextMix-mix)/float(count),gainStep=(nextGain-gain)/float(count);
   for(int j=0;j<count;++j){float dryL=inL[offset+j],dryR=inR[offset+j];float a=(mix*l[j]+(1.f-mix)*dryL)*gain,b=(mix*r[j]+(1.f-mix)*dryR)*gain;outL[offset+j]=std::clamp(a,-.9999f,.9999f);outR[offset+j]=std::clamp(b,-.9999f,.9999f);mix+=mixStep;gain+=gainStep;}
  }
 }
};
}
