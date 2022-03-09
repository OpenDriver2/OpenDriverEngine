local audio = engine.Audio

local BankDriver1 = {
	{
		Drive 		= "music/nyc_day.ogg",
		Chase 		= "music/nyc_day_esc.ogg",
	},
	{
		Drive 		= "music/nyc_night.ogg",
		Chase 		= "music/nyc_night_esc.ogg",
	},
	{
		Drive 		= "music/la_day.ogg",
		Chase 		= "music/la_day_esc.ogg",
	},
	{
		Drive 		= "music/miami_day.ogg",
		Chase 		= "music/miami_day_esc.ogg",
	},
	{
		Drive 		= "music/miami_night.ogg",
		Chase 		= "music/miami_night_esc.ogg",
	},
	{
		Drive 		= "music/frisco_day.ogg",
		Chase 		= "music/frisco_day_esc.ogg",
	},
	{
		Drive 		= "music/frisco_night.ogg",
		Chase 		= "music/frisco_night_esc.ogg",
	},
	{
		Drive 		= "music/la_night.ogg",
		Chase 		= "music/la_night_esc.ogg",
	}
}

local BankDriver2 = {
	{
		Drive 		= "music/Driver1_drive.ogg",
		Chase 		= "music/Driver1_chase.ogg",
	},
	{
		Drive 		= "music/Driver2_drive.ogg",
		Chase 		= "music/Driver2_chase.ogg",
	},
	{
		Drive 		= "music/Driver3_drive.ogg",
		Chase 		= "music/Driver3_chase.ogg",
	},
	{
		Drive 		= "music/Driver4_drive.ogg",
		Chase 		= "music/Driver4_chase.ogg",
	},
	{
		Drive 		= "music/Driver5_drive.ogg",
		Chase 		= "music/Driver5_chase.ogg",
	},
	{
		Drive 		= "music/Driver6_drive.ogg",
		Chase 		= "music/Driver6_chase.ogg",
	},
	{
		Drive 		= "music/Driver7_drive.ogg",
		Chase 		= "music/Driver7_chase.ogg",
	},
	{
		Drive 		= "music/Driver8_drive.ogg",
		Chase 		= "music/Driver8_chase.ogg",
	}
}

local GameMusic = {
	BankDriver1, BankDriver2
}

Music = {}

local CurrentMusicBankSamples = nil
local CurrentMusicSource = nil

local CurrentMusicBank = nil

function Music.Init(gameId, musicId)
	local newBank = GameMusic[gameId][musicId]
	if CurrentMusicBank ~= newBank then
		ReleaseSoundbank("music")
		CurrentMusicBankSamples = LoadSoundbank("music", newBank)
		CurrentMusicBank = newBank
	end
end

function Music.FunkUpDaBGMTunez(funk)
	if funk == nil then
		funk = false
	end
	-- create audio source
	if CurrentMusicSource == nil then
		CurrentMusicSource = audio:CreateSource()
	end
	local sample = if_then_else(funk, CurrentMusicBankSamples.Chase, CurrentMusicBankSamples.Drive)
	if sample == nil then
		MsgError("Can't start music, no sample!")
		return
	end

	CurrentMusicSource:Setup(0, sample)

	local sparams = CurrentMusicSource.params
	
	sparams.position = vec.vec3(0,0,0)
	sparams.state = SoundState.Playing
	sparams.looping = true
	sparams.relative = true
	sparams.volume = 0.3
	sparams.pitch = 1
	
	-- set new parameters and as result it will play
	CurrentMusicSource.params = sparams
end