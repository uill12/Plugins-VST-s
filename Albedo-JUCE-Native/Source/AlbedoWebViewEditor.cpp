#include "AlbedoWebViewEditor.h"
#include "AlbedoWindowFit.h"
#include <BinaryData.h>
#include <cmath>
#include <cstring>
#include <limits>
#if JUCE_WINDOWS
 #ifndef NOMINMAX
  #define NOMINMAX
 #endif
 #include <windows.h>
#endif

namespace
{
juce::File profileFolder()
{
    const auto p = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Albedo").getChildFile("CP33-JUCE-WebView2");
    p.createDirectory();
    return p;
}
std::vector<std::byte> toBytes(const char* data, int n)
{
    if (data == nullptr || n <= 0) return {};
    std::vector<std::byte> bytes((size_t) n);
    std::memcpy(bytes.data(), data, (size_t) n);
    return bytes;
}
juce::WebBrowserComponent::Resource htmlResource(const juce::String& html)
{
    auto utf8 = html.toRawUTF8();
    return { toBytes(utf8, (int) std::strlen(utf8)), "text/html" };
}
}

AlbedoWebViewEditor::Browser::Browser(AlbedoWebViewEditor& o, const Options& opts)
    : juce::WebBrowserComponent(opts), owner(o) {}
bool AlbedoWebViewEditor::Browser::pageAboutToLoad(const juce::String& url)
{
    // The DAW editor loads only the embedded, trusted factory interface.
    return url.startsWith(juce::WebBrowserComponent::getResourceProviderRoot())
        || url == "about:blank";
}
void AlbedoWebViewEditor::Browser::pageFinishedLoading(const juce::String&)
{
    owner.onBrowserPageLoaded();
}
bool AlbedoWebViewEditor::Browser::pageLoadHadNetworkError(const juce::String& reason)
{
    owner.activateFallback(reason);
    return false;
}

juce::WebBrowserComponent::Options AlbedoWebViewEditor::makeOptions(AlbedoWebViewEditor* owner)
{
    using Options = juce::WebBrowserComponent::Options;
    return Options{}
        .withBackend(Options::Backend::webview2)
        .withWinWebView2Options(Options::WinWebView2{}.withUserDataFolder(profileFolder()))
        .withKeepPageLoadedWhenBrowserIsHidden()
        .withNativeIntegrationEnabled()
        .withEventListener("albedoMessage", [owner](juce::var message)
        {
            if (owner != nullptr) owner->receive(message);
        })
        .withResourceProvider([owner](const juce::String& path)
            -> std::optional<juce::WebBrowserComponent::Resource>
        {
            if (owner != nullptr) return owner->resource(path);
            return std::nullopt;
        });
}

bool AlbedoWebViewEditor::isAvailable()
{
    return juce::WebBrowserComponent::areOptionsSupported(makeOptions(nullptr));
}

AlbedoWebViewEditor::AlbedoWebViewEditor(AlbedoProcessor& p)
 : juce::AudioProcessorEditor(&p), processor(p), browser(*this, makeOptions(this))
{
    lastValues.assign(processor.definitions().size(), std::numeric_limits<float>::quiet_NaN());
    lastProgram=processor.getCurrentProgram();
    addAndMakeVisible(browser);
    const auto fit=albedo_window_fit::calculate();
    aspect.setFixedAspectRatio(1.5);
    aspect.setSizeLimits(fit.minWidth,fit.minHeight,fit.maxWidth,fit.maxHeight);
    // WebView2 is an HWND drawn in front of JUCE's native corner-resizer.
    // Allow host resize but use the HTML drag-grip for user interaction.
    setResizable(true,false);
    setConstrainer(&aspect);
    setSize(fit.initialWidth,fit.initialHeight);
    browser.goToURL(juce::WebBrowserComponent::getResourceProviderRoot() + juce::String(JucePlugin_VersionString) + "/index.html");
    startTimerHz(12);
}

