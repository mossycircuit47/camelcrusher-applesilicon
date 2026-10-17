cmake_minimum_required(VERSION 3.22)
if(NOT CMAKE_OSX_ARCHITECTURES)
 set(CMAKE_OSX_ARCHITECTURES arm64 CACHE STRING "Native Apple Silicon" FORCE)
endif()
if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
 set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "macOS deployment target" FORCE)
endif()
project(CamelCrusherRebuilt VERSION 0.1.0 LANGUAGES C CXX)
set(JUCE_ROOT "" CACHE PATH "Path to a JUCE 8.0.12 source checkout")
if(NOT EXISTS "${JUCE_ROOT}/CMakeLists.txt")
 message(FATAL_ERROR "Set JUCE_ROOT to a JUCE 8 source checkout")
endif()
add_subdirectory("${JUCE_ROOT}" "${CMAKE_BINARY_DIR}/JUCE")
juce_add_plugin(CamelCrusherRebuilt
 COMPANY_NAME "Local Reconstruction" BUNDLE_ID "local.camelcrusher.rebuilt"
 PLUGIN_MANUFACTURER_CODE LRec PLUGIN_CODE CCr1
 FORMATS VST3 AU Standalone PRODUCT_NAME "CamelCrusher Rebuilt"
 IS_SYNTH FALSE NEEDS_MIDI_INPUT TRUE NEEDS_MIDI_OUTPUT FALSE
 IS_MIDI_EFFECT FALSE EDITOR_WANTS_KEYBOARD_FOCUS FALSE
 COPY_PLUGIN_AFTER_BUILD FALSE VST3_CATEGORIES Fx Distortion Filter Dynamics
 AU_MAIN_TYPE kAudioUnitType_Effect)
# A standard AU audio effect must not publish MusicDeviceMIDIEvent. Keep MIDI
# input on the shared engine and VST3, but disable it in the AU wrapper only.
target_compile_options(CamelCrusherRebuilt_AU PRIVATE
 -UJucePlugin_WantsMidiInput -DJucePlugin_WantsMidiInput=0)
file(GLOB SKIN_ASSETS CONFIGURE_DEPENDS "Resources/default/*.png" "Resources/default/SkinParameters.txt")
juce_add_binary_data(CamelCrusherAssets SOURCES ${SKIN_ASSETS})
juce_generate_juce_header(CamelCrusherRebuilt)
target_sources(CamelCrusherRebuilt PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_features(CamelCrusherRebuilt PRIVATE cxx_std_17)
target_compile_options(CamelCrusherRebuilt PRIVATE -ffp-contract=off -fno-builtin)
target_compile_definitions(CamelCrusherRebuilt PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0)
target_link_libraries(CamelCrusherRebuilt PRIVATE CamelCrusherAssets juce::juce_audio_utils PUBLIC juce::juce_recommended_config_flags)
