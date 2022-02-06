#ifndef AUDIOSYSTEMAL_H
#define AUDIOSYSTEMAL_H

#include "IAudioSystem.h"
#include "source/snd_source.h"

//-----------------------------------------------------------------

#define STREAM_BUFFER_COUNT		(4)
#define STREAM_BUFFER_SIZE		(1024*8) // 8 kb

//-----------------------------------------------------------------

// Audio system, controls voices
class CAudioSystemAL : public IAudioSystem
{
	friend class CAudioSourceAL;
public:
	CAudioSystemAL();
	~CAudioSystemAL();

	void						Init();
	void						Shutdown();

	IAudioSource*				CreateSource();
	void						DestroySource(IAudioSource* source);

	void						Update();

	void						StopAllSounds(int chanType = -1, void* callbackObject = nullptr);
	void						PauseAllSounds(int chanType = -1, void* callbackObject = nullptr);
	void						ResumeAllSounds(int chanType = -1, void* callbackObject = nullptr);

	void						SetMasterVolume(float value);

	// sets listener properties
	void						SetListener(const Vector3D& position,
											const Vector3D& velocity,
											const Vector3D& forwardVec,
											const Vector3D& upVec);

	// gets listener properties
	void						GetListener(Vector3D& position, Vector3D& velocity);

	// loads sample source data
	ISoundSource*				LoadSample(const char* filename);
	void						FreeSample(ISoundSource* sample);

	// finds the effect. May return EFFECT_ID_NONE
	effectId_t					FindEffect(const char* name) const;

	// sets the new effect
	void						SetEffect(int slot, effectId_t effect);

private:
	struct sndEffect_t
	{
		char		name[32];
		ALuint		nAlEffect;
	};

	// bool			CreateALEffect(const char* pszName, kvkeybase_t* pSection, sndEffect_t& effect);
	void			SuspendSourcesWithSample(ISoundSource* sample);

	bool			InitContext();
	void			InitEffects();

	void			DestroyContext();
	void			DestroyEffects();

	Array<CRefPointer<CAudioSourceAL*>>	m_sources;	// tracked sources
	Array<ISoundSource*>					m_samples;
	Array<sndEffect_t>						m_effects;

	ALCcontext*								m_ctx;
	ALCdevice*								m_dev;

	ALuint									m_effectSlots[SOUND_EFX_SLOTS];
	int										m_currEffectSlotIdx;

	sndEffect_t*							m_currEffect;

	bool									m_noSound;
};

//-----------------------------------------------------------------
// Sound source

class CAudioSourceAL : public IAudioSource
{
	friend class CAudioSystemAL;
public:
	CAudioSourceAL(CAudioSystemAL* owner);
	CAudioSourceAL(int typeId, ISoundSource* sample, UpdateCallback fnCallback, void* callbackObject);
	~CAudioSourceAL();

	void					Setup(int chanType, ISoundSource* sample, UpdateCallback fnCallback = nullptr, void* callbackObject = nullptr);
	void					Release();

	// full scale
	void					GetParams(Params& params);
	void					UpdateParams(Params params, int mask = 0);

	// atomic
	State					GetState() const { return m_state; }
	bool					IsLooping() const { return m_looping; }

protected:
	void					Ref_DeleteObject();

	bool					QueueStreamChannel(ALuint buffer);
	void					SetupSample(ISoundSource* sample);

	void					InitSource();

	void					EmptyBuffers();
	bool					DoUpdate();

	CAudioSystemAL*			m_owner;

	ALuint					m_buffers[STREAM_BUFFER_COUNT];
	ISoundSource*			m_sample;

	UpdateCallback			m_callback;
	void*					m_callbackObject;

	ALuint					m_source;
	int						m_streamPos;
	State					m_state;

	int						m_chanType;
	bool					m_releaseOnStop;
	bool					m_forceStop;
	bool					m_looping;
};

#endif // AUDIOSYSTEMAL_H