AlbedoWebViewEditor::~AlbedoWebViewEditor()
{
    stopTimer();
    if(currentGesture!=UINT32_MAX)
        if(auto* p=processor.parameter(currentGesture)) p->endChangeGesture();
    setConstrainer(nullptr);
}

std::optional<juce::WebBrowserComponent::Resource>
AlbedoWebViewEditor::resource(const juce::String& path) const
{
    const auto versionPrefix = "/" + juce::String(JucePlugin_VersionString);
    const auto assetPath = path.startsWith(versionPrefix + "/") ? path.substring(versionPrefix.length()) : path;
    if(assetPath=="/" || assetPath=="/index.html")
    {
        int size=0;
        auto* data=BinaryData::getNamedResource("Albedo_CP30_CP22_Landscape_3x2_html",size);
        if(data==nullptr) return std::nullopt;
        auto html=juce::String::fromUTF8(data,size);
        // Redirect the existing CP31 message bridge into JUCE 8. The complete
        // HTML/JS/SVG/Arbor DOM remains intact: no bitmap overlay or DOM reset.
        const juce::String oldHost="const host=window.chrome && window.chrome.webview;";
        const juce::String juceHost=
            "const host={postMessage:(payload)=>window.__JUCE__?.backend?.emitEvent"
            "('albedoMessage',{payload:String(payload)})};";
        if(!html.contains(oldHost)) return std::nullopt; // fail closed on mismatched HTML
        html=html.replace(oldHost,juceHost);
        html=html.replace("</body>","<script src=\"juce_cp33_bridge.js\"></script><script src=\"cp35_editor_geometry.js\"></script><script src=\"cp38_rotary_overlays.js\"></script><script src=\"cp52_nereida_browser.js\"></script><script src=\"cp54_essential_sound_layout.js\"></script><script src=\"cp56_sound_priority.js\"></script><script src=\"cp57_tempo_controls.js\"></script></body>");
        return htmlResource(html);
    }

    const char* name=nullptr;
    const char* mime=nullptr;
    if(assetPath=="/juce_cp33_bridge.js")
    { name="juce_cp33_bridge_js"; mime="text/javascript"; }
    else if(assetPath=="/cp35_editor_geometry.js")
    { name="cp35_editor_geometry_js"; mime="text/javascript"; }
    else if(assetPath=="/cp38_rotary_overlays.js")
    { name="cp38_rotary_overlays_js"; mime="text/javascript"; }
    else if(assetPath=="/cp52_nereida_browser.js")
    { name="cp52_nereida_browser_js"; mime="text/javascript"; }
    else if(assetPath=="/cp54_essential_sound_layout.js")
    { name="cp54_essential_sound_layout_js"; mime="text/javascript"; }
    else if(assetPath=="/cp56_sound_priority.js")
    { name="cp56_sound_priority_js"; mime="text/javascript"; }
    else if(assetPath=="/cp57_tempo_controls.js")
    { name="cp57_tempo_controls_js"; mime="text/javascript"; }
    else if(assetPath=="/factory_preset_taxonomy.json")
    { name="factory_preset_taxonomy_json"; mime="application/json"; }
    else return std::nullopt;
    int size=0;
    const auto* data=BinaryData::getNamedResource(name,size);
    if(data==nullptr) return std::nullopt;
    return juce::WebBrowserComponent::Resource{toBytes(data,size),mime};
}

