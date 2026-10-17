#include "PluginEditor.h"
#include "BinaryData.h"
#include <sstream>
CrusherEditor::CrusherEditor(CrusherProcessor& p):AudioProcessorEditor(p),processor(p){
 auto asset=[](juce::String n,int& size){return BinaryData::getNamedResource(n.replaceCharacter('.','_').toRawUTF8(),size);};
 for(auto name:{"Background","Knob","OnButton","PhatButton","ButtonRandom","SelectorPreset","FontDisplay"}){int n=0;auto* b=asset(juce::String(name)+".png",n);if(b)images[name]=juce::ImageFileFormat::loadFrom(b,size_t(n));}
 setSize(images["Background"].getWidth(),images["Background"].getHeight());
 int n=0;auto* b=asset("SkinParameters.txt",n);juce::StringArray lines;lines.addLines(juce::String::fromUTF8(b,n));bool inControls=false;
 for(int i=0;i<lines.size();++i){auto line=lines[i].trim();if(line=="FontDisplay"&&i+3<lines.size()&&lines[i+1].trim()=="75"){glyphs=lines[i+2];std::istringstream stream(lines[i+3].toStdString());int w;while(stream>>w)widths.push_back(w);}
 if(line=="CONTROLS"){inControls=true;continue;}if(!inControls||line.isEmpty()||line.startsWith("---"))continue;
 std::istringstream stream(line.toStdString());std::string name,type,image,page;Control c;int x2=0,y2=0;if(!(stream>>name>>type>>image>>page>>c.x>>c.y))continue;stream>>x2>>y2;c.name=name;c.type=type;c.image=image;
 for(int id=0;id<17;++id)if(c.name==crusher::names[size_t(id)])c.id=id;
 auto im=images[c.image];int w=im.getWidth(),h=im.getHeight();if(c.type=="Knob")h=w;else if(c.type=="OnButton"||c.type=="KickButton")h/=2;
 c.bounds=x2>0?juce::Rectangle<int>(c.x,c.y,x2-c.x,y2-c.y):juce::Rectangle<int>(c.x-w/2,c.y-h/2,w,h);controls.push_back(c);
 }startTimerHz(30);
}
juce::RangedAudioParameter* CrusherEditor::parameter(int i)const{return i>=0&&i<17?processor.state.getParameter(crusher::names[size_t(i)]):nullptr;}
void CrusherEditor::set(int i,float v){if(auto* p=parameter(i)){p->setValueNotifyingHost(juce::jlimit(0.f,1.f,v));display=p->getText(p->getValue(),40);displayUntil=juce::Time::getMillisecondCounterHiRes()+1800;repaint();}}
void CrusherEditor::text(juce::Graphics& g,juce::String s,int x,int y){auto im=images["FontDisplay"];for(auto ch:s){int index=glyphs.indexOfChar(ch);if(index<0||index>=int(widths.size())){x+=5;continue;}int offset=0;for(int j=0;j<index;++j)offset+=widths[size_t(j)];int w=widths[size_t(index)];g.drawImage(im,x,y,w,im.getHeight(),offset,0,w,im.getHeight());x+=w;}}
void CrusherEditor::paint(juce::Graphics& g){g.drawImageAt(images["Background"],0,0);
 for(auto& c:controls){auto im=images[c.image];int w=im.getWidth(),h=im.getHeight();auto* p=parameter(c.id);float v=p?p->getValue():0;
 if(c.type=="Knob"){if(w==0)continue;int frame=juce::jlimit(0,h/w-1,int(v*float(h/w-1)));g.drawImage(im,c.bounds.getX(),c.bounds.getY(),w,w,0,frame*w,w,w);}
 else if(c.type=="OnButton"||c.type=="KickButton"){h/=2;g.drawImage(im,c.bounds.getX(),c.bounds.getY(),w,h,0,(v>=.5f||(c.type=="KickButton"&&pressed>=0&&controls[size_t(pressed)].name==c.name))?h:0,w,h);}
 else if(c.type=="PresetSelector"){g.drawImage(im,c.x,c.y,w,h/3,0,0,w,h/3);juce::Graphics::ScopedSaveState save(g);g.reduceClipRegion(c.x+25,c.y,w-50,h/3);text(g,display.isEmpty()?processor.getProgramName(processor.getCurrentProgram()):display,c.x+29,c.y+9);}
 }
}
void CrusherEditor::mouseDown(const juce::MouseEvent& e){for(int i=0;i<int(controls.size());++i){auto& c=controls[size_t(i)];if(!c.bounds.contains(e.getPosition()))continue;
 if(c.type=="KickButton"){pressed=i;processor.randomize();repaint();return;}
 if(c.type=="PresetSelector"){if(e.x<c.x+25||e.x>c.x+168){int n=processor.getNumPrograms();processor.setCurrentProgram((processor.getCurrentProgram()+(e.x<c.x+25?-1:1)+n)%n);display.clear();repaint();}else menu(c);return;}
 if(e.mods.isPopupMenu()&&c.id>=0){juce::PopupMenu m;m.addItem(1,"MIDI Learn");m.addItem(2,"Clear MIDI mapping");m.addItem(3,"Linear knob",true,!circular);m.addItem(4,"Circular knob",true,circular);juce::Component::SafePointer<CrusherEditor> safe(this);m.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(localAreaToGlobal(c.bounds)),[safe,id=c.id](int r){if(!safe)return;if(r==1)safe->processor.learn(id);if(r==2){safe->processor.clearMapping(id);}if(r==3||r==4)safe->circular=r==4;});return;}
 if(auto* p=parameter(c.id)){p->beginChangeGesture();if(c.type=="OnButton"){set(c.id,p->getValue()>=.5f?0:1);p->endChangeGesture();}else{dragging=i;start=p->getValue();origin=e.position;}return;}
 }}
