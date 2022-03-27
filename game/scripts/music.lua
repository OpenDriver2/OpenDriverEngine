local audio = engine.Audio

local BankDriver1 = {
	{
		Drive 		= "music/d1music_song0.xm", 	-- "music/nyc_day.ogg",
		Chase 		= "music/d1music_song0.xm#11" 	-- "music/nyc_day_esc.ogg",
	},
	{
		Drive 		= "music/d1music_song1.xm", 	-- "music/nyc_night.ogg",
		Chase 		= "music/d1music_song1.xm#12" 	-- "music/nyc_night_esc.ogg",
	},
	{
		Drive 		= "music/d1music_song2.xm", 	-- "music/la_day.ogg",
		Chase 		= "music/d1music_song2.xm#11" 	-- "music/la_day_esc.ogg",
	},
	{
		Drive 		= "music/d1music_song3.xm", 	-- "music/miami_day.ogg",
		Chase 		= "music/d1music_song3.xm#11" 	-- "music/miami_day_esc.ogg",
	},
	{
		Drive 		= "music/d1music_song4.xm", 	-- "music/miami_night.ogg",
		Chase 		= "music/d1music_song4.xm#8" 	-- "music/miami_night_esc.ogg",
	},
	{
		Drive 		= "music/d1music_song5.xm", 	-- "music/frisco_day.ogg",
		Chase 		= "music/d1music_song5.xm#11" 	-- "music/frisco_day_esc.ogg",
	},
	{
		Drive 		= "music/d1music_song6.xm", 	-- "music/frisco_night.ogg",
		Chase 		= "music/d1music_song6.xm#15" 	-- "music/frisco_night_esc.ogg",
	},
	{
		Drive 		= "music/d1music_song7.xm", 	-- "music/la_night.ogg",
		Chase 		= "music/d1music_song7.xm#12" 	-- "music/la_night_esc.ogg",
	}
}

local BankDriver2 = {
	{
		Drive 		= "music/d2music_song0.xm", 	-- "music/Driver1_drive.ogg",
		Chase 		= "music/d2music_song0.xm#16" 	-- "music/Driver1_chase.ogg",
	},
	{
		Drive 		= "music/d2music_song1.xm", 	-- "music/Driver2_drive.ogg",
		Chase 		= "music/d2music_song1.xm#11" 	-- "music/Driver2_chase.ogg",
	},
	{
		Drive 		= "music/d2music_song2.xm", 	--"music/Driver3_drive.ogg",
		Chase 		= "music/d2music_song2.xm#7" 	--"music/Driver3_chase.ogg",
	},
	{
		Drive 		= "music/d2music_song3.xm", 	-- "music/Driver4_drive.ogg",
		Chase 		= "music/d2music_song3.xm#18" 	-- "music/Driver4_chase.ogg",
	},
	{
		Drive 		= "music/d2music_song4.xm", 	-- "music/Driver5_drive.ogg",
		Chase 		= "music/d2music_song4.xm#12" 	-- "music/Driver5_chase.ogg",
	},
	{
		Drive 		= "music/d2music_song5.xm", 	-- "music/Driver6_drive.ogg",
		Chase 		= "music/d2music_song5.xm#9" 	-- "music/Driver6_chase.ogg",
	},
	{
		Drive 		= "music/d2music_song6.xm", 	-- "music/Driver7_drive.ogg",
		Chase 		= "music/d2music_song6.xm#8" 	-- "music/Driver7_chase.ogg",
	},
	{
		Drive 		= "music/d2music_song7.xm", 	-- "music/Driver8_drive.ogg",
		Chase 		= "music/d2music_song7.xm#10" 	-- "music/Driver8_chase.ogg",
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

	CurrentMusicSource:Setup(ChannelId.Music, sample)

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