void AlbedoWebViewEditor::receive(const juce::var& value)
{
    const auto* obj=value.getDynamicObject();
    if(obj==nullptr) return;
    const auto message=obj->getProperty("payload").toString();
    juce::StringArray parts;
    parts.addTokens(message,"|","");
    if(parts.isEmpty()) return;

    if(parts[0]=="ready")
    {
        pageReady=true;
        lastViewportW=-1;lastViewportH=-1;
        fullSyncPending=true;
        pendingTicks=0;
        return;
    }
    if(parts.size()<2) return;
    if(parts[0]=="resize" && parts.size()==3)
    {
        const auto requestedWidth=(int)parts[1].getIntValue();
        const auto requestedHeight=(int)parts[2].getIntValue();
        if(requestedWidth>=480 && requestedWidth<=1900 &&
           requestedHeight>=320 && requestedHeight<=1400)
            requestHostResize(requestedWidth);
        return;
    }
    if(parts[0]=="action")
    {
        handleAction(parts[1]);
        return;
    }
    const auto id64=parts[1].getLargeIntValue();
    if(parts[0]=="preset")
    {
        if(id64>=0 && id64<processor.getNumPrograms())
        {
            processor.loadFactoryPreset((int)id64);
            fullSyncPending=true;
        }
        return;
    }
    if(id64<0 || id64>100000) return;
    const auto id=(uint32_t)id64;
    auto* parameter=processor.parameter(id);
    if(parameter==nullptr) return;

    if(parts[0]=="begin")
    {
        if(currentGesture!=UINT32_MAX)
            if(auto* old=processor.parameter(currentGesture)) old->endChangeGesture();
        parameter->beginChangeGesture();
        currentGesture=id;
        return;
    }
    if(parts[0]=="end")
    {
        if(currentGesture!=UINT32_MAX)
            if(auto* old=processor.parameter(currentGesture)) old->endChangeGesture();
        currentGesture=UINT32_MAX;
        return;
    }
    if(parts[0]!="param" || parts.size()!=3) return;

    const double proposed=parts[2].getDoubleValue();
    if(!std::isfinite(proposed)) return;
    const auto valueToSet=id==3111u?0.0f:(float)proposed;
    const auto normalized=parameter->convertTo0to1(valueToSet);
    if(currentGesture!=id) parameter->beginChangeGesture();
    parameter->setValueNotifyingHost(normalized);
    if(currentGesture!=id) parameter->endChangeGesture();
}


void AlbedoWebViewEditor::postActionStatus(const juce::String& msg)
{
    if(pageReady && browserHasLoaded && !nativeFallback)
        browser.emitEventIfBrowserIsVisible("albedoActionStatus", juce::var(msg));
}

void AlbedoWebViewEditor::handleAction(const juce::String& action)
{
    if(action=="mutate")
    {
        // Match the original CP22 action: four source morphologies,
        // sixteen source-shape controls, seven macros, and XY form/material.
        // All changes are JUCE host parameters, never visual-only patches.
        juce::Random rng;
        const auto write=[this](uint32_t id,float value)
        {
            if(auto* p=processor.parameter(id))
            {
                const auto scaled=juce::jlimit(p->range.start,p->range.end,value);
                p->beginChangeGesture();
                p->setValueNotifyingHost(p->convertTo0to1(scaled));
                p->endChangeGesture();
            }
        };
        for(uint32_t engine=0;engine<4;++engine)
        {
            const auto base=1000u+32u*engine;
            write(base,static_cast<float>(rng.nextInt(10)));
            for(uint32_t k=6;k<=9;++k)write(base+k,rng.nextFloat()*100.0f);
        }
        for(uint32_t id=2000;id<=2006;++id)write(id,rng.nextFloat()*100.0f);
        write(2100,10.0f+rng.nextFloat()*80.0f);
        write(2101,10.0f+rng.nextFloat()*80.0f);
        fullSyncPending=true; // one coherent host->WebView2 visual update
        postActionStatus("Audio morphology mutated: 4 sources, 7 macros, XY");
        return;
    }
    if(patchDialogOpen || (action!="import" && action!="export"))return;
    patchDialogOpen=true;
    const bool saving=action=="export";
    const auto defaultFile=juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("Albedo_Patch.albedopatch");
    patchChooser=std::make_unique<juce::FileChooser>(
        saving?"Save Albedo audio patch":"Import Albedo audio patch",
        defaultFile,"*.albedopatch",true);
    const int flags=juce::FileBrowserComponent::canSelectFiles
        | (saving ? juce::FileBrowserComponent::saveMode
                  : juce::FileBrowserComponent::openMode);
    juce::Component::SafePointer<AlbedoWebViewEditor> weakThis(this);
    patchChooser->launchAsync(flags,[weakThis,saving](const juce::FileChooser& chooser)
    {
        if(weakThis==nullptr)return;
        weakThis->patchDialogOpen=false;
        auto file=chooser.getResult();
        if(file.getFullPathName().isEmpty())return; // User cancelled
        if(saving)
        {
            if(!file.hasFileExtension(".albedopatch"))
                file=file.withFileExtension(".albedopatch");
            juce::MemoryBlock binary;
            weakThis->processor.getStateInformation(binary);
            const bool ok=binary.getSize()>12 && file.replaceWithData(binary.getData(),binary.getSize());
            weakThis->postActionStatus(ok?"Audio patch saved: "+file.getFileName()
                                         :"Could not save the Albedo audio patch");
            return;
        }
        juce::MemoryBlock binary;
        if(!file.loadFileAsData(binary) || binary.getSize()<12 || binary.getSize()>256*1024)
        {
            weakThis->postActionStatus("Import rejected: missing or oversized patch");
            return;
        }
        juce::MemoryInputStream probe(binary,false);
        if(probe.readInt()!=0x414C4232) // Native JUCE patch state magic: ALB2
        {
            weakThis->postActionStatus("Import rejected: unsupported audio patch format");
            return;
        }
        weakThis->processor.setStateInformation(binary.getData(),(int)binary.getSize());
        weakThis->fullSyncPending=true;
        weakThis->postActionStatus("Audio patch imported: "+file.getFileName());
    });
}

