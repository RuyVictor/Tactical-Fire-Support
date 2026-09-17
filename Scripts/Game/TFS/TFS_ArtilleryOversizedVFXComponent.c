[ComponentEditorProps(category: "Tactical Fire Support", description: "Builds a wide visual-only 240 mm impact from two blended rings of TNT Large effects.")]
class TFS_ArtilleryOversizedVFXComponentClass : ScriptComponentClass
{
}

class TFS_ArtilleryOversizedVFXComponent : ScriptComponent
{
	static const ResourceName TNT_LARGE_PARTICLE = "{79ED2EDBC38185AB}Particles/Logistics/Explosion/TNT/Explosion_TNT_Large.ptc";

	[Attribute("18", UIWidgets.EditBox, "Outer radius in metres of the visual-only TNT Large effect ring.")]
	protected float m_fOuterRadiusMeters;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		if (!owner)
			return;

		// This prefab is instantiated at the real detonation position. The authority uses it to
		// remove the synchronized warning marker after impact, including on a headless server.
		if (!Replication.IsRunning() || Replication.IsServer())
			TFS_ArtilleryMissionMarkerService.NotifyImpact(owner.GetOrigin());

		if (m_fOuterRadiusMeters <= 0 || System.IsConsoleApp())
			return;

		// Defer by one frame so the effect prefab has its final impact transform. The inherited
		// HitEffectComponent supplies the central TNT Large effect; this component only widens it.
		GetGame().GetCallqueue().CallLater(TFS_SpawnCompositeEffect, 0, false, owner.GetOrigin());
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SpawnCompositeEffect(vector centre)
	{
		float innerRadius = m_fOuterRadiusMeters * 0.48;
		float diagonalRadius = m_fOuterRadiusMeters * 0.707107;

		// Four inner plumes fill the central body of the blast.
		TFS_SpawnEffectAt(centre + Vector(innerRadius, 0, 0));
		TFS_SpawnEffectAt(centre + Vector(-innerRadius, 0, 0));
		TFS_SpawnEffectAt(centre + Vector(0, 0, innerRadius));
		TFS_SpawnEffectAt(centre + Vector(0, 0, -innerRadius));

		// Eight outer plumes create a roughly 50 m-wide visual footprint without changing damage.
		TFS_SpawnEffectAt(centre + Vector(m_fOuterRadiusMeters, 0, 0));
		TFS_SpawnEffectAt(centre + Vector(-m_fOuterRadiusMeters, 0, 0));
		TFS_SpawnEffectAt(centre + Vector(0, 0, m_fOuterRadiusMeters));
		TFS_SpawnEffectAt(centre + Vector(0, 0, -m_fOuterRadiusMeters));
		TFS_SpawnEffectAt(centre + Vector(diagonalRadius, 0, diagonalRadius));
		TFS_SpawnEffectAt(centre + Vector(diagonalRadius, 0, -diagonalRadius));
		TFS_SpawnEffectAt(centre + Vector(-diagonalRadius, 0, diagonalRadius));
		TFS_SpawnEffectAt(centre + Vector(-diagonalRadius, 0, -diagonalRadius));
	}

	//------------------------------------------------------------------------------------------------
	protected void TFS_SpawnEffectAt(vector worldPosition)
	{
		worldPosition[1] = SCR_TerrainHelper.GetTerrainY(worldPosition, noUnderwater: true) + 0.05;

		ParticleEffectEntitySpawnParams spawnParams = new ParticleEffectEntitySpawnParams();
		spawnParams.TargetWorld = GetGame().GetWorld();
		spawnParams.TransformMode = ETransformMode.WORLD;
		Math3D.MatrixIdentity4(spawnParams.Transform);
		spawnParams.Transform[3] = worldPosition;
		spawnParams.PlayOnSpawn = false;
		spawnParams.PlayOnHeadlessClient = false;
		spawnParams.DeleteWhenStopped = true;
		spawnParams.UseFrameEvent = true;

		ParticleEffectEntity effect = ParticleEffectEntity.SpawnParticleEffect(TNT_LARGE_PARTICLE, spawnParams);
		if (!effect)
		{
			Print("TFS: Failed to spawn part of the composite 240 mm impact VFX.", LogLevel.WARNING);
			return;
		}

		effect.Play();
	}
}
