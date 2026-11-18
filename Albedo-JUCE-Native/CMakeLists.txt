cmake_minimum_required(VERSION 3.24)
project(AlbedoJUCE VERSION 1.0.32 LANGUAGES C CXX RC)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Renderer identity: JUCE 8 native C++ graphics. No WebBrowserComponent, WebView2 or HTML runtime.
# This is a distinct plugin identity; do not reuse the CP33/CP34 WebView2 identifiers.
if(NOT DEFINED ALBEDO_DSP_ROOT)
  set(ALBEDO_DSP_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../Albedo-Quintessential-Synth")
endif()
if(NOT EXISTS "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp")
  message(FATAL_ERROR "The verified CP28/CP31 native DSP source must be supplied via ALBEDO_DSP_ROOT")
endif()

if(NOT TARGET juce::juce_audio_processors)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext/JUCE/CMakeLists.txt")
    add_subdirectory(ext/JUCE)
  else()
    include(FetchContent)
    FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.17-security GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(JUCE)
  endif()
endif()

juce_add_plugin(AlbedoNative
  COMPANY_NAME "Albedo"
  BUNDLE_ID "org.vitriol.albedo.juce.native.noview2"
  PLUGIN_MANUFACTURER_CODE Albd
  PLUGIN_CODE AJNW
  PRODUCT_NAME "Albedo Quintessential Synth - JUCE Native (No WebView2)"
  FORMATS VST3
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  PRODUCES_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Instrument Synth
)
juce_generate_juce_header(AlbedoNative)
juce_add_binary_data(AlbedoArtwork SOURCES Assets/albedo_logo.svg)

target_sources(AlbedoNative PRIVATE
  Source/AlbedoProcessor.cpp
  Source/AlbedoEditor.cpp
  "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp"
)
target_include_directories(AlbedoNative PRIVATE "${ALBEDO_DSP_ROOT}/include")
target_link_libraries(AlbedoNative PRIVATE
  AlbedoArtwork
  juce::juce_audio_processors
  juce::juce_gui_basics
  juce::juce_dsp
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)
target_compile_definitions(AlbedoNative PUBLIC JUCE_WEB_BROWSER=0 ALBEDO_GUI_NATIVE_JUCE=1 ALBEDO_GUI_WEBVIEW2=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)

# Keep all 32 original materials. The MIDI engine loads this exact bank from
# Windows resource 102 as it does in the existing non-JUCE Windows plugins.
set(ALBEDO_PCM_BANK "${ALBEDO_DSP_ROOT}/resources/Albedo_Materia32_PCM24_96k.vtrb")
if(WIN32)
  if(NOT EXISTS "${ALBEDO_PCM_BANK}")
    message(FATAL_ERROR "Original 32-material PCM bank is absent")
  endif()
  file(TO_CMAKE_PATH "${ALBEDO_PCM_BANK}" ALBEDO_BANK_RESOURCE_PATH)
  configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Assets/albedo_bank.rc.in"
                 "${CMAKE_CURRENT_BINARY_DIR}/albedo_bank.rc" @ONLY)
  target_sources(AlbedoNative PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/albedo_bank.rc")
endif()

option(ALBEDO_BUILD_CLAP "Build CLAP using unofficial clap-juce-extensions" ON)
if(ALBEDO_BUILD_CLAP)
  if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/ext/clap-juce-extensions/CMakeLists.txt")
    add_subdirectory(ext/clap-juce-extensions EXCLUDE_FROM_ALL)
    clap_juce_extensions_plugin(TARGET AlbedoNative
      CLAP_ID "org.vitriol.albedo.juce.native.noview2"
      CLAP_FEATURES instrument synthesizer stereo)
  else()
    message(FATAL_ERROR "CLAP was requested but clap-juce-extensions is missing; no partial CLAP+VST3 release")
  endif()
endif()

# Complete factory-taxonomy contract: standalone C++20, independent of JUCE/UI.
include(CTest)
if(BUILD_TESTING)
  add_executable(AlbedoPresetTaxonomyTest tests/preset_taxonomy_compile.cpp)
  target_compile_features(AlbedoPresetTaxonomyTest PRIVATE cxx_std_20)
  target_include_directories(AlbedoPresetTaxonomyTest PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${ALBEDO_DSP_ROOT}/include")
  add_test(NAME cp32_factory_atlas_64_complete COMMAND AlbedoPresetTaxonomyTest)
endif()
