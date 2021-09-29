/**
 * ExpansionVehicleWheel.c
 *
 * DayZ Expansion Mod
 * www.dayzexpansion.com
 * © 2021 DayZ Expansion Mod Team
 *
 * This work is licensed under the Creative Commons Attribution-NonCommercial-NoDerivatives 4.0 International License. 
 * To view a copy of this license, visit http://creativecommons.org/licenses/by-nc-nd/4.0/.
 *
*/

class ExpansionVehicleWheel : ExpansionVehicleModule
{
	string m_InventorySlot;
	string m_Selection;
	string m_Name;

	ExpansionVehicleAxle m_Axle;
	ExpansionWheelBase m_WheelItem;

	float m_Radius;
	float m_TraceRadius;

	vector m_InitialTransform[4];
	vector m_WheelOffset;

	vector m_TransformWS[4];
	vector m_TransformMS[4];

	// wheel animations
	string m_AnimTurn;
	string m_AnimRotation;
	string m_AnimDamper;
	string m_WheelHub;

	// Suspension
	bool m_HasContact;
	Object m_ContactObject;
	vector m_ContactPosition;
	vector m_ContactNormal;
	float m_ContactFraction;
	float m_ContactLength;

	vector m_TraceStart;
	float m_TraceUp;
	float m_TraceDown;

	vector m_ContactPositionWS;
	vector m_ContactNormalWS;

	float m_SuspensionLength;
	float m_SuspensionFraction;
	vector m_SuspensionForce;

	float m_SuspensionRelativeVelocity;
	float m_SuspensionInvContact;

	vector m_ContactVelocity;

	vector m_TireLongDir;
	vector m_TireLatDir;
	float m_TireLongSpeed;
	float m_TireLatSpeed;

	vector m_RayStartMS;
	vector m_RayStartWS;
	vector m_RayEndMS;
	vector m_RayEndWS;

	vector m_ForwardWS;
	vector m_AxleWS;

	float m_Acceleration;
	float m_Velocity;
	float m_RotationPosition;
	float m_RPM;

	float m_ForwardForce;
	float m_SideForce;

	float m_WheelTorque;
	float m_BrakeTorque;

	float m_Steering;

	string m_Surface;

	float m_SurfaceFriction;

	void ExpansionVehicleWheel(EntityAI pVehicle, string pName, ExpansionVehicleAxle pAxle)
	{
		m_Name = pName;
		m_Axle = pAxle;

		string path;

		path = "CfgVehicles " + m_Vehicle.GetType() + " SimulationModule Axles " + m_Axle.m_Name + " Wheels " + m_Name + " inventorySlot";
		m_InventorySlot = GetGame().ConfigGetTextOut(path);

		path = "CfgVehicles " + m_Vehicle.GetType() + " SimulationModule Axles " + m_Axle.m_Name + " Wheels " + m_Name + " animTurn";
		m_AnimTurn = GetGame().ConfigGetTextOut(path);

		path = "CfgVehicles " + m_Vehicle.GetType() + " SimulationModule Axles " + m_Axle.m_Name + " Wheels " + m_Name + " animRotation";
		m_AnimRotation = GetGame().ConfigGetTextOut(path);

		path = "CfgVehicles " + m_Vehicle.GetType() + " SimulationModule Axles " + m_Axle.m_Name + " Wheels " + m_Name + " animDamper";
		m_AnimDamper = GetGame().ConfigGetTextOut(path);

		path = "CfgVehicles " + m_Vehicle.GetType() + " SimulationModule Axles " + m_Axle.m_Name + " Wheels " + m_Name + " wheelHub";
		m_WheelHub = GetGame().ConfigGetTextOut(path);

		m_InitialTransform[0] = "-1 0 0";
		m_InitialTransform[1] = "0 -1 0";
		m_InitialTransform[2] = "0 0 -1";
		m_InitialTransform[3] = ExpansionVehiclesStatic.GetCenterSelection(m_Vehicle, "geometry", m_WheelHub);

		string n_axis_start;
		string n_axis_end;

		path = "CfgVehicles " + m_Vehicle.GetType() + " SimulationModule Axles " + m_Axle.m_Name + " Wheels " + m_Name + " axis_start";
		if (GetGame().ConfigGetText(path, n_axis_start))
		{
			path = "CfgVehicles " + m_Vehicle.GetType() + " SimulationModule Axles " + m_Axle.m_Name + " Wheels " + m_Name + " axis_end";
			if (GetGame().ConfigGetText(path, n_axis_end))
			{
				vector axis_start = m_Vehicle.GetMemoryPointPos(n_axis_start);
				vector axis_end = m_Vehicle.GetMemoryPointPos(n_axis_end);
				m_InitialTransform[1] = vector.Direction(axis_end, axis_start).Normalized();
				m_WheelOffset = m_InitialTransform[3] - axis_start;
			}
		}
	}

	void OnWheelAttach(notnull ExpansionWheelBase pWheel)
	{
		m_WheelItem = pWheel;

		m_Axle.UpdateWheelRadius();
	}

