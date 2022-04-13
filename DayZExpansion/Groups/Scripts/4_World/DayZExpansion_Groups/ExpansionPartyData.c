/**
 * ExpansionPartyData.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionPartyData 
{
	protected int PartyID;

	protected string PartyName;
	protected string OwnerUID;
	protected string OwnerName;

	protected ref array< ref ExpansionPartyPlayerData > Players;
	protected ref array< ref ExpansionPartyInviteData > Invites;
#ifdef EXPANSIONMODNAVIGATION
	protected ref array< ref ExpansionMarkerData > Markers;
#endif
	
	protected ref map< string, ExpansionPartyPlayerData > PlayersMap;
	protected ref map< string, ExpansionPartyInviteData > InvitesMap;
#ifdef EXPANSIONMODNAVIGATION
	protected ref map< string, ExpansionMarkerData > MarkersMap;
	//! Server
	protected ref TStringArray m_SyncMarkersPlayers;
	//! Client
	bool m_MarkersSynced;
#endif

	protected int MoneyDeposited;
		
	// ------------------------------------------------------------
	// Expansion ExpansionPartyData Consturctor
	// ------------------------------------------------------------
	void ExpansionPartyData( int partyId = -1 ) 
	{
		PartyID = partyId;

		Players = new array< ref ExpansionPartyPlayerData >;
		Invites = new array< ref ExpansionPartyInviteData >;
	#ifdef EXPANSIONMODNAVIGATION
		Markers = new array< ref ExpansionMarkerData >;
	#endif
		
		PlayersMap = new map< string, ExpansionPartyPlayerData >;
		InvitesMap = new map< string, ExpansionPartyInviteData >;
	#ifdef EXPANSIONMODNAVIGATION
		MarkersMap = new map< string, ExpansionMarkerData >;
		m_SyncMarkersPlayers = new TStringArray;
	#endif
	}
	
	// ------------------------------------------------------------
	// Expansion ExpansionPartyData Destructor
	// ------------------------------------------------------------
	void ~ExpansionPartyData() 
	{
		int i;

		for ( i = 0; i < Players.Count(); ++i )
			delete Players[i];
		for ( i = 0; i < Invites.Count(); ++i )
			delete Invites[i];
	#ifdef EXPANSIONMODNAVIGATION
		for ( i = 0; i < Markers.Count(); ++i )
			delete Markers[i];
	#endif

		delete Players;
		delete Invites;
		
	#ifdef EXPANSIONMODNAVIGATION
		delete Markers;
	#endif
		
		delete PlayersMap;
		delete InvitesMap;
	#ifdef EXPANSIONMODNAVIGATION
		delete MarkersMap;
	#endif
	}
	
	// ------------------------------------------------------------
	// Expansion InitMaps
	// ------------------------------------------------------------
	void InitMaps()
	{
		PlayersMap.Clear();
		InvitesMap.Clear();
	#ifdef EXPANSIONMODNAVIGATION
		MarkersMap.Clear();
	#endif

		int index = 0;
		
		for ( index = 0; index < Players.Count(); ++index )
			if ( Players[index] )
				PlayersMap.Insert( Players[index].UID, Players[index] );
		
		for ( index = 0; index < Invites.Count(); ++index )
			if ( Invites[index] )
				InvitesMap.Insert( Invites[index].UID, Invites[index] );
		
	#ifdef EXPANSIONMODNAVIGATION
		for ( index = 0; index < Markers.Count(); ++index )
			if ( Markers[index] )
				MarkersMap.Insert( Markers[index].GetUID(), Markers[index] );
	#endif
	}	
	// ------------------------------------------------------------
	// Expansion GetPartyID
	// ------------------------------------------------------------
	int GetPartyID()
	{
		return PartyID;
	}
	
	// ------------------------------------------------------------
	// Expansion GetPartyName
	// ------------------------------------------------------------
	string GetPartyName() 
	{
		return PartyName;
	}
	
	// ------------------------------------------------------------
	// Expansion ChangePartyName
	// ------------------------------------------------------------
	void SetPartyName(string name) 
	{
		PartyName = name;
	}
	
	// ------------------------------------------------------------
	// Expansion GetPlayers
	// ------------------------------------------------------------
	array< ref ExpansionPartyPlayerData > GetPlayers()
	{
		return Players;
	}
	
#ifdef EXPANSIONMODNAVIGATION
	// ------------------------------------------------------------
	// Expansion GetAllMarkers
	// ------------------------------------------------------------
	array<ref ExpansionMarkerData> GetAllMarkers()
	{
		return Markers;
	}
#endif
	
	// ------------------------------------------------------------
	// Expansion SetupExpansionPartyData
	// ------------------------------------------------------------
	void SetupExpansionPartyData( PlayerBase pPb, string partyName )
	{
	   	ExpansionPartyPlayerData player = AddPlayer( pPb, true );

		PartyName = partyName;
		OwnerUID = player.UID;
		OwnerName = player.Name;	
	}

	// ------------------------------------------------------------
	// Expansion AddPlayer
	// ------------------------------------------------------------
	ExpansionPartyPlayerData AddPlayer( PlayerBase pPb, bool owner = false )
	{
		string uid = pPb.GetIdentityUID();
		string name = pPb.GetIdentityName();

		ExpansionPartyPlayerData player = new ExpansionPartyPlayerData(this);
		player.UID = uid;
		player.Name = name;
		player.OnJoin( pPb );
		OnJoin( player );
	#ifdef EXPANSIONMODNAVIGATION
		player.CreateMarker();
	#endif
		player.SetPermissions(ExpansionPartyPlayerPermissions.NONE);
		
		if (owner)
		{
			player.SetPermissions(ExpansionPartyPlayerPermissions.CAN_EDIT | ExpansionPartyPlayerPermissions.CAN_INVITE | ExpansionPartyPlayerPermissions.CAN_KICK | ExpansionPartyPlayerPermissions.CAN_DELETE | ExpansionPartyPlayerPermissions.CAN_WITHDRAW_MONEY);
		}

		Players.Insert( player );
		PlayersMap.Insert( player.UID, player );

		return player;
	}
	
	// ------------------------------------------------------------
	// Expansion AcceptInvite
	// ------------------------------------------------------------
	bool AcceptInvite(PlayerBase player) 
	{
		if (RemoveInvite(player.GetIdentityUID()))
		{
			ExpansionPartyPlayerData newPlayerData = AddPlayer(player);
			if (!newPlayerData)
			{
				return false;
			}
			else
			{	
				return true;
			}
		}
		return false;
	}

	// ------------------------------------------------------------
	// Expansion DeclineInvite
	// ------------------------------------------------------------
	bool DeclineInvite(string uid)
	{
		return RemoveInvite(uid);
	}

	// ------------------------------------------------------------
	// Expansion CancelInvite
	// ------------------------------------------------------------
	bool CancelInvite( string uid )
	{
		return RemoveInvite( uid );
	}

	// ------------------------------------------------------------
	// Expansion RemoveInvite
	// ------------------------------------------------------------
	bool RemoveInvite( string uid )
	{
		ExpansionPartyInviteData invite = InvitesMap.Get( uid );
		if ( invite != NULL )
		{
			Invites.RemoveItem( invite );
			InvitesMap.Remove( uid );
			return true;
		}

		return false;
	}
	
	// ------------------------------------------------------------
	// Expansion AddInvite
	// ------------------------------------------------------------
	ExpansionPartyInviteData AddInvite( string playerID ) 
	{
		ExpansionPartyInviteData invite = InvitesMap.Get( playerID );
		if ( invite != NULL )
		{
			Error("ExpansionPartyData::AddInvite Add a player already exist");
			DumpStack();
			return invite;
		}

		invite = new ExpansionPartyInviteData;
		invite.partyID = PartyID;
		invite.UID = playerID;
		invite.partyName = PartyName;
		
		Invites.Insert( invite );
		InvitesMap.Insert( playerID, invite );
		
		return invite;
	}
		
	// ------------------------------------------------------------
	// Expansion RemoveMember
	// ------------------------------------------------------------
	bool RemoveMember(string uid)
	{
		ExpansionPartyPlayerData player = PlayersMap.Get(uid);
		if (player)
		{
			Players.RemoveItem(player);
			PlayersMap.Remove(uid);
			player.OnLeave();
			OnLeave( player );
			return true;
		}

		return false;
	}

	// ------------------------------------------------------------
	// Expansion GetPlayer
	// ------------------------------------------------------------
	ExpansionPartyPlayerData GetPlayer( string uid )
	{
		return PlayersMap.Get( uid );
	}

	// ------------------------------------------------------------
	// Expansion HasPlayerInvite
	// ------------------------------------------------------------
	bool HasPlayerInvite( string uid )
	{
		return InvitesMap.Get( uid ) != NULL;
	}
	
	// ------------------------------------------------------------
	// Expansion GetPlayerInvite
	// ------------------------------------------------------------
	ExpansionPartyInviteData GetPlayerInvite( string uid )
	{
		return InvitesMap.Get( uid );
	}
	
	// ------------------------------------------------------------
	// Expansion GetTerritoryMembers
	// ------------------------------------------------------------
	array<ref ExpansionPartyInviteData> GetPartyInvites()
	{
		return Invites;
	}
	
#ifdef EXPANSIONMODNAVIGATION
	// ------------------------------------------------------------
	// Expansion AddMarker
	// ------------------------------------------------------------
	bool AddMarker(  ExpansionMarkerData marker )
	{
		if ( marker.GetUID() == "" )
		{
			string newUid = "" + Math.RandomInt( 0, int.MAX );
			while ( MarkersMap.Get( newUid ) )
				newUid = "" + Math.RandomInt( 0, int.MAX );
			
			marker.SetUID( newUid );
		} else if ( MarkersMap.Get( marker.GetUID() ) )
			return false;

		Markers.Insert( marker );
		MarkersMap.Insert( marker.GetUID(), marker );

		SetSyncMarkers();

		return true;
	}

	void SetSyncMarkers()
	{
		m_SyncMarkersPlayers.Clear();
	}

	ExpansionMarkerData GetMarker( string uid )
	{
		return MarkersMap.Get( uid );
	}
		
	// ------------------------------------------------------------
	// Expansion UpdateMarker
	// ------------------------------------------------------------
	bool UpdateMarker(  ExpansionMarkerData marker )
	{
		ExpansionMarkerData orgi = MarkersMap.Get( marker.GetUID() );
		if ( !orgi )
			return false;

		int dummy_version = int.MAX;
		ScriptReadWriteContext ctx = new ScriptReadWriteContext;
		marker.OnStoreSave( ctx.GetWriteContext() );
		orgi.OnStoreLoad( ctx.GetReadContext(), dummy_version );

		SetSyncMarkers();

		return true;
	}

	// ------------------------------------------------------------
	// Expansion RemoveMarker
	// ------------------------------------------------------------
	bool RemoveMarker( string markerName )
	{
		ExpansionMarkerData marker = MarkersMap.Get( markerName );
		if ( marker )
		{
			int removeIndex = Markers.Find( marker );
			if ( removeIndex != -1 )
				Markers.RemoveOrdered( removeIndex );

			MarkersMap.Remove( markerName );
			delete marker;

			SetSyncMarkers();

			return true;
		}

		return false;
	}
	
	// ------------------------------------------------------------
	// Expansion UpdateMarkerPosition
	// ------------------------------------------------------------
	bool UpdateMarkerPosition( string markerName, vector markerPosition )
	{
		ExpansionMarkerData marker = MarkersMap.Get( markerName );
		if ( marker )
		{
			marker.SetPosition( markerPosition );

			SetSyncMarkers();

			return true;
		}

		return false;
	}
#endif
	
	// ------------------------------------------------------------
	// Expansion GetPartyName
	// ------------------------------------------------------------
	string GetOwnerName() 
	{
		return OwnerName;
	}
		
	// ------------------------------------------------------------
	// Expansion GetOwnerUID
	// ------------------------------------------------------------
	string GetOwnerUID()
	{
		return OwnerUID;
	}
	
	// ------------------------------------------------------------
	// Expansion SetOwnerUID
	// ------------------------------------------------------------
	void SetOwnerUID(string playerUID)
	{
		OwnerUID = playerUID;
	}
		
	// ------------------------------------------------------------
	// Expansion Save
	// ------------------------------------------------------------
	void Save() 
	{
		if ( !IsMissionHost() )
			return;
		
		FileSerializer file = new FileSerializer;
		
		if (file.Open(EXPANSION_GROUPS_FOLDER + PartyID + ".bin", FileMode.WRITE))
		{
			file.Write(EXPANSION_VERSION_MAP_MARKER_SAVE);
			OnStoreSave(file);
			
			file.Close();
		}
	}
	
	// ------------------------------------------------------------
	// Expansion Delete
	// ------------------------------------------------------------
	void Delete()
	{
		if ( !IsMissionHost() )
			return;
		
		if (EXPANSION_VERSION_MAP_MARKER_SAVE >= 4)
		{
			DeleteFile( EXPANSION_GROUPS_FOLDER + PartyID + ".bin" );
		}
		else
		{
			DeleteFile( EXPANSION_GROUPS_FOLDER + PartyID + ".json" );
		}
	}
	
	// ------------------------------------------------------------
	// Expansion CountPlayers
	// ------------------------------------------------------------
	private int CountPlayers()
	{
		int nmb = 0;
		
		for (int i = 0; i < Players.Count(); ++i)
		{
			if (!Players[i])
				continue;
			
			nmb++;
		}
		
		return nmb;
	}
	
	// ------------------------------------------------------------
	// Expansion CountInvites
	// ------------------------------------------------------------
	private int CountInvites()
	{
		int nmb = 0;
		
		for (int i = 0; i < Invites.Count(); ++i)
		{
			if (!Invites[i])
				continue;
			
			nmb++;
		}
		
		return nmb;
	}
	
#ifdef EXPANSIONMODNAVIGATION
	// ------------------------------------------------------------
	// Expansion CountMarkers
	// ------------------------------------------------------------
	private int CountMarkers()
	{
		int nmb = 0;
		
		for (int i = 0; i < Markers.Count(); ++i)
		{
			if ( !Markers[i] )
				continue;
			
			nmb++;
		}
		
		return Markers.Count();
	}
#endif

	void OnSend( ParamsWriteContext ctx, bool syncMarkers = true, string playerID = string.Empty )
	{
		ctx.Write( PartyID );
		ctx.Write( PartyName );
		ctx.Write( OwnerUID );
		ctx.Write( OwnerName );

		int count = 0;
		int index = 0;

		count = Players.Count();
		ctx.Write( count );
		for ( index = 0; index < count; ++index )
		{
			ctx.Write( Players[index].UID );
			ctx.Write( Players[index].Name );
			ctx.Write( Players[index].Permissions );

		#ifdef EXPANSIONMODNAVIGATION
			if ( Players[index].Marker )
			{
				bool hasMarker = true;
				if ( !Players[index].Marker.GetObject() )
				{
					hasMarker = false;
					// todo: update marker, maybe player respawned?
				}

				if ( Players[index].Marker.GetObject() )
				{
					Players[index].Marker.Update();
				}

				ctx.Write( hasMarker );

				if ( hasMarker )
					Players[index].Marker.OnSend( ctx );
			} else
			{
				ctx.Write( false );
			}

			if ( Players[index].QuickMarker )
			{
				ctx.Write( true );

				Players[index].QuickMarker.OnSend( ctx );
			} else
			{
				ctx.Write( false );
			}
		#endif
		}

		count = Invites.Count();
		ctx.Write( count );
		for ( index = 0; index < count; ++index )
		{
			ctx.Write( Invites[index].UID );
		}

	#ifdef EXPANSIONMODNAVIGATION
		bool syncMarkersPlayer = playerID && m_SyncMarkersPlayers.Find(playerID) > -1;
		if (!syncMarkersPlayer && syncMarkers)
		{
			count = Markers.Count();
			m_SyncMarkersPlayers.Insert(playerID);
		}
		else
		{
			count = -1;
		}

		ctx.Write( count );
		for ( index = 0; index < count; ++index )
		{
			ctx.Write( Markers[index].GetUID() );
			Markers[index].OnSend( ctx );
		}
	#endif
		
	#ifdef EXPANSIONMODMARKET
		ctx.Write( MoneyDeposited );
	#endif
	}

	bool OnRecieve( ParamsReadContext ctx )
	{
		// PartyID is read in ExpansionPartyModule		
		if ( !ctx.Read( PartyName ) )
			return false;
		if ( !ctx.Read( OwnerUID ) )
			return false;
		if ( !ctx.Read( OwnerName ) )
			return false;

		string uid = "";

		int count = 0;
		int index = 0;
		int removeIndex = 0;
		array< string > checkArr;
		if ( !ctx.Read( count ) )
			return false;

		ExpansionPartyPlayerData player;
		checkArr = PlayersMap.GetKeyArray();
		for ( index = 0; index < count; ++index )
		{
			if ( !ctx.Read( uid ) )
				return false;

			removeIndex = checkArr.Find( uid );
			if ( removeIndex != -1 )
				checkArr.Remove( removeIndex );

			player = PlayersMap.Get( uid );
			if ( !player )
			{
				player = new ExpansionPartyPlayerData(this);
				PlayersMap.Insert( uid, player );
				Players.Insert( player );
			}

			player.UID = uid;

			if ( !ctx.Read( player.Name ) )
				return false;
			
			if ( !ctx.Read( player.Permissions ) )
				return false;

		#ifdef EXPANSIONMODNAVIGATION
			bool hasMarker;
			if ( !ctx.Read( hasMarker ) )
				return false;

			if ( hasMarker )
			{
				if ( !player.Marker )
					player.Marker = new ExpansionPlayerMarkerData( uid );

				if ( !player.Marker.OnRecieve( ctx ) )
					return false;
			} else
			{
				if ( player.Marker )
					delete player.Marker;
			}

			bool hasQuickMarker;
			if ( !ctx.Read( hasQuickMarker ) )
				return false;

			if ( hasQuickMarker )
			{
				if ( !player.QuickMarker )
					player.QuickMarker = new ExpansionPartyQuickMarkerData( "QuickMarker" + uid );

				if ( !player.QuickMarker.OnRecieve( ctx ) )
					return false;
			} else
			{
				if ( player.QuickMarker )
					delete player.QuickMarker;
			}
		#endif
		}
		
		for ( index = 0; index < checkArr.Count(); ++index )
		{
			player = PlayersMap.Get( checkArr[index] );
			if ( player )
			{
				removeIndex = Players.Find( player );
				if ( removeIndex != -1 )
					Players.Remove( removeIndex );

				PlayersMap.Remove( checkArr[index] );
				delete player;
			}
		}
		
		if ( !ctx.Read( count ) )
			return false;

		ExpansionPartyInviteData invite;
		checkArr = InvitesMap.GetKeyArray();
		for ( index = 0; index < count; ++index )
		{
			if ( !ctx.Read( uid ) )
				return false;

			removeIndex = checkArr.Find( uid );
			if ( removeIndex != -1 )
				checkArr.Remove( removeIndex );

			invite = InvitesMap.Get( uid );
			if ( !invite )
			{
				invite = new ExpansionPartyInviteData();
				InvitesMap.Insert( uid, invite );
				Invites.Insert( invite );
			}
		}
		for ( index = 0; index < checkArr.Count(); ++index )
		{
			invite = InvitesMap.Get( checkArr[index] );
			if ( invite )
			{
				removeIndex = Invites.Find( invite );
				if ( removeIndex != -1 )
					Invites.Remove( removeIndex );

				InvitesMap.Remove( checkArr[index] );
				delete invite;
			}
		}
		
		if ( !ctx.Read( count ) )
			return false;

	#ifdef EXPANSIONMODNAVIGATION
		if (count > -1)
		{
			ExpansionMarkerData marker;
			checkArr = MarkersMap.GetKeyArray();
			for ( index = 0; index < count; ++index )
			{
				if ( !ctx.Read( uid ) )
					return false;

				removeIndex = checkArr.Find( uid );
				if ( removeIndex != -1 )
					checkArr.Remove( removeIndex );

				marker = MarkersMap.Get( uid );
				if ( !marker )
				{
					marker = ExpansionMarkerData.Create( ExpansionMapMarkerType.PARTY, uid );
					MarkersMap.Insert( uid, marker );
					Markers.Insert( marker );
				}

				if ( !marker.OnRecieve( ctx ) )
					return false;
			}
			for ( index = 0; index < checkArr.Count(); ++index )
			{
				marker = MarkersMap.Get( checkArr[index] );
				if ( marker )
				{
					removeIndex = Markers.Find( marker );
					if ( removeIndex != -1 )
						Markers.RemoveOrdered( removeIndex );

					MarkersMap.Remove( checkArr[index] );
					delete marker;
				}
			}

			m_MarkersSynced = true;
		}
	#endif
	
	#ifdef EXPANSIONMODMARKET
		if ( !ctx.Read( MoneyDeposited ) )
			return false;
	#endif

		return true;
	}
	
	// ------------------------------------------------------------
	// Expansion OnStoreSave
	// ------------------------------------------------------------
	void OnStoreSave( ParamsWriteContext ctx )
	{
		ctx.Write(PartyID);
		ctx.Write(PartyName);
		ctx.Write(OwnerUID);
		ctx.Write(OwnerName);
		
		int i = 0;
		
		ctx.Write( Players.Count() );
		for ( i = 0; i < Players.Count(); ++i )
			Players[i].OnStoreSave( ctx );
		
		ctx.Write( Invites.Count() );
		for ( i = 0; i < Invites.Count(); ++i)
			Invites[i].OnStoreSave( ctx );
		
	#ifdef EXPANSIONMODNAVIGATION
		ctx.Write( Markers.Count() );
		for ( i = 0; i < Markers.Count(); ++i )
		{
			ctx.Write( Markers[i].GetUID() );
			Markers[i].OnStoreSave( ctx );
		}
	#else
		//! Always write zero marker count so it doesn't cause issues when navigation is added later
		ctx.Write( 0 );
	#endif
		
		//! Always write money deposited so it doesn't cause issues when market is added later
		ctx.Write( MoneyDeposited );
	}
	
	// ------------------------------------------------------------
	// Expansion OnStoreLoad
	// ------------------------------------------------------------
	bool OnStoreLoad( ParamsReadContext ctx, int version )
	{
		if ( Expansion_Assert_False( ctx.Read( PartyID ), "Failed reading party uid" ) )
			return false;
		
		if ( Expansion_Assert_False( ctx.Read( PartyName ), "Failed reading party name" ) )
			return false;
		
		if ( Expansion_Assert_False( ctx.Read( OwnerUID ), "Failed reading owner uid" ) )
			return false;
		
		if ( Expansion_Assert_False( ctx.Read( OwnerName ), "Failed reading owner name" ) )
			return false;
		
		int i;

		int countPlayers;
		if ( Expansion_Assert_False( ctx.Read( countPlayers ), "Failed reading player count" ) )
			return false;
		
		for ( i = 0; i < countPlayers; ++i )
			Players.Insert( new ExpansionPartyPlayerData(this) );

		for ( i = 0; i < Players.Count(); ++i )
			if ( Expansion_Assert_False( Players[i].OnStoreLoad( ctx, version ), "Failed reading player [" + i + "]" ) )
				return false;
		
		int countInvites;
		if ( Expansion_Assert_False( ctx.Read( countInvites ), "Failed reading invite count" ) )
			return false;
		
		for ( i = 0; i < countInvites; ++i )
			Invites.Insert( new ExpansionPartyInviteData );

		for ( i = 0; i < Invites.Count(); ++i )
			if ( Expansion_Assert_False( Invites[i].OnStoreLoad( ctx, version ), "Failed reading invite [" + i + "]" ) )
				return false;

		//! Always read marker count so it doesn't cause issues when navigation is added later
		int countMarkers;
		if ( Expansion_Assert_False( ctx.Read( countMarkers ), "Failed reading marker count" ) )
			return false;
		
	#ifdef EXPANSIONMODNAVIGATION
		for ( i = 0; i < countMarkers; ++i )
			Markers.Insert( ExpansionMarkerData.Create( ExpansionMapMarkerType.PARTY ) );

		for ( i = 0; i < Markers.Count(); ++i )
		{
			string marker_uid;

			if ( version >= 8 )
			{
				if ( Expansion_Assert_False( ctx.Read( marker_uid ), "Failed reading marker_uid [" + i + "]" ) )
					return false;
			} else
			{
				marker_uid = "marker_old_" + i;
			}

			Markers[i].SetUID( marker_uid );

			if ( Expansion_Assert_False( Markers[i].OnStoreLoad( ctx, version ), "Failed reading marker [" + i + "]" ) )
				return false;
		}
	#endif
		
		//! Read money deposited so it doesn't cause issues when market is added later
		bool readMoneyDeposited;
	#ifdef EXPANSIONMODMARKET
		readMoneyDeposited = version >= 26;
	#else
		readMoneyDeposited = version >= 39;
	#endif
		if ( readMoneyDeposited && Expansion_Assert_False( ctx.Read( MoneyDeposited ), "Failed reading party money deposit data" ) )
			return false;
		
		return true;
	}

	void OnJoin(ExpansionPartyPlayerData player)
	{
		EXPrint(ToString() + "::OnJoin party " + PartyName + " player " + player.Name + " (ID " + player.GetID() + ")");
	#ifdef EXPANSIONMODNAVIGATION
		SyncMarkers_RemovePlayer(player);
	#endif
	}

	void OnLeave(ExpansionPartyPlayerData player)
	{
		EXPrint(ToString() + "::OnLeave party " + PartyName + " player " + player.Name + " (ID " + player.GetID() + ")");
	#ifdef EXPANSIONMODNAVIGATION
		SyncMarkers_RemovePlayer(player);
	#endif
	}
	
#ifdef EXPANSIONMODNAVIGATION
	void SyncMarkers_RemovePlayer(ExpansionPartyPlayerData player)
	{
		int idx = m_SyncMarkersPlayers.Find(player.GetID());
		if (idx > -1)
			m_SyncMarkersPlayers.Remove(idx);
	}
#endif

	bool IsMember(string uid)
	{		
		if (GetPlayer(uid))	
			return true;
		
		return false;
	}
	
	bool HasInvite(string uid)
	{
		for (int i = 0; i < Invites.Count(); ++i)
		{
			if (Invites[i] && Invites[i].GetID() == uid)
			{
				return true;
			}
		}
		
		return false;
	}
	
#ifdef EXPANSIONMODMARKET
	int GetMoneyDeposited()
	{
		return MoneyDeposited;
	}
	
	void RemoveMoney(int amount)
	{
		MoneyDeposited -= amount;
	}
	
	void AddMoney(int amount)
	{
		MoneyDeposited += amount;
	}
	
	void SetMoney(int amount)
	{
		MoneyDeposited = amount;
	}
#endif
};