void CrusherEditor::mouseDrag(const juce::MouseEvent& e){if(dragging<0)return;auto& c=controls[size_t(dragging)];float v=start+(origin.y-e.position.y)/(e.mods.isShiftDown()?1000.f:200.f);if(circular){float angle=std::atan2(e.position.x-float(c.x),float(c.y)-e.position.y);v=(angle+juce::MathConstants<float>::pi*.75f)/(juce::MathConstants<float>::pi*1.5f);}set(c.id,v);}
void CrusherEditor::mouseUp(const juce::MouseEvent&){if(dragging>=0)if(auto* p=parameter(controls[size_t(dragging)].id))p->endChangeGesture();dragging=pressed=-1;repaint();}
void CrusherEditor::mouseDoubleClick(const juce::MouseEvent& e){for(auto& c:controls)if(c.type=="Knob"&&c.bounds.contains(e.getPosition()))if(auto* p=parameter(c.id)){p->beginChangeGesture();set(c.id,c.id==4?.5f:(c.id==10||c.id==11?1.f:0.f));p->endChangeGesture();}}
void CrusherEditor::mouseWheelMove(const juce::MouseEvent& e,const juce::MouseWheelDetails& w){for(auto& c:controls)if(c.type=="Knob"&&c.bounds.contains(e.getPosition()))if(auto* p=parameter(c.id)){p->beginChangeGesture();set(c.id,p->getValue()+w.deltaY*(e.mods.isShiftDown()?.01f:.1f));p->endChangeGesture();return;}}
void CrusherEditor::menu(const Control& c){juce::PopupMenu m;m.addItem(1001,"Load Preset...");m.addItem(1002,"Load Bank...");m.addItem(1003,"Save Preset...");m.addItem(1004,"Save Bank...");m.addItem(1005,"Clear Preset");m.addSeparator();for(int i=0;i<processor.getNumPrograms();++i)m.addItem(i+1,processor.getProgramName(i),true,i==processor.getCurrentProgram());juce::PopupMenu factory;for(int i=0;i<20;++i)factory.addItem(2001+i,crusher::presets[size_t(i)].name);m.addSubMenu("Factory Bank",factory);
 juce::Component::SafePointer<CrusherEditor> safe(this);m.showMenuAsync(juce::PopupMenu::Options().withTargetScreenArea(localAreaToGlobal(c.bounds)),[safe](int r){if(!safe||r==0)return;if(r==1005)safe->processor.clearPreset();else if(r>=1001&&r<=1004)safe->chooseFile(r>=1003,r==1002||r==1004);else if(r>=2001&&r<=2020)safe->processor.factory(r-2001);else if(r<1001)safe->processor.setCurrentProgram(r-1);safe->display.clear();safe->repaint();});
}
void CrusherEditor::chooseFile(bool save,bool bank){chooser=std::make_unique<juce::FileChooser>(save?"Save CamelCrusher":"Load CamelCrusher",juce::File(),bank?"*.fxb":"*.fxp");int flags=juce::FileBrowserComponent::canSelectFiles|(save?juce::FileBrowserComponent::saveMode:juce::FileBrowserComponent::openMode);if(save)flags|=juce::FileBrowserComponent::warnAboutOverwriting;
 juce::Component::SafePointer<CrusherEditor> safe(this);chooser->launchAsync(flags,[safe,save,bank](const juce::FileChooser& fc){if(!safe)return;auto file=fc.getResult();if(file==juce::File())return;bool ok=save?safe->processor.saveLegacy(file.withFileExtension(bank?"fxb":"fxp"),bank):safe->processor.importLegacy(file);if(!ok)juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::WarningIcon,"Preset file",save?"The file could not be saved.":"The file is not a supported CamelCrusher preset or bank.");safe->display.clear();safe->repaint();});
}