	void OnWheelDetach()
	{
		m_WheelItem = NULL;
	}

#ifdef CF_DebugUI
	override bool CF_OnDebugUpdate(CF_Debug instance, CF_DebugUI_Type type)
	{
		instance.Add("Wheel", m_WheelHub);

		instance.Add("Contact", m_HasContact);
		instance.Add("Surface", m_Surface);
		instance.Add("Surface Friction", m_SurfaceFriction);

		return true;
	}
#endif

	override void PreSimulate(ExpansionPhysicsState pState)
	{
		if (!m_WheelItem)
		{
			m_RPM = 0;
			return;
		}

		m_TraceRadius = m_Axle.m_WheelHubRadius;

		m_Radius = m_WheelItem.m_Radius;

		vector wheelTransform[4];
		wheelTransform[0] = -m_InitialTransform[0];
		wheelTransform[1] = -m_InitialTransform[1];
		wheelTransform[2] = -m_InitialTransform[2];
		wheelTransform[3] = m_InitialTransform[3]; // + (m_SuspensionLength * -m_InitialTransform[1]);

		vector rotationTransform[4];
		Math3D.YawPitchRollMatrix(Vector(m_Steering, 0, 0), rotationTransform);
		rotationTransform[3] = m_WheelOffset;

		Math3D.MatrixMultiply4(wheelTransform, rotationTransform, m_TransformMS);

		m_TraceStart = m_TransformMS[3];

		float safeChecking = 0; //Math.Max(-m_Vehicle.GetModelVelocityAt(m_TraceStart)[1] / 40.0, 0) * 5.0; //! checking 2 frames ahead

		m_TraceUp = m_Axle.m_TravelMaxUp + m_Radius + m_Radius;
		m_TraceDown = m_Axle.m_TravelMaxDown + m_Radius + safeChecking;

		m_ContactLength = m_TraceUp + m_TraceDown;

		PhxInteractionLayers collisionLayerMask = PhxInteractionLayers.BUILDING | PhxInteractionLayers.DOOR | PhxInteractionLayers.VEHICLE | PhxInteractionLayers.ROADWAY | PhxInteractionLayers.TERRAIN | PhxInteractionLayers.ITEM_SMALL | PhxInteractionLayers.ITEM_LARGE | PhxInteractionLayers.FENCE;

		m_RayStartMS = m_TraceStart - (m_InitialTransform[1] * (m_TraceUp - m_TraceRadius));
		m_RayEndMS = m_TraceStart + (m_InitialTransform[1] * (m_TraceDown - m_TraceRadius));

		m_RayStartWS = m_RayStartMS.Multiply4(pState.m_Transform);
		m_RayEndWS = m_RayEndMS.Multiply4(pState.m_Transform);

#ifndef EXPANSION_WHEEL_DEBUG_DISABLE
		//pState.DBGDrawLineMS(m_TraceStart - (m_InitialTransform[1] * m_TraceUp), m_TraceStart + (m_InitialTransform[1] * m_TraceDown), 0xFFFFFFFFF);
#endif

		m_HasContact = DayZPhysics.SphereCastBullet(m_RayStartWS, m_RayEndWS, m_TraceRadius, collisionLayerMask, m_Vehicle, m_ContactObject, m_ContactPositionWS, m_ContactNormalWS, m_ContactFraction);

		/*
		int contactComponent;
		set<Object> results = new set<Object>();
		m_HasContact = DayZPhysics.RaycastRV(m_RayStartWS, m_RayEndWS, m_ContactPositionWS, m_ContactNormalWS, contactComponent, results, m_Vehicle, m_Vehicle, false, false, ObjIntersectGeom, m_TraceRadius, CollisionFlags.FIRSTCONTACT);
		
		m_ContactPosition = m_ContactPositionWS.InvMultiply4(pState.m_Transform);
		m_ContactNormal = m_ContactNormalWS.InvMultiply3(pState.m_Transform);

		m_ContactFraction = vector.Distance(m_TraceStart - (m_InitialTransform[1] * m_TraceUp), m_ContactPosition) / m_ContactLength;
		*/

#ifndef EXPANSION_WHEEL_DEBUG_DISABLE
		pState.DBGDrawSphereMS(m_TransformMS[3], 0.05, 0xFF0000FF);

		pState.DBGDrawSphere(m_RayStartWS, m_TraceRadius, 0xFFFF0000);
		pState.DBGDrawSphere(m_RayEndWS, m_TraceRadius, 0xFFFF0000);
		pState.DBGDrawSphere(m_ContactPositionWS, m_TraceRadius, 0xFFFF00FF);

		pState.DBGDrawLineDirectionMS(m_ContactPosition, m_ContactNormal, 0xFFFF00FF);
#endif

		if (m_HasContact)
		{
			float wheelDiff = vector.Dot(m_ContactNormal, m_InitialTransform[1]);
			if (wheelDiff >= -0.1)
			{
				m_HasContact = false;
			}
			else
			{
				m_ContactVelocity = pState.GetModelVelocityAt(m_ContactPosition);

				float invWheelDiff = -1.0 / wheelDiff;
				m_SuspensionRelativeVelocity = Math.Clamp(vector.Dot(m_ContactNormal, m_ContactVelocity) * invWheelDiff, -1, 1);

				m_SuspensionInvContact = invWheelDiff;

				m_SuspensionLength = m_ContactLength * (1.0 - m_ContactFraction);
				m_SuspensionLength = Math.Clamp(m_SuspensionLength, 0, m_Axle.m_TravelMax);

				m_SuspensionFraction = m_SuspensionLength / m_Axle.m_TravelMax;

				float compressionDelta = m_Axle.m_Damping * m_SuspensionRelativeVelocity;
				if (m_SuspensionRelativeVelocity < 0)
					compressionDelta = m_Axle.m_Compression * m_SuspensionRelativeVelocity;
				float suspension = m_Axle.m_Stiffness * m_SuspensionLength;

				m_SuspensionForce = m_ContactNormal * (suspension - compressionDelta) * pState.m_DeltaTime;

				GetGame().SurfaceGetType3D(m_ContactPositionWS[0], m_ContactPositionWS[1], m_ContactPositionWS[2], m_Surface);
			}
		}

		if (!m_HasContact)
		{
			m_SuspensionForce = vector.Zero;

			m_ContactFraction = 1.0;

			m_ContactVelocity = "0 0 0";

			m_SuspensionRelativeVelocity = 0.0;
			m_SuspensionInvContact = 10.0;

			m_ContactPosition = m_TraceStart + (m_InitialTransform[1] * m_TraceDown);
			m_ContactNormal = -m_InitialTransform[1];

			m_ContactPositionWS = m_ContactPosition.Multiply4(pState.m_Transform);
			m_ContactNormalWS = m_TransformWS[1];

			m_Surface = "";
		}

		Math3D.MatrixMultiply4(pState.m_Transform, m_TransformMS, m_TransformWS);

#ifndef EXPANSION_WHEEL_DEBUG_DISABLE
		pState.DBGDrawSphere(m_TransformWS[3], 0.05, 0xFF0000FF);
#endif
	}

