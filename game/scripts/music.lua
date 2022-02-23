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

local CurrentMusicBank = nil
local CurrentMusicSource = nil

function Music.Start(gameId, musicId)
	ReleaseSoundbank("music")
	CurrentMusicBank = LoadSoundbank("music", GameMusic[gameId][musicId])

	-- create audio source
	if CurrentMusicSource == nil then
		CurrentMusicSource = audio:CreateSource()
	end
	CurrentMusicSource:Setup(0, CurrentMusicBank.Drive)

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