void AlbedoWebViewEditor::requestHostResize(int width)
{
    const auto fit=albedo_window_fit::calculate();
    int clipped=juce::jlimit(fit.minWidth,fit.maxWidth,width);
    clipped=juce::jmax(fit.minWidth,(clipped/6)*6);
    const int height=clipped*2/3;
    if(getWidth()==clipped && getHeight()==height) return;
    // Request on the next JUCE message-loop turn, outside WebView2's native
    // event callback. VST3/CLAP hosts may synchronously negotiate view size.
    juce::Component::SafePointer<AlbedoWebViewEditor> safe(this);
    juce::MessageManager::callAsync([safe,clipped,height]
    {
        if(safe==nullptr)return;
        if(safe->getWidth()!=clipped || safe->getHeight()!=height)
            safe->setSize(clipped,height);
        // Report the result the host actually granted, not the requested size.
        safe->sendViewportSize(true);
    });
}
void AlbedoWebViewEditor::sendViewportSize(bool force)
{
    if(!pageReady || !browserHasLoaded || nativeFallback)return;
    const int w=getWidth(),h=getHeight();
    int physicalW=0,physicalH=0;
#if JUCE_WINDOWS
    if (auto* peer=getPeer())
    {
        // Measure client rectangles in a common, physical screen coordinate space.
        // Walk child parents only; a floating window's owner does not clip it.
        const auto previousDpi=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        RECT visible{};
        bool haveRect=false;
        for (HWND hwnd=static_cast<HWND>(peer->getNativeHandle()); hwnd!=nullptr; hwnd=GetParent(hwnd))
        {
            RECT client{};
            if (!GetClientRect(hwnd,&client)) break;
            MapWindowPoints(hwnd,nullptr,reinterpret_cast<POINT*>(&client),2);
            if (!haveRect) { visible=client; haveRect=true; }
            else if (!IntersectRect(&visible,&visible,&client)) break;
            if ((GetWindowLongPtrW(hwnd,GWL_STYLE)&WS_CHILD)==0) break;
        }
        if (previousDpi!=nullptr) SetThreadDpiAwarenessContext(previousDpi);
        if (haveRect) { physicalW=int(visible.right-visible.left); physicalH=int(visible.bottom-visible.top); }
    }
#endif
    if(!force && w==lastViewportW && h==lastViewportH && physicalW==lastPhysicalW && physicalH==lastPhysicalH)return;
    lastPhysicalW=physicalW;lastPhysicalH=physicalH;
    lastViewportW=w;lastViewportH=h;
    browser.emitEventIfBrowserIsVisible("albedoViewport",
        juce::var(juce::String(w)+","+juce::String(h)+","+juce::String(physicalW)+","+juce::String(physicalH)));
}

