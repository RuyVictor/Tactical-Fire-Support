// Tactical Fire Support - server-authoritative artillery support command.
class TFS_ArtillerySupportCommand : SCR_BaseRadialCommand
{
	static const string COMMAND_NAME = "tfsArtillerySupport";
	static const ResourceName STRIKE_PREFAB = "{6A67F8C1D0E5A2B4}Prefabs/TFS_ArtilleryBarrage.et";
	static const float MAX_REQUEST_DISTANCE = 10000.0;

	protected static ref map<int, int> s_mLastRequestByPlayer = new map<int, int>();

	//------------------------------------------------------------------------------------------------
	void TFS_ArtillerySupportCommand()
	{
		m_sCommandName = COMMAND_NAME;
		m_sCommandDisplayName = "Artilharia 240 mm";
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
		// create the strike; clients receive the replicated projectile entity from the server.
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
			TFS_SendMessage(playerID, "Artilharia 240 mm", "O alvo esta fora do alcance de 10 km.", 6.0);
			return false;
		}

		TFS_ArtillerySettings settings = TFS_ArtillerySettings.Load();
		int cooldownSeconds = TFS_ArtillerySettings.DEFAULT_COOLDOWN_SECONDS;
		float initialMissionDelaySeconds = TFS_ArtillerySettings.DEFAULT_INITIAL_MISSION_DELAY_SECONDS;
		float barrageDelaySeconds = TFS_ArtillerySettings.DEFAULT_BARRAGE_DELAY_SECONDS;
		float shotCueDelaySeconds = TFS_ArtillerySettings.DEFAULT_SHOT_CUE_DELAY_SECONDS;
		float splashLeadSeconds = TFS_ArtillerySettings.DEFAULT_SPLASH_LEAD_SECONDS;
		float radioInterCueDelaySeconds = TFS_ArtillerySettings.DEFAULT_RADIO_INTER_CUE_DELAY_SECONDS;
		float alertToneToDangerCloseDelaySeconds = TFS_ArtillerySettings.DEFAULT_ALERT_TONE_TO_DANGER_CLOSE_DELAY_SECONDS;
		float dangerCloseToClearAreaDelaySeconds = TFS_ArtillerySettings.DEFAULT_DANGER_CLOSE_TO_CLEAR_AREA_DELAY_SECONDS;
		bool radioCuesEnabled = true;
		bool playRadioCueSound = true;
		bool showImpactMarker = true;
		bool distantFireSoundEnabled = true;
		float distantFireOffsetMeters = TFS_ArtillerySettings.DEFAULT_DISTANT_FIRE_OFFSET_METERS;
		float dangerCloseRadiusMeters = TFS_ArtillerySettings.DEFAULT_DANGER_CLOSE_RADIUS_METERS;
		int audioRangeProfile = TFS_ArtillerySettings.DEFAULT_AUDIO_RANGE_PROFILE;
		int radioLanguage = TFS_ERadioLanguage.ENGLISH;

		if (settings)
		{
			cooldownSeconds = settings.GetCooldownSeconds();
			initialMissionDelaySeconds = settings.GetInitialMissionDelaySeconds();
			barrageDelaySeconds = settings.GetBarrageDelaySeconds();
			shotCueDelaySeconds = settings.GetShotCueDelaySeconds();
			splashLeadSeconds = settings.GetSplashLeadSeconds();
			radioInterCueDelaySeconds = settings.GetRadioInterCueDelaySeconds();
			alertToneToDangerCloseDelaySeconds = settings.GetAlertToneToDangerCloseDelaySeconds();
			dangerCloseToClearAreaDelaySeconds = settings.GetDangerCloseToClearAreaDelaySeconds();
			radioCuesEnabled = settings.AreRadioCuesEnabled();
			playRadioCueSound = settings.ShouldPlayRadioCueSound();
			showImpactMarker = settings.ShouldShowImpactMarker();
			distantFireSoundEnabled = settings.IsDistantFireSoundEnabled();
			distantFireOffsetMeters = settings.GetDistantFireOffsetMeters();
			dangerCloseRadiusMeters = settings.GetDangerCloseRadiusMeters();
			audioRangeProfile = settings.GetAudioRangeProfile();
		}
		else
		{
			Print("TFS: Could not load artillery settings; using default values.", LogLevel.WARNING);
		}

