// Owner-targeted feedback for server-side validation and confirmation.
modded class SCR_PlayerController
{
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
	void TFS_ShowOwnerRadioMessage(string title, string message, float duration, bool playSound)
	{
		if (!Replication.IsRunning())
		{
			TFS_DoShowOwnerRadioMessage(title, message, duration, playSound);
			return;
		}

		Rpc(TFS_DoShowOwnerRadioMessage, title, message, duration, playSound);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void TFS_DoShowOwnerRadioMessage(string title, string message, float duration, bool playSound)
	{
		if (playSound)
			SCR_UISoundEntity.SoundEvent(SCR_SoundEvent.HINT, true);

		SCR_HintManagerComponent.HideHint();
		SCR_HintManagerComponent.ShowCustomHint(message, title, duration, true);
	}
}
