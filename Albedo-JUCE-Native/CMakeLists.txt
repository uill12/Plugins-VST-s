cmake_minimum_required(VERSION 3.24)
project(AlbedoJUCE VERSION 1.0.51 LANGUAGES C CXX RC)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CP41 uses JUCE 8 WebBrowserComponent/WebView2 to render the canonical Albedo HTML.
# Keep stable plugin identifiers for host/session compatibility; version differentiates build.
# JUCE-native editor remains compiled as a full fallback.
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
  NEEDS_WEBVIEW2 TRUE
  COMPANY_NAME "Albedo"
  BUNDLE_ID "org.vitriol.albedo.juce32"
  PLUGIN_MANUFACTURER_CODE Albd
  PLUGIN_CODE AQ32
  PRODUCT_NAME "Albedo Quintessential Synth JUCE"
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
juce_add_binary_data(AlbedoArtwork SOURCES
  Assets/albedo_logo.svg
  Assets/juce_cp33_bridge.js
  Assets/cp35_editor_geometry.js
  Assets/cp38_rotary_overlays.js
  Assets/cp52_nereida_browser.js
  Metadata/factory_preset_taxonomy.json
  "${CMAKE_CURRENT_SOURCE_DIR}/../ui/Albedo_CP30_CP22_Landscape_3x2.html"
)

target_sources(AlbedoNative PRIVATE
  Source/AlbedoProcessor.cpp
  Source/AlbedoEditor.cpp
  Source/AlbedoWebViewEditor.cpp
  "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp"
)
target_include_directories(AlbedoNative PRIVATE "${ALBEDO_DSP_ROOT}/include")
target_link_libraries(AlbedoNative PRIVATE
  AlbedoArtwork
  juce::juce_audio_processors
  juce::juce_gui_basics
  juce::juce_gui_extra
  juce::juce_dsp
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags
)
target_compile_definitions(AlbedoNative PUBLIC JUCE_WEB_BROWSER=1 JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
# Use the JUCE FindWebView2.cmake target directly: its include/library paths are
# guaranteed to be identical and it correctly propagates to SharedCode.
if(WIN32)
  if(NOT TARGET juce::juce_webview2)
    message(FATAL_ERROR "CP33 JUCE did not discover the WebView2 NuGet SDK")
  endif()
  target_link_libraries(AlbedoNative PRIVATE juce::juce_webview2)
endif()

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
      CLAP_ID "org.vitriol.opussonorum.juce32"
      CLAP_FEATURES instrument synthesizer stereo)
  else()
    message(WARNING "CLAP extension checkout missing; build only VST3 until it is present")
  endif()
endif()

