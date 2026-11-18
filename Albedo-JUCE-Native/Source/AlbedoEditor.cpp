#include "AlbedoEditor.h"
#include "AlbedoPresetTaxonomy.hpp"
#include <algorithm>
#include <BinaryData.h>
#include <cmath>

namespace {
const auto canvas  = juce::Colour::fromRGB (232, 232, 218);
const auto surface = juce::Colour::fromRGB (239, 239, 225);
const auto text    = juce::Colour::fromRGB (28, 71, 64);
const auto accent  = juce::Colour::fromRGB (177, 118, 67);
const auto gold    = juce::Colour::fromRGB (209, 164, 100);
const auto muted   = juce::Colour::fromRGB (91, 119, 108);
const auto teal    = juce::Colour::fromRGB (31, 89, 84);
const auto air     = juce::Colour::fromRGB (95, 145, 165);
const auto terra   = juce::Colour::fromRGB (110, 149, 107);
const auto colours = std::array<juce::Colour,4> {accent, gold, air, terra};
void label (juce::Graphics& g, juce::String s, juce::Rectangle<float> b, float pt, bool serif=false, bool bold=false,
            juce::Justification j=juce::Justification::centredLeft, juce::Colour c=text)
{
    g.setColour (c);
    g.setFont (juce::Font (juce::FontOptions (serif ? "Georgia" : "Segoe UI", pt, bold ? juce::Font::bold : juce::Font::plain)));
    g.drawFittedText (s, b.toNearestInt(), j, 1, 0.86f);
}
void panel (juce::Graphics& g, juce::Rectangle<float> b)
{
    g.setColour (surface); g.fillRoundedRectangle (b, 5.0f);
    g.setColour (teal.withAlpha (0.25f)); g.drawRoundedRectangle (b.reduced (0.5f), 5.0f, 1.0f);
}
}

struct AlbedoEditor::AlbedoLookAndFeel final : juce::LookAndFeel_V4
{
    AlbedoLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, text);
        setColour (juce::Slider::thumbColourId, accent);
        setColour (juce::ComboBox::backgroundColourId, surface);
        setColour (juce::ComboBox::textColourId, text);
        setColour (juce::TextButton::buttonColourId, surface);
        setColour (juce::TextButton::textColourOffId, teal);
        setColour (juce::TextEditor::backgroundColourId, surface);
        setColour (juce::TextEditor::textColourId, text);
        setColour (juce::ListBox::backgroundColourId, surface);
    }
    void drawRotarySlider (juce::Graphics& g,int x,int y,int w,int h,float value,
                           float start,float end,juce::Slider&) override
    {
        const auto side=(float) juce::jmin(w,h);
        const auto c=juce::Point<float>((float)x+w*.5f,(float)y+h*.5f);
        const auto rad=side*.32f;
        g.setColour (teal.withAlpha(.14f));g.fillEllipse(c.x-rad-3,c.y-rad-3,2*(rad+3),2*(rad+3));
        g.setColour (muted.withAlpha(.56f));g.drawEllipse(c.x-rad,c.y-rad,2*rad,2*rad,1.2f);
        juce::Path arc;arc.addCentredArc(c.x,c.y,rad,rad,0,start,start+(end-start)*value,true);
        g.setColour(accent);g.strokePath(arc,juce::PathStrokeType(2.2f));
        g.setColour(surface.brighter(.1f));g.fillEllipse(c.x-rad*.65f,c.y-rad*.65f,2*rad*.65f,2*rad*.65f);
        g.setColour(teal);juce::Path pointer;pointer.addLineSegment({c.x,c.y,c.x+rad*.68f*std::cos(start+(end-start)*value-juce::MathConstants<float>::halfPi),c.y+rad*.68f*std::sin(start+(end-start)*value-juce::MathConstants<float>::halfPi)},1.8f);g.strokePath(pointer,juce::PathStrokeType(1.6f));
    }
    void drawLinearSlider(juce::Graphics&g,int x,int y,int w,int h,float sliderPos,float,float,
                          juce::Slider::SliderStyle,juce::Slider&) override
    {
        const float cy=y+h*.50f;
        g.setColour(muted.withAlpha(.28f));g.fillRoundedRectangle((float)x+5,cy-2,(float)w-10,4,2);
        g.setColour(teal.withAlpha(.7f));g.fillRoundedRectangle((float)x+5,cy-2,juce::jmax(1.f,sliderPos-x-5),4,2);
        g.setColour(accent);g.fillEllipse(sliderPos-5,cy-5,10,10);
    }
};

struct AlbedoEditor::Module final : juce::Component
{
    juce::String title,subtitle;
    int id;
    Module(int n,juce::String t,juce::String st={}) : title(std::move(t)),subtitle(std::move(st)),id(n){}
    void paint(juce::Graphics&g) override
    {
        auto b=getLocalBounds().toFloat();panel(g,b.reduced(1));
        g.setColour(teal.withAlpha(.12f));g.fillRect(b.getX()+1,b.getY()+1,b.getWidth()-2,29.f);
        label(g,title,{b.getX()+13,b.getY()+5,b.getWidth()-26,21},19.f,true,true);
        if(subtitle.isNotEmpty()) label(g,subtitle,{b.getX()+13,b.getY()+29,b.getWidth()-26,14},9.f,false,true,juce::Justification::centredLeft,muted);
        g.setColour(accent.withAlpha(.4f));g.fillRect(b.getX()+11,b.getY()+28,b.getWidth()-22,1.f);
    }
};