	override void Simulate(ExpansionPhysicsState pState)
	{
		vector impulse;
		vector impulseTorque;

		m_SurfaceFriction = 1.0;

		if (m_HasContact && m_Surface != "")
			m_SurfaceFriction = Surface.GetParamFloat(m_Surface, "friction");

		m_ForwardForce = m_WheelTorque * m_SurfaceFriction * pState.m_Mass * m_Radius;
		m_ForwardForce -= m_WheelItem.m_TyreRollResistance * m_Velocity * pState.m_Mass;

		if (m_HasContact)
		{
			impulse += m_SuspensionForce;
			impulseTorque += m_ContactPosition * m_SuspensionForce;

			m_Velocity = m_ContactVelocity[2];

			m_ForwardForce -= Math.Sign(m_Velocity) * m_BrakeTorque * Math.Lerp(0.0, 1.0, Math.Clamp(Math.AbsFloat(m_Velocity), 0, 1));
			//m_ForwardForce -= m_SurfaceFriction * m_Velocity * pState.m_Mass;

			vector axle = m_TransformWS[0];
			float proj = vector.Dot(axle, m_ContactNormalWS);
			axle -= m_ContactNormalWS * proj;
			axle.Normalize();

			m_SideForce = ExpansionPhysics.ResolveSingleBilateral(m_Vehicle, m_ContactPosition, m_ContactVelocity.Multiply3(pState.m_Transform), m_ContactObject, "0 0 0", axle);

			vector forward = m_TransformMS[2] * m_ForwardForce * pState.m_DeltaTime;

			impulse += forward;
			impulseTorque += m_ContactPosition * forward;

			vector side = m_TransformMS[0] * m_SideForce * pState.m_DeltaTime;

			impulse += side;
			impulseTorque += m_ContactPosition * side;
		}

		m_Acceleration = m_ForwardForce / pState.m_Mass;
		m_Velocity += m_Acceleration * pState.m_DeltaTime;

		m_RotationPosition += m_Velocity * pState.m_DeltaTime / m_Radius;
		#ifndef DAYZ_1_13
		m_RotationPosition = Math.WrapFloatInclusive(m_RotationPosition, 0, Math.PI2);
		#else
		if (m_RotationPosition > Math.PI2)
			m_RotationPosition -= Math.PI2;
		else if (m_RotationPosition < 0)
			m_RotationPosition += Math.PI2;
		#endif

		m_RPM = Math.AbsFloat((m_Velocity * 30.0) / (Math.PI * m_Radius));

		// convert wheel forces to world space
		pState.m_Impulse += impulse.Multiply3(pState.m_Transform);
		pState.m_ImpulseTorque += impulseTorque.Multiply3(pState.m_Transform);
	}

	override void Animate(ExpansionPhysicsState pState)
	{
		m_Vehicle.SetAnimationPhase(m_AnimDamper, m_SuspensionFraction);
		m_Vehicle.SetAnimationPhase(m_AnimTurn, m_Steering * Math.DEG2RAD);
		m_Vehicle.SetAnimationPhase(m_AnimRotation, m_RotationPosition);
	}
};