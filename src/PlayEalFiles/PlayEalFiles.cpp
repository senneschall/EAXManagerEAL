#include <iostream>
#include <limits.h>
#include <cstring>
#include <stdio.h>
#include <vector>

#include "AL/al.h" // dependency: libopenal
#include "AL/alc.h"
#include "AL/alext.h"
#include "AL/efx.h" // tested with openal-soft
#include "AL/efx-presets.h"

#include "sndfile.h" // dependency: libsndfile

#include "PlayEalFiles.h" // struct definitions copied from eax.h to avoid dependency on COM (and therefore Windows)
// in a proper implementation, openal-soft's eax/api.h should be used, but this tool tries to stay in sync with ReadEal, so it uses the same data struct definitions

// macro from openal-soft alhelpers.h
#ifdef __cplusplus
extern "C" {
#endif
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#define FUNCTION_CAST(T, ptr) (union{void *p; T f;}){ptr}.f
#elif defined(__cplusplus)
#define FUNCTION_CAST(T, ptr) reinterpret_cast<T>(ptr)
#else
#define FUNCTION_CAST(T, ptr) (T)(ptr)
#endif
#ifdef __cplusplus
} // extern "C"
#endif

static EFXEAXREVERBPROPERTIES EAXtoEFX(EAXLISTENERPROPERTIES env)
{
	// default values for EAX2 environments taken from api.h
	switch (env.dwEnvironment)
	{
	case EAX_ENVIRONMENT_PADDEDCELL:
		return { 0.1715f, 1.0000f, 0.3162f, 0.0010f, 1.0000f, 0.1700f, 0.1000f, 1.0000f, 0.2500f, 0.0010f, { 0.0000f, 0.0000f, 0.0000f }, 1.2691f, 0.0020f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_ROOM:
		return { 0.4287f, 1.0000f, 0.3162f, 0.5929f, 1.0000f, 0.4000f, 0.8300f, 1.0000f, 0.1503f, 0.0020f, { 0.0000f, 0.0000f, 0.0000f }, 1.0629f, 0.0030f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_BATHROOM:
		return { 0.1715f, 1.0000f, 0.3162f, 0.2512f, 1.0000f, 1.4900f, 0.5400f, 1.0000f, 0.6531f, 0.0070f, { 0.0000f, 0.0000f, 0.0000f }, 3.2734f, 0.0110f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_LIVINGROOM:
		return { 0.9766f, 1.0000f, 0.3162f, 0.0010f, 1.0000f, 0.5000f, 0.1000f, 1.0000f, 0.2051f, 0.0030f, { 0.0000f, 0.0000f, 0.0000f }, 0.2805f, 0.0040f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_STONEROOM:
		return { 1.0000f, 1.0000f, 0.3162f, 0.7079f, 1.0000f, 2.3100f, 0.6400f, 1.0000f, 0.4411f, 0.0120f, { 0.0000f, 0.0000f, 0.0000f }, 1.1003f, 0.0170f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_AUDITORIUM:
		return { 1.0000f, 1.0000f, 0.3162f, 0.5781f, 1.0000f, 4.3200f, 0.5900f, 1.0000f, 0.4032f, 0.0200f, { 0.0000f, 0.0000f, 0.0000f }, 0.7170f, 0.0300f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_CONCERTHALL:
		return { 1.0000f, 1.0000f, 0.3162f, 0.5623f, 1.0000f, 3.9200f, 0.7000f, 1.0000f, 0.2427f, 0.0200f, { 0.0000f, 0.0000f, 0.0000f }, 0.9977f, 0.0290f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_CAVE:
		return { 1.0000f, 1.0000f, 0.3162f, 1.0000f, 1.0000f, 2.9100f, 1.3000f, 1.0000f, 0.5000f, 0.0150f, { 0.0000f, 0.0000f, 0.0000f }, 0.7063f, 0.0220f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x0 };
	case EAX_ENVIRONMENT_ARENA:
		return { 1.0000f, 1.0000f, 0.3162f, 0.4477f, 1.0000f, 7.2400f, 0.3300f, 1.0000f, 0.2612f, 0.0200f, { 0.0000f, 0.0000f, 0.0000f }, 1.0186f, 0.0300f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_HANGAR:
		return { 1.0000f, 1.0000f, 0.3162f, 0.3162f, 1.0000f, 10.0500f, 0.2300f, 1.0000f, 0.5000f, 0.0200f, { 0.0000f, 0.0000f, 0.0000f }, 1.2560f, 0.0300f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_CARPETEDHALLWAY:
		return { 0.4287f, 1.0000f, 0.3162f, 0.0100f, 1.0000f, 0.3000f, 0.1000f, 1.0000f, 0.1215f, 0.0020f, { 0.0000f, 0.0000f, 0.0000f }, 0.1531f, 0.0300f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_HALLWAY:
		return { 0.3645f, 1.0000f, 0.3162f, 0.7079f, 1.0000f, 1.4900f, 0.5900f, 1.0000f, 0.2458f, 0.0070f, { 0.0000f, 0.0000f, 0.0000f }, 1.6615f, 0.0110f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_STONECORRIDOR:
		return { 1.0000f, 1.0000f, 0.3162f, 0.7612f, 1.0000f, 2.7000f, 0.7900f, 1.0000f, 0.2472f, 0.0130f, { 0.0000f, 0.0000f, 0.0000f }, 1.5758f, 0.0200f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_ALLEY:
		return { 1.0000f, 0.3000f, 0.3162f, 0.7328f, 1.0000f, 1.4900f, 0.8600f, 1.0000f, 0.2500f, 0.0070f, { 0.0000f, 0.0000f, 0.0000f }, 0.9954f, 0.0110f, { 0.0000f, 0.0000f, 0.0000f }, 0.1250f, 0.9500f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_FOREST:
		return { 1.0000f, 0.3000f, 0.3162f, 0.0224f, 1.0000f, 1.4900f, 0.5400f, 1.0000f, 0.0525f, 0.1620f, { 0.0000f, 0.0000f, 0.0000f }, 0.7682f, 0.0880f, { 0.0000f, 0.0000f, 0.0000f }, 0.1250f, 1.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_CITY:
		return { 1.0000f, 0.5000f, 0.3162f, 0.3981f, 1.0000f, 1.4900f, 0.6700f, 1.0000f, 0.0730f, 0.0070f, { 0.0000f, 0.0000f, 0.0000f }, 0.1427f, 0.0110f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_MOUNTAINS:
		return { 1.0000f, 0.2700f, 0.3162f, 0.0562f, 1.0000f, 1.4900f, 0.2100f, 1.0000f, 0.0407f, 0.3000f, { 0.0000f, 0.0000f, 0.0000f }, 0.1919f, 0.1000f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 1.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x0 };
	case EAX_ENVIRONMENT_QUARRY:
		return { 1.0000f, 1.0000f, 0.3162f, 0.3162f, 1.0000f, 1.4900f, 0.8300f, 1.0000f, 0.0000f, 0.0610f, { 0.0000f, 0.0000f, 0.0000f }, 1.7783f, 0.0250f, { 0.0000f, 0.0000f, 0.0000f }, 0.1250f, 0.7000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_PLAIN:
		return { 1.0000f, 0.2100f, 0.3162f, 0.1000f, 1.0000f, 1.4900f, 0.5000f, 1.0000f, 0.0585f, 0.1790f, { 0.0000f, 0.0000f, 0.0000f }, 0.1089f, 0.1000f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 1.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_PARKINGLOT:
		return { 1.0000f, 1.0000f, 0.3162f, 1.0000f, 1.0000f, 1.6500f, 1.5000f, 1.0000f, 0.2082f, 0.0080f, { 0.0000f, 0.0000f, 0.0000f }, 0.2652f, 0.0120f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x0 };
	case EAX_ENVIRONMENT_SEWERPIPE:
		return { 0.3071f, 0.8000f, 0.3162f, 0.3162f, 1.0000f, 2.8100f, 0.1400f, 1.0000f, 1.6387f, 0.0140f, { 0.0000f, 0.0000f, 0.0000f }, 3.2471f, 0.0210f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_UNDERWATER:
		return { 0.3645f, 1.0000f, 0.3162f, 0.0100f, 1.0000f, 1.4900f, 0.1000f, 1.0000f, 0.5963f, 0.0070f, { 0.0000f, 0.0000f, 0.0000f }, 7.0795f, 0.0110f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 1.1800f, 0.3480f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	case EAX_ENVIRONMENT_DRUGGED:
		return { 0.4287f, 0.5000f, 0.3162f, 1.0000f, 1.0000f, 8.3900f, 1.3900f, 1.0000f, 0.8760f, 0.0020f, { 0.0000f, 0.0000f, 0.0000f }, 3.1081f, 0.0300f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 1.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x0 };
	case EAX_ENVIRONMENT_DIZZY:
		return { 0.3645f, 0.6000f, 0.3162f, 0.6310f, 1.0000f, 17.2300f, 0.5600f, 1.0000f, 0.1392f, 0.0200f, { 0.0000f, 0.0000f, 0.0000f }, 0.4937f, 0.0300f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 1.0000f, 0.8100f, 0.3100f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x0 };
	case EAX_ENVIRONMENT_PSYCHOTIC:
		return { 0.0625f, 0.5000f, 0.3162f, 0.8404f, 1.0000f, 7.5600f, 0.9100f, 1.0000f, 0.4864f, 0.0200f, { 0.0000f, 0.0000f, 0.0000f }, 2.4378f, 0.0300f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 4.0000f, 1.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x0 };
	case EAX_ENVIRONMENT_GENERIC:
	default:
		return { 1.0000f, 1.0000f, 0.3162f, 0.8913f, 1.0000f, 1.4900f, 0.8300f, 1.0000f, 0.0500f, 0.0070f, { 0.0000f, 0.0000f, 0.0000f }, 1.2589f, 0.0110f, { 0.0000f, 0.0000f, 0.0000f }, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, 0x1 };
	}
}

/* LoadEffect loads the given reverb properties into a new OpenAL effect
 * object, and returns the new effect ID. */
static ALuint LoadEffect(const EFXEAXREVERBPROPERTIES *reverb)
{
    ALuint effect = 0;

    /* Clear error state. */
    alGetError();

    /* Create the effect object and check if we can do EAX reverb. */
    alGenEffects(1, &effect);
    alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
    if(alGetError() == AL_NO_ERROR)
    {
        alEffectf(effect, AL_EAXREVERB_DENSITY, reverb->flDensity);
        alEffectf(effect, AL_EAXREVERB_DIFFUSION, reverb->flDiffusion);
        alEffectf(effect, AL_EAXREVERB_GAIN, reverb->flGain);
        alEffectf(effect, AL_EAXREVERB_GAINHF, reverb->flGainHF);
        alEffectf(effect, AL_EAXREVERB_GAINLF, reverb->flGainLF);
        alEffectf(effect, AL_EAXREVERB_DECAY_TIME, reverb->flDecayTime);
        alEffectf(effect, AL_EAXREVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
        alEffectf(effect, AL_EAXREVERB_DECAY_LFRATIO, reverb->flDecayLFRatio);
        alEffectf(effect, AL_EAXREVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
        alEffectf(effect, AL_EAXREVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
        alEffectfv(effect, AL_EAXREVERB_REFLECTIONS_PAN, reverb->flReflectionsPan);
        alEffectf(effect, AL_EAXREVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
        alEffectf(effect, AL_EAXREVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
        alEffectfv(effect, AL_EAXREVERB_LATE_REVERB_PAN, reverb->flLateReverbPan);
        alEffectf(effect, AL_EAXREVERB_ECHO_TIME, reverb->flEchoTime);
        alEffectf(effect, AL_EAXREVERB_ECHO_DEPTH, reverb->flEchoDepth);
        alEffectf(effect, AL_EAXREVERB_MODULATION_TIME, reverb->flModulationTime);
        alEffectf(effect, AL_EAXREVERB_MODULATION_DEPTH, reverb->flModulationDepth);
        alEffectf(effect, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
        alEffectf(effect, AL_EAXREVERB_HFREFERENCE, reverb->flHFReference);
        alEffectf(effect, AL_EAXREVERB_LFREFERENCE, reverb->flLFReference);
        alEffectf(effect, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
        alEffecti(effect, AL_EAXREVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);
    }
    else
    {
        /* No EAX Reverb. Set the standard reverb effect type then load the
         * available reverb properties.
         */
        alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);

        alEffectf(effect, AL_REVERB_DENSITY, reverb->flDensity);
        alEffectf(effect, AL_REVERB_DIFFUSION, reverb->flDiffusion);
        alEffectf(effect, AL_REVERB_GAIN, reverb->flGain);
        alEffectf(effect, AL_REVERB_GAINHF, reverb->flGainHF);
        alEffectf(effect, AL_REVERB_DECAY_TIME, reverb->flDecayTime);
        alEffectf(effect, AL_REVERB_DECAY_HFRATIO, reverb->flDecayHFRatio);
        alEffectf(effect, AL_REVERB_REFLECTIONS_GAIN, reverb->flReflectionsGain);
        alEffectf(effect, AL_REVERB_REFLECTIONS_DELAY, reverb->flReflectionsDelay);
        alEffectf(effect, AL_REVERB_LATE_REVERB_GAIN, reverb->flLateReverbGain);
        alEffectf(effect, AL_REVERB_LATE_REVERB_DELAY, reverb->flLateReverbDelay);
        alEffectf(effect, AL_REVERB_AIR_ABSORPTION_GAINHF, reverb->flAirAbsorptionGainHF);
        alEffectf(effect, AL_REVERB_ROOM_ROLLOFF_FACTOR, reverb->flRoomRolloffFactor);
        alEffecti(effect, AL_REVERB_DECAY_HFLIMIT, reverb->iDecayHFLimit);
    }

    /* Check if an error occurred, and clean up if so. */
    if(alGetError() != AL_NO_ERROR)
    {
        if(alIsEffect(effect))
            alDeleteEffects(1, &effect);
        return 0;
    }

    return effect;
}

/* LoadBuffer loads the named audio file into an OpenAL buffer object, and
 * returns the new buffer ID.
 */
static ALuint LoadSound(const char *filename) // implementation from the openal-soft src examples
{
    enum FormatType sample_format = Int16;
    ALint byteblockalign = 0;
    ALint splblockalign = 0;
    sf_count_t num_frames;
    ALenum err;
    ALenum format;
    ALsizei num_bytes;
    SNDFILE *sndfile;
    SF_INFO sfinfo;
    ALuint buffer;
    void *membuf;

    /* Open the audio file and check that it's usable. */
    sndfile = sf_open(filename, SFM_READ, &sfinfo);
    if(!sndfile)
    {
    	std::cout << "Error opening audio file " << filename << "\n";
        return 0;
    }
    if(sfinfo.frames < 1)
    {
    	std::cout << "Bad sample count\n";
        sf_close(sndfile);
        return 0;
    }

    /* Detect a suitable format to load. Formats like Vorbis and Opus use float
     * natively, so load as float to avoid clipping when possible. Formats
     * larger than 16-bit can also use float to preserve a bit more precision.
     */
    switch((sfinfo.format&SF_FORMAT_SUBMASK))
    {
    case SF_FORMAT_PCM_24:
    case SF_FORMAT_PCM_32:
    case SF_FORMAT_FLOAT:
    case SF_FORMAT_DOUBLE:
    case SF_FORMAT_VORBIS:
    case SF_FORMAT_OPUS:
    case SF_FORMAT_ALAC_20:
    case SF_FORMAT_ALAC_24:
    case SF_FORMAT_ALAC_32:
    case 0x0080/*SF_FORMAT_MPEG_LAYER_I*/:
    case 0x0081/*SF_FORMAT_MPEG_LAYER_II*/:
    case 0x0082/*SF_FORMAT_MPEG_LAYER_III*/:
        if(alIsExtensionPresent("AL_EXT_FLOAT32"))
            sample_format = Float;
        break;
    case SF_FORMAT_IMA_ADPCM:
        /* ADPCM formats require setting a block alignment as specified in the
         * file, which needs to be read from the wave 'fmt ' chunk manually
         * since libsndfile doesn't provide it in a format-agnostic way.
         */
        if(sfinfo.channels <= 2 && (sfinfo.format&SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
            && alIsExtensionPresent("AL_EXT_IMA4")
            && alIsExtensionPresent("AL_SOFT_block_alignment"))
            sample_format = IMA4;
        break;
    case SF_FORMAT_MS_ADPCM:
        if(sfinfo.channels <= 2 && (sfinfo.format&SF_FORMAT_TYPEMASK) == SF_FORMAT_WAV
            && alIsExtensionPresent("AL_SOFT_MSADPCM")
            && alIsExtensionPresent("AL_SOFT_block_alignment"))
            sample_format = MSADPCM;
        break;
    }

    if(sample_format == IMA4 || sample_format == MSADPCM)
    {
        /* For ADPCM, lookup the wave file's "fmt " chunk, which is a
         * WAVEFORMATEX-based structure for the audio format.
         */
        SF_CHUNK_INFO inf = { "fmt ", 4, 0, NULL };
        SF_CHUNK_ITERATOR *iter = sf_get_chunk_iterator(sndfile, &inf);

        /* If there's an issue getting the chunk or block alignment, load as
         * 16-bit and have libsndfile do the conversion.
         */
        if(!iter || sf_get_chunk_size(iter, &inf) != SF_ERR_NO_ERROR || inf.datalen < 14)
            sample_format = Int16;
        else
        {
            ALubyte *fmtbuf = (ALubyte*)calloc(inf.datalen, 1);
            inf.data = fmtbuf;
            if(sf_get_chunk_data(iter, &inf) != SF_ERR_NO_ERROR)
                sample_format = Int16;
            else
            {
                /* Read the nBlockAlign field, and convert from bytes- to
                 * samples-per-block (verifying it's valid by converting back
                 * and comparing to the original value).
                 */
                byteblockalign = fmtbuf[12] | (fmtbuf[13]<<8);
                if(sample_format == IMA4)
                {
                    splblockalign = (byteblockalign/sfinfo.channels - 4)/4*8 + 1;
                    if(splblockalign < 1
                        || ((splblockalign-1)/2 + 4)*sfinfo.channels != byteblockalign)
                        sample_format = Int16;
                }
                else
                {
                    splblockalign = (byteblockalign/sfinfo.channels - 7)*2 + 2;
                    if(splblockalign < 2
                        || ((splblockalign-2)/2 + 7)*sfinfo.channels != byteblockalign)
                        sample_format = Int16;
                }
            }
            free(fmtbuf);
        }
    }

    if(sample_format == Int16)
    {
        splblockalign = 1;
        byteblockalign = sfinfo.channels * 2;
    }
    else if(sample_format == Float)
    {
        splblockalign = 1;
        byteblockalign = sfinfo.channels * 4;
    }

    /* Figure out the OpenAL format from the file and desired sample type. */
    format = AL_NONE;
    if(sfinfo.channels == 1)
    {
        if(sample_format == Int16)
            format = AL_FORMAT_MONO16;
        else if(sample_format == Float)
            format = AL_FORMAT_MONO_FLOAT32;
        else if(sample_format == IMA4)
            format = AL_FORMAT_MONO_IMA4;
        else if(sample_format == MSADPCM)
            format = AL_FORMAT_MONO_MSADPCM_SOFT;
    }
    else if(sfinfo.channels == 2)
    {
        if(sample_format == Int16)
            format = AL_FORMAT_STEREO16;
        else if(sample_format == Float)
            format = AL_FORMAT_STEREO_FLOAT32;
        else if(sample_format == IMA4)
            format = AL_FORMAT_STEREO_IMA4;
        else if(sample_format == MSADPCM)
            format = AL_FORMAT_STEREO_MSADPCM_SOFT;
    }
    else if(sfinfo.channels == 3)
    {
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
        {
            if(sample_format == Int16)
                format = AL_FORMAT_BFORMAT2D_16;
            else if(sample_format == Float)
                format = AL_FORMAT_BFORMAT2D_FLOAT32;
        }
    }
    else if(sfinfo.channels == 4)
    {
        if(sf_command(sndfile, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
        {
            if(sample_format == Int16)
                format = AL_FORMAT_BFORMAT3D_16;
            else if(sample_format == Float)
                format = AL_FORMAT_BFORMAT3D_FLOAT32;
        }
    }
    if(!format)
    {
    	std::cout << "Unsupported channel count:\n";
        sf_close(sndfile);
        return 0;
    }

    if(sfinfo.frames/splblockalign > (sf_count_t)(INT_MAX/byteblockalign))
    {
    	std::cout << "Too many samples in file\n";
        sf_close(sndfile);
        return 0;
    }

    /* Decode the whole audio file to a buffer. */
    membuf = malloc((size_t)(sfinfo.frames / splblockalign * byteblockalign));

    if(sample_format == Int16)
        num_frames = sf_readf_short(sndfile, (short*)membuf, sfinfo.frames);
    else if(sample_format == Float)
        num_frames = sf_readf_float(sndfile, (float*)membuf, sfinfo.frames);
    else
    {
        sf_count_t count = sfinfo.frames / splblockalign * byteblockalign;
        num_frames = sf_read_raw(sndfile, membuf, count);
        if(num_frames > 0)
            num_frames = num_frames / byteblockalign * splblockalign;
    }
    if(num_frames < 1)
    {
        free(membuf);
        sf_close(sndfile);
        std::cout << "Failed to read samples\n";
        return 0;
    }
    num_bytes = (ALsizei)(num_frames / splblockalign * byteblockalign);

    fflush(stdout);

    /* Buffer the audio data into a new buffer object, then free the data and
     * close the file.
     */
    buffer = 0;
    alGenBuffers(1, &buffer);
    if(splblockalign > 1)
        alBufferi(buffer, AL_UNPACK_BLOCK_ALIGNMENT_SOFT, splblockalign);
    alBufferData(buffer, format, membuf, num_bytes, sfinfo.samplerate);

    free(membuf);
    sf_close(sndfile);

    /* Check if an error occurred, and clean up if so. */
    err = alGetError();
    if(err != AL_NO_ERROR)
    {
    	std::cout << "OpenAL Error:\n";
        if(buffer && alIsBuffer(buffer))
            alDeleteBuffers(1, &buffer);
        return 0;
    }

    return buffer;
}

// reference: https://github.com/ffainelli/openal-example
int PlaySound(const char* sndfile)
{
	ALCdevice *device;
	device = alcOpenDevice(NULL);
	if (!device)
	{
		std::cout << "Error opening device\n";
		return -1;
	}

	ALCenum error = alGetError();

	ALCcontext *context;
	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context))
	{
		std::cout << "Error creating context\n";
		return -1;
	}
	error = alGetError();
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error creating context\n";
		return -1;
	}

	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	alListener3f(AL_POSITION, 0, 0, 1.0f);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting listener position\n";
		return -1;
	}
	alListener3f(AL_VELOCITY, 0, 0, 0);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting listener velocity\n";
		return -1;
	}
	alListenerfv(AL_ORIENTATION, listenerOri);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting listener orientation\n";
		return -1;
	}

	ALuint source;
	alGenSources((ALuint)1, &source);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error generating sources\n";
		return -1;
	}
	alSourcef(source, AL_PITCH, 1);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source pitch\n";
		return -1;
	}
	alSourcef(source, AL_GAIN, 1);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source gain\n";
		return -1;
	}
	alSource3f(source, AL_POSITION, 0, 0, 0);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source position\n";
		return -1;
	}
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source velocity\n";
		return -1;
	}
	alSourcei(source, AL_LOOPING, AL_FALSE);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting looping\n";
		return -1;
	}

	ALuint buffer;
    /* Load the sound into a buffer. */
    buffer = LoadSound(sndfile);

    alSourcei(source, AL_BUFFER, buffer);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error binding source to buffer\n";
		return -1;
	}

    alSourcePlay(source);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error playing source\n";
		return -1;
	}

    ALint source_state;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error checking source state\n";
		return -1;
	}
    while (source_state == AL_PLAYING)
    {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    	if (error != AL_NO_ERROR)
    	{
    		std::cout << "Error checking source state\n";
    		return -1;
    	}
    }

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}

int PlaySoundWithEffects(const char* sndfile, EAXLISTENERPROPERTIES environment)
{
	/* Define a macro to help load the function pointers. */
	#define LOAD_PROC(T, x)  ((x) = FUNCTION_CAST(T, alGetProcAddress(#x)))
	LOAD_PROC(LPALGENEFFECTS, alGenEffects);
	LOAD_PROC(LPALDELETEEFFECTS, alDeleteEffects);
	LOAD_PROC(LPALISEFFECT, alIsEffect);
	LOAD_PROC(LPALEFFECTI, alEffecti);
	LOAD_PROC(LPALEFFECTIV, alEffectiv);
	LOAD_PROC(LPALEFFECTF, alEffectf);
	LOAD_PROC(LPALEFFECTFV, alEffectfv);
	LOAD_PROC(LPALGETEFFECTI, alGetEffecti);
	LOAD_PROC(LPALGETEFFECTIV, alGetEffectiv);
	LOAD_PROC(LPALGETEFFECTF, alGetEffectf);
	LOAD_PROC(LPALGETEFFECTFV, alGetEffectfv);

	LOAD_PROC(LPALGENAUXILIARYEFFECTSLOTS, alGenAuxiliaryEffectSlots);
	LOAD_PROC(LPALDELETEAUXILIARYEFFECTSLOTS, alDeleteAuxiliaryEffectSlots);
	LOAD_PROC(LPALISAUXILIARYEFFECTSLOT, alIsAuxiliaryEffectSlot);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTI, alAuxiliaryEffectSloti);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTIV, alAuxiliaryEffectSlotiv);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTF, alAuxiliaryEffectSlotf);
	LOAD_PROC(LPALAUXILIARYEFFECTSLOTFV, alAuxiliaryEffectSlotfv);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTI, alGetAuxiliaryEffectSloti);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTIV, alGetAuxiliaryEffectSlotiv);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTF, alGetAuxiliaryEffectSlotf);
	LOAD_PROC(LPALGETAUXILIARYEFFECTSLOTFV, alGetAuxiliaryEffectSlotfv);
	#undef LOAD_PROC

	ALCdevice *device;
	device = alcOpenDevice(NULL);
	if (!device)
	{
		std::cout << "Error opening device\n";
		return -1;
	}

	ALCenum error = alGetError();

	ALCcontext *context;
	context = alcCreateContext(device, NULL);
	if (!alcMakeContextCurrent(context))
	{
		std::cout << "Error creating context\n";
		return -1;
	}
	error = alGetError();
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error creating context\n";
		return -1;
	}

	ALfloat listenerOri[] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f };
	alListener3f(AL_POSITION, 0, 0, 1.0f);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting listener position\n";
		return -1;
	}
	alListener3f(AL_VELOCITY, 0, 0, 0);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting listener velocity\n";
		return -1;
	}
	alListenerfv(AL_ORIENTATION, listenerOri);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting listener orientation\n";
		return -1;
	}

	ALuint effect;
    EFXEAXREVERBPROPERTIES reverb = EAXtoEFX(environment);
    // Load the reverb into an effect.
    effect = LoadEffect(&reverb);
    if(!effect)
    {
    	std::cout << "EAX effect could not be loaded\n";
        return 1;
    }

    ALuint slot;
    // Create the effect slot object. This is what "plays" an effect on sources that connect to it.
    slot = 0;
    alGenAuxiliaryEffectSlots(1, &slot);

    // Tell the effect slot to use the loaded effect object. Note that the this
    // effectively copies the effect properties. You can modify or delete the
    // effect object afterward without affecting the effect slot.
    alAuxiliaryEffectSloti(slot, AL_EFFECTSLOT_EFFECT, (ALint)effect);
    if (alGetError() != AL_NO_ERROR)
    {
    	std::cout << "EAX effect could not be applied\n";
    }

    ALuint source;
	alGenSources((ALuint)1, &source);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error generating sources\n";
		return -1;
	}
	alSourcef(source, AL_PITCH, 1);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source pitch\n";
		return -1;
	}
	alSourcef(source, AL_GAIN, 1);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source gain\n";
		return -1;
	}
	alSource3f(source, AL_POSITION, 0, 0, 0);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source position\n";
		return -1;
	}
	alSource3f(source, AL_VELOCITY, 0, 0, 0);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting source velocity\n";
		return -1;
	}
	alSourcei(source, AL_LOOPING, AL_FALSE);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error setting looping\n";
		return -1;
	}

	ALuint buffer;
    /* Load the sound into a buffer. */
    buffer = LoadSound(sndfile);

    alSourcei(source, AL_BUFFER, buffer);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error binding source to buffer\n";
		return -1;
	}

    /* Connect the source to the effect slot. This tells the source to use the
     * effect slot 'slot', on send #0 with the AL_FILTER_NULL filter object.
     */
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, (ALint)slot, 0, AL_FILTER_NULL);
    if (alGetError() != AL_NO_ERROR)
    {
    	std::cout << "Failed to setup sound source\n";
    }

    alSourcePlay(source);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error playing source\n";
		return -1;
	}

    ALint source_state;
    alGetSourcei(source, AL_SOURCE_STATE, &source_state);
	if (error != AL_NO_ERROR)
	{
		std::cout << "Error checking source state\n";
		return -1;
	}
    while (source_state == AL_PLAYING)
    {
        alGetSourcei(source, AL_SOURCE_STATE, &source_state);
    	if (error != AL_NO_ERROR)
    	{
    		std::cout << "Error checking source state\n";
    		return -1;
    	}
    }

    alDeleteSources(1, &source);
    alDeleteBuffers(1, &buffer);
    device = alcGetContextsDevice(context);
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    return 0;
}

