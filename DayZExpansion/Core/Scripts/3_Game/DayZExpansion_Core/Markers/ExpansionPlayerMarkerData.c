/**
 * ExpansionPlayerMarkerData.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2022 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

#ifdef EXPANSIONMODGROUPS
class ExpansionPlayerMarkerData : ExpansionMarkerData
{
	override ExpansionMapMarkerType GetType()
	{
		return ExpansionMapMarkerType.PLAYER;
	}

	override vector GetAdjustedPosition()
	{
		vector position = GetPosition();
		Object object = GetObject();

		Human human;
		if ( Class.CastTo( human, object ) )
		{
			UpdateObject( human );

			if ( human.IsAlive() )
			{
				position = human.GetBonePositionWS( human.GetBoneIndexByName( "Head" ) ) + "0.0 0.25 0.0";
			} else
			{
				position = human.GetPosition();
			}
		}

		return position;
	}

	override void OnSend( ParamsWriteContext ctx )
	{
		ctx.Write( m_Color );
		
		ctx.Write( m_Position );
		
		OnSendObject( ctx );
	}

	override bool OnRecieve( ParamsReadContext ctx )
	{
		if ( !ctx.Read( m_Color ) )
			return false;

		if ( !ctx.Read( m_Position ) )
			return false;

		return OnReceiveObject( ctx );
	}
};
#endif