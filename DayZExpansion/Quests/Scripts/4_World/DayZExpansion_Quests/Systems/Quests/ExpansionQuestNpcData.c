/**
 * ExpansionQuestNpcData.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License.
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionQuestNpcDataBase
{
	int ConfigVersion;
	int ID;
	string ClassName;
	bool IsAI;
	vector Position;
	vector Orientation;
	ref array<int> QuestIDs = new array<int>;
	string NPCName = "Unknown";
	string DefaultNPCText = "What do you want? Leave me alown..!";

#ifdef EXPANSIONMODAI
	ref array<vector> Waypoints = new array<vector>;
	int NPCEmoteID = EmoteConstants.ID_EMOTE_WATCHING;
#endif
}

class ExpansionQuestNpcData: ExpansionQuestNpcDataBase
{
	[NonSerialized()]
	static int CONFIGVERSION = 1;
	
#ifdef EXPANSIONMODAI
	bool NPCEmoteIsStatic;
#endif
	string NPCLoadoutFile;
	bool IsStatic;
	
	void ExpansionQuestNpcData()
	{
		ConfigVersion = 1;
	}
	
	void SetID(int id)
	{
		ID = id;
	}

	int GetID()
	{
		return ID;
	}

	void SetName(string className)
	{
		ClassName = className;
	}

	string GetName()
	{
		return ClassName;
	}

	void SetIsAI(bool state)
	{
		IsAI = state;
	}

	bool IsAI()
	{
		return IsAI;
	}

	void SetQuestIDs(array<int> questIDs)
	{
		QuestIDs.InsertAll(questIDs);
	}

	array<int> GetQuestIDs()
	{
		return QuestIDs;
	}

	void SetPosition(vector pos)
	{
		Position = pos;
	}

	vector GetPosition()
	{
		return Position;
	}

	void SetOrientation(vector orientation)
	{
		Orientation = orientation;
	}

	vector GetOrientation()
	{
		return Orientation;
	}

	void SetLoadoutFile(string fileName)
	{
		NPCLoadoutFile = fileName;
	}
	
	string GetLoadoutFile()
	{
		return NPCLoadoutFile;
	}

	void SetNPCName(string name)
	{
		NPCName = name;
	}

	string GetNPCName()
	{
		return NPCName;
	}

	void SetDefaultNPCText(string text)
	{
		DefaultNPCText = text;
	}

	string GetDefaultNPCText()
	{
		return DefaultNPCText;
	}

#ifdef EXPANSIONMODAI
	void SetIsEmoteStatic(bool state)
	{
		NPCEmoteIsStatic = state;
	}
	
	bool IsEmoteStatic()
	{
		return NPCEmoteIsStatic;
	}
	
	void AddWaypoint(vector waypoint)
	{
		Waypoints.Insert(waypoint);
	}

	array<vector> GetWaypoints()
	{
		return Waypoints;
	}

	void SetEmoteID(int emoteID)
	{
		NPCEmoteID = emoteID;
	}

	int GetEmoteID()
	{
		return NPCEmoteID;
	}
#endif
	
	void SetIsStatic(bool state)
	{
		IsStatic = state;
	}
	
	bool IsStatic()
	{
		return IsStatic;
	}

	ExpansionQuestNpcBase SpawnNPC()
	{
		Object obj = GetGame().CreateObjectEx(GetName(), GetPosition(), ECE_INITAI | ECE_CREATEPHYSICS | ECE_ROTATIONFLAGS | ECE_PLACE_ON_SURFACE);

		if (Position)
			obj.SetPosition(Position);

		if (Orientation)
			obj.SetOrientation(Orientation);

		EntityAI enity = EntityAI.Cast(obj);
		ExpansionQuestNpcBase questNPC = ExpansionQuestNpcBase.Cast(enity);
		ExpansionHumanLoadout.Apply(questNPC, GetLoadoutFile(), false);

		if (Position)
			questNPC.SetPosition(Position);

		if (Orientation)
			questNPC.SetOrientation(Orientation);

		return questNPC;
	}

#ifdef EXPANSIONMODAI
	ExpansionQuestNpcAIBase SpawnNPCAI()
	{
		Object obj = GetGame().CreateObjectEx(GetName(), GetPosition(), ECE_INITAI | ECE_CREATEPHYSICS | ECE_ROTATIONFLAGS | ECE_PLACE_ON_SURFACE);

		if (Position)
			obj.SetPosition(Position);

		if (Orientation)
			obj.SetOrientation(Orientation);

		EntityAI enity = EntityAI.Cast(obj);
		ExpansionQuestNpcAIBase questNPC = ExpansionQuestNpcAIBase.Cast(enity);
		ExpansionHumanLoadout.Apply(questNPC, GetLoadoutFile(), false);

		questNPC.SetPosition(Position);
		questNPC.SetOrientation(Orientation);
		vector roll = questNPC.GetOrientation();
		roll [ 2 ] = roll [ 2 ] - 1;
		questNPC.SetOrientation(roll);
		roll [ 2 ] = roll [ 2 ] + 1;
		questNPC.SetOrientation(roll);

		if (questNPC.CanAffectPathgraph())
		{
			questNPC.SetAffectPathgraph(true, false);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(GetGame().UpdatePathgraphRegionByObject, 100, false, questNPC);
		}

	#ifdef EXPANSIONMODAI
		eAIGroup ownerGrp = questNPC.GetGroup();
	#else
		if (eAIGlobal_HeadlessClient)
			GetRPCManager().SendRPC("eAI", "HCLinkObject", new Param1< PlayerBase >(questNPC), false, eAIGlobal_HeadlessClient);

		eAIGame game = MissionServer.Cast(GetGame().GetMission()).GetEAIGame();
		eAIGroup ownerGrp = game.GetGroupByLeader(questNPC);
	#endif
		ownerGrp.SetFaction(new eAIFactionCivilian());
		for (int j = 0; j < Waypoints.Count(); j++)
		{
			EXPrint("Adding waypoint " + Waypoints[j]);
			ownerGrp.AddWaypoint(Waypoints[j]);
		}

	#ifdef EXPANSIONMODAI
		if (Waypoints.Count() > 1)
			ownerGrp.SetWaypointBehaviour(eAIWaypointBehavior.ALTERNATE);
		else if (Waypoints.Count() <= 1)
			ownerGrp.SetWaypointBehaviour(eAIWaypointBehavior.HALT);
	#else
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(questNPC.RequestTransition, 10000, false, "Rejoin");

		questNPC.SetAI(ownerGrp);
	#endif

		return questNPC;
	}
#endif

	ExpansionQuestStaticObject SpawnObject()
	{
		Object obj = GetGame().CreateObjectEx(GetName(), GetPosition(), ECE_ROTATIONFLAGS | ECE_PLACE_ON_SURFACE);
		
		if (Position)
			obj.SetPosition(Position);

		if (Orientation)
			obj.SetOrientation(Orientation);
		
		EntityAI enity = EntityAI.Cast(obj);
		ExpansionQuestStaticObject questObject = ExpansionQuestStaticObject.Cast(enity);
		questObject.SetPosition(Position);
		questObject.SetOrientation(Orientation);
		
		return questObject;
	}
	
	static ExpansionQuestNpcData Load(string fileName)
	{
		bool save;
		CF_Log.Info("[ExpansionQuestNPCData] Load existing configuration file:" + fileName);

		ExpansionQuestNpcData npcConfig;
		ExpansionQuestNpcDataBase npcConfigBase;
		JsonFileLoader<ExpansionQuestNpcDataBase>.JsonLoadFile(EXPANSION_QUESTS_NPCS_FOLDER + fileName, npcConfigBase);
		
		if (npcConfigBase.ConfigVersion < CONFIGVERSION)
		{
			CF_Log.Info("[ExpansionQuestNPCData] Convert existing configuration file:" + fileName + " to version " + CONFIGVERSION);
			npcConfig = new ExpansionQuestNpcData();			
			//! Copy over old configuration that haven't changed
			npcConfig.CopyConfig(npcConfigBase);
			npcConfig.ConfigVersion = CONFIGVERSION;
			save = true;
		}
		else
		{
			JsonFileLoader<ExpansionQuestNpcData>.JsonLoadFile(EXPANSION_QUESTS_NPCS_FOLDER + fileName, npcConfig);
		}
		
		if (save)
		{
			JsonFileLoader<ExpansionQuestNpcData>.JsonSaveFile(EXPANSION_QUESTS_NPCS_FOLDER + fileName, npcConfig);
		}
		
		return npcConfig;
	}

	void Save(string fileName)
	{
		JsonFileLoader<ExpansionQuestNpcData>.JsonSaveFile(EXPANSION_QUESTS_NPCS_FOLDER + fileName + ".JSON", this);
	}
	
	void CopyConfig(ExpansionQuestNpcDataBase npcDataBase)
	{
		ID = npcDataBase.ID;
		ClassName = npcDataBase.ClassName;
		IsAI = npcDataBase.IsAI;
		Position = npcDataBase.Position;
		Orientation = npcDataBase.Orientation;
		QuestIDs = npcDataBase.QuestIDs;
		NPCName = npcDataBase.NPCName;
		DefaultNPCText = npcDataBase.DefaultNPCText; 
	
	#ifdef EXPANSIONMODAI
		Waypoints = npcDataBase.Waypoints;
		NPCEmoteID = npcDataBase.NPCEmoteID;
	#endif
	}

	void OnSend(ParamsWriteContext ctx)
	{
		ctx.Write(ID);
		ctx.Write(ClassName);
		ctx.Write(NPCName);
		ctx.Write(DefaultNPCText);
	}

	bool OnRecieve(ParamsReadContext ctx)
	{
		if (!ctx.Read(ID))
		{
			Error(ToString() + "::OnRecieve - ID");
			return false;
		}

		if (!ctx.Read(ClassName))
		{
			Error(ToString() + "::OnRecieve - ClassName");
			return false;
		}

		if (!ctx.Read(NPCName))
		{
			Error(ToString() + "::OnRecieve - NPCName");
			return false;
		}

		if (!ctx.Read(DefaultNPCText))
		{
			Error(ToString() + "::OnRecieve - DefaultNPCText");
			return false;
		}

		return true;
	}

	void QuestDebug()
	{
	#ifdef EXPANSIONMODQUESTSMODULEDEBUG
		Print("------------------------------------------------------------");
		Print(ToString() + "::QuestDebug - ID: " + ID);
		Print(ToString() + "::QuestDebug - ClassName: " + ClassName);
		Print(ToString() + "::QuestDebug - NPCName: " + NPCName);
		Print(ToString() + "::QuestDebug - DefaultNPCText: " + DefaultNPCText);
		Print("------------------------------------------------------------");
	#endif
	}
};