// returns true if the string ends with the string end, and false otherwise
bool endswith(char *string, char *end)
{
  int string_length = strlen(string);
  int end_length = strlen(end);

  if (end_length > string_length) return false;

  for (int i = 0; i < end_length; i++)
    if (string[string_length - i] != end[end_length - i])
      return false;

  return true;
}

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

int Chunk::read(FILE* in)
{
	char buf[8];
	[[maybe_unused]] auto r = fread(buf, sizeof(char), 8, in);
	if (feof(in)) return -1;
	type.code = GetNumber(buf);
	length = GetNumber(buf + 4);
	return 0;
}

void Chunk::skip(FILE* in) const
{
	int32_t nbytes;
	nbytes = (length & 1 ? length + 1 : length); // round up to even word boundary
	fseek(in, nbytes, SEEK_CUR);
}

int FourCC::read(FILE* in)
{
	[[maybe_unused]] auto r = fread((char*)&code, sizeof(FourCC), 1, in);
	return (feof(in) ? -1 : 0);
}

struct EALFileContent
{
    EALGlobalDiffractionModel gdfm; // Global Diffraction Model
    LISTENERATTRIBUTES lisa; // Default Listener Attributes
    EAXLISTENERPROPERTIES denv; // Default Environment
    SOURCEATTRIBUTES dsrc; // Default Source
    MATERIALATTRIBUTES dmat; // Default Obstacles
    std::string dfil;
    std::vector<EAXLISTENERPROPERTIES> Environments;
    std::vector<std::string> EnvironmentNames;
    std::vector<MATERIALATTRIBUTES> Materials;
    std::vector<std::string> MaterialNames;
    std::vector<SOURCEATTRIBUTES> Sources;
    std::vector<std::string> SourceNames;
};

