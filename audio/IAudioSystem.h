#ifndef IAUDIOSYSTEM_H
#define IAUDIOSYSTEM_H

#include "math/Vector.h"
#include "util/refcounted.h"

class ISoundSource;

typedef uint effectId_t;

#define EFFECT_ID_NONE			(0)
#define SOUND_EFX_SLOTS			(2)

//-----------------------------------------------------------------
// Audio source interface

class IEqAudioSource : public RefCountedObject
{
public:
	enum ESoundSourceUpdate
	{
		UPDATE_POSITION = (1 << 0),
		UPDATE_VELOCITY = (1 << 1),
		UPDATE_VOLUME = (1 << 2),
		UPDATE_PITCH = (1 << 3),
		UPDATE_REF_DIST = (1 << 4),
		UPDATE_AIRABSORPTION = (1 << 5),
		UPDATE_RELATIVE = (1 << 6),
		UPDATE_STATE = (1 << 7),
		UPDATE_LOOPING = (1 << 8),
		UPDATE_EFFECTSLOT = (1 << 9),

		UPDATE_DO_REWIND = (1 << 16),
		UPDATE_RELEASE_ON_STOP = (1 << 17)
	};

	enum ESourceState
	{
		STOPPED = 0,
		PLAYING,
		PAUSED
	};

	struct Params
	{
		Vector3D			position;
		Vector3D			velocity;
		float				volume;					// [0.0, 1.0]
		float				pitch;					// [0.0, 100.0]
		float				referenceDistance;
		float				rolloff;
		float				airAbsorption;
		ESourceState		state;
		int					effectSlot;
		bool				relative;
		bool				looping;
		bool				releaseOnStop;
		int					id;						// read-only
	};

	typedef int				(*UpdateCallback)(void* obj, Params& params);		// returns EVoiceUpdateFlags

	virtual ~IEqAudioSource() {}

	virtual void			Setup(int typeId, ISoundSource* sample, UpdateCallback fnCallback = nullptr, void* callbackObject = nullptr) = 0;
	virtual void			Release() = 0;

	// full scale
	virtual void			GetParams(Params& params) = 0;
	virtual void			UpdateParams(Params params, int mask) = 0;

	// atomic
	virtual ESourceState	GetState() const = 0;
	virtual bool			IsLooping() const = 0;
};


//-----------------------------------------------------------------
// Audio system interface

// Audio system, controls voices
class IAudioSystem
{
public:
	virtual ~IAudioSystem() {}

	virtual void				Init() = 0;
	virtual void				Shutdown() = 0;

	virtual IEqAudioSource*		CreateSource() = 0;
	virtual void				DestroySource(IEqAudioSource* source) = 0;

	virtual void				Update() = 0;

	virtual void				StopAllSounds(int chanType = -1, void* callbackObject = nullptr) = 0;
	virtual void				PauseAllSounds(int chanType = -1, void* callbackObject = nullptr) = 0;
	virtual void				ResumeAllSounds(int chanType = -1, void* callbackObject = nullptr) = 0;

	virtual void				SetMasterVolume(float value) = 0;

	// sets listener properties
	virtual void				SetListener(const Vector3D& position,
											const Vector3D& velocity,
											const Vector3D& forwardVec,
											const Vector3D& upVec) = 0;

	// gets listener properties
	virtual void				GetListener(Vector3D& position, Vector3D& velocity) = 0;

	// loads sample source data
	virtual ISoundSource*		LoadSample(const char* filename) = 0;
	virtual void				FreeSample(ISoundSource* sample) = 0;

	// finds the effect. May return EFFECT_ID_NONE
	virtual effectId_t			FindEffect(const char* name) const = 0;

	// sets the new effect
	virtual void				SetEffect(int slot, effectId_t effect) = 0;
};

extern IAudioSystem* g_audioSystem;

#endif // IAUDIOSYSTEM_H