#pragma once

// convert 32bit FourCC and 32bit size into int
inline static uint32_t GetNumber(char* buf) { return (*(uint32_t*)buf); }

// store 32bit FourCC as a single variable
class FourCC
{
public:
	uint32_t code;
	int read(FILE* in);
	inline bool Equals(const char* str) const { return (*(uint32_t*)str == code); }
};

// represents a Chunk of the RIFF file
class Chunk
{
public:
	FourCC type;
	uint32_t length; // data field length of this chunk (without the 8 bytes used for FourCC and length
	int read(FILE* in);
	void skip(FILE* in) const; // don't read the data field of this chunk
	inline bool Equals(const char* str) const { return (*(uint32_t*)str == type.code); }
};

// neither eax.h nor EaxMan.h define a struct for the global diffraction model used in EAGLE (and therefore stored in .eal), so here it is:
struct EALGlobalDiffractionModel
{
    int32_t MaxAttenuation;
    float LFRatio;
    int32_t AngleMaxAttenuation; // EAGLE clips value into range 0 .. 90, so it cannot be derived with certainty if field is signed or unsigned
};

// basically the EAXLISTENERPROPERTIES struct members plus the additional name (as shown in EAGLE Environment Models)
// also the flags are separated into booleans and the EAX Environment is translated from the enum into a string
struct EALEnvironment
{
    std::string name;
    int32_t Room;               // room effect level at low frequencies
    int32_t RoomHF;             // room effect high-frequency level re. low frequency level
    float RoomRolloffFactor;    // like DS3D flRolloffFactor but for room effect
    float DecayTime;            // reverberation decay time at low frequencies
    float DecayHFRatio;         // high-frequency to low-frequency decay time ratio
    int32_t Reflections;        // early reflections level relative to room effect
    float ReflectionsDelay;     // initial reflection delay time
    int32_t Reverb;             // late reverberation level relative to room effect
    float ReverbDelay;          // late reverberation delay time relative to initial reflection
    uint32_t Environment;       // sets all listener properties
    std::string EAXEnvironment; // show EAX Environment
    float EnvironmentSize;      // environment size in meters
    float EnvironmentDiffusion; // environment diffusion
    float AirAbsorptionHF;      // change in level per meter at 5 kHz
    uint32_t Flags;             // modifies the behavior of properties
    bool DecayTimeScale;        // scale decay time with size
    bool ReflectionsScale;      // scale reflections with size
    bool ReflectionsDelayScale; // scale reflections delay with size
    bool ReverbScale;           // scale reverb with size
    bool ReverbDelayScale;      // scale reverb delay with size
    bool DecayHFLimit;          // clip HF ratio
};

// basically the SOURCEATTRIBUTES struct members plus the additional name (as shown in EAGLE Source Models) and the filename
struct EALSourceAttributes
{
    std::string name;
    std::string file;
    EAXBUFFERPROPERTIES eaxAttributes;
    uint32_t InsideConeAngle;
    uint32_t OutsideConeAngle;
    int32_t ConeOutsideVolume;
    float ConeXdir;
    float ConeYdir;
    float ConeZdir;
    float MinDistance;
    float MaxDistance;
    int32_t DupCount;
    int32_t Priority;
    int32_t NrSourceInstances;
    std::vector<EMPOINT> LocationSourceInstances;
};

// basically the MATERIALATTRIBUTES struct members plus the additional name (as shown in EAGLE Obstacle Models) and the filename
struct EALMaterials
{
    std::string name;
    int32_t Level;
    float LFRatio;
    float RoomRatio;
    uint32_t Flags;
};

// the unknown geometry data blob plus the additional name (as shown in EAGLE Geometry Sets) and the filename
struct EALGeometry
{
    std::string name;
    std::string filename;
    std::vector<char> data;
};