void AnalyzeEalGlobals(FILE* in, int length, const char* field, EALFileContent* ealfile)
{
	if (std::strcmp(field, "gdfm") == 0)
	{
		EALGlobalDiffractionModel gdfm;
		[[maybe_unused]] auto r = fread(&gdfm, 12, 1, in);
		ealfile->gdfm = gdfm;
	}

	if (std::strcmp(field, "lisa") == 0)
	{
		LISTENERATTRIBUTES lisa;
		[[maybe_unused]] auto r = fread(&lisa, sizeof(LISTENERATTRIBUTES), 1, in);
		ealfile->lisa = lisa;
	}

	if (std::strcmp(field, "denv") == 0)
	{
		EAXLISTENERPROPERTIES denv;
		[[maybe_unused]] auto r = fread(&denv, sizeof(EAXLISTENERPROPERTIES), 1, in);
		ealfile->denv = denv;
	}

	if (std::strcmp(field, "dsrc") == 0)
	{
		SOURCEATTRIBUTES dsrc;
		[[maybe_unused]] auto r = fread(&dsrc, sizeof(SOURCEATTRIBUTES), 1, in);
		ealfile->dsrc = dsrc;
	}

	if (std::strcmp(field, "dfil") == 0)
	{
		char dfil[260];
		[[maybe_unused]] auto r = fread(&dfil, sizeof(char), 260, in);
		ealfile->dfil = dfil;
	}

	if (std::strcmp(field, "dmat") == 0)
	{
		MATERIALATTRIBUTES dmat;
		[[maybe_unused]] auto r = fread(&dmat, sizeof(MATERIALATTRIBUTES), 1, in);
		ealfile->dmat = dmat;
	}
}

int AnalyzeEalMaterialAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	[[maybe_unused]] auto r = fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	/* nams has a limit of 31 characters (32 bytes but with \0 ending) */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		[[maybe_unused]] auto r = fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'mata' chunk */
	/* mata has a limit of 259 characters (260 bytes but with \0 ending) */
	Chunk mata{};
	mata.read(in);
	bytesread += sizeof(Chunk);
	if (!mata.type.Equals("mata")) return bytesread;

	std::vector<MATERIALATTRIBUTES> materials(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		MATERIALATTRIBUTES mats;
		[[maybe_unused]] auto r = fread(&mats, sizeof(MATERIALATTRIBUTES), 1, in);
		if (feof(in)) return bytesread;
		bytesread += sizeof(MATERIALATTRIBUTES);
		materials.push_back(mats);
	}

	ealfile->Materials = materials;
	ealfile->MaterialNames = names;

	return bytesread;
}

int AnalyzeEalSourceAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	[[maybe_unused]] auto r = fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	/* nams has a limit of 31 characters (32 bytes but with \0 ending) */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		[[maybe_unused]] auto r = fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'fils' chunk */
	/* fils has a limit of 259 characters (260 bytes but with \0 ending) */
	Chunk fils{};
	fils.read(in);
	bytesread += sizeof(Chunk);
	if (!fils.type.Equals("fils")) return bytesread;

	std::vector<std::string> files(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		char filename[260];
		[[maybe_unused]] auto r = fread(&filename, sizeof(char), 260, in);
		if (feof(in)) return bytesread;
		bytesread += 260 * sizeof(char);
		files.push_back(filename);
	}

	/* reading 'srca' chunk */
	Chunk srca{};
	srca.read(in);
	bytesread += sizeof(Chunk);
	if (!srca.type.Equals("srca")) return bytesread;

	std::vector<SOURCEATTRIBUTES> srcattr(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		SOURCEATTRIBUTES sa;
		[[maybe_unused]] auto r = fread(&sa, sizeof(SOURCEATTRIBUTES), 1, in);
		if (feof(in)) return bytesread;
		bytesread += sizeof(SOURCEATTRIBUTES);
		srcattr.push_back(sa);
	}

	ealfile->Sources = srcattr;
	ealfile->SourceNames = names;

	return bytesread;
}

int AnalyzeEalGeometryAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	[[maybe_unused]] auto r = fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	/* nams has a limit of 31 characters (32 bytes but with \0 ending) */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		[[maybe_unused]] auto r = fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'fils' chunk */
	/* fils has a limit of 259 characters (260 bytes but with \0 ending) */
	Chunk fils{};
	fils.read(in);
	bytesread += sizeof(Chunk);
	if (!fils.type.Equals("fils")) return bytesread;

	std::vector<std::string> files(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		char filename[260];
		[[maybe_unused]] auto r = fread(&filename, sizeof(char), 260, in);
		if (feof(in)) return bytesread;
		bytesread += 260 * sizeof(char);
		files.push_back(filename);
	}

	/* reading 'gema' chunk */
	Chunk gema{};
	gema.read(in);
	bytesread += sizeof(Chunk);
	if (!gema.type.Equals("gema")) return bytesread;
	std::vector<std::vector<char>> geometries(0);

	if (quantity == 0) {
		fseek(in, gema.length, SEEK_CUR);
		bytesread += gema.length;
		geometries.push_back({});
	}
	else {
		const size_t datalen = gema.length / quantity;

		for (uint32_t i = 0; i < quantity; i++)
		{
			std::vector<char> geom(0);
			for (size_t d = 0; d < datalen; d++)
			{
				char bits;
				[[maybe_unused]] auto r = fread(&bits, sizeof(char), 1, in);
				if (feof(in)) break;
				geom.push_back(bits);
				bytesread++;
			}
			geometries.push_back(geom);
		}
	}

	return bytesread;
}

