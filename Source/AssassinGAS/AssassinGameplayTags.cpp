
#include "AssassinGameplayTags.h"

namespace AssassinGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Stunned, "State.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Attacking, "State.Attacking");
	UE_DEFINE_GAMEPLAY_TAG(State_Sprinting, "State.Sprinting");
	UE_DEFINE_GAMEPLAY_TAG(State_Hidden, "State.Hidden");

	UE_DEFINE_GAMEPLAY_TAG(Ability_Attack, "Ability.Attack");
	UE_DEFINE_GAMEPLAY_TAG(Ability_HitReact, "Ability.HitReact");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Stealth, "Ability.Stealth");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Heal, "Ability.Heal");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Sprint, "Ability.Sprint");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Jumping, "State.Jumping");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Jump, "Ability.Jump");
	
	UE_DEFINE_GAMEPLAY_TAG(State_Dodging, "State.Dodging");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Dodge, "Ability.Dodge");
	
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Hit, "Event.Attack.Hit");
	UE_DEFINE_GAMEPLAY_TAG(Event_Attack_Trace_Begin, "Event.Attack.Trace.Begin");
	UE_DEFINE_GAMEPLAY_TAG(Event_HitReact, "Event.HitReact");
}