		// Pack the three server-authoritative radio gaps into one RPC/call-queue argument.
		vector radioTimingSeconds = Vector(
			radioInterCueDelaySeconds,
			alertToneToDangerCloseDelaySeconds,
			dangerCloseToClearAreaDelaySeconds
		);

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

		Resource strikeResource = Resource.Load(STRIKE_PREFAB);
		if (!strikeResource || !strikeResource.IsValid())
		{
			Print("TFS: Failed to load 240 mm artillery strike prefab.", LogLevel.ERROR);
			TFS_SendMessage(playerID, "Artilharia 240 mm", "Nao foi possivel iniciar o apoio.", 6.0);
			return false;
		}

		s_mLastRequestByPlayer.Set(playerID, System.GetTickCount());
		int requesterFactionIndex = TFS_GetEntityFactionIndex(requester);

		if (showImpactMarker)
		{
			TFS_ArtilleryMissionMarkerService.CreateImpactMarker(
				targetPosition,
				requesterFactionIndex,
				initialMissionDelaySeconds + barrageDelaySeconds
			);
		}

		int initialMissionDelayMilliseconds = initialMissionDelaySeconds * 1000;
		int barrageDelayMilliseconds = barrageDelaySeconds * 1000;
		// Never schedule the incoming warning before mission confirmation. This preserves the stage
		// order even when an administrator configures a lead longer than the fire delay.
		int splashLeadMilliseconds = Math.Min(splashLeadSeconds * 1000, barrageDelayMilliseconds);
		int strikeDelayMilliseconds = initialMissionDelayMilliseconds + barrageDelayMilliseconds;
		int shotCueDelayMilliseconds = initialMissionDelayMilliseconds + Math.Min(
			shotCueDelaySeconds * 1000,
			barrageDelayMilliseconds
		);

		if (radioCuesEnabled)
		{
			if (initialMissionDelayMilliseconds <= 0)
				TFS_SendRequestReceivedCue(playerID, playRadioCueSound, radioLanguage, radioTimingSeconds);
			else
				GetGame().GetCallqueue().CallLater(
					TFS_SendRequestReceivedCue,
					initialMissionDelayMilliseconds,
					false,
					playerID,
					playRadioCueSound,
					radioLanguage,
					radioTimingSeconds
				);

			int splashDelayMilliseconds = strikeDelayMilliseconds - splashLeadMilliseconds;
			if (barrageDelayMilliseconds > 0 && splashDelayMilliseconds <= 0)
			{
				TFS_SendIncomingStage(
					playerID,
					Math.Ceil(barrageDelaySeconds),
					playRadioCueSound,
					radioLanguage,
					targetPosition,
					requesterFactionIndex,
					dangerCloseRadiusMeters,
					radioTimingSeconds
				);
			}
			else if (splashDelayMilliseconds > 0 && splashDelayMilliseconds < strikeDelayMilliseconds)
			{
				GetGame().GetCallqueue().CallLater(
					TFS_SendIncomingStage,
					splashDelayMilliseconds,
					false,
					playerID,
					Math.Ceil(splashLeadMilliseconds * 0.001),
					playRadioCueSound,
					radioLanguage,
					targetPosition,
					requesterFactionIndex,
					dangerCloseRadiusMeters,
					radioTimingSeconds
				);
			}
		}

		vector batterySoundPosition = vector.Zero;
		if (distantFireSoundEnabled)
			batterySoundPosition = TFS_GetBatterySoundPosition(
				requester.GetOrigin(),
				targetPosition,
				distantFireOffsetMeters
			);