int AnalyzeEalEnvironmentAttributes(FILE* in, EALFileContent* ealfile)
{
	size_t bytesread = 0;

	/* reading 'num ' chunk */
	Chunk num{};
	num.read(in);
	bytesread += sizeof(Chunk);
	if (!num.type.Equals("num ")) return bytesread;

	uint32_t quantity = 0;
	size_t sizeUI = sizeof(uint32_t);
	[[maybe_unused]] auto r = fread(&quantity, sizeUI, 1, in);
	if (feof(in)) return bytesread;
	bytesread += sizeUI;

	/* reading 'nams' chunk */
	Chunk nams{};
	nams.read(in);
	bytesread += sizeof(Chunk);
	if (!nams.type.Equals("nams")) return bytesread;

	std::vector<std::string> names(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		char name[32];
		[[maybe_unused]] auto r = fread(&name, sizeof(char), 32, in);
		if (feof(in)) return bytesread;
		bytesread += 32 * sizeof(char);
		names.push_back(name);
	}

	/* reading 'lisp' chunk */
	Chunk lisp{};
	lisp.read(in);
	bytesread += sizeof(Chunk);
	if (!lisp.type.Equals("lisp")) return bytesread;

	std::vector<EAXLISTENERPROPERTIES> eprops(0);
	for (uint32_t i = 0; i < quantity; i++)
	{
		EAXLISTENERPROPERTIES eprop;
		[[maybe_unused]] auto r = fread(&eprop, sizeof(EAXLISTENERPROPERTIES), 1, in);
		if (feof(in)) return bytesread;
		bytesread += sizeof(EAXLISTENERPROPERTIES);
		eprops.push_back(eprop);
	}

	ealfile->Environments = eprops;
	ealfile->EnvironmentNames = names;

	return bytesread;
}

