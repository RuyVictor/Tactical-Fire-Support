// Server-owned lifecycle for faction-only artillery impact markers.
class TFS_ArtilleryMissionMarkerData
{
	int m_iMarkerID;
	vector m_vTargetPosition;
	bool m_bRemovalScheduled;

	//------------------------------------------------------------------------------------------------
	void TFS_ArtilleryMissionMarkerData(int markerID, vector targetPosition)
	{
		m_iMarkerID = markerID;
		m_vTargetPosition = targetPosition;
	}
}

class TFS_ArtilleryMissionMarkerService
{
	static const float IMPACT_MATCH_RADIUS_METERS = 25.0;
	static const float MINIMUM_FALLBACK_LIFETIME_SECONDS = 120.0;
	static const float MAXIMUM_FALLBACK_LIFETIME_SECONDS = 600.0;

	protected static ref array<ref TFS_ArtilleryMissionMarkerData> s_aMissionMarkers = {};

	//------------------------------------------------------------------------------------------------
	static int CreateImpactMarker(vector targetPosition, int factionIndex, float expectedDelaySeconds)
	{
		if (factionIndex < 0 || (Replication.IsRunning() && !Replication.IsServer()))
			return -1;

		SCR_MapMarkerManagerComponent markerManager = TFS_GetMarkerManager();
		if (!markerManager)
		{
			Print("TFS: Map marker manager is unavailable; impact marker was not created.", LogLevel.WARNING);
			return -1;
		}

		SCR_MapMarkerBase marker = new SCR_MapMarkerBase();
		marker.SetType(SCR_EMapMarkerType.PLACED_CUSTOM);
		marker.SetWorldPos(targetPosition[0], targetPosition[2]);
		marker.SetIconEntry(SCR_EScenarioFrameworkMarkerCustom.TARGET_REFERENCE_POINT);
		marker.SetColorEntry(SCR_EScenarioFrameworkMarkerCustomColor.RED);
		marker.SetCustomText("IMPACTO ARTILHARIA 240 MM");
		marker.SetCanBeRemovedByOwner(false);
		marker.AddMarkerFactionFlags(factionIndex);

		// A server marker is synchronized but does not consume the players' marker allowance.
		markerManager.InsertStaticMarker(marker, false, true);
		int markerID = marker.GetMarkerID();
		if (markerID < 0)
			return -1;

		s_aMissionMarkers.Insert(new TFS_ArtilleryMissionMarkerData(markerID, targetPosition));

		float fallbackLifetimeSeconds = expectedDelaySeconds + 60.0;
		if (fallbackLifetimeSeconds < MINIMUM_FALLBACK_LIFETIME_SECONDS)
			fallbackLifetimeSeconds = MINIMUM_FALLBACK_LIFETIME_SECONDS;
		else if (fallbackLifetimeSeconds > MAXIMUM_FALLBACK_LIFETIME_SECONDS)
			fallbackLifetimeSeconds = MAXIMUM_FALLBACK_LIFETIME_SECONDS;

		GetGame().GetCallqueue().CallLater(
			RemoveMarkerByID,
			fallbackLifetimeSeconds * 1000,
			false,
			markerID
		);

		return markerID;
	}

	//------------------------------------------------------------------------------------------------
	// Called by the impact warhead. Matching the actual detonation is more accurate than guessing
	// projectile flight time from the mission delay.
	static void NotifyImpact(vector impactPosition)
	{
		if (Replication.IsRunning() && !Replication.IsServer())
			return;

		TFS_ArtilleryMissionMarkerData closestMarker;
		float closestDistanceSq = IMPACT_MATCH_RADIUS_METERS * IMPACT_MATCH_RADIUS_METERS;

		foreach (TFS_ArtilleryMissionMarkerData markerData : s_aMissionMarkers)
		{
			if (!markerData || markerData.m_bRemovalScheduled)
				continue;

			float distanceSq = vector.DistanceSq(markerData.m_vTargetPosition, impactPosition);
			if (distanceSq > closestDistanceSq)
				continue;

			closestMarker = markerData;
			closestDistanceSq = distanceSq;
		}

		if (!closestMarker)
			return;

		closestMarker.m_bRemovalScheduled = true;
		float postImpactLifetimeSeconds = TFS_ArtillerySettings.DEFAULT_MARKER_POST_IMPACT_LIFETIME_SECONDS;
		TFS_ArtillerySettings settings = TFS_ArtillerySettings.Load();
		if (settings)
			postImpactLifetimeSeconds = settings.GetMarkerPostImpactLifetimeSeconds();

		if (postImpactLifetimeSeconds <= 0)
		{
			RemoveMarkerByID(closestMarker.m_iMarkerID);
			return;
		}

		GetGame().GetCallqueue().CallLater(
			RemoveMarkerByID,
			postImpactLifetimeSeconds * 1000,
			false,
			closestMarker.m_iMarkerID
		);
	}

	//------------------------------------------------------------------------------------------------
	static void RemoveMarkerByID(int markerID)
	{
		if (Replication.IsRunning() && !Replication.IsServer())
			return;

		SCR_MapMarkerManagerComponent markerManager = TFS_GetMarkerManager();
		if (markerManager)
		{
			SCR_MapMarkerBase marker = markerManager.GetStaticMarkerByID(markerID);
			if (marker)
				markerManager.RemoveStaticMarker(marker);
		}

		for (int i = s_aMissionMarkers.Count() - 1; i >= 0; i--)
		{
			TFS_ArtilleryMissionMarkerData markerData = s_aMissionMarkers[i];
			if (markerData && markerData.m_iMarkerID == markerID)
				s_aMissionMarkers.Remove(i);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected static SCR_MapMarkerManagerComponent TFS_GetMarkerManager()
	{
		SCR_MapMarkerManagerComponent markerManager = SCR_MapMarkerManagerComponent.GetInstance();
		if (markerManager)
			return markerManager;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return null;

		return SCR_MapMarkerManagerComponent.Cast(gameMode.FindComponent(SCR_MapMarkerManagerComponent));
	}
}