# Complete factory-taxonomy contract: standalone C++20, independent of JUCE/UI.
include(CTest)
if(BUILD_TESTING)
  add_executable(AlbedoCP53OutputGuardTest tests/cp53_output_guard.cpp)
  target_include_directories(AlbedoCP53OutputGuardTest PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source")
  target_compile_features(AlbedoCP53OutputGuardTest PRIVATE cxx_std_20)
  add_test(NAME cp53_true_peak_guard_and_float64_integrity COMMAND AlbedoCP53OutputGuardTest)
  add_executable(AlbedoPresetTaxonomyTest tests/preset_taxonomy_compile.cpp)
  target_compile_features(AlbedoPresetTaxonomyTest PRIVATE cxx_std_20)
  target_include_directories(AlbedoPresetTaxonomyTest PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${ALBEDO_DSP_ROOT}/include")
  add_test(NAME cp32_factory_atlas_64_complete COMMAND AlbedoPresetTaxonomyTest)
  file(TO_CMAKE_PATH "${ALBEDO_DSP_ROOT}" ALBEDO_CP36_DSP_ROOT)
  add_executable(AlbedoCP36HifiDspTest
    tests/cp36_hifi_quality.cpp
    "${ALBEDO_CP36_DSP_ROOT}/src/vitriol_dsp.cpp")
  target_compile_features(AlbedoCP36HifiDspTest PRIVATE cxx_std_20)
  target_include_directories(AlbedoCP36HifiDspTest PRIVATE "${ALBEDO_DSP_ROOT}/include")
  add_test(NAME cp36_hifi_double_precision_polyphony COMMAND AlbedoCP36HifiDspTest)
  add_executable(AlbedoCP38CreativeAudioTest
    tests/cp38_recursive_audio_qa.cpp
    "${ALBEDO_CP36_DSP_ROOT}/src/vitriol_dsp.cpp")
  target_compile_features(AlbedoCP38CreativeAudioTest PRIVATE cxx_std_20)
  target_include_directories(AlbedoCP38CreativeAudioTest PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Source"
    "${ALBEDO_CP36_DSP_ROOT}/include")
  add_executable(AlbedoRearFieldTest tests/rear_field.cpp)
  target_compile_features(AlbedoRearFieldTest PRIVATE cxx_std_20)
  add_test(NAME cp45_quad_rear_impulse_and_block_continuity COMMAND AlbedoRearFieldTest)
  add_test(NAME cp45_recursive_forms_64_presets_polyphony COMMAND AlbedoCP38CreativeAudioTest)
endif()

if(WIN32 AND BUILD_TESTING)
  file(GLOB_RECURSE albedoClapHeaders "${CMAKE_CURRENT_SOURCE_DIR}/ext/clap-juce-extensions/*/clap.h")
  list(FILTER albedoClapHeaders INCLUDE REGEX "/include/clap/clap[.]h$")
  list(GET albedoClapHeaders 0 albedoClapHeader)
  get_filename_component(albedoClapDir "${albedoClapHeader}" DIRECTORY)
  add_executable(AlbedoViewportHost tests/cp46_host.cpp)
  target_include_directories(AlbedoViewportHost PRIVATE "${albedoClapDir}/..")
  target_link_libraries(AlbedoViewportHost PRIVATE user32)
endif()


if(BUILD_TESTING AND WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Source/AlbedoPreviousBank.hpp")
  add_executable(AlbedoBankAudition tests/cp48_audition.cpp
    "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp" "${CMAKE_CURRENT_BINARY_DIR}/albedo_bank.rc")
  target_compile_features(AlbedoBankAudition PRIVATE cxx_std_20)
  target_include_directories(AlbedoBankAudition PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source" "${ALBEDO_DSP_ROOT}/include")
endif()

if(BUILD_TESTING AND WIN32)
  add_executable(AlbedoControlResponseTest tests/cp49_response.cpp "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp" "${CMAKE_CURRENT_BINARY_DIR}/albedo_bank.rc")
  target_compile_features(AlbedoControlResponseTest PRIVATE cxx_std_20)
  target_include_directories(AlbedoControlResponseTest PRIVATE "${ALBEDO_DSP_ROOT}/include")
  add_test(NAME cp49_control_audio_response COMMAND AlbedoControlResponseTest)
endif()

if(BUILD_TESTING AND WIN32)
  add_executable(AlbedoShapeAudition tests/cp49_shape_audition.cpp
    "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp" "${CMAKE_CURRENT_BINARY_DIR}/albedo_bank.rc")
  target_compile_features(AlbedoShapeAudition PRIVATE cxx_std_20)
  target_include_directories(AlbedoShapeAudition PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source" "${ALBEDO_DSP_ROOT}/include")
endif()

if(BUILD_TESTING AND WIN32)
  add_executable(AlbedoArpTest ../patches/cp50/arp_test.cpp "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp")
  target_compile_features(AlbedoArpTest PRIVATE cxx_std_20)
  target_include_directories(AlbedoArpTest PRIVATE "${ALBEDO_DSP_ROOT}/include")
  add_test(NAME cp50_arpeggiator COMMAND AlbedoArpTest)
endif()

if(BUILD_TESTING AND WIN32)
  add_executable(AlbedoTargetRenderer tools/target/render_target.cpp
    "${ALBEDO_DSP_ROOT}/src/vitriol_dsp.cpp" "${CMAKE_CURRENT_BINARY_DIR}/albedo_bank.rc")
  target_compile_features(AlbedoTargetRenderer PRIVATE cxx_std_20)
  target_include_directories(AlbedoTargetRenderer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source" "${ALBEDO_DSP_ROOT}/include")
endif()
