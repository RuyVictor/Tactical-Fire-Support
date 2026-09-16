// Editable settings loaded by the authoritative server when support is requested.
[BaseContainerProps(configRoot: true)]
class TFS_ArtillerySettings
{
	static const ResourceName CONFIG_RESOURCE = "{6A67F8C3D91C24E1}Configs/TFS_ArtillerySettings.conf";
	static const int DEFAULT_COOLDOWN_SECONDS = 60;
	static const float DEFAULT_BARRAGE_DELAY_SECONDS = 10.0;
	static const float DEFAULT_SPLASH_LEAD_SECONDS = 5.0;

	[Attribute(
		"60",
		UIWidgets.EditBox,
		"Cooldown in seconds for each player. Set to 0 to disable the cooldown."
	)]
	int m_iCooldownSeconds;

	[Attribute(
		"10",
		UIWidgets.EditBox,
		"Seconds between accepting the request and starting the barrage. Set to 0 for immediate fire."
	)]
	float m_fBarrageDelaySeconds;

	[Attribute(
		"5",
		UIWidgets.EditBox,
		"How many seconds before the barrage the SPLASH radio cue is sent."
	)]
	float m_fSplashLeadSeconds;

	[Attribute("1", UIWidgets.CheckBox, "Show FIRE MISSION, SHOT and SPLASH cues to the requesting player.")]
	bool m_bEnableRadioCues;

	[Attribute("1", UIWidgets.CheckBox, "Play a discreet UI cue together with each radio message.")]
	bool m_bPlayRadioCueSound;

	//------------------------------------------------------------------------------------------------
	int GetCooldownSeconds()
	{
		if (m_iCooldownSeconds < 0)
			return 0;

		return m_iCooldownSeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetBarrageDelaySeconds()
	{
		if (m_fBarrageDelaySeconds < 0)
			return 0;

		return m_fBarrageDelaySeconds;
	}

	//------------------------------------------------------------------------------------------------
	float GetSplashLeadSeconds()
	{
		if (m_fSplashLeadSeconds < 0)
			return 0;

		return m_fSplashLeadSeconds;
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
	static TFS_ArtillerySettings Load()
	{
		return SCR_ConfigHelperT<TFS_ArtillerySettings>.GetConfigObject(CONFIG_RESOURCE);
	}
}