struct AlbedoEditor::ParamControl final : juce::Component
{
    AlbedoProcessor& processor; uint32_t id;
    juce::Slider slider;
    juce::String name;
    bool dial=true,showLabel=true;
    float last=-1;
    ParamControl(AlbedoProcessor& p,uint32_t id_,bool d,bool show) : processor(p),id(id_),dial(d),showLabel(show)
    {
        addAndMakeVisible(slider);
        slider.setSliderStyle(dial?juce::Slider::RotaryVerticalDrag:juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        bind(id);
        slider.onDragStart=[this]{if(auto* p=processor.parameter(id))p->beginChangeGesture();};
        slider.onDragEnd=[this]{if(auto* p=processor.parameter(id))p->endChangeGesture();};
        slider.onValueChange=[this]
        {
            if(auto* p=processor.parameter(id))
                p->setValueNotifyingHost(p->convertTo0to1((float)slider.getValue()));
            repaint();
        };
    }
    void bind(uint32_t newId)
    {
        id=newId;
        if(auto*p=processor.parameter(id))
        {
            auto range=p->range;
            slider.setRange(range.start,range.end,range.interval);
            slider.setValue(p->get(),juce::dontSendNotification);
            name=p->getName(100);
            last=p->get();
        }
    }
    void sync()
    {
        if(auto* p=processor.parameter(id))
        {
            const float v=p->get();
            if(v!=last){last=v;slider.setValue(v,juce::dontSendNotification);repaint();}
        }
    }
    void resized() override
    {
        if(dial)slider.setBounds(getLocalBounds().withTrimmedTop(1).withTrimmedBottom(showLabel?31:1));
        else slider.setBounds(getLocalBounds().withTrimmedLeft(showLabel?84:2).reduced(2,7));
    }
    void paint(juce::Graphics&g) override
    {
        if(!showLabel)return;
        if(dial)
        {
            label(g,name,{0.f,(float)getHeight()-33,(float)getWidth(),17},12.f,false,true,juce::Justification::centred);
            label(g,juce::String(slider.getValue(),slider.getInterval()>=1?0:2),
                    {0.f,(float)getHeight()-18,(float)getWidth(),16},12.f,true,false,juce::Justification::centred,muted);
        }
        else label(g,name,{0,0,83.f,(float)getHeight()},11.f,false,true);
    }
};

struct AlbedoEditor::ArborView final : juce::Component, private juce::Timer
{
    AlbedoProcessor& processor;
    float t=0.f;
    explicit ArborView(AlbedoProcessor&p):processor(p){startTimerHz(30);}
    void timerCallback() override {t+=.017f;repaint();}
    void paint(juce::Graphics&g) override
    {
        auto b=getLocalBounds().toFloat().reduced(5);
        juce::ColourGradient grad(juce::Colour::fromRGB(9,43,42),b.getTopLeft(),juce::Colour::fromRGB(22,70,67),b.getBottomRight(),false);
        g.setGradientFill(grad);g.fillRoundedRectangle(b,5);
        auto centre=b.getCentre();const float R=juce::jmin(b.getWidth()*.28f,b.getHeight()*.36f);
        const juce::Point<float> nodes[4]={{centre.x-R*1.9f,centre.y-R*.8f},{centre.x+R*1.9f,centre.y-R*.8f},{centre.x-R*1.9f,centre.y+R*.9f},{centre.x+R*1.9f,centre.y+R*.9f}};
        for(int i=0;i<4;i++)
        {
            float level=juce::jlimit(0.f,1.f,processor.paramValue(1000+i*32+2)/100.f);
            bool enabled=processor.paramValue(1000+i*32+1)>.5f;
            juce::Path path;
            auto start=nodes[i];
            path.startNewSubPath(start);
            path.cubicTo(juce::Point<float>(juce::jmap(.4f,start.x,centre.x),start.y),
                         juce::Point<float>(juce::jmap(.6f,start.x,centre.x),centre.y),centre);
            g.setColour(colours[(size_t)i].withAlpha(enabled?.68f:.16f));g.strokePath(path,juce::PathStrokeType(1.8f));
            if(enabled)
            {
                // Geometry and animation are local to Arbor; no full-window redraw.
                const auto frac=std::fmod(t*(.3f+level*.9f)+i*.23f,1.f);
                
                // Exact cubic interpolation: no browser and no path-flattening allocations.
                const float u=1.f-frac;
                const auto c1=juce::Point<float>(juce::jmap(.4f,start.x,centre.x),start.y);
                const auto c2=juce::Point<float>(juce::jmap(.6f,start.x,centre.x),centre.y);
                const auto cubic=[&](float a,float b,float d,float e)
                {return u*u*u*a + 3*u*u*frac*b + 3*u*frac*frac*d + frac*frac*frac*e;};
                const auto dx=cubic(start.x,c1.x,c2.x,centre.x);
                const auto dy=cubic(start.y,c1.y,c2.y,centre.y);
                g.setColour(colours[(size_t)i].withAlpha(.6f+level*.4f));g.fillEllipse(dx-3,dy-3,6,6);
            }
            g.setColour(colours[(size_t)i]);g.drawEllipse(start.x-14,start.y-14,28,28,1.3f);
            g.setColour(colours[(size_t)i].withAlpha(.18f));g.fillEllipse(start.x-11,start.y-11,22,22);
            label(g,juce::String::charToString("ABCD"[i]),{start.x-15,start.y-8,30,16},10,false,true,juce::Justification::centred,colours[(size_t)i]);
        }
        g.setColour(gold.withAlpha(.65f));g.drawEllipse(centre.x-R*.67f,centre.y-R*.67f,R*1.34f,R*1.34f,2.f);
        g.setColour(gold.withAlpha(.24f));g.fillEllipse(centre.x-R*.52f,centre.y-R*.52f,R*1.04f,R*1.04f);
        g.setColour(accent);g.fillEllipse(centre.x-6,centre.y-6,12,12);
        label(g,"QUINTESSENTIA",{centre.x-90,centre.y-R*.85f,180,14},10.f,true,true,juce::Justification::centred,gold);
    }
};
// CP32: Animated panels reproduce the CP22 signal-flow language without any HTML
// runtime. All repaint calls remain confined to these individual components.
struct AlbedoEditor::DynamicGraph final : juce::Component
{
    enum Kind { engineWave, filterResponse, envelope, cellular, effects, pulse };
    AlbedoProcessor& processor;
    Kind kind;
    int index=0;
    float seconds=0.f;
    int previousVoices=0;

