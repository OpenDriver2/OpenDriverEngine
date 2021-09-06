local audio = engine.Audio

SoundBanks = {}

function LoadSoundbank( key, bankTable )

	if SoundBanks[key] ~= nil then
		MsgWarning("LoadSoundbank - bank '".. key.. "' already loaded!")
		return SoundBanks[key]
	end

	local bank = {}
	
	for k,v in pairs(bankTable) do
		local sample = audio:LoadSample(v)
		bank[k] = sample
	end
	
	SoundBanks[key] = bank
	
	return bank
end

function ReleaseSoundbank( key )

	if SoundBanks[key] == nil then
		MsgWarning("ReleaseSoundbank - bank '".. key.. "' is not loaded!")
		return
	end

	local bank = SoundBanks[key]

	for k,v in pairs(bank) do
		audio:FreeSample(v)
	end
	SoundBanks[key] = nil
end

-----------------------------------------------------

SBK_Permanent = {
	Hit_Box 	= "voices2/Bank_1/0.wav",
	Hit_Cone 	= "voices2/Bank_1/1.wav",
	Hit_Barrel 	= "voices2/Bank_1/2.wav",
	Hit_Fence 	= "voices2/Bank_1/3.wav",
	
	Hit_Car_1 	= "voices2/Bank_1/4.wav",
	Hit_Car_2 	= "voices2/Bank_1/5.wav",
	Hit_Car_3a	= "voices2/Bank_1/6.wav",
	Hit_Car_3b 	= "voices/Bank_0/0.wav",
	
	SkidLoop	= "voices2/Bank_1/7.wav",
	Thunder 	= "voices2/Bank_1/8.wav",
	
	WetLoop 	= "voices2/Bank_1/9.wav",
	GravelLoop 	= "voices2/Bank_1/10.wav",
	AlleyLoop 	= "voices2/Bank_1/11.wav",
}

--[[ Usage
	-- load bank
	local spermBank = LoadSoundbank("permanent", SBK_Permanent)
	
	-- create audio source
	local audioSource = audio:CreateSource()
	audioSource:Setup(0, spermBank.AlleyLoop)
	
	-- modify current parameters
	-- you cannot modify audioSource.params itself
	-- this will copy into new parameters instance
	local sparams = audioSource.params
	
	sparams.position = vec.vec3(0,0,0)
	sparams.state = SoundState.Playing
	sparams.looping = true
	sparams.relative = true
	sparams.volume = 0.5
	sparams.pitch = 0.5
	
	-- set new parameters and as result it will play
	audioSource.params = sparams
]]