local audio = engine.Audio

SoundBanks = {}

function LoadSoundbank( key, bankTable )

	if SoundBanks[key] ~= nil then
		MsgWarning("LoadSoundbank - bank '".. key.. "' already loaded!\n")
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
		MsgWarning("ReleaseSoundbank - bank '".. key.. "' is not loaded!\n")
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
	Hit_Car_3 	= "voices/Bank_0/0.wav", --"voices2/Bank_1/6.wav",
	
	SkidLoop	= "voices2/Bank_1/7.wav",
	Thunder 	= "voices2/Bank_1/8.wav",
	
	WetLoop 	= "voices2/Bank_1/9.wav",
	GravelLoop 	= "voices2/Bank_1/10.wav",
	AlleyLoop 	= "voices2/Bank_1/11.wav",
}