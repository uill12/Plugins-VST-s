#pragma once
// Generated from Metadata/factory_preset_taxonomy.json.
// Curatorial metadata only. Does not alter DSP parameters, factory names or IDs.
// Rebuild with: python tools/generate_preset_taxonomy.py
#include <array>
#include <cstddef>

namespace albedo_preset_taxonomy {
struct PresetCategory {
    int index;
    const char* name;           // Original immutable patch name
    const char* family;         // Factory material family
    const char* role;           // Instrument role
    const char* character;      // Audible/timbral descriptor
    const char* motion;         // Suggested modulation/envelope behavior
    const char* context;        // Suggested musical context (not a restriction)
    const char* description;    // Curatorial description, not a measurement
};

inline constexpr std::array<const char*, 8> families = {{
    "Vitreous",
    "Organic",
    "Metallic",
    "Cellular",
    "Nocturnal",
    "Aerial",
    "Kinetic",
    "Chimeric"
}};

inline constexpr std::array<const char*, 8> roles = {{
    "Bass",
    "Lead",
    "Pad",
    "Pluck",
    "Keys",
    "Percussion",
    "Drone",
    "Texture"
}};

inline constexpr std::array<const char*, 12> characters = {{
    "Airy",
    "Bright",
    "Dark",
    "Deep",
    "Hybrid",
    "Resonant",
    "Rhythmic",
    "Sharp",
    "Shimmering",
    "Soft",
    "Textured",
    "Warm"
}};

inline constexpr std::array<const char*, 9> motions = {{
    "Breathing",
    "Decaying",
    "Evolving",
    "Expressive",
    "Granular",
    "Pulsing",
    "Rhythmic",
    "Steady",
    "Struck"
}};

inline constexpr std::array<const char*, 7> contexts = {{
    "Ambient",
    "Cinematic",
    "Electronica",
    "Experimental",
    "IDM",
    "Lo-Fi",
    "Progressive Techno"
}};

inline constexpr std::array<PresetCategory, 64> entries = {{
    {0, "Vitreous | Glass Pressure (Bass)", "Vitreous", "Bass", "Deep", "Steady", "Electronica", "Compressed glass-like low foundation"},
    {1, "Vitreous | Silica Glint (Lead)", "Vitreous", "Lead", "Bright", "Expressive", "Electronica", "Reflective silica lead for upper melodies"},
    {2, "Vitreous | Crystal Veil (Pad)", "Vitreous", "Pad", "Shimmering", "Evolving", "Ambient", "Translucent sustaining harmonic veil"},
    {3, "Vitreous | Ceramic Speck (Pluck)", "Vitreous", "Pluck", "Sharp", "Decaying", "IDM", "Small ceramic attacks with sparse tails"},
    {4, "Vitreous | Porcelain Chord (Keys)", "Vitreous", "Keys", "Resonant", "Struck", "Cinematic", "Porcelain-toned harmonic keyboard layers"},
    {5, "Vitreous | Ice Fragment (Percussion)", "Vitreous", "Percussion", "Sharp", "Struck", "IDM", "Brittle ice-like percussive accents"},
    {6, "Vitreous | Hollow Horizon (Drone)", "Vitreous", "Drone", "Resonant", "Evolving", "Ambient", "Open hollow glass-toned sustained bed"},
    {7, "Vitreous | Refractive Dust (Texture)", "Vitreous", "Texture", "Textured", "Granular", "Experimental", "Refracted micro-grains and spectral dust"},
    {8, "Organic | Reed Floor (Bass)", "Organic", "Bass", "Warm", "Steady", "Lo-Fi", "Reed-bodied rounded bass foundation"},
    {9, "Organic | Woodgrain Voice (Lead)", "Organic", "Lead", "Resonant", "Expressive", "Electronica", "Woody singing upper-register voice"},
    {10, "Organic | Breathing Canopy (Pad)", "Organic", "Pad", "Airy", "Breathing", "Ambient", "Broad breathing canopy of natural overtones"},
    {11, "Organic | Bamboo Needle (Pluck)", "Organic", "Pluck", "Sharp", "Decaying", "Lo-Fi", "Dry bamboo-like articulated plucks"},
    {12, "Organic | Resonant Bark (Keys)", "Organic", "Keys", "Warm", "Struck", "Cinematic", "Bark and wood resonances for chordal parts"},
    {13, "Organic | Root Percussion (Percussion)", "Organic", "Percussion", "Rhythmic", "Struck", "Experimental", "Rooted tactile percussion articulations"},
    {14, "Organic | Air Column Earth (Drone)", "Organic", "Drone", "Airy", "Evolving", "Ambient", "Sustained earthy air-column atmosphere"},
    {15, "Organic | Living Branch (Texture)", "Organic", "Texture", "Textured", "Breathing", "Experimental", "Shifting branch-like organic texture"},
    {16, "Metallic | Copper Foundation (Bass)", "Metallic", "Bass", "Deep", "Steady", "Progressive Techno", "Dense copper-like tonal bass support"},
    {17, "Metallic | Bronze Filament (Lead)", "Metallic", "Lead", "Bright", "Expressive", "Electronica", "Narrow bronze-like leading filament"},
    {18, "Metallic | Alloy Halo (Pad)", "Metallic", "Pad", "Shimmering", "Evolving", "Ambient", "Luminous alloy harmonic suspension"},
    {19, "Metallic | Spring Needle (Pluck)", "Metallic", "Pluck", "Sharp", "Decaying", "IDM", "Springy metallic short attacks"},
    {20, "Metallic | Bellplate Keys (Keys)", "Metallic", "Keys", "Resonant", "Struck", "Cinematic", "Plate and bell-inspired keyboard tones"},
    {21, "Metallic | Iron Strike (Percussion)", "Metallic", "Percussion", "Sharp", "Struck", "Progressive Techno", "Focused iron-like transient accent"},
    {22, "Metallic | Metallic Monolith (Drone)", "Metallic", "Drone", "Resonant", "Evolving", "Experimental", "Massive slowly changing metallic resonance"},
    {23, "Metallic | Resonant Shards (Texture)", "Metallic", "Texture", "Textured", "Granular", "Experimental", "Scattered ringing metal fragments"},
    {24, "Cellular | Soft Circuit (Bass)", "Cellular", "Bass", "Deep", "Pulsing", "Progressive Techno", "Soft digital low-frequency circuitry"},
    {25, "Cellular | Voltage Edge (Lead)", "Cellular", "Lead", "Bright", "Expressive", "IDM", "Crisp electric edge for melodic lines"},
    {26, "Cellular | Circuit Bloom (Pad)", "Cellular", "Pad", "Shimmering", "Evolving", "Ambient", "Growing harmonics with cellular coloration"},
    {27, "Cellular | Logic Grain (Pluck)", "Cellular", "Pluck", "Sharp", "Decaying", "IDM", "Compact logic-clock plucked impulses"},
    {28, "Cellular | Electric Keyway (Keys)", "Cellular", "Keys", "Bright", "Struck", "Electronica", "Electric stepped harmonic keyboard voice"},
    {29, "Cellular | Mechanical Tick (Percussion)", "Cellular", "Percussion", "Rhythmic", "Rhythmic", "Progressive Techno", "Tight machine-like rhythmic tick"},
    {30, "Cellular | Self Similar Current (Drone)", "Cellular", "Drone", "Textured", "Evolving", "Experimental", "Self-similar sustained current field"},
    {31, "Cellular | Automata Rain (Texture)", "Cellular", "Texture", "Textured", "Granular", "Experimental", "Particle rain with cellular movement"},
    {32, "Nocturnal | Carbon Sub (Bass)", "Nocturnal", "Bass", "Dark", "Steady", "Progressive Techno", "Dense carbon-toned sub foundation"},
    {33, "Nocturnal | Inkline Signal (Lead)", "Nocturnal", "Lead", "Dark", "Expressive", "Electronica", "Ink-dark slender leading signal"},
    {34, "Nocturnal | Shadow Canopy (Pad)", "Nocturnal", "Pad", "Dark", "Evolving", "Ambient", "Shadowed wide harmonic canopy"},
    {35, "Nocturnal | Ash Splinter (Pluck)", "Nocturnal", "Pluck", "Sharp", "Decaying", "Lo-Fi", "Short ash-like muted articulations"},
    {36, "Nocturnal | Dark Matter Keys (Keys)", "Nocturnal", "Keys", "Resonant", "Struck", "Cinematic", "Dark reflective keyboard resonances"},
    {37, "Nocturnal | Black Dust Hit (Percussion)", "Nocturnal", "Percussion", "Rhythmic", "Struck", "IDM", "Muted black-dust transient hits"},
    {38, "Nocturnal | Earth Nocturne (Drone)", "Nocturnal", "Drone", "Dark", "Evolving", "Ambient", "Deep slowly changing nocturnal space"},
    {39, "Nocturnal | Ember Drift (Texture)", "Nocturnal", "Texture", "Textured", "Granular", "Experimental", "Diffuse ember-like drifting particles"},
    {40, "Aerial | Aqua Current (Bass)", "Aerial", "Bass", "Soft", "Steady", "Electronica", "Fluid rounded low-end movement"},
    {41, "Aerial | Vowel Ribbon (Lead)", "Aerial", "Lead", "Airy", "Expressive", "Electronica", "Vowel-shaped floating lead articulation"},
    {42, "Aerial | Azure Suspension (Pad)", "Aerial", "Pad", "Airy", "Evolving", "Ambient", "Wide suspended atmospheric harmonics"},
    {43, "Aerial | Pearl Bead (Pluck)", "Aerial", "Pluck", "Bright", "Decaying", "IDM", "Light pearl-like articulated beads"},
    {44, "Aerial | Cloud Glass (Keys)", "Aerial", "Keys", "Shimmering", "Struck", "Cinematic", "Clouded glass chordal keyboard timbre"},
    {45, "Aerial | Rainfall Skin (Percussion)", "Aerial", "Percussion", "Soft", "Struck", "Experimental", "Fine rainfall-like soft rhythmic attacks"},
    {46, "Aerial | High Atmosphere (Drone)", "Aerial", "Drone", "Airy", "Evolving", "Ambient", "High sustained open-air drone"},
    {47, "Aerial | Floating Particles (Texture)", "Aerial", "Texture", "Textured", "Granular", "Experimental", "Loose floating airborne micro-particles"},
    {48, "Kinetic | Felt Foundation (Bass)", "Kinetic", "Bass", "Warm", "Steady", "Lo-Fi", "Felt-damped weighty rhythmic low support"},
    {49, "Kinetic | Mechanism Signal (Lead)", "Kinetic", "Lead", "Bright", "Pulsing", "Progressive Techno", "Mechanized articulate sequenced lead"},
    {50, "Kinetic | Pulse Loom (Pad)", "Kinetic", "Pad", "Rhythmic", "Pulsing", "Progressive Techno", "Wide pulsing harmonic fabric"},
    {51, "Kinetic | Sand Trigger (Pluck)", "Kinetic", "Pluck", "Sharp", "Decaying", "IDM", "Dry sand-like triggered transient"},
    {52, "Kinetic | Membrane Keys (Keys)", "Kinetic", "Keys", "Warm", "Struck", "Lo-Fi", "Membrane-styled tactile harmonic keys"},
    {53, "Kinetic | Transient Atlas (Percussion)", "Kinetic", "Percussion", "Rhythmic", "Struck", "Progressive Techno", "Short functional percussive transients"},
    {54, "Kinetic | Motor Field (Drone)", "Kinetic", "Drone", "Rhythmic", "Evolving", "Progressive Techno", "Persistent rotating mechanical tone"},
    {55, "Kinetic | Rhythmic Detritus (Texture)", "Kinetic", "Texture", "Textured", "Rhythmic", "IDM", "Broken rhythmic particles and detritus"},
    {56, "Chimeric | Stoneweight Bass (Bass)", "Chimeric", "Bass", "Deep", "Steady", "Progressive Techno", "Mineral-heavy hybrid bass weight"},
    {57, "Chimeric | Wireline Lead (Lead)", "Chimeric", "Lead", "Sharp", "Expressive", "Experimental", "Wired hybrid melodic edge"},
    {58, "Chimeric | Bellfield Pad (Pad)", "Chimeric", "Pad", "Shimmering", "Evolving", "Ambient", "Hybrid bell-like sustained harmonics"},
    {59, "Chimeric | Quartz Pluck (Pluck)", "Chimeric", "Pluck", "Bright", "Decaying", "IDM", "Quartz-inspired articulated hybrid plucks"},
    {60, "Chimeric | Mineral Keys (Keys)", "Chimeric", "Keys", "Resonant", "Struck", "Cinematic", "Composite mineral keyboard resonances"},
    {61, "Chimeric | Fracture Knock (Percussion)", "Chimeric", "Percussion", "Rhythmic", "Struck", "Experimental", "Fractured hybrid impact articulation"},
    {62, "Chimeric | Composite Resonance (Drone)", "Chimeric", "Drone", "Hybrid", "Evolving", "Experimental", "Dense composite sustained resonances"},
    {63, "Chimeric | Hybrid Matter (Texture)", "Chimeric", "Texture", "Hybrid", "Granular", "Experimental", "Mixed-material evolving sound-design layer"},
}};
static_assert(entries.size() == 64, "Factory taxonomy must retain all 64 patch IDs");
inline constexpr const PresetCategory& byIndex(int index) noexcept
{
    return entries[static_cast<std::size_t>(index)];
}
} // namespace albedo_preset_taxonomy
