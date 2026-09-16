// Inserts Tactical Fire Support through the map commanding pipeline used by the base game.
modded class SCR_PlayerControllerCommandingComponent
{
	//------------------------------------------------------------------------------------------------
	override void SetupMapRadialMenu()
	{
		super.SetupMapRadialMenu();

		if (!m_MapContextualMenu)
		{
			Print("TFS: Map contextual menu is unavailable.", LogLevel.WARNING);
			return;
		}

		SCR_RadialMenuController radialController = m_MapContextualMenu.GetRadialController();
		SCR_RadialMenu radialMenu;
		if (radialController)
			radialMenu = radialController.GetRadialMenu();

		// Setup can run more than once; never insert a duplicate entry.
		if (radialMenu)
		{
			foreach (SCR_SelectionMenuEntry existingEntry : radialMenu.GetEntries())
			{
				SCR_MapMenuCommandingEntry existingCommand = SCR_MapMenuCommandingEntry.Cast(existingEntry);
				if (existingCommand && existingCommand.GetEntryIdentifier() == TFS_ArtillerySupportCommand.COMMAND_NAME)
					return;
			}
		}

		SCR_MapMenuCommandingEntry supportEntry = new SCR_MapMenuCommandingEntry(TFS_ArtillerySupportCommand.COMMAND_NAME);
		supportEntry.SetId(TFS_ArtillerySupportCommand.COMMAND_NAME);
		supportEntry.SetName("Apoio de artilharia");
		supportEntry.SetDescription("Solicitar uma salva de morteiro neste ponto.");
		supportEntry.SetIcon(
			"{A37CF52DBA874559}UI/Imagesets/WeaponInfo/WeaponInfo_Ammo.imageset",
			"ammotype-HE"
		);
		supportEntry.Enable(true);

		m_MapContextualMenu.InsertCustomRadialEntry(supportEntry);

		int entryCount;
		if (radialMenu)
			entryCount = radialMenu.GetEntryCount();

		Print(string.Format(
			"TFS: Artillery command inserted through map commanding pipeline. Root entries: %1.",
			entryCount
		), LogLevel.NORMAL);
	}
}
