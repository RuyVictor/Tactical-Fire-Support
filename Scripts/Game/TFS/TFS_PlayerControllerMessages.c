enum TFS_ERadioCue
{
	NONE = 0,
	REQUEST_RECEIVED,
	SHOT,
	ROUNDS_INCOMING,
	ALERT_TONE,
	DANGER_CLOSE,
	CLEAR_AREA
}

enum TFS_ERadioLanguage
{
	ENGLISH = 0,
	RUSSIAN
}

// Owner-targeted feedback for server-side validation and confirmation.
modded class SCR_PlayerController
{
	static const ResourceName TFS_ARTILLERY_FIRE_SOUND = "{6A67F8C6F2400001}Sounds/Artillery/TFS_ArtilleryFire.acp";
	static const ResourceName TFS_RADIO_SOUND = "{6A67F8C6A2500001}Sounds/Radio/TFS_ArtilleryRadio.acp";
	static const string TFS_ARTILLERY_FIRE_EVENT_STANDARD = "TFS_ARTILLERY_FIRE_STANDARD";
	static const string TFS_ARTILLERY_FIRE_EVENT_EXTENDED = "TFS_ARTILLERY_FIRE_EXTENDED";
	static const string TFS_ARTILLERY_FIRE_EVENT_EXTREME = "TFS_ARTILLERY_FIRE_EXTREME";

	protected ref array<int> m_aTFSRadioCueQueue = {};
	protected ref array<int> m_aTFSRadioLanguageQueue = {};
	protected ref array<int> m_aTFSRadioGapQueue = {};
	protected bool m_bTFSRadioCuePlaying;

	//------------------------------------------------------------------------------------------------
	void TFS_ShowOwnerMessage(string title, string message, float duration)
	{
		if (!Replication.IsRunning())
		{
			TFS_DoShowOwnerMessage(title, message, duration);
			return;
		}

		Rpc(TFS_DoShowOwnerMessage, title, message, duration);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void TFS_DoShowOwnerMessage(string title, string message, float duration)
	{
		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ShowCustomHint(message, title, duration, true);
	}

	//------------------------------------------------------------------------------------------------
	void TFS_ShowOwnerRadioMessage(
		string title,
		string message,
		float duration,
		bool playSound,
		int radioCue,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		if (!Replication.IsRunning())
		{
			TFS_DoShowOwnerRadioMessage(
				title,
				message,
				duration,
				playSound,
				radioCue,
				radioLanguage,
				radioTimingSeconds
			);
			return;
		}

		Rpc(
			TFS_DoShowOwnerRadioMessage,
			title,
			message,
			duration,
			playSound,
			radioCue,
			radioLanguage,
			radioTimingSeconds
		);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void TFS_DoShowOwnerRadioMessage(
		string title,
		string message,
		float duration,
		bool playSound,
		int radioCue,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		if (playSound)
			TFS_QueueRadioCue(
				radioCue,
				radioLanguage,
				radioTimingSeconds
			);

		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ShowCustomHint(message, title, duration, true);
	}

	//------------------------------------------------------------------------------------------------
	// Custom voices and their configured silence gaps are serialized locally. DANGER CLOSE jumps
	// ahead of queued routine traffic without cutting off a voice that is already playing.
	protected void TFS_QueueRadioCue(
		int radioCue,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		if (radioCue <= TFS_ERadioCue.NONE)
			return;

		int gapMilliseconds = TFS_GetRadioGapMilliseconds(radioTimingSeconds[0]);
		int alertToneGapMilliseconds = TFS_GetRadioGapMilliseconds(radioTimingSeconds[1]);
		int dangerCloseGapMilliseconds = TFS_GetRadioGapMilliseconds(radioTimingSeconds[2]);

		if (radioCue == TFS_ERadioCue.DANGER_CLOSE)
		{
			// The alert tone is deliberately separate from the voice. Its configured gap creates
			// a short attention pause before the spoken danger-close order.
			m_aTFSRadioCueQueue.InsertAt(TFS_ERadioCue.ALERT_TONE, 0);
			m_aTFSRadioLanguageQueue.InsertAt(radioLanguage, 0);
			m_aTFSRadioGapQueue.InsertAt(alertToneGapMilliseconds, 0);
			m_aTFSRadioCueQueue.InsertAt(TFS_ERadioCue.DANGER_CLOSE, 1);
			m_aTFSRadioLanguageQueue.InsertAt(radioLanguage, 1);
			m_aTFSRadioGapQueue.InsertAt(dangerCloseGapMilliseconds, 1);
			m_aTFSRadioCueQueue.InsertAt(TFS_ERadioCue.CLEAR_AREA, 2);
			m_aTFSRadioLanguageQueue.InsertAt(radioLanguage, 2);
			m_aTFSRadioGapQueue.InsertAt(gapMilliseconds, 2);
		}
		else
		{
			m_aTFSRadioCueQueue.Insert(radioCue);
			m_aTFSRadioLanguageQueue.Insert(radioLanguage);
			m_aTFSRadioGapQueue.Insert(gapMilliseconds);
		}

		if (!m_bTFSRadioCuePlaying)
			TFS_PlayNextRadioCue();
	}

	//------------------------------------------------------------------------------------------------
	protected int TFS_GetRadioGapMilliseconds(float delaySeconds)
	{
		if (delaySeconds < 0)
			delaySeconds = 0;
		else if (delaySeconds > 5.0)
			delaySeconds = 5.0;

		return delaySeconds * 1000;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_PlayNextRadioCue()
	{
		if (m_aTFSRadioCueQueue.IsEmpty())
		{
			m_bTFSRadioCuePlaying = false;
			return;
		}

		m_bTFSRadioCuePlaying = true;
		int radioCue = m_aTFSRadioCueQueue[0];
		int radioLanguage = m_aTFSRadioLanguageQueue[0];
		int gapMilliseconds = m_aTFSRadioGapQueue[0];
		m_aTFSRadioCueQueue.Remove(0);
		m_aTFSRadioLanguageQueue.Remove(0);
		m_aTFSRadioGapQueue.Remove(0);

		string eventName = TFS_GetRadioEventName(radioCue, radioLanguage);
		if (!eventName.IsEmpty())
		{
			vector soundTransform[4];
			Math3D.MatrixIdentity4(soundTransform);
			AudioSystem.PlayEvent(TFS_RADIO_SOUND, eventName, soundTransform);
		}

		GetGame().GetCallqueue().CallLater(
			TFS_OnRadioCueFinished,
			TFS_GetRadioCueDurationMilliseconds(radioCue) + gapMilliseconds,
			false
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_OnRadioCueFinished()
	{
		m_bTFSRadioCuePlaying = false;
		TFS_PlayNextRadioCue();
	}

	//------------------------------------------------------------------------------------------------
	protected string TFS_GetRadioEventName(int radioCue, int radioLanguage)
	{
		// The event naming is language-ready. Until Russian samples are added, every unsupported
		// language intentionally falls back to the complete English bank.
		string languageSuffix = "_ENGLISH";

		switch (radioCue)
		{
			case TFS_ERadioCue.REQUEST_RECEIVED:
				return "TFS_RADIO_REQUEST_RECEIVED" + languageSuffix;
			case TFS_ERadioCue.SHOT:
				return "TFS_RADIO_SHOT" + languageSuffix;
			case TFS_ERadioCue.ROUNDS_INCOMING:
				return "TFS_RADIO_ROUNDS_INCOMING" + languageSuffix;
			case TFS_ERadioCue.ALERT_TONE:
				return "TFS_RADIO_ALERT_TONE" + languageSuffix;
			case TFS_ERadioCue.DANGER_CLOSE:
				return "TFS_RADIO_DANGER_CLOSE" + languageSuffix;
			case TFS_ERadioCue.CLEAR_AREA:
				return "TFS_RADIO_CLEAR_AREA" + languageSuffix;
		}

		return string.Empty;
	}

	//------------------------------------------------------------------------------------------------
	protected int TFS_GetRadioCueDurationMilliseconds(int radioCue)
	{
		switch (radioCue)
		{
			case TFS_ERadioCue.REQUEST_RECEIVED:
				return 2500;
			case TFS_ERadioCue.SHOT:
				return 3000;
			case TFS_ERadioCue.ROUNDS_INCOMING:
				return 2100;
			case TFS_ERadioCue.ALERT_TONE:
				return 1800;
			case TFS_ERadioCue.DANGER_CLOSE:
			case TFS_ERadioCue.CLEAR_AREA:
				return 3000;
		}

		return 250;
	}

	//------------------------------------------------------------------------------------------------
	// Called by the authority on every connected PlayerController. The owner RPC lets each client
	// create the one-shot locally, while SCR_SoundManagerModule handles distance and occlusion.
	void TFS_PlayArtilleryFireSound(vector worldPosition, int audioRangeProfile)
	{
		if (!Replication.IsRunning())
		{
			TFS_DoPlayArtilleryFireSound(worldPosition, audioRangeProfile);
			return;
		}

		Rpc(TFS_DoPlayArtilleryFireSound, worldPosition, audioRangeProfile);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void TFS_DoPlayArtilleryFireSound(vector worldPosition, int audioRangeProfile)
	{
		string eventName = TFS_ARTILLERY_FIRE_EVENT_STANDARD;
		if (audioRangeProfile == 1)
			eventName = TFS_ARTILLERY_FIRE_EVENT_EXTENDED;
		else if (audioRangeProfile >= 2)
			eventName = TFS_ARTILLERY_FIRE_EVENT_EXTREME;

		vector soundTransform[4];
		Math3D.MatrixIdentity4(soundTransform);
		soundTransform[3] = worldPosition;

		PrintFormat(
			"TFS: Playing distant battery event %1 at %2.",
			eventName,
			worldPosition
		);
		AudioSystem.PlayEvent(TFS_ARTILLERY_FIRE_SOUND, eventName, soundTransform);
	}
}