void AnalyzeEalList(FILE* in, int length, EALFileContent* ealfile)
{
	FourCC type{};
	int count = sizeof(FourCC); //  count includes code just read
	if (length > count) type.read(in);
	else return;
	int remaining = length - count;

	if (type.Equals("envp"))
		remaining -= AnalyzeEalEnvironmentAttributes(in, ealfile);
	if (type.Equals("matp"))
		remaining -= AnalyzeEalMaterialAttributes(in, ealfile);
	if (type.Equals("srcp"))
		remaining -= AnalyzeEalSourceAttributes(in, ealfile);
	if (type.Equals("gemp"))
		remaining -= AnalyzeEalGeometryAttributes(in, ealfile);

	if (remaining > 0) AnalyzeEalList(in, remaining, ealfile);
}


void AnalyzeEal(FILE* in, int length, EALFileContent* ealfile)
{
	FourCC type{};
	type.read(in);
	int count = sizeof(FourCC); //  count includes code just read
	Chunk ck{};
	bool ckanalyzed;

	while (count < length) { // reads the next 8 bytes and starts analysis if it's the next chunk
		if (ck.read(in) != 0) break;
		ckanalyzed = false;

		if (ck.Equals("gdfm"))
		{
			AnalyzeEalGlobals(in, ck.length, "gdfm", ealfile);
			ckanalyzed = true;
		}

		if (ck.Equals("lisa"))
		{
			AnalyzeEalGlobals(in, ck.length, "lisa", ealfile);
			ckanalyzed = true;
		}

		if (ck.Equals("denv"))
		{
			AnalyzeEalGlobals(in, ck.length, "denv", ealfile);
			ckanalyzed = true;
		}

		if (ck.Equals("dsrc"))
		{
			AnalyzeEalGlobals(in, ck.length, "dsrc", ealfile);
			ckanalyzed = true;
		}

		if (ck.Equals("dfil"))
		{
			AnalyzeEalGlobals(in, ck.length, "dfil", ealfile);
			ckanalyzed = true;
		}

		if (ck.Equals("dmat"))
		{
			AnalyzeEalGlobals(in, ck.length, "dmat", ealfile);
			ckanalyzed = true;
		}

		if (ck.Equals("LIST"))
		{
			AnalyzeEalList(in, ck.length, ealfile);
			ckanalyzed = true;
		}
		if (!ckanalyzed) { ck.skip(in); } // Chunk has not been read, so skip the data

		count += ck.length + 8;
		if (ck.length & 1) count++; // account for extra odd byte
	}
}