		if (radioCuesEnabled || distantFireSoundEnabled)
		{
			if (shotCueDelayMilliseconds <= 0)
			{
				TFS_ExecuteShotStage(
					playerID,
					radioCuesEnabled,
					playRadioCueSound,
					distantFireSoundEnabled,
					batterySoundPosition,
					audioRangeProfile,
					radioLanguage,
					radioTimingSeconds
				);
			}
			else
			{
				GetGame().GetCallqueue().CallLater(
					TFS_ExecuteShotStage,
					shotCueDelayMilliseconds,
					false,
					playerID,
					radioCuesEnabled,
					playRadioCueSound,
					distantFireSoundEnabled,
					batterySoundPosition,
					audioRangeProfile,
					radioLanguage,
					radioTimingSeconds
				);
			}
		}

		if (strikeDelayMilliseconds <= 0)
			TFS_SpawnStrike(targetPosition, playerID);
		else
			GetGame().GetCallqueue().CallLater(TFS_SpawnStrike, strikeDelayMilliseconds, false, targetPosition, playerID);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SendRequestReceivedCue(
		int playerID,
		bool playSound,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		TFS_SendRadioMessage(
			playerID,
			"FIRE MISSION",
			"Solicitacao recebida. Coordenadas confirmadas.",
			4.0,
			playSound,
			TFS_ERadioCue.REQUEST_RECEIVED,
			radioLanguage,
			radioTimingSeconds
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SendShotCue(
		int playerID,
		bool playSound,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		TFS_SendRadioMessage(
			playerID,
			"SHOT",
			"Disparo de 240 mm efetuado.",
			4.0,
			playSound,
			TFS_ERadioCue.SHOT,
			radioLanguage,
			radioTimingSeconds
		);
	}

	//------------------------------------------------------------------------------------------------
	// Keeps the radio SHOT cue and the actual battery discharge on the same configurable stage.
	protected void TFS_ExecuteShotStage(
		int playerID,
		bool radioCuesEnabled,
		bool playRadioCueSound,
		bool distantFireSoundEnabled,
		vector batterySoundPosition,
		int audioRangeProfile,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		if (radioCuesEnabled)
			TFS_SendShotCue(
				playerID,
				playRadioCueSound,
				radioLanguage,
				radioTimingSeconds
			);

		if (distantFireSoundEnabled)
			TFS_BroadcastArtilleryFireSound(batterySoundPosition, audioRangeProfile);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SendIncomingStage(
		int playerID,
		int secondsToImpact,
		bool playSound,
		int radioLanguage,
		vector targetPosition,
		int requesterFactionIndex,
		float dangerCloseRadiusMeters,
		vector radioTimingSeconds
	)
	{
		// A requester inside the danger-close zone receives the urgent warning instead of two
		// competing messages. Requesters outside the zone receive the normal incoming report.
		bool requesterIsDangerClose;
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (playerManager && dangerCloseRadiusMeters > 0)
		{
			IEntity requester = playerManager.GetPlayerControlledEntity(playerID);
			if (requester && TFS_GetEntityFactionIndex(requester) == requesterFactionIndex)
			{
				float dangerRadiusSq = dangerCloseRadiusMeters * dangerCloseRadiusMeters;
				requesterIsDangerClose = vector.DistanceSq(requester.GetOrigin(), targetPosition) <= dangerRadiusSq;
			}
		}

		if (!requesterIsDangerClose)
		{
			TFS_SendRadioMessage(
				playerID,
				"ROUNDS INCOMING",
				string.Format("Projetil a caminho. Impacto em aproximadamente %1 s.", secondsToImpact),
				4.0,
				playSound,
				TFS_ERadioCue.ROUNDS_INCOMING,
				radioLanguage,
				radioTimingSeconds
			);
		}

		TFS_BroadcastDangerClose(
			targetPosition,
			requesterFactionIndex,
			dangerCloseRadiusMeters,
			playSound,
			radioLanguage,
			radioTimingSeconds
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_BroadcastDangerClose(
		vector targetPosition,
		int requesterFactionIndex,
		float dangerCloseRadiusMeters,
		bool playSound,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		if (requesterFactionIndex < 0 || dangerCloseRadiusMeters <= 0)
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		float radiusSq = dangerCloseRadiusMeters * dangerCloseRadiusMeters;
		array<int> playerIDs = {};
		playerManager.GetPlayers(playerIDs);

		foreach (int alliedPlayerID : playerIDs)
		{
			IEntity alliedEntity = playerManager.GetPlayerControlledEntity(alliedPlayerID);
			if (!alliedEntity || TFS_GetEntityFactionIndex(alliedEntity) != requesterFactionIndex)
				continue;

			if (vector.DistanceSq(alliedEntity.GetOrigin(), targetPosition) > radiusSq)
				continue;

			TFS_SendRadioMessage(
				alliedPlayerID,
				"DANGER CLOSE",
				"Impacto de artilharia iminente. Afaste-se da area marcada.",
				6.0,
				playSound,
				TFS_ERadioCue.DANGER_CLOSE,
				radioLanguage,
				radioTimingSeconds
			);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected vector TFS_GetBatterySoundPosition(vector requesterPosition, vector targetPosition, float offsetMeters)
	{
		vector awayFromTarget = vector.Direction(targetPosition, requesterPosition);
		awayFromTarget[1] = 0;

		if (awayFromTarget.LengthSq() < 0.01)
			awayFromTarget = vector.Forward;
		else
			awayFromTarget.Normalize();

		vector batteryPosition = requesterPosition + awayFromTarget * offsetMeters;
		batteryPosition[1] = SCR_TerrainHelper.GetTerrainY(batteryPosition, noUnderwater: true);
		return batteryPosition;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_BroadcastArtilleryFireSound(vector worldPosition, int audioRangeProfile)
	{
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;

		array<int> playerIDs = {};
		playerManager.GetPlayers(playerIDs);

		foreach (int playerID : playerIDs)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerID));
			if (playerController)
				playerController.TFS_PlayArtilleryFireSound(worldPosition, audioRangeProfile);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SpawnStrike(vector targetPosition, int playerID)
	{
		Resource strikeResource = Resource.Load(STRIKE_PREFAB);
		if (!strikeResource || !strikeResource.IsValid())
		{
			Print("TFS: Failed to load 240 mm artillery strike prefab.", LogLevel.ERROR);
			TFS_SendMessage(playerID, "Artilharia 240 mm", "Nao foi possivel iniciar o apoio.", 6.0);
			return;
		}

		targetPosition[1] = SCR_TerrainHelper.GetTerrainY(targetPosition, noUnderwater: true);

		EntitySpawnParams spawnParams = EntitySpawnParams();
		spawnParams.TransformMode = ETransformMode.WORLD;
		spawnParams.Transform[3] = targetPosition;

		IEntity strike = GetGame().SpawnEntityPrefab(strikeResource, GetGame().GetWorld(), spawnParams);
		if (!strike)
		{
			Print("TFS: Failed to spawn 240 mm artillery strike.", LogLevel.ERROR);
			TFS_SendMessage(playerID, "Artilharia 240 mm", "Nao foi possivel iniciar o apoio.", 6.0);
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
	protected void TFS_SendRadioMessage(
		int playerID,
		string title,
		string message,
		float duration,
		bool playSound,
		int radioCue,
		int radioLanguage,
		vector radioTimingSeconds
	)
	{
		SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerManager().GetPlayerController(playerID));
		if (playerController)
			playerController.TFS_ShowOwnerRadioMessage(
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
	protected int TFS_GetEntityFactionIndex(IEntity entity)
	{
		if (!entity)
			return -1;

		FactionAffiliationComponent factionComponent = FactionAffiliationComponent.Cast(
			entity.FindComponent(FactionAffiliationComponent)
		);
		if (!factionComponent)
			return -1;

		Faction faction = factionComponent.GetAffiliatedFaction();
		FactionManager factionManager = GetGame().GetFactionManager();
		if (!faction || !factionManager)
			return -1;

		return factionManager.GetFactionIndex(faction);
	}
}
