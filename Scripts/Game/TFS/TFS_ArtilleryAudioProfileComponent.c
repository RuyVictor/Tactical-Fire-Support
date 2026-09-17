[ComponentEditorProps(category: "Tactical Fire Support", description: "Selects the configured artillery audio range profile.")]
class TFS_ArtilleryAudioProfileComponentClass : ScriptComponentClass
{
}

class TFS_ArtilleryAudioProfileComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.CheckBox, "Play an impact profile event instead of an incoming profile event.")]
	protected bool m_bImpactSound;
	protected bool m_bEventPlayed;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		GetGame().GetCallqueue().CallLater(TFS_PlaySelectedEvent, 0, false, owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_PlaySelectedEvent(IEntity owner)
	{
		if (!owner || m_bEventPlayed)
			return;

		SoundComponent soundComponent = SoundComponent.Cast(owner.FindComponent(SoundComponent));
		if (!soundComponent)
		{
			Print("TFS: Artillery audio profile component could not find a SoundComponent.", LogLevel.WARNING);
			return;
		}

		int profile = TFS_ArtillerySettings.DEFAULT_AUDIO_RANGE_PROFILE;
		TFS_ArtillerySettings settings = TFS_ArtillerySettings.Load();
		if (settings)
			profile = settings.GetAudioRangeProfile();

		string eventName;
		if (m_bImpactSound)
		{
			eventName = "TFS_IMPACT_STANDARD";
			if (profile == 1)
				eventName = "TFS_IMPACT_EXTENDED";
			else if (profile >= 2)
				eventName = "TFS_IMPACT_EXTREME";
		}
		else
		{
			eventName = "TFS_INCOMING_STANDARD";
			if (profile == 1)
				eventName = "TFS_INCOMING_EXTENDED";
			else if (profile >= 2)
				eventName = "TFS_INCOMING_EXTREME";
		}

		// Projectiles and one-shot warheads never reuse this component. Keep the trigger idempotent so
		// a repeated lifecycle callback cannot stack the same incoming or impact event on itself.
		m_bEventPlayed = true;
		soundComponent.SoundEvent(eventName);
	}
}