static int ReadEalEnvironments(const char* filename, EALFileContent* ealfile)
{
	FILE* in;
	in = fopen(filename,"rb");
	if (!in)
	{
		std::cout << filename << " not found\n";
		return -1;
	}
	Chunk hd{};
	hd.read(in);
	if (hd.Equals("RIFF")) AnalyzeEal(in, hd.length, ealfile);
	fclose(in);
	return 0;
}

int main(int argc, char **argv)
{
	const char* sndfile = "sounds.wav";
	const char* ealfile = "";
	for (int i = 0; i < argc; i++)
	{
		if (endswith(argv[i], (char*)".wav")) sndfile = argv[i];
		if (endswith(argv[i], (char*)".eal")) ealfile = argv[i];
	}

	std::cout << "Playing " << sndfile << " without EAX effects\n";
	PlaySound(sndfile);

	if (strlen(ealfile) < 4)
	{
		std::cout << "No .eal file specified to load EAX effects from\n";
		return -1;
	}
	EALFileContent ealdata;
	ReadEalEnvironments(ealfile, &ealdata);

    if(!alcIsExtensionPresent(alcGetContextsDevice(alcGetCurrentContext()), "ALC_EXT_EFX"))
    {
    	std::cout << "EFX no supported on this system\n";
        return 1;
    }

	for (size_t i = 0; i < ealdata.Environments.size(); i++)
	{
		std::cout << "Playing " << sndfile << " with " << ealfile << " environment effects: " << ealdata.EnvironmentNames.at(i) << "\n";
		PlaySoundWithEffects(sndfile, ealdata.Environments.at(i));
	}

	return 0;
}
