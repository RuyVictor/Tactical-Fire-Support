// Tactical Fire Support - server-authoritative artillery support command.
class TFS_ArtillerySupportCommand : SCR_BaseRadialCommand
{
	static const string COMMAND_NAME = "tfsArtillerySupport";
	static const ResourceName BARRAGE_PREFAB = "{6A67F8C1D0E5A2B4}Prefabs/TFS_ArtilleryBarrage.et";
	static const float MAX_REQUEST_DISTANCE = 10000.0;

	protected static ref map<int, int> s_mLastRequestByPlayer = new map<int, int>();

	//------------------------------------------------------------------------------------------------
	void TFS_ArtillerySupportCommand()
	{
		m_sCommandName = COMMAND_NAME;
		m_sCommandDisplayName = "Apoio de artilharia";
		m_sImageset = "{A37CF52DBA874559}UI/Imagesets/WeaponInfo/WeaponInfo_Ammo.imageset";
		m_sIconName = "ammotype-HE";
	}

	//------------------------------------------------------------------------------------------------
	override bool CanBeShown()
	{
		ChimeraCharacter character = ChimeraCharacter.Cast(SCR_PlayerController.GetLocalControlledEntity());
		if (!character)
			return false;

		CharacterControllerComponent controller = character.GetCharacterController();
		return controller && controller.GetLifeState() == ECharacterLifeState.ALIVE;
	}

	//------------------------------------------------------------------------------------------------
	// The command is available in the contextual radial opened on the full-screen map.
	override bool CanShowOnMap()
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	override bool Execute(IEntity cursorTarget, IEntity groupEnt, vector targetPosition, int playerID, bool isClient)
	{
		// SCR_CommandingManagerComponent broadcasts commands. Only the authoritative instance may
		// create the barrage; clients receive the replicated projectile entities from the server.
		if (isClient)
			return true;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return false;

		IEntity requester = playerManager.GetPlayerControlledEntity(playerID);
		if (!requester)
			return false;

		if (targetPosition == vector.Zero)
			return false;

		if (vector.Distance(requester.GetOrigin(), targetPosition) > MAX_REQUEST_DISTANCE)
		{
			TFS_SendMessage(playerID, "Apoio de artilharia", "O alvo esta fora do alcance de 10 km.", 6.0);
			return false;
		}

		TFS_ArtillerySettings settings = TFS_ArtillerySettings.Load();
		int cooldownSeconds = TFS_ArtillerySettings.DEFAULT_COOLDOWN_SECONDS;
		float barrageDelaySeconds = TFS_ArtillerySettings.DEFAULT_BARRAGE_DELAY_SECONDS;
		float splashLeadSeconds = TFS_ArtillerySettings.DEFAULT_SPLASH_LEAD_SECONDS;
		bool radioCuesEnabled = true;
		bool playRadioCueSound = true;

		if (settings)
		{
			cooldownSeconds = settings.GetCooldownSeconds();
			barrageDelaySeconds = settings.GetBarrageDelaySeconds();
			splashLeadSeconds = settings.GetSplashLeadSeconds();
			radioCuesEnabled = settings.AreRadioCuesEnabled();
			playRadioCueSound = settings.ShouldPlayRadioCueSound();
		}
		else
		{
			Print("TFS: Could not load artillery settings; using default values.", LogLevel.WARNING);
		}

		int cooldownMilliseconds = cooldownSeconds * 1000;

		int previousRequest;
		if (cooldownMilliseconds > 0 && s_mLastRequestByPlayer.Find(playerID, previousRequest))
		{
			int elapsed = System.GetTickCount(previousRequest);
			if (elapsed < cooldownMilliseconds)
			{
				int secondsLeft = Math.Ceil((cooldownMilliseconds - elapsed) * 0.001);
				TFS_SendMessage(playerID, "Apoio indisponivel", string.Format("Aguarde %1 s para uma nova solicitacao.", secondsLeft), 6.0);
				return false;
			}
		}

		Resource barrageResource = Resource.Load(BARRAGE_PREFAB);
		if (!barrageResource || !barrageResource.IsValid())
		{
			Print("TFS: Failed to load artillery barrage prefab.", LogLevel.ERROR);
			TFS_SendMessage(playerID, "Apoio de artilharia", "Nao foi possivel iniciar o apoio.", 6.0);
			return false;
		}

		s_mLastRequestByPlayer.Set(playerID, System.GetTickCount());

		int barrageDelayMilliseconds = barrageDelaySeconds * 1000;
		int splashLeadMilliseconds = splashLeadSeconds * 1000;

		if (radioCuesEnabled)
		{
			TFS_SendRadioMessage(
				playerID,
				"FIRE MISSION",
				"Coordenadas recebidas. Aguarde o disparo.",
				4.0,
				playRadioCueSound
			);

			GetGame().GetCallqueue().CallLater(
				TFS_SendShotCue,
				Math.Min(1000, barrageDelayMilliseconds),
				false,
				playerID,
				playRadioCueSound
			);

			int splashDelayMilliseconds = barrageDelayMilliseconds - splashLeadMilliseconds;
			if (splashDelayMilliseconds > 1000 && splashDelayMilliseconds < barrageDelayMilliseconds)
			{
				GetGame().GetCallqueue().CallLater(
					TFS_SendSplashCue,
					splashDelayMilliseconds,
					false,
					playerID,
					Math.Ceil(splashLeadSeconds),
					playRadioCueSound
				);
			}
		}

		if (barrageDelayMilliseconds <= 0)
			TFS_SpawnBarrage(targetPosition, playerID);
		else
			GetGame().GetCallqueue().CallLater(TFS_SpawnBarrage, barrageDelayMilliseconds, false, targetPosition, playerID);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SendShotCue(int playerID, bool playSound)
	{
		TFS_SendRadioMessage(playerID, "SHOT", "Municao disparada. Salva a caminho.", 4.0, playSound);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SendSplashCue(int playerID, int secondsToImpact, bool playSound)
	{
		TFS_SendRadioMessage(
			playerID,
			"SPLASH",
			string.Format("Impactos em aproximadamente %1 s.", secondsToImpact),
			4.0,
			playSound
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SpawnBarrage(vector targetPosition, int playerID)
	{
		Resource barrageResource = Resource.Load(BARRAGE_PREFAB);
		if (!barrageResource || !barrageResource.IsValid())
		{
			Print("TFS: Failed to load artillery barrage prefab.", LogLevel.ERROR);
			TFS_SendMessage(playerID, "Apoio de artilharia", "Nao foi possivel iniciar o apoio.", 6.0);
			return;
		}

		targetPosition[1] = SCR_TerrainHelper.GetTerrainY(targetPosition, noUnderwater: true);

		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = targetPosition;

		IEntity barrage = GetGame().SpawnEntityPrefab(barrageResource, GetGame().GetWorld(), spawnParams);
		if (!barrage)
		{
			Print("TFS: Failed to spawn artillery barrage.", LogLevel.ERROR);
			TFS_SendMessage(playerID, "Apoio de artilharia", "Nao foi possivel iniciar o apoio.", 6.0);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SendMessage(int playerID, string title, string message, float duration)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (playerController)
			playerController.TFS_ShowOwnerMessage(title, message, duration);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SendRadioMessage(int playerID, string title, string message, float duration, bool playSound)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (playerController)
			playerController.TFS_ShowOwnerRadioMessage(title, message, duration, playSound);
	}
}
