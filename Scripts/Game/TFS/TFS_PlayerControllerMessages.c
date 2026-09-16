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
}
