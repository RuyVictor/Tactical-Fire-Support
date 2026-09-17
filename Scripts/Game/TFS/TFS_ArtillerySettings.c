// Editable settings loaded by the authoritative server when support is requested.
[BaseContainerProps(configRoot: true)]
class TFS_ArtillerySettings
{
	static const ResourceName CONFIG_RESOURCE = "{6A67F8C3D91C24E1}Configs/TFS_ArtillerySettings.conf";
	static const int DEFAULT_COOLDOWN_SECONDS = 60;
	static const float DEFAULT_INITIAL_MISSION_DELAY_SECONDS = 2.0;
	static const float DEFAULT_BARRAGE_DELAY_SECONDS = 20.0;
	static const float DEFAULT_SHOT_CUE_DELAY_SECONDS = 3.75;
	static const float DEFAULT_SPLASH_LEAD_SECONDS = 12.0;
	static const float DEFAULT_RADIO_INTER_CUE_DELAY_SECONDS = 1.25;
	static const float DEFAULT_ALERT_TONE_TO_DANGER_CLOSE_DELAY_SECONDS = 1.25;
	static const float DEFAULT_DANGER_CLOSE_TO_CLEAR_AREA_DELAY_SECONDS = 1.25;
	static const float DEFAULT_MARKER_POST_IMPACT_LIFETIME_SECONDS = 0.0;
	static const float DEFAULT_DISTANT_FIRE_OFFSET_METERS = 2500.0;
	static const float DEFAULT_DANGER_CLOSE_RADIUS_METERS = 300.0;
	static const int DEFAULT_AUDIO_RANGE_PROFILE = 1;

	[Attribute(
		"60",
		UIWidgets.EditBox,
		"Cooldown in seconds for each player. Set to 0 to disable the cooldown."
	)]
	int m_iCooldownSeconds;

	[Attribute(
		"2",
		UIWidgets.EditBox,
		"Initial quiet delay in seconds after selecting the target, before mission confirmation and the fire sequence begin."
	)]
	float m_fInitialMissionDelaySeconds;

	[Attribute(
		"20",
		UIWidgets.EditBox,
		"Seconds between mission confirmation and starting the strike. The initial mission delay is added before this interval."
	)]
	float m_fBarrageDelaySeconds;

	[Attribute(
		"3.75",
		UIWidgets.EditBox,
		"Seconds after mission confirmation before the SHOT cue and battery discharge. Clamped to the strike delay."
	)]
	float m_fShotCueDelaySeconds;

	[Attribute(
		"12",
		UIWidgets.EditBox,
		"How many seconds before the strike the ROUNDS INCOMING and nearby DANGER CLOSE cues are sent."
	)]
	float m_fSplashLeadSeconds;

	[Attribute(
		"1.25",
		UIWidgets.EditBox,
		"Quiet pause in seconds between queued radio voice clips. Clamped from 0 to 5 seconds."
	)]
	float m_fRadioInterCueDelaySeconds;

	[Attribute(
		"1.25",
		UIWidgets.EditBox,
		"Quiet pause after the alert tone before DANGER CLOSE. Clamped from 0 to 5 seconds."
	)]
	float m_fAlertToneToDangerCloseDelaySeconds;

	[Attribute(
		"1.25",
		UIWidgets.EditBox,
		"Quiet pause after DANGER CLOSE before CLEAR AREA. Clamped from 0 to 5 seconds."
	)]
	float m_fDangerCloseToClearAreaDelaySeconds;

	[Attribute(
		"0",
		UIWidgets.EditBox,
		"Seconds to keep the faction impact marker visible after detonation. Clamped from 0 to 60 seconds."
	)]
	float m_fMarkerPostImpactLifetimeSeconds;

	[Attribute("1", UIWidgets.CheckBox, "Show FIRE MISSION, SHOT and ROUNDS INCOMING cues, plus DANGER CLOSE to nearby allies.")]
	bool m_bEnableRadioCues;

	[Attribute("1", UIWidgets.CheckBox, "Play the custom English radio voice assigned to each mission stage.")]
	bool m_bPlayRadioCueSound;

	[Attribute("1", UIWidgets.CheckBox, "Show a faction-only map marker over the impact point while the fire mission is active.")]
	bool m_bShowImpactMarker;

	[Attribute(
		"300",
		UIWidgets.EditBox,
		"Allied players within this distance of the target receive the DANGER CLOSE voice and alert tone. Clamped from 0 to 2000 metres."
	)]
	float m_fDangerCloseRadiusMeters;

