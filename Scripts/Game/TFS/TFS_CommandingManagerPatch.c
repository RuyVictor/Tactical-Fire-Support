// Registers the custom command without replacing the vanilla Commands.conf resource.
modded class SCR_CommandingManagerComponent
{
	//------------------------------------------------------------------------------------------------
	override void InitiateCommandMaps()
	{
		super.InitiateCommandMaps();

		if (!m_aCommands || m_mNameCommand.Get(TFS_ArtillerySupportCommand.COMMAND_NAME))
			return;

		SCR_BaseRadialCommand supportCommand = new TFS_ArtillerySupportCommand();
		m_aCommands.Insert(supportCommand);
		m_mNameCommand.Insert(supportCommand.GetCommandName(), supportCommand);
	}
}