    DynamicGraph(AlbedoProcessor& p,Kind k,int i=0):processor(p),kind(k),index(i)
    {
        setInterceptsMouseClicks(false,false);
        setOpaque(true);
    }
    void advance(float dt)
    {
        if(!isShowing() || getWidth()<2 || getHeight()<2)return;
        seconds=std::fmod(seconds+dt,4096.f);
        if(kind==pulse && processor.paramValue(6020)<.5f && previousVoices==0)return;
        previousVoices=processor.activeVoices();
        repaint(); // dirty rectangle is this component only, never the editor HWND
    }
    void paint(juce::Graphics&g) override
    {
        auto b=getLocalBounds().toFloat();
        if(b.getWidth()<4.f||b.getHeight()<4.f)return;
        const auto dark=kind==engineWave || kind==cellular;
        g.setColour(dark?juce::Colour::fromRGB(19,57,54):surface.darker(.045f));
        g.fillRoundedRectangle(b,3.f);
        b=b.reduced(4.f);
        const auto x=[&](float f){return b.getX()+f*b.getWidth();};
        const auto y=[&](float f){return b.getY()+f*b.getHeight();};
        if(kind==engineWave)
        {
            const bool enabled=processor.paramValue(1001u+32u*(uint32_t)index)>.5f;
            const float level=juce::jlimit(0.f,1.f,processor.paramValue(1002u+32u*(uint32_t)index)/100.f);
            const auto key=processor.activeVoices();
            const float activity=key>0?1.f:.55f;
            juce::Path wave;
            for(int j=0;j<=160;++j)
            {
                const float t=j/160.f;
                const float phase=juce::MathConstants<float>::twoPi*(2.f+float(index)*.63f)*t;
                const float fundamental=std::sin(phase+seconds*(1.1f+.25f*index));
                const float timbre=.22f*std::sin(phase*(2.f+.2f*index)-seconds*(.5f+.1f*index));
                const float noise=(fundamental+timbre)*.32f*level*activity*(enabled?1.f:.12f);
                if(j==0)wave.startNewSubPath(x(t),y(.5f-noise));
                else wave.lineTo(x(t),y(.5f-noise));
            }
            g.setColour(colours[(size_t)index].withAlpha(enabled?.95f:.2f));
            g.strokePath(wave,juce::PathStrokeType(1.6f));
            g.setColour(muted.withAlpha(.28f));g.drawHorizontalLine(int(y(.5f)),x(0),x(1));
        }
        else if(kind==filterResponse)
        {
            const int n=index==2?0:index;
            const float cutoff=juce::jlimit(20.f,20000.f,processor.paramValue(3000u+3u*(uint32_t)n));
            const float res=juce::jlimit(0.f,1.f,processor.paramValue(3001u+3u*(uint32_t)n)/100.f);
            juce::Path response;
            for(int j=0;j<=160;++j)
            {
                const float t=j/160.f;
                const float hz=20.f*std::pow(1000.f,t);
                const float ratio=hz/cutoff;
                const float fall=1.f/std::sqrt(1.f+std::pow(ratio,4.f));
                const float resonant=.30f*res*std::exp(-8.f*std::pow(std::log(juce::jmax(.001f,ratio)),2.f));
                const float value=juce::jlimit(0.f,1.f,fall+resonant);
                if(j==0)response.startNewSubPath(x(t),y(.90f-.80f*value));
                else response.lineTo(x(t),y(.90f-.80f*value));
            }
            g.setColour(teal.withAlpha(.85f));g.strokePath(response,juce::PathStrokeType(1.9f));
            const float t=juce::jlimit(0.f,1.f,std::log(cutoff/20.f)/std::log(1000.f));
            g.setColour(gold.withAlpha(.7f));g.fillEllipse(x(t)-2.f,y(.14f)-2.f,4,4);
        }
        else if(kind==envelope)
        {
            const float a=juce::jlimit(.08f,.30f,processor.paramValue(3100)/18.f+.07f);
            const float d=juce::jlimit(.08f,.27f,processor.paramValue(3101)/25.f+.11f);
            const float sus=juce::jlimit(.05f,.94f,processor.paramValue(3102)/100.f);
            const float r=juce::jlimit(.08f,.31f,processor.paramValue(3103)/28.f+.09f);
            const float fade=juce::jmax(.01f,1.f-a-d-r);
            juce::Path env;
            env.startNewSubPath(x(0.f),y(.92f));env.lineTo(x(a),y(.10f));
            env.lineTo(x(a+d),y(.92f-.82f*sus));
            env.lineTo(x(a+d+fade),y(.92f-.82f*sus));env.lineTo(x(1.f),y(.92f));
            g.setColour(accent);g.strokePath(env,juce::PathStrokeType(2.f));
            if(processor.activeVoices()>0)
            {
                const float t=std::fmod(seconds*.3f,1.f);
                g.setColour(gold);g.fillEllipse(x(t)-3.f,y(.08f)-3.f,6,6);
            }
        }
        else if(kind==cellular)
        {
            // A faithful one-dimensional elementary automaton *visualisation*.
            // Real per-voice cellular DSP state is intentionally not read cross-thread.
            constexpr int cols=32, rows=15;
            const int rules[]={30,90,110,150};
            const auto rule=rules[juce::jlimit(0,3,int(std::round(processor.paramValue(4003))))];
            const int offset=int(seconds*3.f)%32;
            uint32_t state=uint32_t(1)<<((16+offset)%32);
            for(int row=0;row<rows;++row)
            {
                const float cw=b.getWidth()/cols,ch=b.getHeight()/rows;
                for(int col=0;col<cols;++col)
                {
                    if((state>>col)&1u)
                    {
                        const float life=(float(row)+3.f)/float(rows+3);
                        g.setColour(gold.withAlpha(.38f+.56f*life));
                        g.fillRect(b.getX()+cw*col,b.getY()+ch*row,cw*.87f,ch*.86f);
                    }
                }
                uint32_t next=0;
                for(int col=0;col<cols;++col)
                {
                    const int left=(state>>((col+1)%32))&1u;
                    const int centre=(state>>col)&1u;
                    const int right=(state>>((col+31)%32))&1u;
                    if((rule>>((left<<2)|(centre<<1)|right))&1)next|=(uint32_t(1)<<col);
                }
                state=next;
            }
        }
        else if(kind==effects)
        {
            const float rate=juce::jlimit(.02f,6.f,processor.paramValue(5008));
            const float depth=juce::jlimit(.01f,1.f,processor.paramValue(5002)/100.f);
            const float space=juce::jlimit(0.f,1.f,processor.paramValue(5004)/100.f);
            juce::Path p;
            for(int i=0;i<=120;++i)
            {
                const float t=i/120.f;
                const float value=.5f+.25f*depth*std::sin(juce::MathConstants<float>::twoPi*(3.f*t+seconds*rate*.2f))
                                     +.08f*space*std::sin(juce::MathConstants<float>::twoPi*(10.f*t-seconds*.13f));
                if(i==0)p.startNewSubPath(x(t),y(value));else p.lineTo(x(t),y(value));
            }
            g.setColour(teal);g.strokePath(p,juce::PathStrokeType(1.4f));
        }
        else if(kind==pulse)
        {
            const auto enabled=processor.paramValue(6020)>.5f;
            const float duration=juce::jmax(.05f,processor.paramValue(6021));
            const int step=int(seconds/duration)%16;
            const float cw=b.getWidth()/16.f;
            for(int i=0;i<16;i++)
            {
                const bool on=processor.paramValue(6000u+(uint32_t)i)>.5f;
                const float height=on?b.getHeight()*.77f:b.getHeight()*.24f;
                g.setColour(i==step&&enabled?gold:(on?teal.withAlpha(.70f):muted.withAlpha(.24f)));
                g.fillRect(b.getX()+i*cw+1.f,b.getBottom()-height,cw-2.f,height);
            }
        }
    }
};

struct AlbedoEditor::Atlas final : juce::Component, juce::ListBoxModel
{
    // All taxonomy records are indexed by the immutable factory preset ID.
    // No string-parsing heuristics or Unassigned family/role fallback is needed.
    using Meta = albedo_preset_taxonomy::PresetCategory;
    AlbedoProcessor& processor;
    juce::TextEditor search;
    juce::ComboBox family, role, character, motion, context, sort;
    juce::ListBox list {"Factory", this};
    juce::TextButton load {"LOAD PATCH"}, close {"CLOSE"}, clear {"RESET"};
    juce::TextButton listView {"LIST"}, matrixView {"8 x 8 MATRIX"};
    std::vector<int> rows;
    juce::Rectangle<int> gridBounds;
    int selectedIndex=0;
    int lastHostProgram=0;
    bool matrixMode=false;
    std::function<void()> dismiss;

