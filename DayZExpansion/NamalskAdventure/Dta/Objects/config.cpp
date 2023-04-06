#define _ARMA_

class CfgPatches
{
	class DayZExpansion_NamalskAdventure_Objects
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgSlots
{
	class Slot_Att_ExpansionAnomalyCore
	{
		name = "Att_ExpansionAnomalyCore";
		displayName = "Anomaly Core";
		selection = "att_anomaly_core";
		ghostIcon = "set:expansion_inventory image:anomaly";
	};
};
class CfgVehicles
{
	class WorldContainer_Base;
	class Expansion_Anomaly_Base: WorldContainer_Base
	{
		scope = 2;
		model = "\DayZExpansion\NamalskAdventure\Dta\Objects\Anomalies\Expansion_Anomaly.p3d";
		bounding = "BSphere";
		forceFarBubble = "true";
		itemBehaviour = 2;
		allowOwnedCargoManipulation = 1;
		attachments[] = {"Att_ExpansionAnomalyCore"};
		itemsCargoSize[] = {10,100};
		weight = 1000000;
		inventoryCondition = "true";
		itemSize[] = {5,5};
		storageCategory = 1;
		openable = 1;
		lootCategory = "Containers";
		vehicleClass = "Inventory";
		mapSize = 1;
		class DamageSystem
		{
			class GlobalHealth
			{
				class Health
				{
					hitpoints = 100000;
					healthLevels[] = {};
				};
			};
			class GlobalArmor
			{
				class Projectile
				{
					class Health
					{
						damage = 0.0;
					};
					class Blood
					{
						damage = 0.0;
					};
					class Shock
					{
						damage = 0.0;
					};
				};
				class FragGrenade
				{
					class Health
					{
						damage = 0.0;
					};
					class Blood
					{
						damage = 0.0;
					};
					class Shock
					{
						damage = 0.0;
					};
				};
			};
		};
		class Cargo
		{
			itemsCargoSize[] = {10,100};
		};
		class GUIInventoryAttachmentsProps
		{
			class Attachments
			{
				name = "$STR_attachment_accessories";
				description = "";
				attachmentSlots[] = {"Att_ExpansionAnomalyCore"};
				icon = "set:expansion_inventory image:anomaly";
			};
		};
	};
	class Expansion_Anomaly_Singularity: Expansion_Anomaly_Base
	{
		scope = 2;
		displayName = "Anomaly - Singularity";
	};
	class Expansion_Anomaly_Teleport: Expansion_Anomaly_Base
	{
		scope = 2;
		displayName = "Anomaly - Warper";
	};
};