// here the casted data from the manual .eal file analysis is shown
// not all info stored in the .eal can also be retrieved through EaxMan
struct EALFileContent
{
    EALGlobalDiffractionModel gdfm; // Global Diffraction Model
    LISTENERATTRIBUTES lisa; // Default Listener Attributes
    EAXLISTENERPROPERTIES denv; // Default Environment
    SOURCEATTRIBUTES dsrc; // Default Source
    MATERIALATTRIBUTES dmat; // Default Obstacles
    std::string dfil;
    std::vector<EALEnvironment> Environments;
    std::vector<EALSourceAttributes> Sources;
    std::vector<EALMaterials> Materials;
    std::vector<EALGeometry> Geometries;
};

// EAXManager does not provide a way to retrieve the models and sets names, but has functions that use them to access the data behind it
// as a result of the manual .eal analysis, all names are read and can be passes to the EaxMan part of the tool to test the EaxMan functions
struct EALFileNames
{
    std::vector<std::string> sourceNames;
    std::vector<std::string> materialNames;
    std::vector<std::string> environmentNames;
    std::vector<std::string> geometryNames;
};

// translates the enum back into human-readable strings - which are also used by EAGLE to show the EAX Environment
static std::string IdentifyEAXEnvironment(uint32_t);

// splits the EAX flags into its individual parts
static std::tuple<bool, bool, bool, bool, bool, bool> IdentifyEAXFlags(uint32_t);

// coordinates the manual analysis of the .eal files
static int ReadEalManually(char*, EALFileContent*);

// starts the analysis of the RIFF file -> derived from https://johnloomis.org/cpe102/asgn/asgn1/riff.html
void AnalyzeEal(FILE*, int, EALFileContent*);

// starts the analysis of the LIST chunk of the RIFF file -> derived from https://johnloomis.org/cpe102/asgn/asgn1/riff.html
void AnalyzeEalList(FILE*, int, EALFileContent*);

// analyses the RIFF chunk to cast the global variables
void AnalyzeEalGlobals(FILE*, int, const char*, EALFileContent*);

// analyses the 'envp' chunk to cast the environment models
int AnalyzeEalEnvironmentAttributes(FILE*, EALFileContent*);

// analyses the 'matp' chunk to cast the obstacle models
int AnalyzeEalMaterialAttributes(FILE*, EALFileContent*);

// analyses the 'srcp' chunk to cast the source models
int AnalyzeEalSourceAttributes(FILE*, EALFileContent*);

// analyses the 'gemp' chunk to ... well, just read the geometry set data without knowing how to interpret that data
int AnalyzeEalGeometryAttributes(FILE*, EALFileContent*);

// coordinates the analysis of the .eal files with EAX Manager
static int ReadEalWithEaxman(char*, EALFileContent*, EALFileNames*, bool);

// try IEaxManager_GetDataSetSize(p,a,b)
static int EaxmanReadDataSize(LPEAXMANAGER, EALFileContent*);

// use IEaxManager_GetListenerAttributes(p,a) to retrieve the listener attributes with EaxMan
static int EaxmanReadListenerAttributes(LPEAXMANAGER, EALFileContent*);

// use IEaxManager_GetSourceAttributes(p,a,b) to retrieve the source models data with EaxMan
static int EaxmanReadSourceAttributes(LPEAXMANAGER, EALFileContent*, EALFileNames*);

// use IEaxManager_GetMaterialAttributes(p,a,b) to retrieve the obstacle models data with EaxMan
static int EaxmanReadMaterialAttributes(LPEAXMANAGER, EALFileContent*, EALFileNames*);

// use IEaxManager_GetEnvironmentAttributes(p,a,b) to retrieve the environment models data with EaxMan
static int EaxmanReadEnvironmentAttributes(LPEAXMANAGER, EALFileContent*, EALFileNames*);

// use IEaxManager_GetListenerDynamicAttributes(p,a,b,c,d) to dynamically query the listener attributes according to input coordinates
static void EaxmanDynamicQuery(LPEAXMANAGER, EALFileContent*, char*);

// read commandline input and sanitize it
static float ReadInputFloat(std::string);

// compare the data retrieved by EaxMan with the data read by analysing the RIFF .eal file
static void CompareEALFiles(EALFileContent*, EALFileContent*);