    static const Meta& meta (int i) { return albedo_preset_taxonomy::byIndex(i); }
    static juce::String timbreName (int i)
    {
        auto n=juce::String(meta(i).name);
        return n.fromFirstOccurrenceOf(" | ",false,false).upToFirstOccurrenceOf(" (",false,false);
    }
    static void fill (juce::ComboBox& target,const char* title,
                      const auto& values)
    {
        target.clear(juce::dontSendNotification);
        target.addItem(title,1);
        int id=2;
        for(const auto* value:values)target.addItem(juce::String::fromUTF8(value),id++);
        target.setSelectedId(1,juce::dontSendNotification);
    }
    static bool accepts (const juce::ComboBox& selector,const char* value)
    {
        return selector.getSelectedId()<=1 || selector.getText()==juce::String::fromUTF8(value);
    }
    static juce::String field (int i,int mode)
    {
        const auto& m=meta(i);
        switch(mode)
        {
            case 3:return m.family;
            case 4:return m.role;
            case 5:return m.character;
            case 6:return m.motion;
            case 7:return m.context;
            default:return timbreName(i);
        }
    }
    explicit Atlas (AlbedoProcessor& p):processor(p)
    {
        setOpaque(true);
        addAndMakeVisible(search);
        search.setTextToShowWhenEmpty("Search name, category, character, motion, use...",muted);
        for(auto* c:{&family,&role,&character,&motion,&context,&sort})addAndMakeVisible(*c);
        fill(family,"ALL FAMILIES",albedo_preset_taxonomy::families);
        fill(role,"ALL ROLES",albedo_preset_taxonomy::roles);
        fill(character,"ALL CHARACTERS",albedo_preset_taxonomy::characters);
        fill(motion,"ALL MOTIONS",albedo_preset_taxonomy::motions);
        fill(context,"ALL CONTEXTS",albedo_preset_taxonomy::contexts);
        const char* labels[]={"Name A-Z","Name Z-A","Material family","Instrument role",
                              "Sound character","Motion","Suggested context","Factory index"};
        for(int i=0;i<8;i++)sort.addItem(labels[i],i+1);
        sort.setSelectedId(8,juce::dontSendNotification);
        addAndMakeVisible(list);
        list.setRowHeight(36);
        list.setMultipleSelectionEnabled(false);
        for(auto* b:{&load,&close,&clear,&listView,&matrixView})addAndMakeVisible(*b);
        search.onTextChange=[this]{refresh();};
        family.onChange=[this]{refresh();};role.onChange=[this]{refresh();};
        character.onChange=[this]{refresh();};motion.onChange=[this]{refresh();};
        context.onChange=[this]{refresh();};sort.onChange=[this]{refresh();};
        clear.onClick=[this]
        {
            search.clear();
            for(auto* c:{&family,&role,&character,&motion,&context})
                c->setSelectedId(1,juce::dontSendNotification);
            sort.setSelectedId(8,juce::dontSendNotification);
            refresh();
        };
        listView.onClick=[this]{setMatrixMode(false);};
        matrixView.onClick=[this]{setMatrixMode(true);};
        load.onClick=[this]{loadSelected();};
        close.onClick=[this]{if(dismiss)dismiss();};
        selectedIndex=processor.getCurrentProgram();
        lastHostProgram=selectedIndex;
        refresh();
    }
    void setMatrixMode(bool on)
    {
        matrixMode=on;
        list.setVisible(!matrixMode);
        repaint();
    }
    void refresh()
    {
        rows.clear();
        const auto term=search.getText().trim();
        for(int i=0;i<64;i++)
        {
            const auto& m=meta(i);
            if(!accepts(family,m.family)||!accepts(role,m.role)||
               !accepts(character,m.character)||!accepts(motion,m.motion)||
               !accepts(context,m.context))continue;
            const juce::String searchable=juce::String(m.name)+" " +m.family+" "+m.role+" "+
                    m.character+" "+m.motion+" "+m.context+" "+m.description;
            if(term.isNotEmpty()&&!searchable.containsIgnoreCase(term))continue;
            rows.push_back(i);
        }
        const int mode=sort.getSelectedId();
        std::stable_sort(rows.begin(),rows.end(),[mode](int a,int b)
        {
            if(mode==8)return a<b;
            const auto comparison=field(a,mode).compareIgnoreCase(field(b,mode));
            return comparison==0?a<b:(mode==2?comparison>0:comparison<0);
        });
        list.updateContent();
        auto it=std::find(rows.begin(),rows.end(),selectedIndex);
        if(it==rows.end())
            selectedIndex=rows.empty()?-1:rows.front();
        const auto selected=std::find(rows.begin(),rows.end(),selectedIndex);
        if(selected!=rows.end())list.selectRow(int(std::distance(rows.begin(),selected)));
        else list.deselectAllRows();
        repaint();
    }
    int getNumRows() override{return (int)rows.size();}
    void selectedRowsChanged(int row) override
    {
        if(row>=0&&row<(int)rows.size())selectedIndex=rows[(size_t)row];
        if(matrixMode)repaint(gridBounds);
    }
    void paintListBoxItem(int row,juce::Graphics&g,int w,int h,bool selected) override
    {
        if(row<0||row>=(int)rows.size())return;
        const int id=rows[(size_t)row];const auto& m=meta(id);
        g.setColour(selected?teal.withAlpha(.18f):(row%2?surface:canvas));g.fillRect(0,0,w,h);
        g.setColour(teal.withAlpha(.12f));g.drawHorizontalLine(h-1,0,(float)w);
        const float ww=(float)w;
        label(g,juce::String(id+1).paddedLeft('0',2),{7,3,ww*.048f,(float)h-6},12.f,false,true);
        label(g,timbreName(id),{ww*.058f,3,ww*.235f,(float)h-6},14.f,true,true);
        label(g,m.family,{ww*.31f,3,ww*.125f,(float)h-6},12.f);
        label(g,m.role,{ww*.447f,3,ww*.117f,(float)h-6},12.f);
        label(g,m.character,{ww*.574f,3,ww*.124f,(float)h-6},12.f);
        label(g,m.motion,{ww*.709f,3,ww*.117f,(float)h-6},12.f);
        label(g,m.context,{ww*.839f,3,ww*.15f,(float)h-6},12.f);
    }
    void listBoxItemDoubleClicked(int row,const juce::MouseEvent&) override
    {if(row>=0&&row<(int)rows.size()){selectedIndex=rows[(size_t)row];loadSelected();}}
    void loadSelected()
    {
        if(selectedIndex>=0&&selectedIndex<64)
        {
            processor.loadFactoryPreset(selectedIndex);
            lastHostProgram=selectedIndex;
            if(dismiss)dismiss();
        }
    }
    void syncHostSelection()
    {
        // A host preset load updates the selection without rebuilding filters.
        const int current=processor.getCurrentProgram();
        if(current==lastHostProgram||current<0||current>=64)return;
        lastHostProgram=current;
        selectedIndex=current;
        if(!matrixMode)
        {
            auto it=std::find(rows.begin(),rows.end(),selectedIndex);
            if(it!=rows.end())list.selectRow(int(std::distance(rows.begin(),it)));
        }
        repaint(gridBounds);
    }
    void paint(juce::Graphics&g) override
    {
        g.fillAll(canvas);const auto w=(float)getWidth();const auto h=(float)getHeight();
        g.setColour(teal.withAlpha(.36f));g.drawRect(getLocalBounds().toFloat(),1.f);
        g.setColour(teal.withAlpha(.085f));g.fillRect(1.f,1.f,w-2.f,70.f);
        label(g,"THE PRESET ATLAS",{23,12,w-46,37},29.f,true,true);
        label(g,"64 FACTORY TIMBRES  |  8 MATERIALS x 8 INSTRUMENT ROLES",{23,49,w-46,19},12.f,false,true);
        const int x=20;const auto ww=w-40.f;
        label(g,"NAME / ALL CATEGORIES",{(float)x,88,ww*.42f,17},12.f,false,true);
        label(g,"MATERIAL FAMILY",{x+ww*.44f,88,ww*.24f,17},12.f,false,true);
        label(g,"INSTRUMENT ROLE",{x+ww*.7f,88,ww*.29f,17},12.f,false,true);
        label(g,"SOUND CHARACTER",{(float)x,151,ww*.23f,17},11.f,false,true);
        label(g,"MOVEMENT",{x+ww*.245f,151,ww*.225f,17},11.f,false,true);
        label(g,"SUGGESTED CONTEXT",{x+ww*.49f,151,ww*.25f,17},11.f,false,true);
        label(g,"SORT BY",{x+ww*.755f,151,ww*.24f,17},11.f,false,true);
        const auto count=juce::String(rows.size())+" / 64 MATCHES";
        label(g,count,{x,205,ww*.38f,18},12.f,false,true);
        label(g,"Select a timbre to preview its categories; double-click to load",{x+ww*.32f,205,ww*.66f,18},10.5f,false,false,
              juce::Justification::centredRight,muted);
        if(!matrixMode)
        {
            const float y=253.f;
            auto heading=[&](const juce::String&s,float a,float width)
            {label(g,s,{x+ww*a,y,ww*width,18},10.8f,false,true,juce::Justification::centredLeft,muted);};
            heading("#",.0f,.045f);heading("NAME",.058f,.235f);heading("FAMILY",.31f,.125f);
            heading("ROLE",.447f,.117f);heading("CHARACTER",.574f,.124f);
            heading("MOTION",.709f,.117f);heading("CONTEXT",.839f,.15f);
        }
        else if(!gridBounds.isEmpty())
        {
            const int labelW=juce::jmax(108,gridBounds.getWidth()/8);
            const int headH=29;
            const float cw=(float)(gridBounds.getWidth()-labelW)/8.f;
            const float ch=(float)(gridBounds.getHeight()-headH)/8.f;
            g.setColour(teal.withAlpha(.13f));g.fillRoundedRectangle(gridBounds.toFloat(),4.f);
            for(int col=0;col<8;col++)
                label(g,albedo_preset_taxonomy::roles[(size_t)col],
                      {float(gridBounds.getX()+labelW)+col*cw,float(gridBounds.getY()),cw,float(headH)},
                      10.f,false,true,juce::Justification::centred,muted);
            for(int row=0;row<8;row++)
            {
                const float top=gridBounds.getY()+headH+row*ch;
                label(g,albedo_preset_taxonomy::families[(size_t)row],
                      {(float)gridBounds.getX()+4,top,(float)labelW-6,ch},
                      12.f,true,true);
                for(int col=0;col<8;col++)
                {
                    const int id=row*8+col;
                    const bool inFilter=std::find(rows.begin(),rows.end(),id)!=rows.end();
                    const auto cell=juce::Rectangle<float>(gridBounds.getX()+labelW+col*cw,top,cw,ch).reduced(2.f);
                    auto bg=(id==selectedIndex?gold.withAlpha(.29f):
                                    colours[(size_t)(row%4)].withAlpha(inFilter?.11f:.035f));
                    g.setColour(bg);g.fillRoundedRectangle(cell,3.f);
                    g.setColour(inFilter?teal.withAlpha(.32f):muted.withAlpha(.12f));
                    g.drawRoundedRectangle(cell,3.f,id==selectedIndex?1.7f:.7f);
                    label(g,timbreName(id),cell.reduced(4,2),10.8f,true,id==selectedIndex,
                          juce::Justification::centred,inFilter?text:muted.withAlpha(.4f));
                }
            }
        }
        const auto& m=meta(juce::jlimit(0,63,selectedIndex));
        auto info=selectedIndex<0?juce::String("No matches"):
            juce::String(m.name)+"  |  "+m.character+" / "+m.motion+"  |  "+m.description;
        label(g,info,{24,h-90,w-48,27},12.f,true,false);
        label(g,"Context is a suggested use, not a restriction; factory data remain unchanged.",
              {24,h-63,w-410,17},10.6f,false,false,juce::Justification::centredLeft,muted);
    }
    void resized() override
    {
        const auto b=getLocalBounds();const int x=20,w=b.getWidth()-40;const int gap=8;
        // Two full-width selection rows, with controls in their own non-overlapping cells.
        const int a=int(w*.44f),c=int(w*.26f),d=w-a-c;
        search.setBounds(x,106,a-gap,33);
        family.setBounds(x+a,106,c-gap,33);
        role.setBounds(x+a+c,106,d,33);
        const int charW=int(w*.245f),motW=int(w*.245f),ctxW=int(w*.265f);
        character.setBounds(x,169,charW-gap,33);
        motion.setBounds(x+charW,169,motW-gap,33);
        context.setBounds(x+charW+motW,169,ctxW-gap,33);
        sort.setBounds(x+charW+motW+ctxW,169,w-charW-motW-ctxW,33);
        listView.setBounds(x,228,86,27);matrixView.setBounds(x+94,228,137,27);
        clear.setBounds(x+w-102,228,102,27);
        // Views share the exact same rectangle, without allocating a second UI tree.
        gridBounds=juce::Rectangle<int>(x,276,w,juce::jmax(75,b.getHeight()-390));
        list.setBounds(gridBounds);
        list.setVisible(!matrixMode);
        load.setBounds(x+w-145,b.getBottom()-56,145,37);
        close.setBounds(x+w-289,b.getBottom()-56,136,37);
    }
    void mouseUp(const juce::MouseEvent& e) override
    {
        if(!matrixMode||!gridBounds.contains(e.getPosition()))return;
        const int labelW=juce::jmax(108,gridBounds.getWidth()/8),headH=29;
        const int dx=e.x-gridBounds.getX()-labelW,dy=e.y-gridBounds.getY()-headH;
        if(dx<0||dy<0)return;
        const int col=(dx*8)/juce::jmax(1,gridBounds.getWidth()-labelW);
        const int row=(dy*8)/juce::jmax(1,gridBounds.getHeight()-headH);
        if(row<0||row>=8||col<0||col>=8)return;
        const int id=row*8+col;
        if(std::find(rows.begin(),rows.end(),id)==rows.end())return;
        selectedIndex=id;
        repaint(gridBounds);
        repaint(juce::Rectangle<int>(20,getHeight()-96,getWidth()-40,40));
    }
    void mouseDoubleClick(const juce::MouseEvent& e) override
    {if(matrixMode&&gridBounds.contains(e.getPosition())){mouseUp(e);loadSelected();}}
};

AlbedoEditor::ParamControl* AlbedoEditor::makeControl(uint32_t id,bool dial,bool showLabel)
{
    if(!processor.parameter(id))return nullptr;
    auto* control=controls.add(new ParamControl(processor,id,dial,showLabel));
    addAndMakeVisible(control);return control;
}
void AlbedoEditor::setupModules()
{
    const std::pair<juce::String,juce::String> names[] = {
      {"Prima Materia","FOUR PRIMARY SOUND SOURCES"},
      {"Source Inspector","CHARACTER / REFINEMENT / SOURCE FILTER"},
      {"Arbor Resonantia","RECIPROCAL PATHS"},
      {"Corpus & Character","SPECTRAL SHAPING / ENVELOPE"},
      {"Correspondentiae","SYMBOLIC MODULATION"},
      {"Motus & Cellular","ORGANIC MODULATION"},
      {"Spatium & Transform","POST-SYNTHESIS PROCESSING"},
      {"Pulsus","16-STEP AMPLITUDE RHYTHM"}
    };
    for(int i=0;i<8;i++) {auto*m=modules.add(new Module(i,names[i].first,names[i].second));addAndMakeVisible(m);}
    for(int i=0;i<4;i++)
    {
        auto* b=engineButtons.add(new juce::TextButton(juce::String::charToString("ABCD"[i])));
        addAndMakeVisible(b);
        b->onClick=[this,i]
        {
            const auto id=1001u+(uint32_t)i*32;
            if(auto* p=processor.parameter(id))p->setValueNotifyingHost(p->get()>.5f?0.f:1.f);
        };
        if(auto* c=makeControl(1002u+(uint32_t)i*32,false,false))sourceRackControls.push_back(c);
    }
    for(int j=0;j<4;j++)if(auto*c=makeControl(1006u+(uint32_t)j))inspectorControls.push_back(c);
    for(int j=0;j<3;j++)if(auto*c=makeControl(1002u+(uint32_t)j))inspectorControls.push_back(c);
    for(int i=0;i<7;i++)if(auto*c=makeControl(2000u+(uint32_t)i))macroControls.push_back(c);
    for(int i=0;i<6;i++)if(auto*c=makeControl(2200u+(uint32_t)i,false))couplingControls.push_back(c);
    for(int i=0;i<6;i++)if(auto*c=makeControl(3000u+(uint32_t)i))filterControls.push_back(c);
    for(int i=0;i<4;i++)if(auto*c=makeControl(3100u+(uint32_t)i))envelopeControls.push_back(c);
    for(int i=0;i<4;i++)if(auto*c=makeControl(4000u+(uint32_t)i))cellularControls.push_back(c);
    for(int i=0;i<4;i++)if(auto*c=makeControl(5000u+(uint32_t)i*2,false))fxControls.push_back(c);
    for(int i=0;i<4;i++)if(auto*c=makeControl(8002u+(uint32_t)i*3,false))routingControls.push_back(c);
    for(int i=0;i<16;i++)if(auto*c=makeControl(6000u+(uint32_t)i,false,false))stepControls.push_back(c);
}
void AlbedoEditor::layoutControls(const std::vector<ParamControl*>& items,juce::Rectangle<int>b,int cols)
{
    if(items.empty()||cols<1)return;
    const int rows=(int(items.size())+cols-1)/cols;
    for(size_t i=0;i<items.size();i++)
    {
        const int col=int(i)%cols,row=int(i)/cols;
        const int x=b.getX()+col*b.getWidth()/cols;
        const int y=b.getY()+row*b.getHeight()/rows;
        items[i]->setBounds(x+2,y+2,juce::jmax(20,b.getWidth()/cols-4),juce::jmax(20,b.getHeight()/rows-4));
    }
}
void AlbedoEditor::drawSection(juce::Graphics&g,juce::Rectangle<float>b,const juce::String&title,int index)
{
    panel(g,b);
    g.setColour(colours[(size_t)(index%4)]);g.fillRect(b.getX()+7.f,b.getY()+8.f,3.f,24.f);
    label(g,title,{b.getX()+15,b.getY()+4,b.getWidth()-28,30},18.f,true,true);
}
AlbedoEditor::AlbedoEditor(AlbedoProcessor& p):juce::AudioProcessorEditor(&p),processor(p)
{
    setOpaque(true);
    theme=std::make_unique<AlbedoLookAndFeel>();setLookAndFeel(theme.get());
    std::unique_ptr<juce::XmlElement> xml (juce::XmlDocument::parse(juce::String::fromUTF8(BinaryData::albedo_logo_svg, BinaryData::albedo_logo_svgSize)));
    if(xml)logo=juce::Drawable::createFromSVG(*xml);
    addAndMakeVisible(atlasButton);addAndMakeVisible(allControlsButton);
    atlasButton.onClick=[this]{atlas->setVisible(true);atlas->toFront(true);atlas->resized();};
    allControlsButton.onClick=[this]{setAdvancedVisible(true);};
    setupModules();
    arbor=std::make_unique<ArborView>(processor);addAndMakeVisible(*arbor);
    // All CP22 graph modules animate natively; none is a texture-overlay or WebView.
    auto graph=[this](DynamicGraph::Kind kind,int i=0)
    {auto* v=graphs.add(new DynamicGraph(processor,kind,i));addAndMakeVisible(v);return v;};
    for(int i=0;i<4;i++)enginePlots[i]=graph(DynamicGraph::engineWave,i);
    inspectorPlot=graph(DynamicGraph::filterResponse,2);
    for(int i=0;i<2;i++)filterPlots[i]=graph(DynamicGraph::filterResponse,i);
    envelopePlot=graph(DynamicGraph::envelope);
    cellularPlot=graph(DynamicGraph::cellular);
    effectsPlot=graph(DynamicGraph::effects);
    pulsePlot=graph(DynamicGraph::pulse);
    for(int i=0;i<4;i++)sourceSelector.addItem("SOURCE " + juce::String::charToString("ABCD"[i]),i+1);
    sourceSelector.setSelectedId(1);sourceSelector.onChange=[this]
    {
        activeSource=sourceSelector.getSelectedId()-1;
        if(activeSource<0||activeSource>3)return;
        const auto base=1000u+(uint32_t)activeSource*32;
        for(int j=0;j<4;j++)inspectorControls[(size_t)j]->bind(base+6u+(uint32_t)j);
        for(int j=0;j<3;j++)inspectorControls[(size_t)j+4]->bind(base+2u+(uint32_t)j);
    };
    addAndMakeVisible(sourceSelector);
    atlas=std::make_unique<Atlas>(processor);addAndMakeVisible(*atlas);
    atlas->dismiss=[this]{atlas->setVisible(false);};atlas->setVisible(false);
    addChildComponent(advancedPanel);
    advancedPanel.addAndMakeVisible(advancedBackground);
    advancedBackground.setColour(juce::Label::backgroundColourId,canvas);
    advancedBackground.setInterceptsMouseClicks(false,false);
    advancedPanel.addAndMakeVisible(closeAdvanced);
    closeAdvanced.onClick=[this]{setAdvancedVisible(false);};
    advancedPanel.addAndMakeVisible(advancedViewport);
    advancedViewport.setViewedComponent(&advancedContent,false);
    for(const auto& d:processor.definitions())
    {
        auto*ctrl=advancedOwned.add(new ParamControl(processor,d.id,false,true));
        advancedContent.addAndMakeVisible(ctrl);
        advancedControls.push_back(ctrl);
    }
    constrainer.setFixedAspectRatio(1.5);
    constrainer.setSizeLimits(1050,700,1800,1200);
    setResizable(true,true);setConstrainer(&constrainer);
    setSize(1110,740);
    startTimerHz(24); // animated regions 24 Hz; host parameter sync 12 Hz
}
AlbedoEditor::~AlbedoEditor(){stopTimer();setLookAndFeel(nullptr);}
void AlbedoEditor::setAdvancedVisible(bool visible)
{
    showAdvanced=visible;advancedPanel.setVisible(visible);
    if(visible)advancedPanel.toFront(true);
    resized();
}
void AlbedoEditor::timerCallback()
{
    if((++animationTick&1)==0)
    {
        for(auto*c:controls)c->sync();
        for(auto*c:advancedControls)c->sync();
        if(atlas && atlas->isVisible())atlas->syncHostSelection();
    }
    for(auto* graph:graphs)graph->advance(1.f/24.f);
    // The Atlas is static until text, selection or a host preset actually changes.

}
void AlbedoEditor::paint(juce::Graphics&g)
{
    g.fillAll(canvas);
    auto bounds=getLocalBounds().toFloat();
    g.setColour(surface);g.fillRect(0,0,getWidth(),74);
    g.setColour(teal.withAlpha(.25f));g.fillRect(0,72,getWidth(),1);
    if(logo)logo->drawWithin(g,juce::Rectangle<float>(16,12,48,48),juce::RectanglePlacement::centred,1.f);
    label(g,"ALBEDO",{76,13,240,29},29.f,true,true);
    label(g,"QUINTESSENTIAL SYNTH  ·  NATIVE TONAL ARCHITECTURE",{77,46,450,17},10,false,true,juce::Justification::centredLeft,muted);
    label(g,"JUCE 8 · DIRECT2D · 3:2",{bounds.getWidth()*.68f,33,bounds.getWidth()*.21f,20},12,false,true,juce::Justification::centredRight,muted);
    label(g,"HI-FI  ·  64 FACTORY PATCHES  ·  12-NOTE KEYBOARD POLYPHONY",{16,bounds.getBottom()-22,bounds.getWidth()-32,16},10,false,true,juce::Justification::centredLeft,muted);
    if(showAdvanced)
    {
        auto x=advancedPanel.getBounds().toFloat();
        g.setColour(teal.withAlpha(.32f));g.fillRect(bounds);
        panel(g,x);
    }
}
void AlbedoEditor::resized()
{
    const auto W=getWidth(),H=getHeight();
    atlasButton.setBounds(W-150,20,130,36);
    allControlsButton.setBounds(W-295,20,133,36);
    auto r=juce::Rectangle<int>(10,78,juce::jmax(0,W-20),juce::jmax(0,H-102));
    const int gap=8;
    const int engineH=juce::jlimit(100,135,(int)(r.getHeight()*.16f));
    const int lowerH=juce::jlimit(155,275,(int)(r.getHeight()*.27f));
    auto engine=r.removeFromTop(engineH);r.removeFromTop(gap);
    auto lower=r.removeFromBottom(lowerH);r.removeFromBottom(gap);
    auto upper=r;
    const int leftW=(int)(upper.getWidth()*.25f),rightW=(int)(upper.getWidth()*.25f);
    auto inspector=upper.removeFromLeft(leftW);upper.removeFromLeft(gap);
    auto body=upper.removeFromRight(rightW);upper.removeFromRight(gap);
    auto centre=upper;
    moduleRects={engine,inspector,centre,body};
    const int lowerCols[4]={int(lower.getWidth()*.29f),int(lower.getWidth()*.22f),int(lower.getWidth()*.26f),0};
    for(int i=0;i<4;i++)
    {
        auto b=i<3?lower.removeFromLeft(lowerCols[i]):lower;
        if(i<3)lower.removeFromLeft(gap);
        moduleRects.push_back(b);
    }
    for(size_t i=0;i<modules.size();i++)modules[(int)i]->setBounds(moduleRects[i]);
    // Rack: four engines with separate full-width clickable activation surfaces.
    auto rack=moduleRects[0].reduced(8).withTrimmedTop(37);
    const int cellW=rack.getWidth()/4;
    for(int i=0;i<4;i++)
    {
        auto part=rack.withX(rack.getX()+cellW*i).withWidth(cellW).reduced(3);
        const auto buttonW=juce::jmax(40,int(cellW*.20f));
        engineButtons[i]->setBounds(part.removeFromLeft(buttonW).withHeight(42));
        auto visual=part.reduced(5,1);
        const int graphHeight=juce::jlimit(24,32,visual.getHeight()/2);
        enginePlots[i]->setBounds(visual.withHeight(graphHeight));
        sourceRackControls[(size_t)i]->setBounds(visual.withTrimmedTop(graphHeight+3)
            .withHeight(juce::jmax(18,visual.getHeight()-graphHeight-3)));
    }
    auto ins=moduleRects[1].reduced(8).withTrimmedTop(54);
    sourceSelector.setBounds(ins.getX(),ins.getY(),ins.getWidth(),30);ins.removeFromTop(38);
    inspectorPlot->setBounds(ins.removeFromTop(54).reduced(4,1));ins.removeFromTop(7);
    layoutControls(inspectorControls,ins.withTrimmedBottom(12),4);
    auto central=moduleRects[2].reduced(9).withTrimmedTop(37);
    const int arborH=(int)(central.getHeight()*.46f);
    arbor->setBounds(central.removeFromTop(arborH));central.removeFromTop(6);
    const int macroH=(int)(central.getHeight()*.52f);
    layoutControls(macroControls,central.removeFromTop(macroH),7);
    layoutControls(couplingControls,central,3);
    auto filt=moduleRects[3].reduced(8).withTrimmedTop(45);
    const auto filterSectionH=juce::jlimit(76,126,int(filt.getHeight()*.27f));
    for(int i=0;i<2;i++)
    {
        auto part=filt.removeFromTop(filterSectionH);
        const int graphH=juce::jlimit(30,46,part.getHeight()/2);
        filterPlots[i]->setBounds(part.removeFromTop(graphH).reduced(3,1));
        layoutControls(std::vector<ParamControl*>{filterControls[(size_t)i*3],filterControls[(size_t)i*3+1],filterControls[(size_t)i*3+2]},part,3);
        filt.removeFromTop(3);
    }
    envelopePlot->setBounds(filt.removeFromTop(juce::jlimit(31,46,filt.getHeight()/3)).reduced(3,1));
    layoutControls(envelopeControls,filt,4);
    const auto lowerContent=[](juce::Rectangle<int>b){return b.reduced(8).withTrimmedTop(47);};
    layoutControls(routingControls,lowerContent(moduleRects[4]),1);
    auto cellular=lowerContent(moduleRects[5]);
    cellularPlot->setBounds(cellular.removeFromTop(juce::jmin(72,juce::jmax(40,cellular.getHeight()/2))).reduced(2,2));
    cellular.removeFromTop(3);
    layoutControls(cellularControls,cellular,4);
    auto effects=lowerContent(moduleRects[6]);
    effectsPlot->setBounds(effects.removeFromTop(juce::jmin(46,juce::jmax(30,effects.getHeight()/3))).reduced(2,1));
    effects.removeFromTop(4);
    layoutControls(fxControls,effects,2);
    auto pulse=lowerContent(moduleRects[7]);
    pulsePlot->setBounds(pulse.removeFromTop(juce::jmin(58,juce::jmax(35,pulse.getHeight()/2))).reduced(2,2));
    pulse.removeFromTop(3);
    layoutControls(stepControls,pulse,8);
    // Constrain floating Atlas overlay to the editor with generous source typography.
    atlas->setBounds(getLocalBounds().reduced(juce::jmax(18,W/35),juce::jmax(18,H/28)));
    const auto advanced=getLocalBounds().reduced(W/14,H/12);
    advancedPanel.setBounds(advanced);
    advancedBackground.setBounds(0,0,advanced.getWidth(),advanced.getHeight());
    closeAdvanced.setBounds(advanced.getWidth()-125,9,115,34);
    advancedViewport.setBounds(12,48,advanced.getWidth()-24,advanced.getHeight()-60);
    const int width=advancedViewport.getWidth()-24;
    const int cell=juce::jmax(280,width/3);
    const int cols=juce::jmax(1,width/cell);
    const int rowH=41;
    const int rowCount=(int(advancedControls.size())+cols-1)/cols;
    advancedContent.setSize(width,rowCount*rowH+10);
    layoutControls(advancedControls,advancedContent.getLocalBounds(),cols);
}
