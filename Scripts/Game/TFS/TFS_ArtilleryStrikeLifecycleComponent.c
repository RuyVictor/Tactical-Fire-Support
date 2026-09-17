[ComponentEditorProps(category: "Tactical Fire Support", description: "Removes the server-owned impact marker when the barrage projectile finishes.")]
class TFS_ArtilleryStrikeLifecycleComponentClass : ScriptComponentClass
{
}

class TFS_ArtilleryStrikeLifecycleComponent : ScriptComponent
{
	static const int CHILD_CHECK_INTERVAL_MILLISECONDS = 100;
	static const int CHILD_SPAWN_TIMEOUT_MILLISECONDS = 30000;

	protected SCR_EffectsModuleComponent m_pEffectsModule;
	protected vector m_vTargetPosition;
	protected int m_iStartedAt;
	protected bool m_bSawProjectile;
	protected bool m_bImpactNotified;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!owner || (Replication.IsRunning() && !Replication.IsServer()))
			return;

		m_pEffectsModule = SCR_EffectsModuleComponent.Cast(owner.FindComponent(SCR_EffectsModuleComponent));
		if (!m_pEffectsModule)
		{
			Print("TFS: Strike lifecycle could not find SCR_EffectsModuleComponent.", LogLevel.WARNING);
			return;
		}

		m_vTargetPosition = owner.GetOrigin();
		m_iStartedAt = System.GetTickCount();
		GetGame().GetCallqueue().CallLater(TFS_CheckProjectileLifecycle, 0, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_CheckProjectileLifecycle()
	{
		if (m_bImpactNotified || !m_pEffectsModule)
			return;

		if (m_pEffectsModule.HasEffectsModuleChildren())
		{
			m_bSawProjectile = true;
		}
		else if (m_bSawProjectile)
		{
			TFS_NotifyImpactOnce();
			return;
		}
		else if (System.GetTickCount(m_iStartedAt) >= CHILD_SPAWN_TIMEOUT_MILLISECONDS)
		{
			Print("TFS: Strike lifecycle timed out before observing the artillery projectile.", LogLevel.WARNING);
			return;
		}

		GetGame().GetCallqueue().CallLater(
			TFS_CheckProjectileLifecycle,
			CHILD_CHECK_INTERVAL_MILLISECONDS,
			false
		);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if ((!Replication.IsRunning() || Replication.IsServer()) && m_bSawProjectile)
			TFS_NotifyImpactOnce();
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_NotifyImpactOnce()
	{
		if (m_bImpactNotified)
			return;

		m_bImpactNotified = true;
		TFS_ArtilleryMissionMarkerService.NotifyImpact(m_vTargetPosition);
	}
}