void AlbedoWebViewEditor::onBrowserPageLoaded()
{
    browserHasLoaded=true;
    pendingTicks=0;
    // Wait for the original HTML to send the explicit 'ready' bridge event.
}
void AlbedoWebViewEditor::activateFallback(const juce::String& reason)
{
    if(nativeFallback) return;
    fallbackMessage=reason;
    browser.setVisible(false);
    nativeFallback=std::make_unique<AlbedoEditor>(processor);
    addAndMakeVisible(*nativeFallback);
    resized(); // Fit its proven 1260x840 logical canvas into the current editor.
    nativeFallback->toFront(true);
    pageReady=false;
}
void AlbedoWebViewEditor::sendDelta(bool full)
{
    juce::String csv;
    const auto& defs=processor.definitions();
    for(size_t i=0;i<defs.size();++i)
    {
        const float v=processor.paramValue(defs[i].id);
        if(!std::isfinite(v))continue;
        if(full||!std::isfinite(lastValues[i])||std::abs(v-lastValues[i])>0.00005f)
        {
            if(csv.isNotEmpty()) csv<<",";
            csv<<juce::String((int)defs[i].id)<<":"<<juce::String(v,7);
            lastValues[i]=v;
        }
    }
    if(csv.isNotEmpty())browser.emitEventIfBrowserIsVisible("albedoUpdate",juce::var(csv));
}
void AlbedoWebViewEditor::timerCallback()
{
    if(nativeFallback) return;
    // Never call JUCE WebView emitEvent before pageFinishedLoading: it can
    // freeze DAWs with multiple editor instances.
    if(!pageReady || !browserHasLoaded)
    {
        if(++pendingTicks>120)
            activateFallback("WebView2 startup timed out; reverted to native JUCE editor");
        return;
    }
    sendViewportSize();
    const int step=processor.arpStep();
    if(step!=lastArpStep){lastArpStep=step;browser.emitEventIfBrowserIsVisible("albedoArpStep",juce::var(step));}
    sendDelta(fullSyncPending);
    fullSyncPending=false;
    const int current=processor.getCurrentProgram();
    if(current!=lastProgram)
    {
        lastProgram=current;
        browser.emitEventIfBrowserIsVisible("albedoProgram",juce::var(current));
    }
}
void AlbedoWebViewEditor::resized()
{
    browser.setBounds(getLocalBounds());
    sendViewportSize();
    if(nativeFallback)
    {
        // The original native GUI requires at least 1260x840 for its
        // filter/envelope rows. Scale it uniformly rather than clipping or
        // reflowing its labels when the host window starts at 1152x768.
        constexpr float logicalW=1260.0f,logicalH=840.0f;
        nativeFallback->setTransform(juce::AffineTransform());
        nativeFallback->setBounds(0,0,(int)logicalW,(int)logicalH);
        const auto fit=juce::jmin((float)getWidth()/logicalW,(float)getHeight()/logicalH);
        const auto ox=juce::jmax(0.0f,((float)getWidth()-logicalW*fit)*.5f);
        const auto oy=juce::jmax(0.0f,((float)getHeight()-logicalH*fit)*.5f);
        nativeFallback->setTransform(juce::AffineTransform::scale(fit).translated(ox,oy));
    }
}
void AlbedoWebViewEditor::paint(juce::Graphics& g)
{
    // This background is visible only while the embedded page starts.
    g.fillAll(juce::Colour::fromRGB(232,232,218));
}
