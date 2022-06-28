/**
 * ExpansionQuestObjectiveAIPatrolEvent.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

#ifdef EXPANSIONMODAI
class ExpansionQuestObjectiveAICampEvent: ExpansionQuestObjectiveEventBase
{
	private ref array<eAIDynamicPatrol> AIPatrols = new array<eAIDynamicPatrol>;
	private int m_TotalKillCount = 0;
	private int m_UnitsToSpawn = 0;
	private int m_TotalUnitsAmount = 0;

	//! Event called when the player starts the quest
	override void OnStart()
	{
		ObjectivePrint(ToString() + "::OnStart - Start");

		CreateAICamp();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupZeds, 30000, true);
		
		super.OnStart();

		ObjectivePrint(ToString() + "::OnStart - End");
	}

	//! Event called when the player continues the quest after a server restart/reconnect
	override void OnContinue()
	{
		ObjectivePrint(ToString() + "::OnContinue - Start");

		CreateAICamp();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(CleanupZeds, 30000, true);
		
		super.OnContinue();

		ObjectivePrint(ToString() + "::OnContinue - End");
	}

	//! Event called when the quest gets cleaned up (server shutdown/player disconnect).
	override void OnCleanup()
	{
		ObjectivePrint(ToString() + "::OnCleanup - Start");

		CleanupPatrol();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(CleanupZeds);
		
		super.OnCleanup();

		ObjectivePrint(ToString() + "::OnCleanup - End");
	}

	//! Event called when the quest gets manualy canceled by the player.
	override void OnCancel()
	{
		ObjectivePrint(ToString() + "::OnCancel - Start");

		CleanupPatrol();

		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).Remove(CleanupZeds);
		
		super.OnCancel();

		ObjectivePrint(ToString() + "::OnCancel - End");
	}

	private void CleanupPatrol()
	{
		for (int i = 0; i < AIPatrols.Count(); i++)
		{
			eAIDynamicPatrol patrol = AIPatrols[i];
			if (patrol.m_CanSpawn)
				continue;

			eAIGroup group = patrol.m_Group;

			ObjectivePrint(ToString() + "::OnCancel - Patrol: " + patrol.ToString());
			ObjectivePrint(ToString() + "::OnCancel - Patrol group: " + group.ToString());
			ObjectivePrint(ToString() + "::OnCancel - Patrol group members: " + group.Count());

			for (int j = 0; j < group.Count(); j++)
			{
				DayZPlayerImplement member = group.GetMember(j);
				if (!member)
					continue;

				ObjectivePrint(ToString() + "::OnCancel - Delete member: [" + j + "] " + member.ToString());

				GetGame().ObjectDelete(member);
			}

			patrol.Despawn();
			eAIGroup.DeleteGroup(group);
		}

		AIPatrols.Clear();
	}

	private bool KilledAICampMember(EntityAI victim)
	{
		DayZPlayerImplement victimPlayer;
		if (!Class.CastTo(victimPlayer, victim))
			return false;
			
		for (int i = 0; i < AIPatrols.Count(); i++)
		{
			eAIDynamicPatrol patrol = AIPatrols[i];
			if (patrol.m_CanSpawn)
				continue;

			eAIGroup group = patrol.m_Group;
			if (group && group.IsMember(victimPlayer))
				return true;
		}

		return false;
	}
	
	void OnEntityKilled(EntityAI victim, EntityAI killer)
	{
	#ifdef EXPANSIONTRACE
		auto trace = CF_Trace_0(ExpansionTracing.QUESTS, this, "OnEntityKilled");
	#endif

		ExpansionQuestObjectiveAICamp aiCamp = GetObjectiveConfig().GetAICamp();
		if (!aiCamp)
			return;
		
		int findIndex;
		//! If the ai camp need to be killed with a special weapon check incoming killer class type
		if (aiCamp.NeedSpecialWeapon())
		{
			findIndex = -1;
			findIndex = aiCamp.GetAllowedWeapons().Find(killer.GetType());
			if (findIndex == -1)
				return;
		}
		
		//! Check if killed entities class name is a valid one from our objective config
		int amount = aiCamp.GetPositions().Count();
		m_TotalUnitsAmount = amount;
		findIndex = -1;
		findIndex = aiCamp.GetClassNames().Find(victim.ClassName());
		Print(ToString() + "::OnEntityKilled - Target find index: " + findIndex);

		if (findIndex == -1)
			return;
		
		//! Check if killed ai entity was part of this objective event group
		if (KilledAICampMember(victim))	
		{
			if (m_TotalKillCount < m_TotalUnitsAmount)
			{
				m_TotalKillCount++;
				
				if (GetQuest())
					GetQuest().UpdateQuestPlayersObjectiveData();
			}
		}
		
		if (m_TotalKillCount >= m_TotalUnitsAmount)
		{
			SetCompleted(true);
			OnComplete();
		}
	}

	private void CreateAICamp()
	{
		ObjectivePrint(ToString() + "::CreateAICamp - Start");

		if (!GetObjectiveConfig() || !GetQuest() || !GetQuest().GetQuestConfig())
			return;

		ExpansionQuestObjectiveAICamp aiCamp = GetObjectiveConfig().GetAICamp();
		if (!aiCamp)
			return;
		
		m_TotalUnitsAmount = aiCamp.GetPositions().Count();

		//! If all the targets are already killed dont create patrol group
		if (m_TotalKillCount >= m_TotalUnitsAmount)
			return;

		m_UnitsToSpawn = m_TotalUnitsAmount - m_TotalKillCount;

		for (int i = 0; i < m_UnitsToSpawn; i++)
		{
			vector pos = aiCamp.GetPositions()[i];
			array<vector> waypoint = new array<vector>;
			waypoint.Insert(pos);

			ExpansionQuestAIGroup group = new ExpansionQuestAIGroup(1, aiCamp.GetNPCSpeed(), aiCamp.GetNPCMode(), "HALT", aiCamp.GetNPCFaction(), aiCamp.GetNPCLoadoutFile(), true, false, waypoint);
			eAIDynamicPatrol patrol = CreateQuestPatrol(group);
			if (!patrol)
				return;

			AIPatrols.Insert(patrol);
		}

	#ifdef EXPANSIONMODNAVIGATION
		string markerName = "[Target] Bandit Camp";
		GetQuest().CreateClientMarker(aiCamp.GetPositions()[0], markerName);
	#endif

		ObjectivePrint(ToString() + "::CreateAICamp - End");
	}

	static eAIDynamicPatrol CreateQuestPatrol(ExpansionQuestAIGroup group, int killCount = 0, int respawnTime = -1, float minDistRadius = 20, float maxDistRadius = 600 )
	{
		Print("=================== Expansion Quest Camp AI ===================");
		int aiSum;
		if ( group.NumberOfAI != 0 )
		{
			if ( group.NumberOfAI < 0 )
			{
				aiSum = Math.RandomInt(1,-group.NumberOfAI);
			}
			else
			{
				aiSum = group.NumberOfAI - killCount;
			}
		}
		else
		{
            Print("[QUESTS] WARNING: NumberOfAI shouldn't be set to 0, skipping this group...");
			return NULL;
		}

        if ( !group.Waypoints )
        {
            Print("!!! ERROR !!!");
            Print("[QUESTS] No waypoints (validate your file with a json validator)");
            Print("!!! ERROR !!!");
           return NULL;
        }

		vector startpos = group.Waypoints[0];
		if ( !startpos || startpos == "0 0 0" )
		{
			Print("!!! ERROR !!!");
			Print("[QUESTS] Couldn't find a spawn location. First waypoint is set to 0 0 0 or cannot be read by the system (validate your file with a json validator)");
			Print("!!! ERROR !!!");
			return NULL;
		}

		// Safety in case the Y is bellow the ground
		startpos = ExpansionStatic.GetSurfacePosition(startpos);
		if ( startpos[1] < group.Waypoints[0][1] )
			startpos[1] = group.Waypoints[0][1];

		Print("[QUESTS] Spawning "+aiSum+" "+group.Faction+" bots at "+group.Waypoints[0]+" and will patrol at "+group.Waypoints);

		eAIDynamicPatrol patrol = eAIDynamicPatrol.Create(startpos, group.Waypoints, group.GetBehaviour(), group.LoadoutFile, aiSum, respawnTime, eAIFaction.Create(group.Faction), true, minDistRadius, maxDistRadius, group.GetSpeed(), group.GetThreatSpeed(), group.CanBeLooted, group.UnlimitedReload);

		Print("=================== Expansion Quest Camp AI ===================");
		return patrol;
	}

	private void CleanupZeds()
	{
		Print(ToString() + "::CleanupZeds - Start");

		ExpansionQuestObjectiveAICamp aiCamp = GetObjectiveConfig().GetAICamp();
		if (!aiCamp)
			return;

		array<Object> objects = new array<Object>;
		GetGame().GetObjectsAtPosition3D(aiCamp.GetPositions()[0], 500, objects, NULL);
		for (int i = 0; i < objects.Count(); i++)
		{
			Object obj = objects[i];
			if (obj.IsInherited(ZombieBase))
				GetGame().ObjectDelete(obj);
		}

		Print(ToString() + "::CleanupZeds - End");
	}

	void SetKillCount(int count)
	{
		m_TotalKillCount = count;
	}

	int GetCount()
	{
		return m_TotalKillCount;
	}

	int GetAmount()
	{
		return m_TotalUnitsAmount;
	}

	override int GetObjectiveType()
	{
		return ExpansionQuestObjectiveType.AICAMP;
	}
};
#endif