	[Attribute("1", UIWidgets.CheckBox, "Play the distant 240 mm battery discharge for connected players.")]
	bool m_bEnableDistantFireSound;

	[Attribute(
		"2500",
		UIWidgets.EditBox,
		"Virtual distance in metres from the requester to the off-map battery sound. Clamped from 0 to 10000."
	)]
	float m_fDistantFireOffsetMeters;

	[Attribute(
		"1",
		UIWidgets.EditBox,
		"Audio range profile: 0 = standard, 1 = extended, 2 = extreme. Applies to battery fire, incoming and custom impact layers."
	)]
	int m_iAudioRangeProfile;

	//------------------------------------------------------------------------------------------------
	int GetCooldownSeconds()
	{
		if (m_iCooldownSeconds < 0)
			return 0;

		return m_iCooldownSeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetInitialMissionDelaySeconds()
	{
		if (m_fInitialMissionDelaySeconds < 0)
			return 0;

		if (m_fInitialMissionDelaySeconds > 30.0)
			return 30.0;

		return m_fInitialMissionDelaySeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetBarrageDelaySeconds()
	{
		if (m_fBarrageDelaySeconds < 0)
			return 0;

		return m_fBarrageDelaySeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetShotCueDelaySeconds()
	{
		if (m_fShotCueDelaySeconds < 0)
			return 0;

		return m_fShotCueDelaySeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetSplashLeadSeconds()
	{
		if (m_fSplashLeadSeconds < 0)
			return 0;

		return m_fSplashLeadSeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetRadioInterCueDelaySeconds()
	{
		if (m_fRadioInterCueDelaySeconds < 0)
			return 0;

		if (m_fRadioInterCueDelaySeconds > 5.0)
			return 5.0;

		return m_fRadioInterCueDelaySeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetAlertToneToDangerCloseDelaySeconds()
	{
		if (m_fAlertToneToDangerCloseDelaySeconds < 0)
			return 0;

		if (m_fAlertToneToDangerCloseDelaySeconds > 5.0)
			return 5.0;

		return m_fAlertToneToDangerCloseDelaySeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetDangerCloseToClearAreaDelaySeconds()
	{
		if (m_fDangerCloseToClearAreaDelaySeconds < 0)
			return 0;

		if (m_fDangerCloseToClearAreaDelaySeconds > 5.0)
			return 5.0;

		return m_fDangerCloseToClearAreaDelaySeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetMarkerPostImpactLifetimeSeconds()
	{
		if (m_fMarkerPostImpactLifetimeSeconds < 0)
			return 0;

		if (m_fMarkerPostImpactLifetimeSeconds > 60.0)
			return 60.0;

		return m_fMarkerPostImpactLifetimeSeconds;
	}

	//------------------------------------------------------------------------------------------------
	bool AreRadioCuesEnabled()
	{
		return m_bEnableRadioCues;
	}

	//------------------------------------------------------------------------------------------------
	bool ShouldPlayRadioCueSound()
	{
		return m_bPlayRadioCueSound;
	}

	//------------------------------------------------------------------------------------------------
	bool ShouldShowImpactMarker()
	{
		return m_bShowImpactMarker;
	}

	//------------------------------------------------------------------------------------------------
	float GetDangerCloseRadiusMeters()
	{
		if (m_fDangerCloseRadiusMeters < 0)
			return 0;

		if (m_fDangerCloseRadiusMeters > 2000.0)
			return 2000.0;

		return m_fDangerCloseRadiusMeters;
	}

	//------------------------------------------------------------------------------------------------
	bool IsDistantFireSoundEnabled()
	{
		return m_bEnableDistantFireSound;
	}

	//------------------------------------------------------------------------------------------------
	float GetDistantFireOffsetMeters()
	{
		if (m_fDistantFireOffsetMeters < 0)
			return 0;

		if (m_fDistantFireOffsetMeters > 10000.0)
			return 10000.0;

		return m_fDistantFireOffsetMeters;
	}

	//------------------------------------------------------------------------------------------------
	int GetAudioRangeProfile()
	{
		if (m_iAudioRangeProfile < 0)
			return 0;

		if (m_iAudioRangeProfile > 2)
			return 2;

		return m_iAudioRangeProfile;
	}

	//------------------------------------------------------------------------------------------------
	static TFS_ArtillerySettings Load()
	{
		return SCR_ConfigHelperT<TFS_ArtillerySettings>.GetConfigObject(CONFIG_RESOURCE);
	}
}
