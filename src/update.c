/*
 * $Id: update.c 1019 2007-02-15 00:52:41Z zsuzsu $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT		           *	
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Mandrake}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "debug.h"
#include "update.h"
#include "mob_prog.h"
#include "obj_prog.h"
#include "fight.h"
#include "waffects.h"
#include "stats.h"

/* command procedures needed */
DECLARE_DO_FUN(do_human		);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_rescue	);
DECLARE_DO_FUN(do_stand		);
DECLARE_DO_FUN(do_track		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_asave		);

void	back_home		(CHAR_DATA *ch);

/*
 * Local functions.
 */
int	hit_gain	(CHAR_DATA *ch);
int	mana_gain	(CHAR_DATA *ch);
int	move_gain	(CHAR_DATA *ch);
void	mobile_update	(void);
void	weather_update	(void);
void	char_update	(void);
void	obj_update	(void);
void	aggr_update	(void);
void 	clan_item_update(void);
int	potion_cure_level	(OBJ_DATA *potion);
int	potion_arm_level	(OBJ_DATA *potion);
bool	potion_cure_blind	(OBJ_DATA *potion);
bool	potion_cure_poison	(OBJ_DATA *potion);
bool	potion_cure_disease	(OBJ_DATA *potion);

/* below done by chronos */
void    quest_update    args((void));
void	auction_update	args((void));
void	light_update	args((void));
void	room_update	args((void));
void	room_affect_update	args((void));
void	check_reboot	args((void));
void	track_update	args((void));

/* used for saving */

int	save_number = 0;


int	rebooter = 0;

/*
 * Advancement stuff.
 */
void advance_level(CHAR_DATA *ch)
{
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac;
	int add_dam;
	int add_nim;
	int i;
	class_t *cl;

	if (IS_NPC(ch)) {
		bug("Advance_level: a mob to advance!", 0);
		return;
	}

	if ((cl = class_lookup(ch->class)) == NULL) {
		log_printf("advance_level", "%s: unknown class %d",
			   ch->name, ch->class);
		return;
	}

	ch->pcdata->last_level = (ch->played +
		  (int) (current_time - ch->logon)) / 3600;

	set_title(ch, title_lookup(ch));

	if (ch->pcdata->plevels > 0) {
		ch->pcdata->plevels--;
		return;
	}

	if (ch->level <= 1) {
		char_printf(ch, "\n");
		return;
	}

	/* hp */
	add_hp = ch_stat_avg(ch, STAT_CON);
	add_hp += number_range(add_hp * -20/100,
		add_hp * 20/100);
	add_hp = (add_hp * cl->hp_rate) / 100;
	add_hp = add_hp * 2/3;
	add_hp = UMAX(5, add_hp);

	/* mana */
	add_mana = (ch_stat_avg(ch, STAT_INT) * 2/3
		+ ch_stat_avg(ch, STAT_WIS) * 1/3);
	add_mana += number_range(add_mana * -30/100,
		add_mana * 30/100);
	add_mana = add_mana / 2;
	add_mana = (add_mana * cl->mana_rate) / 100;
	add_mana = UMAX(0, add_mana);

	/* move */
	add_move = ch_stat_avg(ch, STAT_DEX) * 2/3 
		+ ch_stat_avg(ch, STAT_CON) * 1/3;
	add_move += number_range(add_move * -50/100,
		add_move * 50/100);
	add_move = add_move / 7;
	add_move = UMAX(1, add_move);

	/* dam */
	add_dam = ch_stat_avg(ch, STAT_STR);
	add_dam += number_range(add_dam * -20/100,
		add_dam * 20/100);
	add_dam = (add_dam * cl->dam_rate) / 100;
	add_dam = add_dam / 10;
	add_dam = UMAX(0, add_dam);

	/* nimroll */
	add_nim = ch_stat_avg(ch, STAT_DEX);
	add_nim += number_range(add_nim * -20 / 100,
		add_nim * 20/100);
	if (cl->nim_rate < 1)
		BUG("nimrate for class %s isn't set", cl->name);
	add_nim = (add_nim * cl->nim_rate) / 100;
	add_nim = add_nim / 10;
	add_nim = UMAX(0, add_nim);

	/* prac */
	add_prac = ch_practice_per_level(ch);

/*
	if (ch->sex == SEX_FEMALE) {
		add_hp   -= 1;
		add_mana += 2;
	}
*/

	ch->max_hit += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;
	ch->practice += add_prac;

	if (STAT_TRAIN_RATE > 0)
		ch->train += ch->level % 3 ? 0 : 1;

	ch->pcdata->perm_hit += add_hp;
	ch->pcdata->perm_mana += add_mana;
	ch->pcdata->perm_move += add_move;
	ch->pcdata->perm_dam += add_dam;
	ch->pcdata->perm_nim += add_nim;

	for (i = 0; i < MAX_STAT; i++) {
		ch->pcdata->stat_avg[i][STAT_AVG_TOTAL] = get_curr_stat(ch, i);
		ch->pcdata->stat_avg[i][STAT_AVG_NUM] = 1;
		ch->pcdata->trend_stat[i] += ch->perm_stat[i];
	}

	char_printf(ch, "Your gain is");
	if (add_hp > 0)   char_printf(ch, " {C%d{xhp",      add_hp);
	if (add_mana > 0) char_printf(ch, " {C%d{xmana",    add_mana);
	if (add_move > 0) char_printf(ch, " {C%d{xmv",      add_move);
	if (add_dam > 0)  char_printf(ch, " {C%d{xdamroll", add_dam);
	if (add_nim > 0)  char_printf(ch, " {C%d{xnimroll", add_nim);
	if (add_prac > 0) char_printf(ch, " {C%d{xprac",    add_prac);
	char_printf(ch, ".\n");
}   

void gain_exp(CHAR_DATA *ch, int gain)
{
	int i = 0;

	if (IS_NPC(ch) || ch->level >= LEVEL_HERO)
		return;

	if (IS_SET(ch->state_flags, STATE_NOEXP) && gain > 0) {
		char_puts("You can't gain exp without your spirit.\n", ch);
		return;
	}

	ch->exp += gain;
	ch->exp_tl += gain;


	if (IS_NEWBIE(ch) && ch->level == MAX_NEWBIE_LEVEL) {
		char_puts("Sorry, you have reached the maximum level for a newbie.\n"
			  "If you feel you have learned enough about our realm please\n"
			  "Delete this character and create an non-newbie character to\n"
			  "experience everything this realm has to offer.\n", ch);
		return;
	}

	for (i = 0; i < MAX_STATS; i++) {
		ch->pcdata->stat_avg[i][STAT_AVG_TOTAL] += get_curr_stat(ch, i);
		ch->pcdata->stat_avg[i][STAT_AVG_NUM]++;
	}

	while (ch->level < LEVEL_HERO && exp_to_level(ch) <= 0) {
		class_t *cl;

		char_puts("{CYou raise a level!!{x ", ch);
		ch->level++;
		ch->exp_tl = 0;

		if ((cl = class_lookup(ch->class)) != NULL
		&&  cl->death_limit != 0
		&&  ch->level == MIN_PK_LEVEL)
			ch->wimpy = 0;

		if (ch->level == LEVEL_HERO)
	        	log_printf("{R%s made Hero.{x", ch->name);

		wiznet("{W$N {Bhas attained level {Y$j!{x",
			ch, (const void*) ch->level, WIZ_LEVELS, 0, 0);
		advance_level(ch);
		save_char_obj(ch, FALSE);
	}
}

/*
 * Regeneration stuff.
 */
int hit_gain(CHAR_DATA *ch)
{
	int gain;
	int number;
	class_t *cl;

	if (ch->in_room == NULL || (cl = class_lookup(ch->class)) == NULL)
		return 0;

	if (is_affected(ch, gsn_condemnation))
		return 0;

	if (IS_NPC(ch)) {
		gain =  5 + ch->level;
 		if (IS_AFFECTED(ch, AFF_REGENERATION))
			gain *= 2;

		switch(ch->position) {
		default:		gain /= 2;		break;
		case POS_SLEEPING:	gain = 3 * gain/2;	break;
		case POS_RESTING:				break;
		case POS_FIGHTING:	gain /= 3;		break;
 		}
	}
	else {
		gain = UMAX(3, get_curr_stat(ch,STAT_CON)/2 
			+ (7 * ch->level) / 4); 
		gain = (gain * cl->hp_rate) / 100;
 		number = number_percent();
		if (number < get_skill(ch, gsn_fast_healing)) {
			gain += number * gain / 100;
			if (ch->hit < ch->max_hit)
				check_improve(ch, gsn_fast_healing, TRUE, 8);
		}

		if (number < get_skill(ch, gsn_trance)) {
			gain += number * gain / 150;
			if (ch->mana < ch->max_mana)
				check_improve(ch, gsn_trance, TRUE, 8);
		}

		switch (ch->position) {
		default:		gain /= 4;		break;
		case POS_SLEEPING: 				break;
		case POS_RESTING:	gain /= 2;		break;
		case POS_FIGHTING:	gain /= 6;		break;
		}

		if (ch->pcdata->condition[COND_HUNGER]   < 0)
			gain = 0;

		if (ch->pcdata->condition[COND_THIRST] < 0)
			gain = 0;
	}

	gain = gain * ch->in_room->heal_rate / 100;
	
	if (ch->on != NULL && ch->on->pIndexData->item_type == ITEM_FURNITURE)
		gain = gain * ch->on->value[ITEM_FURNITURE_HEAL_RATE] / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
		gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
		gain /= 8;

	if (IS_AFFECTED(ch, AFF_HASTE))
		gain /= 2;

	if (IS_AFFECTED(ch, AFF_SLOW))
		gain /= 2;

	if (IS_HARA_KIRI(ch))
		gain *= 3;

	return UMIN(gain, ch->max_hit - ch->hit);
}

int mana_gain(CHAR_DATA *ch)
{
	int gain;
	int number;
	class_t *cl;

	if (ch->in_room == NULL || (cl = class_lookup(ch->class)) == NULL)
		return 0;

	if (IS_NPC(ch))
	{
	gain = 5 + ch->level;
	switch (ch->position)
	{
	    default:		gain /= 2;		break;
	    case POS_SLEEPING:	gain = 3 * gain/2;	break;
   	    case POS_RESTING:				break;
	    case POS_FIGHTING:	gain /= 3;		break;
		}
	}
	else
	{
	gain = get_curr_stat(ch,STAT_WIS)/3
		      + (get_curr_stat(ch,STAT_INT)/3) + ch->level;
	gain = (gain * cl->mana_rate) / 100;

	number = number_percent();
	if (number < get_skill(ch,gsn_meditation)) {
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,gsn_meditation,TRUE,8);
	}

	if (number < get_skill(ch,gsn_trance)) {
	    gain += number * gain / 100;
	    if (ch->mana < ch->max_mana)
	        check_improve(ch,gsn_trance,TRUE,8);
	}

	if (!IS_SET(cl->flags, CLASS_MAGIC))
		gain /= 2;

	switch (ch->position) {
	    default:		gain /= 4;			break;
	    case POS_SLEEPING: 					break;
	    case POS_RESTING:	gain /= 2;			break;
	    case POS_FIGHTING:	gain /= 6;			break;
	}

	if (ch->pcdata->condition[COND_HUNGER]   < 0)
	    gain = 0;

	if (ch->pcdata->condition[COND_THIRST] < 0)
	    gain = 0;

	}

	gain = gain * ch->in_room->mana_rate / 100;

	if (ch->on != NULL && ch->on->pIndexData->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[ITEM_FURNITURE_MANA_RATE] / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
	    gain /= 8;

	if (IS_AFFECTED(ch,AFF_HASTE))
	    gain /= 2 ;
	if (IS_AFFECTED(ch,AFF_SLOW))
	    gain /= 2 ;
	if (IS_HARA_KIRI(ch))
	gain *= 3;

	return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain(CHAR_DATA *ch)
{
	int gain;

	if (ch->in_room == NULL)
	return 0;

	if (IS_NPC(ch))
	{
	gain = ch->level;
	}
	else
	{
	gain = UMAX(15, 2 * ch->level);

	switch (ch->position)
	{
	case POS_SLEEPING: gain += (get_curr_stat(ch,STAT_DEX)/2);	break;
	case POS_RESTING:  gain += get_curr_stat(ch,STAT_DEX)/4;	break;
	}

	if (ch->pcdata->condition[COND_HUNGER]   < 0)
	    gain = 3;

	if (ch->pcdata->condition[COND_THIRST] < 0)
	    gain = 3;
	}

	gain = gain * ch->in_room->heal_rate/100;

	if (ch->on != NULL && ch->on->pIndexData->item_type == ITEM_FURNITURE)
	gain = gain * ch->on->value[ITEM_FURNITURE_HEAL_RATE] / 100;

	if (IS_AFFECTED(ch, AFF_POISON))
	gain /= 4;

	if (IS_AFFECTED(ch, AFF_PLAGUE))
	    gain /= 8;

	if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
	    gain /=2 ;

	if (IS_HARA_KIRI(ch))
	gain *= 3;

	return UMIN(gain, ch->max_move - ch->move);
}

void gain_condition(CHAR_DATA *ch, int iCond, int value)
{
	int condition;
	int damage_hunger;
	int fdone;
	CHAR_DATA *vch,*vch_next;

	if (value == 0 
	|| IS_DELETED(ch)
	|| IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL)
		return;

	if (HAS_SKILL(ch, gsn_vampire)
	&&  (iCond == COND_THIRST ||
	     iCond == COND_FULL ||
	     iCond == COND_HUNGER))
		return;

	condition = ch->pcdata->condition[iCond];

	ch->pcdata->condition[iCond] = URANGE(-6, condition + value, 96);

	if (iCond == COND_FULL && (ch->pcdata->condition[COND_FULL] < 0))
		ch->pcdata->condition[COND_FULL] = 0;

	if ((iCond == COND_DRUNK) && (ch->pcdata->condition[COND_DRUNK] < 1)) 
		ch->pcdata->condition[COND_DRUNK] = 0;

	if (ch->pcdata->condition[iCond] < 1
	&&  ch->pcdata->condition[iCond] > -6) {
		switch (iCond) {
		case COND_HUNGER:
			char_puts("You are hungry.\n",  ch);
			break;

		case COND_THIRST:
			char_puts("You are thirsty.\n", ch);
			break;
	 
		case COND_DRUNK:
			if (condition != 0)
				char_puts("You are sober.\n", ch);
			break;

		case COND_BLOODLUST:
			if (condition != 0)
				char_puts("You are hungry for blood.\n",
					     ch);
			break;

		case COND_DESIRE:
			if (condition != 0)
				char_puts("You have missed your home.\n",
					     ch);
			break;
		}
	}

	if (iCond == COND_DRUNK) {
		if (IS_DRUNK(ch))
			drunk_effect(ch, ch->level, TRUE);
		else
			drunk_effect(ch, ch->level, FALSE);
	}

	if (ch->pcdata->condition[iCond] == -6 && ch->level >= MIN_PK_LEVEL) {
		switch (iCond) {
		case COND_HUNGER:
			char_puts("You are starving!\n",  ch);
			act("$n is starving!",  ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, TYPE_HUNGER, DAM_HUNGER,
			       TRUE);
			if (ch->position == POS_SLEEPING) 
				return;       
			break;

		case COND_THIRST:
			char_puts("You are dying of thirst!\n", ch);
			act("$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, TYPE_HUNGER, DAM_THIRST,
			       TRUE);
			if (ch->position == POS_SLEEPING) 
				return;       
			break;

		case COND_BLOODLUST:
			fdone = 0;
			char_puts("You are suffering from thirst of blood!\n",
				  ch);
			act("$n is suffering from thirst of blood!",
			    ch, NULL, NULL, TO_ROOM);
			if (ch->in_room && ch->in_room->people
			&&  ch->fighting == NULL) {
				if (!IS_AWAKE(ch)) do_stand(ch, str_empty);
			        for (vch = ch->in_room->people;
			       	     vch != NULL && ch->fighting == NULL;
				     vch = vch_next) {
			        	vch_next = vch->next_in_room;

					if (IS_IMMORTAL(vch))
						continue;

			        	if (ch != vch && can_see(ch, vch)
					&&  !is_safe_nomessage(ch, vch)) {
						do_yell(ch,
						        "BLOOD! I NEED BLOOD!");
						do_murder(ch, vch->name);
						fdone = 1;
					}
			         }
			}

			if (fdone)
				break;

			damage_hunger = ch->max_hit * number_range(2, 4) / 100;
			if (!damage_hunger)
				damage_hunger = 1;
			damage(ch, ch, damage_hunger, TYPE_HUNGER, DAM_THIRST,
			       TRUE);
			if (ch->position == POS_SLEEPING) 
				return;       		
			break;

		case COND_DESIRE:
			char_puts("You want to go your home!\n", ch);
			act("$n desires for $s home!", ch, NULL, NULL, TO_ROOM);
			if (ch->position >= POS_STANDING) 
				move_char(ch, number_door(), FALSE);
			break;
		}
	}
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update(void)
{
	CHAR_DATA *ch, *ch_next;
	EXIT_DATA *pexit;
	int door;
	OBJ_DATA *obj;

	/* Examine all mobs. */
	for (ch = char_list; ch; ch = ch_next) {
		bool bust_prompt = FALSE;
		flag64_t act;

		ch_next = ch->next;

		if (!ch->in_room)
			continue;

		if (ch->position == POS_FIGHTING)
			SET_FIGHT_TIME(ch);
/* permanent spellbane */
		if (!IS_NPC(ch)) {
#if 0
			if (ch->level < LEVEL_IMMORTAL
			&&  get_skill(ch, gsn_spellbane)
			&&  !is_affected(ch, gsn_spellbane))
				do_spellbane(ch, str_empty);
#endif

/* update ghost state */
			if (ch->last_death_time != -1
			&&  current_time - ch->last_death_time >=
							GHOST_DELAY_TIME
			&&  IS_SET(ch->state_flags, STATE_GHOST)) {
				char_puts("You return to your normal form.\n",
					  ch);
				REMOVE_BIT(ch->state_flags, STATE_GHOST);
                                REMOVE_BIT(ch->affected_by, AFF_FLYING);
                                REMOVE_BIT(ch->affected_by, AFF_PASS_DOOR);

			}
		}

/* update pumped state */
		if (ch->last_fight_time != -1
		&&  current_time - ch->last_fight_time >= FIGHT_DELAY_TIME
		&&  IS_PUMPED(ch)) {
			REMOVE_BIT((ch)->state_flags, STATE_PUMPED);
			if (!IS_NPC(ch) && ch->desc != NULL
			&&  ch->desc->pString == NULL 
			&&  (ch->last_death_time == -1 ||
			     ch->last_death_time < ch->last_fight_time)) {
				act("You settle down.",
				    ch, NULL, NULL, TO_CHAR);
			}
		}

		if (IS_AFFECTED(ch, AFF_REGENERATION) && ch->in_room != NULL) {
			if (!is_affected(ch, gsn_condemnation)) {
				ch->hit = UMIN(ch->hit + ch->level / 10, ch->max_hit);
				if (ch->race == 18 /* troll */)
					ch->hit = UMIN(ch->hit + ch->level / 10,
						       ch->max_hit);
				if (ch->hit != ch->max_hit)
					bust_prompt = TRUE;
			}
		}

		if (IS_AFFECTED(ch, AFF_CORRUPTION) && ch->in_room != NULL) {
			ch->hit -=  ch->level / 10;
			if (ch->hit < 1) {
				if (IS_IMMORTAL(ch)) 
					ch->hit = 1;
				else {
					ch->position = POS_DEAD;
					handle_death(ch, ch);
					continue;
				}
			}
			bust_prompt = TRUE;
		}

		if (ch->desc
		&&  bust_prompt
		&&  !ch->desc->pString
		&&  !ch->desc->showstr_point
		&&  !IS_SET(ch->comm, COMM_NOBUST))
			char_puts(str_empty, ch);

/*
 * that's all for PCs and charmed mobiles
 */
		if (!IS_NPC(ch)
		||  IS_AFFECTED(ch, AFF_CHARM))
			continue;

		act = ch->pIndexData->act;
		if (IS_SET(act, ACT_HUNTER) && ch->hunting)
			hunt_victim(ch);

		if (ch->in_room->area->empty
		&&  !IS_SET(act, ACT_UPDATE_ALWAYS))
			continue;

		/* Examine call for special procedure */
		if (ch->spec_fun != 0) {
			if ((*ch->spec_fun) (ch))
				continue;
		}

		if (ch->pIndexData->pShop != NULL /* give him some gold */
		||  (ch->gold * 100 + ch->silver) < ch->pIndexData->wealth) {
			ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
			ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
		}
	 
/* check triggers (only if mobile still in default position) */

		if (ch->position == ch->pIndexData->default_pos) {
			if (HAS_TRIGGER(ch, TRIG_DELAY)
			&&  ch->mprog_delay > 0) {
				if (--ch->mprog_delay <= 0) {
					mp_percent_trigger(ch, NULL, NULL,
							   NULL, TRIG_DELAY);
					continue;
				}
			} 
			if (HAS_TRIGGER(ch, TRIG_RANDOM)) {
				if(mp_percent_trigger(ch, NULL, NULL,
						      NULL, TRIG_RANDOM))
					continue;
			}
		}

/* potion using and stuff for intelligent mobs */

		if (ch->pIndexData->pShop == NULL
		&&  (ch->position == POS_STANDING ||
		     ch->position == POS_RESTING ||
		     ch->position == POS_FIGHTING)
		&&  get_curr_stat(ch, STAT_INT) > 40 
		&&  (ch->hit < ch->max_hit * 90 / 100 ||
		     IS_AFFECTED(ch, AFF_BLIND) ||
		     IS_AFFECTED(ch, AFF_POISON) ||
		     IS_AFFECTED(ch, AFF_PLAGUE) ||
		     ch->fighting != NULL)) {
			for (obj = ch->carrying; obj; obj = obj->next_content) {
				if (obj->pIndexData->item_type != ITEM_POTION)
					continue;

				if (ch->hit < ch->max_hit * 90 / 100) {
					int cl = potion_cure_level(obj);
					if (cl > 0) {
						if (ch->hit < ch->max_hit*0.5
						&&  cl > 3) {
							quaff_obj(ch, obj);
							continue;
						}
						if (ch->hit < ch->max_hit*0.7) {
							quaff_obj(ch, obj);
							continue;
						}
					}
				}

				if (IS_AFFECTED(ch, AFF_POISON)
				&&  potion_cure_poison(obj)) {
					quaff_obj(ch, obj);
					continue;
				}

				if (IS_AFFECTED(ch, AFF_PLAGUE)
				&&  potion_cure_disease(obj)) {
					quaff_obj(ch, obj);
					continue;
				}

				if (IS_AFFECTED(ch, AFF_BLIND)
				&&  potion_cure_blind(obj)) {
					quaff_obj(ch, obj);
					continue;
				}

				if (ch->fighting) {
					int al = potion_arm_level(obj);

					if (ch->level - ch->fighting->level < 7
					&&  al > 3) {
						quaff_obj(ch, obj);
						continue;
					}

					if (ch->level - ch->fighting->level < 8
					&&  al > 2) {
						quaff_obj(ch, obj);
						continue;
					}

					if (ch->level - ch->fighting->level < 9
					&&  al > 1) {
						quaff_obj(ch, obj);
						continue;
					}

					if (ch->level - ch->fighting->level < 10
					&&  al > 0) {
						quaff_obj(ch, obj);
						continue;
					}
				}
			}
		}

/* That's all for sleeping / busy monster, and empty zones */
		if (ch->position != POS_STANDING)
			continue;

/* Scavenge */
		if (IS_SET(act, ACT_SCAVENGER)
		&&  ch->in_room->contents != NULL
		&&  number_bits(6) == 0) {
			OBJ_DATA *obj;
			OBJ_DATA *obj_best = NULL;
			int max = 1;

			for (obj = ch->in_room->contents; obj;
			     obj = obj->next_content) {
				if (CAN_WEAR(obj, ITEM_TAKE)
				&&  can_loot(ch, obj)
				&& !IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST)
				&&  obj->cost > max) {
					obj_best = obj;
					max	 = obj->cost;
				}
			}

			if (obj_best)
				get_obj(ch, obj_best, NULL);
		}

/* Wander */
		if (!IS_SET(act, ACT_SENTINEL) 
		&&  number_bits(3) == 0
		&&  (door = number_bits(5)) <= 5
		&&  !RIDDEN(ch)
		&&  (pexit = ch->in_room->exit[door]) != NULL
		&&  pexit->to_room.r != NULL
		&&  !IS_SET(pexit->exit_info, EX_CLOSED)
		&&  !IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB)
		&&  (!IS_SET(act, ACT_STAY_AREA) ||
		     pexit->to_room.r->area == ch->in_room->area) 
		&&  (!IS_AGGRO(ch, NULL) ||
		     !IS_SET(pexit->to_room.r->room_flags,
			     ROOM_PEACE | ROOM_SAFE))
		&&  (!IS_SET(act, ACT_OUTDOORS) ||
		     !IS_SET(pexit->to_room.r->room_flags, ROOM_INDOORS)) 
		&&  (!IS_SET(act, ACT_INDOORS) ||
		     IS_SET(pexit->to_room.r->room_flags, ROOM_INDOORS)))
			move_char(ch, door, FALSE);
	}
}

int potion_cure_level(OBJ_DATA * potion)
{
	int cl;
	int i;
	cl = 0;
	for (i = 1; i < 5; i++) {
		if (sn_lookup("cure critical") == potion->value[i])
			cl += 3;
		if (sn_lookup("cure light") == potion->value[i])
			cl += 1;
		if (sn_lookup("cure serious") == potion->value[i])
			cl += 2;
		if (sn_lookup("heal") == potion->value[i])
			cl += 4;
	}
	return (cl);
}

int potion_arm_level(OBJ_DATA * potion)
{
	int al;
	int i;
	al = 0;
	for (i = 1; i < 5; i++) {
		if (sn_lookup("armor") == potion->value[i])
			al += 1;
		if (sn_lookup("shield") == potion->value[i])
			al += 1;
		if (sn_lookup("stone skin") == potion->value[i])
			al += 2;
		if (sn_lookup("sanctuary") == potion->value[i])
			al += 4;
		if (sn_lookup("protection") == potion->value[i])
			al += 3;
	}
	return (al);
}

bool potion_cure_blind(OBJ_DATA * potion)
{
	int i;
	for (i = 0; i < 5; i++) {
		if (sn_lookup("cure blindness") == potion->value[i])
			return (TRUE);
	}
	return (FALSE);
}

bool potion_cure_poison(OBJ_DATA * potion)
{
	int i;
	for (i = 0; i < 5; i++) {
		if (sn_lookup("cure poison") == potion->value[i])
			return (TRUE);
	}
	return (FALSE);
}

bool potion_cure_disease(OBJ_DATA * potion)
{
	int i;
	for (i = 0; i < 5; i++) {
		if (sn_lookup("cure disease") == potion->value[i])
			return (TRUE);
	}
	return (FALSE);
}

/*
 * Update the weather.
 */
void weather_update(void)
{
	CHAR_DATA *ch;
	int diff;
	const char *msg_daytime = NULL;
	const char *msg_weather = NULL;

	switch (++time_info.hour) {
	case  5:
		weather_info.sunlight = SUN_LIGHT;
		msg_daytime = "The day has begun.";
		break;

	case  6:
		weather_info.sunlight = SUN_RISE;
		msg_daytime = "The sun rises in the east.";
		break;

	case 19:
		weather_info.sunlight = SUN_SET;
		msg_daytime = "The sun slowly disappears in the west.";
		break;

	case 20:
		weather_info.sunlight = SUN_DARK;
		msg_daytime = "The night has begun.";
		break;

	case 24:
		time_info.hour = 0;
		time_info.day++;
		break;
	}

	if (time_info.day >= 35) {
		time_info.day = 0;
		time_info.month++;
	}

	if (time_info.month >= 17) {
		time_info.month = 0;
		time_info.year++;
	}

	/*
	 * Weather change.
	 */
	if (time_info.month >= 9 && time_info.month <= 16)
		diff = weather_info.mmhg >  985 ? -2 : 2;
	else
		diff = weather_info.mmhg > 1015 ? -2 : 2;

	weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
	weather_info.change    = UMAX(weather_info.change, -12);
	weather_info.change    = UMIN(weather_info.change,  12);

	weather_info.mmhg += weather_info.change;
	weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
	weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

	switch (weather_info.sky) {
	default: 
		bug("Weather_update: bad sky %d.", weather_info.sky);
		weather_info.sky = SKY_CLOUDLESS;
		break;

	case SKY_CLOUDLESS:
		if (weather_info.mmhg < 990
		||  (weather_info.mmhg < 1010 && number_bits(2) == 0)) {
			msg_weather = "The sky is getting cloudy.";
			weather_info.sky = SKY_CLOUDY;
		}
		break;

	case SKY_CLOUDY:
		if (weather_info.mmhg < 970
		||  (weather_info.mmhg < 990 && number_bits(2) == 0)) {
			msg_weather = "It starts to rain.";
			weather_info.sky = SKY_RAINING;
		}
		else if (weather_info.mmhg > 1030 && number_bits(2) == 0) {
			msg_weather = "The clouds disappear.";
			weather_info.sky = SKY_CLOUDLESS;
		}
		break;

	case SKY_RAINING:
		if (weather_info.mmhg > 1030
		||  (weather_info.mmhg > 1010 && number_bits(2) == 0)) {
			msg_weather = "The rain stopped.";
			weather_info.sky = SKY_CLOUDY;
		}
		else if (weather_info.mmhg < 970 && number_bits(2) == 0) {
			msg_weather = "Lightning flashes in the sky.";
			weather_info.sky = SKY_LIGHTNING;
		}
		break;

	case SKY_LIGHTNING:
		if (weather_info.mmhg > 1010
		||  (weather_info.mmhg > 990 && number_bits(2) == 0)) {
			msg_weather = "The lightning has stopped.";
			weather_info.sky = SKY_RAINING;
		}
		break;
	}

	if (!msg_daytime && !msg_weather)
		return;

	for (ch = char_list; ch; ch = ch->next) {
		if (IS_NPC(ch) && !HAS_TRIGGER(ch, TRIG_ACT))
			continue;
		if (!IS_OUTSIDE(ch) || !IS_AWAKE(ch))
			continue;

		act(msg_daytime, ch, NULL, NULL, TO_CHAR);

		if (ch->in_room != NULL)
			continue;
		if (IS_SET(ch->in_room->sector_type, SECT_DESERT)) {
			if (weather_info.sky == SKY_RAINING)
				act("A warm breeze passes over the dunes.", 
					ch, NULL, NULL, TO_CHAR);
			else if (weather_info.sky == SKY_LIGHTNING)
				act("A strong breeze kicks up sand blowing sand in the air.", 
					ch, NULL, NULL, TO_CHAR);
		}
		else if (IS_SET(ch->in_room->sector_type,SECT_ARCTIC))  {
			if (weather_info.sky == SKY_RAINING)
				act("Snow flurries dance by on a light breeze.", 
					ch, NULL, NULL, TO_CHAR);
			else if (weather_info.sky == SKY_LIGHTNING)
				act("A blizzard fills the sky with snow.", 
					ch, NULL, NULL, TO_CHAR);
		}
		else 
			act(msg_weather, ch, NULL, NULL, TO_CHAR);
	}
}

/*
 * Update all chars, including mobs.
*/
void char_update(void)
{   
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;

	static time_t last_save_time = -1;

	/* update save counter */
	save_number++;

	if (save_number > 29)
		save_number = 0;

	for (ch = char_list; ch; ch = ch_next) {
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;
		int chance;
		race_t *r = race_lookup(ch->race);

		ch_next = ch->next;
		if (!r)
			continue;

		/* align_standing update */
		standing_update(ch);

		/* reset path find */
		if (!IS_NPC(ch) && (chance = get_skill(ch, gsn_path_find))) {
			if (number_percent() < chance) {
				ch->endur += chance / 2;
				check_improve(ch, gsn_path_find, TRUE, 8);
			}
			else
				check_improve(ch, gsn_path_find, FALSE, 16);
		}

		if (!ch->fighting) {
			flag64_t skip = AFF_FLYING;

			affect_check(ch, TO_AFFECTS, -1);

			/* Remove caltrops effect after fight off */
			if (is_affected(ch, gsn_caltrops))
				affect_strip(ch, gsn_caltrops);

			if (!MOUNTED(ch)) {
				if (!IS_AFFECTED(ch, AFF_HIDE) 
				&&  (r->aff & AFF_HIDE))
					char_puts("You step back into the shadows.\n", ch);

				if (!IS_AFFECTED(ch, AFF_SNEAK)
				&&  (r->aff & AFF_SNEAK))
					char_puts("You move silently again.\n", ch);
			}
			else
				skip |= AFF_HIDE | AFF_FADE | AFF_INVIS |
					AFF_IMP_INVIS | AFF_SNEAK |
					AFF_CAMOUFLAGE;

			SET_BIT(ch->affected_by, r->aff & ~skip);
		}

		/* Remove vampire effect when morning. */
/*
#		if (is_affected(ch, gsn_vampire)
#		&&  (weather_info.sunlight == SUN_LIGHT ||
#		     weather_info.sunlight == SUN_RISE))
#			do_human(ch, str_empty);
#
#
#
*/

		if (ch->position >= POS_STUNNED) {
			int old_hit = ch->hit;
			int old_mana = ch->mana;
			int old_move = ch->move;

			/* check to see if we need to go home */
			if (IS_NPC(ch) && ch->zone != NULL 
			&&  ch->in_room != NULL
			&&  ch->zone != ch->in_room->area && ch->desc == NULL 
			&&  ch->fighting == NULL
/* && ch->progtypes==0 */
			&& !IS_AFFECTED(ch,AFF_CHARM) && ch->last_fought == NULL
			&& !RIDDEN(ch)) {
				if (ch->in_mind != NULL 
				&& ch->in_mind[0] != '\0'
				&& ch->pIndexData->vnum > 100)
					back_home(ch);
				else {
					act("$n wanders on home.",
					    ch, NULL, NULL, TO_ROOM);
					extract_char(ch, 0);
				}
				continue;
			}

			if (ch->hit < ch->max_hit)
				ch->hit += hit_gain(ch);
			else
				ch->hit = ch->max_hit;

			if (ch->mana < ch->max_mana)
				ch->mana += mana_gain(ch);
			else
				ch->mana = ch->max_mana;

			if (ch->move < ch->max_move)
				ch->move += move_gain(ch);
			else
				ch->move = ch->max_move;

			if (ch->desc
			&&  (old_hit != ch->hit || old_mana != ch->mana ||
			     old_move != ch->move)
			&&  !ch->desc->pString
			&&  !ch->desc->showstr_point
			&&  !IS_SET(ch->comm, COMM_NOBUST))
				char_puts(str_empty, ch);
		}

		if (ch->position == POS_STUNNED)
			update_pos(ch);

		if (!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL
		&&  ch->in_room != NULL) {
			OBJ_DATA *obj;

			if ((obj = get_eq_char(ch, WEAR_LIGHT))
			&&  obj->pIndexData->item_type == ITEM_LIGHT
			&&  obj->value[ITEM_LIGHT_DURATION] > 0
			&&  ch->level > 5) {
				/* expire lights half the time*/
				if (number_percent() > 50) {
					if (--obj->value[ITEM_LIGHT_DURATION] == 0) {
						if (ch->in_room->light > 0)
							--ch->in_room->light;
						act("$p goes out.",
						    ch, obj, NULL, TO_ROOM);
						act("$p flickers and goes out.",
						    ch, obj, NULL, TO_CHAR);
						REMOVE_BIT(obj->extra_flags, ITEM_GLOW);
					}
					else if (obj->value[ITEM_LIGHT_DURATION] <= 5)
						act("$p flickers.",
						    ch, obj, NULL, TO_CHAR);
				}
			}

			if (++ch->timer >= 12) {
				if (ch->was_in_room == NULL) {
					ch->was_in_room = ch->in_room;
					if (ch->fighting != NULL)
						stop_fighting(ch, TRUE);
					act("$n disappears into the void.",
					    ch, NULL, NULL, TO_ROOM);
					char_puts("You disappear "
						  "into the void.\n", ch);
					if (ch->level > 1)
						save_char_obj(ch, FALSE);
  					char_from_room(ch);
					char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
					if (JUST_KILLED(ch))
						continue;
				}
			}

			if (!ch->was_in_room) {
				gain_condition(ch, COND_DRUNK, -1);
				if (HAS_SKILL(ch, gsn_vampire))
					gain_condition(ch, COND_BLOODLUST, -1);
				gain_condition(ch, COND_FULL, 
					     ch->size > SIZE_MEDIUM ? -4 : -2);
				if (ch->level > 5) {
					if (ch->in_room->sector_type == SECT_DESERT)
						gain_condition(ch, COND_THIRST, -3);
					else
						gain_condition(ch, COND_THIRST, -1);
					gain_condition(ch, COND_HUNGER, 
						     ch->size > SIZE_MEDIUM ? -2 : -1);
				}
			}
		}

		for (paf = ch->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next;
			if (paf->duration > 0) {
				paf->duration--;
				if (number_range(0, 4) == 0 && paf->level > 0)
					paf->level--;
				/* spell strength fades with time */
			}
			else if (paf->duration == 0) {
				skill_t *sk;

				if ((paf_next == NULL ||
				     paf_next->type != paf->type ||
				     paf_next->duration > 0)
				&&  paf->type > 0
				&&  (sk = skill_lookup(paf->type))
				&&  !IS_NULLSTR(sk->msg_off)) 
					act_puts(sk->msg_off, ch, NULL, NULL,
						 TO_CHAR, POS_DEAD);
				affect_remove(ch, paf);
			}
		}

		/* Zz - pollen */
		if (is_affected(ch, gsn_pollen)) {
			AFFECT_DATA *pollen, *old_af, af;
			bool was_blind = FALSE;
			bool was_sleep = FALSE;

			for (pollen = ch->affected; pollen!= NULL; pollen = pollen->next)
				if (pollen->type == gsn_pollen)
					break;

			if (ch->in_room != NULL) {
				if (is_affected(ch, gsn_pollen_blind)) {
					for (old_af = ch->affected; old_af!= NULL; 
					     old_af =old_af->next)
						if (old_af->type == gsn_pollen_blind)
							break;
					affect_remove(ch, old_af);
					was_blind = TRUE;
				}

				/* if out of the forest */
				if ((ch->in_room->sector_type != SECT_FOREST
				    &&  ch->in_room->sector_type != SECT_HILLS
				    &&  ch->in_room->sector_type != SECT_FIELD
				    &&  ch->in_room->sector_type != SECT_MOUNTAIN)) {

					char_puts("Your eyes still itch, but at least you can see.\n", ch);
					if (is_affected(ch, gsn_pollen_sleep)) {
						for (old_af = ch->affected; old_af!= NULL; 
						     old_af =old_af->next)
							if (old_af->type == gsn_pollen_sleep)
								break;
						affect_remove(ch, old_af);
						char_puts("You feel less drowsy.\n", ch);
						was_sleep = TRUE;
					}
				}
				/* if still in the forest */
				else {
					if (number_percent() < 50) {
						af.where 	 = TO_AFFECTS;
						af.type 	 = gsn_pollen_blind;
						af.level 	 = pollen->level;
						af.duration	 = pollen->duration;
						af.location	 = APPLY_AC;
						af.modifier	 = 10;
						af.bitvector     = AFF_BLIND;
						affect_to_char(ch ,&af);
						if (IS_AWAKE(ch) && !was_blind)
							char_puts("Your eyes {Rs{rw{Rell{x up so you can't see!\n", ch);
					}
					else if (IS_AWAKE(ch) && was_blind)
						char_puts("Your eyes still itch, but at least you can see.\n", ch);

					if (is_affected(ch, gsn_pollen_sleep)) {
						for (old_af = ch->affected; old_af!= NULL; 
						     old_af =old_af->next)
							if (old_af->type == gsn_pollen_sleep)
								break;
						if (number_percent() < 50) {
							affect_remove(ch, old_af);
							char_puts("You feel less drowsy.\n", ch);
							was_sleep = TRUE;
						}
					}

					if (!was_sleep && number_percent() < 10) {
						af.where 	 = TO_AFFECTS;
						af.type 	 = gsn_pollen_sleep;
						af.level 	 = pollen->level;
						af.duration	 = pollen->duration;
						af.location	 = APPLY_NONE;
						af.modifier	 = 0;
						af.bitvector     = AFF_SLEEP;
						affect_to_char(ch ,&af);
						if (IS_AWAKE(ch)) {
							char_puts("You're overcome with drowsiness!\n", 
								ch);
							act("$n goes to sleep.", 
								ch, NULL, NULL, TO_ROOM);
							ch->position = POS_SLEEPING;
						}
					}
				}
			}	
		}

		/*
		 * Careful with the damages here,
		 *   MUST NOT refer to ch after damage taken,
		 *   as it may be lethal damage (on NPC).
		 */

		if (is_affected(ch, gsn_witch_curse)) {
			AFFECT_DATA *af, witch;
	
			if (ch->in_room == NULL)
				continue;

			act("The witch curse makes $n feel $s life slipping away.",
			    ch, NULL, NULL, TO_ROOM);
			char_puts("The witch curse makes you feeling your life slipping away.\n", ch);
	
			for (af = ch->affected; af!= NULL; af = af->next)
				if (af->type == gsn_witch_curse)
					break;

			if (af == NULL)
				continue;

			if (af->level == 1)
				continue;

			witch.where = af->where;
			witch.type  = af->type;
			witch.level = af->level;
			witch.duration = af->duration;
			witch.location = af->location;
			witch.modifier = af->modifier * 2;
			witch.bitvector = 0;
	
			affect_remove(ch, af);
			affect_to_char(ch ,&witch);
			ch->hit = UMIN(ch->hit, ch->max_hit);
			if (ch->hit < 1) {
				if (IS_IMMORTAL(ch))
					ch->hit = 1;
				else {
					ch->position = POS_DEAD;
					handle_death(ch, ch);
					continue;
				}
			}
		}

		if (IS_AFFECTED(ch, AFF_PLAGUE) && ch != NULL) {
			AFFECT_DATA *af, plague;
			CHAR_DATA *vch;
			int dam;

			if (ch->in_room == NULL)
				continue;
	        
			act("$n writhes in agony as plague sores erupt from $s skin.",
			    ch, NULL, NULL, TO_ROOM);
			char_puts("You writhe in agony from the plague.\n", ch);
			for (af = ch->affected; af != NULL; af = af->next)
				if (af->type == gsn_plague)
				break;
	    
			if (af == NULL) {
				REMOVE_BIT(ch->affected_by, AFF_PLAGUE);
				continue;
			}
	    
			if (af->level == 1)
				continue;
	    
			plague.where 	 = TO_AFFECTS;
			plague.type 	 = gsn_plague;
			plague.level 	 = af->level - 1; 
			plague.duration	 = number_range(1,2 * plague.level);
			plague.location	 = APPLY_STR;
			plague.modifier	 = -5;
			plague.bitvector = AFF_PLAGUE;
	    
			for (vch = ch->in_room->people; vch != NULL; 
			     vch = vch->next_in_room) {
				if (!saves_spell(plague.level + 2, 
						 vch, DAM_DISEASE) 
				&& !IS_IMMORTAL(vch) 
				&& !IS_SET(vch->state_flags, STATE_GHOST)
				&& !IS_AFFECTED(vch, AFF_PLAGUE) 
				&& number_bits(2) == 0) {
					char_puts("You feel hot and feverish.\n", vch);
					act("$n shivers and looks very ill.",
					    vch, NULL, NULL, TO_ROOM);
					affect_join(vch, &plague);
				}
			}

			dam = UMIN(ch->level, af->level/5 + 1);
			ch->mana -= dam;
			ch->move -= dam;
			damage(ch, ch, dam, gsn_plague, DAM_DISEASE,FALSE);
			if (number_range(1, 100) < 70)
				damage(ch, ch, UMAX(ch->max_hit/20, 50), 
				       gsn_plague, DAM_DISEASE, TRUE);
		}
		else if (IS_AFFECTED(ch, AFF_POISON) && ch != NULL
		     &&  !IS_AFFECTED(ch, AFF_SLOW)) {
			AFFECT_DATA *poison;

			poison = affect_find(ch->affected, gsn_poison);

			if (poison != NULL) {
				act("$n shivers and suffers.",
				    ch, NULL, NULL, TO_ROOM); 
				char_puts("You shiver and suffer.\n", ch);
				damage(ch, ch, poison->level/10 + 1, gsn_poison,
				       DAM_POISON, TRUE);
			}
		}
		else if (ch->position == POS_INCAP 
		     &&  number_range(0, 1) == 0)
			damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
		else if (ch->position == POS_MORTAL)
			damage(ch, ch, 1, TYPE_UNDEFINED, DAM_NONE, FALSE);
	} /* global for */

	/*
	 * Autosave and autoquit.
	 * Check that these chars still exist.
	 */
	if (last_save_time == -1 || current_time - last_save_time > 300) {
		last_save_time = current_time;
		for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
			ch_next = ch->next;
			if (ch->timer > 20)
				quit_char(ch, 0);
			else
				save_char_obj(ch, FALSE);
		}
	}
}

void water_float_update(void)
{
	OBJ_DATA *obj_next;
	OBJ_DATA *obj;
	CHAR_DATA *ch;

	for (obj = object_list; obj != NULL; obj = obj_next) {
		obj_next = obj->next;

		if (!obj->in_room || !IS_WATER(obj->in_room))
			continue;

		obj->water_float = obj->water_float > 0 ?
						obj->water_float - 1 : -1;

		if (obj->pIndexData->item_type == ITEM_DRINK_CON) {
			obj->value[ITEM_DRINK_REMAINING] = URANGE(1, obj->value[ITEM_DRINK_REMAINING]+8,
					       obj->value[ITEM_DRINK_TOTAL]);
			if ((ch = obj->in_room->people))
				act("$p makes bubbles on the water.", ch, obj,
				    NULL, TO_ALL);
			obj->water_float = obj->value[ITEM_DRINK_TOTAL]-obj->value[ITEM_DRINK_REMAINING];
			obj->value[ITEM_DRINK_TYPE] = 0;
		}
		if (obj->water_float == 0) {
			if((ch = obj->in_room->people))
				act("$p sinks down the water.", ch, obj, NULL,
				    TO_ALL); 
			extract_obj(obj, 0);
		}
	}
}

void update_obj_affects(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;

	/* go through affects and decrement */
	for (paf = obj->affected; paf != NULL; paf = paf_next) {
		paf_next    = paf->next;
		if (paf->duration > 0) {
        		paf->duration--;
			/* spell strength fades with time */
        		if (number_range(0,4) == 0 && paf->level > 0)
				paf->level--;
        	}
		else if (paf->duration == 0) {
			skill_t *sk;

			if ((paf_next == NULL || paf_next->type != paf->type ||
			     paf_next->duration > 0)
			&&  paf->type > 0
			&&  (sk = skill_lookup(paf->type))
			&&  !IS_NULLSTR(sk->msg_obj)) {
				if (obj->carried_by != NULL) 
					act(sk->msg_obj, obj->carried_by,
					   obj, NULL, TO_CHAR);

				if (obj->in_room != NULL 
				&&  obj->in_room->people)
					act(sk->msg_obj, obj->in_room->people,
					   obj, NULL, TO_ALL);
                	}
			affect_remove_obj(obj, paf);
        	}
	}
}

bool update_ice_obj(OBJ_DATA *obj)
{
	if (obj->carried_by != NULL
	&&  obj->carried_by->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 40) {
		act("The extreme heat melts $p.", obj->carried_by, obj, NULL,
		    TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}
	if (obj->in_room != NULL
	&&  obj->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 50) {
		act("The extreme heat melts $p.", obj->in_room->people, obj,
		    NULL, TO_ALL);
		extract_obj(obj, 0);
		return TRUE;
	}
	return FALSE;
}

bool update_glass_obj(OBJ_DATA *obj)
{
	if (obj->carried_by
	&&  obj->carried_by->in_room->sector_type == SECT_DESERT
	&&  !IS_NPC(obj->carried_by)
	&&  number_percent() < 20)  {
		act("$p evaporates.", obj->carried_by, obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}
	if (obj->in_room
	&&  obj->in_room->sector_type == SECT_DESERT
	&&  number_percent() < 30) {
		act("$p evaporates by the extream heat.", obj->in_room->people,
		    obj, NULL, TO_ROOM);
		act("$p evaporates by the extream heat.", obj->in_room->people,
		    obj, NULL, TO_CHAR);
		extract_obj(obj, 0);
		return TRUE;
	}
	return FALSE;
}

/*
 * empty the donation pits of items
 */
void update_pit(OBJ_DATA *obj)
{
	OBJ_DATA *t_obj, *next_obj;

	if (!IS_SET(obj->pIndexData->extra_flags, ITEM_PIT)
	|| obj->pIndexData->cost == 0)
		return;

	obj->cost--;

	if (obj->cost < 0) obj->cost = 0;

	if (obj->cost > 0) return;

	DEBUG(DEBUG_PITS,
		"Emptying pit: [%d] %s", obj->in_room->vnum, 
		obj->name);

	for (t_obj = obj->contains; t_obj; t_obj = next_obj) {
		next_obj = t_obj->next_content;
		if (IS_OBJ_LIMITED(t_obj->pIndexData)
		|| t_obj->level <= 1 
		|| number_percent() < t_obj->level) {
			obj_from_obj(t_obj);
			extract_obj(t_obj, 0);
		}
	}
	obj->cost = obj->pIndexData->cost;
}

void contents_to_obj(OBJ_DATA *obj, OBJ_DATA *to_obj)
{
	OBJ_DATA *obj_next;

	for (; obj; obj = obj_next) {
		obj_next = obj->next_content;
		obj_from_obj(obj);
		obj_to_obj(obj, to_obj);
	}
}

static inline void
save_corpse_contents(OBJ_DATA *corpse)
{
	OBJ_DATA *obj, *obj_next;
	OBJ_DATA *pit;
	altar_t *altar;

/* in another object */
	if (corpse->in_obj) {
		contents_to_obj(corpse->contains, corpse->in_obj);
		return;
	}

/* carried by */
	if (corpse->carried_by) {
		for (obj = corpse->contains; obj; obj = obj_next) {
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_char(obj, corpse->carried_by);
		}
		return;
	}

/* pit lookup */
	pit = NULL;
	altar = corpse->altar;
	for (pit = altar->room->contents; pit; pit = pit->next_content) {
		if (pit->pIndexData == altar->pit)
			break;
	}

/* put contents into altar */
	if (!pit) {
		for (obj = corpse->contains; obj; obj = obj_next) {
			obj_next = obj->next_content;
			obj_from_obj(obj);
			obj_to_room(obj, altar->room);
		}
		return;
	}

/* put contents into pit */
	contents_to_obj(corpse->contains, pit);
	if (pit->cost != -1)
		pit->cost += 240;	/*increase timer on pit*/
}

void update_one_obj(OBJ_DATA *obj)
{
	OBJ_DATA *t_obj;
	CHAR_DATA *rch;
	char *message;

	update_obj_affects(obj);

	/* find the uppest obj container */
	for(t_obj = obj; t_obj->in_obj; t_obj = t_obj->in_obj)
		;

	if ((t_obj->in_room != NULL &&
	     t_obj->in_room->area->nplayer > 0)
        ||  (t_obj->carried_by &&
	     t_obj->carried_by->in_room &&
	     t_obj->carried_by->in_room->area->nplayer > 0))
		oprog_call(OPROG_AREA, obj, NULL, NULL);

	if (check_material(obj, "ice") 
	&&  update_ice_obj(obj))
		return;

	if (check_material(obj, "glass")
	&&  obj->pIndexData->item_type == ITEM_POTION
	&&  update_glass_obj(obj))
		return;

	if (IS_SET(obj->pIndexData->extra_flags, ITEM_PIT))
		update_pit(obj);

	/* don't decrement timer if immortal is holding corpse */
	if (obj->pIndexData->item_type == ITEM_CORPSE_PC
	&& obj->carried_by
	&& IS_IMMORTAL(obj->carried_by))
		return;

	if (obj->condition > -1 && (obj->timer <= 0 || --obj->timer > 0))
		return;

	if (IS_SET(obj->pIndexData->extra_flags, ITEM_CLAN)) {
		message = "$p disappears in a {Yflash{x of light.";
	} else {
		switch (obj->pIndexData->item_type) {
		default:
			message = "$p crumbles into dust.";
			break;
		case ITEM_FOUNTAIN:
			message = "$p dries up.";
			break;
		case ITEM_CORPSE_NPC:
			message = "$p decays into dust.";
			break;
		case ITEM_CORPSE_PC:
			message = "$p decays into dust.";
			break;
		case ITEM_FOOD:
			message = "$p decomposes.";
			break;
		case ITEM_POTION:
			message = "$p has evaporated from disuse.";	
			break;
		case ITEM_PORTAL:
			message = "$p fades out of existence.";
			break;
		case ITEM_CONTAINER: 
			if (CAN_WEAR(obj, ITEM_WEAR_FLOAT))
				if (obj->contains)
					message = "$p flickers and vanishes, spilling "
						  "its contents on the floor.";
				else
					message = "$p flickers and vanishes.";
			else
				message = "$p crumbles into dust.";
				break;
		}
	}

	if (obj->carried_by) {
		if (IS_NPC(obj->carried_by) 
		&&  obj->carried_by->pIndexData->pShop != NULL)
			obj->carried_by->silver += obj->cost/5;
		else {
			act(message, obj->carried_by, obj, NULL, TO_CHAR);
			if (obj->wear_loc == WEAR_FLOAT)
				act(message, obj->carried_by, obj, NULL,
				    TO_ROOM);
		}
	}

	if (obj->in_room && (rch = obj->in_room->people)
	&&  !IS_SET(obj->pIndexData->extra_flags, ITEM_PIT))
		act(message, rch, obj, NULL, TO_ALL);

	if (obj->pIndexData->item_type == ITEM_CORPSE_PC && obj->contains)
		save_corpse_contents(obj);

	if (IS_SET(obj->pIndexData->extra_flags, ITEM_CLAN)) {
		clan_item_expire(obj);
	}
	else 
		extract_obj(obj, 0);
}

OBJ_DATA *last_updated_obj;

void obj_update_list(OBJ_DATA *obj)
{
	int i;
	OBJ_DATA *obj_next;

/* some diagnostics */
	obj_next = obj;
	for (i = 0; obj && obj->extracted; obj = obj->next, i++)
		;
	if (i) {
		log_printf("obj_update_list: skipped %d extracted objs, "
			   "object_list == %p, obj == %p, "
			   "last_updated_obj == %p, "
			   "last_updated_obj->next == %p",
			   i, object_list, obj_next,
			   last_updated_obj,
			   last_updated_obj ? last_updated_obj->next : NULL);
	}

	for (; obj; obj = obj_next) {
		obj_next = obj->next;

		if (obj->extracted) {
			obj_update_list(last_updated_obj ?
					last_updated_obj->next : object_list);
			return;
		}

		update_one_obj(obj);

		if (!obj->extracted)
			last_updated_obj = obj;
	}
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update(void)
{   
	last_updated_obj = NULL;
	obj_update_list(object_list);
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't want the mob to just attack the first PC
 *   who leads the party into the room.
 *
 * -- Furey
 */
void aggr_update(void)
{
	CHAR_DATA *wch, *wch_next;
	CHAR_DATA *ch, *ch_next;
	CHAR_DATA *vch, *vch_next;
	CHAR_DATA *victim;

	for (wch = char_list; wch && !IS_NPC(wch); wch = wch_next) {
		wch_next = wch->next;

		if (!wch->in_room)
			continue;

		if (IS_AWAKE(wch)
		&&  IS_AFFECTED(wch, AFF_BLOODTHIRST)
		&&  wch->fighting == NULL) {
			for (vch = wch->in_room->people;
			     vch != NULL && wch->fighting == NULL;
			     vch = vch_next) {
				vch_next = vch->next_in_room;

				if (IS_IMMORTAL(vch))
					continue;

				if (wch != vch && can_see(wch,vch)
				&&  !is_safe_nomessage(wch,vch)) {
					act_puts("{RMORE BLOOD! MORE BLOOD! MORE BLOOD!!!{x", wch,NULL,NULL,TO_CHAR,POS_RESTING);
					do_murder(wch, vch->name);
					if (JUST_KILLED(wch))
						continue;
				}
			}
		}


		if (wch->level >= LEVEL_IMMORTAL)
			continue;

		for (ch = wch->in_room->people; ch; ch = ch_next) {
			int count;
			flag64_t act;

			ch_next = ch->next_in_room;

			/* XXX - this condition core dumps rarely for some
			 * reason */
			if (!IS_NPC(ch))
				continue;

			act = ch->pIndexData->act;

			if (!IS_AGGRO(ch, NULL) &&
			     ch->last_fought == NULL &&
			     ch->target == NULL)
				continue;

			if (!ch->in_room
			|| IS_SET(ch->in_room->room_flags,
				   ROOM_PEACE | ROOM_SAFE))
				continue;

			if (IS_AFFECTED(ch, AFF_CALM))
				continue;

			if (ch->fighting != NULL)
				continue;
			if (RIDDEN(ch))
				continue;

			if (IS_AFFECTED(ch, AFF_CHARM))
				continue;

			if (!IS_AWAKE(ch))
				continue;

			if (IS_SET(act, ACT_WIMPY) && IS_AWAKE(wch))
				continue;

			if (!can_see(ch, wch))
				continue;

			if (number_bits(1) == 0)
				continue;

			if (is_safe_nomessage(ch,wch))
				continue;

			/* Mad mob attacks! */
			if (ch->last_fought == wch
			&&  !IS_AFFECTED(ch, AFF_SCREAM)) {
				doprintf(do_yell, ch, "%s! Now you die!",
					 PERS(wch,ch));
				wch = check_guard(wch, ch); 
				multi_hit(ch, wch, TYPE_UNDEFINED);
				continue;
			}

			if (ch->target) {
				if (ch->target == wch)
					multi_hit(ch, wch, TYPE_UNDEFINED);
				continue;
			}

			if (ch->last_fought)
				continue;

			/*
			 * Ok we have a 'wch' player character and a 'ch' npc
			 * aggressor. Now make the aggressor fight a RANDOM
			 * pc victim in the room, giving each 'vch' an equal
			 * chance of selection.
			 */
			count = 0;
			victim = NULL;
			if (!wch->in_room)
				continue;
			for (vch = wch->in_room->people;
			     vch != NULL; vch = vch_next) {
				vch_next = vch->next_in_room;
				if (!IS_NPC(vch)
				&& (!IS_AGGRO(ch, vch)
				|| !IS_AWAKE(vch))
				&&  can_see(ch, vch)
				/* do not attack vampires */
				&&  !HAS_SKILL(vch, gsn_vampire)
				/* good vs good :( */
				&&  !(IS_GOOD(ch) && IS_GOOD(vch))) {
					if (number_range(0, count) == 0)
						victim = vch;
					count++;
				}
			}

			if (victim == NULL)
				continue;

			if (!is_safe_nomessage(ch, victim)) {
				int dt = TYPE_UNDEFINED;
				int bs_chance;

				victim = check_guard(victim, ch); 
				if (get_dam_type(ch,
						 get_eq_char(ch, WEAR_WIELD),
						 &dt) == DAM_PIERCE
				&&  (bs_chance = get_skill(ch, gsn_backstab))
				&&  backstab_ok(NULL, victim))
					backstab(ch, victim, bs_chance);
				else
					multi_hit(ch, victim, TYPE_UNDEFINED);
			}
		}
	}
}



/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

void update_handler(void)
{
	static int pulse_area;
	static int pulse_mobile;
	static int pulse_violence;
	static int pulse_point;
	static int pulse_music;
	static int pulse_water_float;
	static int pulse_raffect;
	static int pulse_quest;
	static int pulse_track;

	if (--pulse_area <= 0) {
		wiznet("AREA & ROOM TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
		pulse_area = PULSE_AREA; 
		area_update();
		room_update();
		do_asave(NULL, "rules");
		wiznet("Saving WAFFs", NULL, NULL, WIZ_TICKS, 0, 0);
		save_waffs();
	}

	if (--pulse_music <= 0) {
		pulse_music	= PULSE_MUSIC;
/*		song_update(); */
	}

	if (--pulse_mobile <= 0) {
		pulse_mobile = PULSE_MOBILE;
		mobile_update();
		light_update();
	}

	if (--pulse_violence <= 0) {
		pulse_violence = PULSE_VIOLENCE;
		violence_update();
	}

	if (--pulse_water_float <= 0) {
		pulse_water_float = PULSE_WATER_FLOAT;
		water_float_update();
	}

	if (--pulse_raffect <= 0) {
		pulse_raffect = PULSE_RAFFECT;
		room_affect_update();
	}

	if (--pulse_track <= 0) {
		pulse_track = PULSE_TRACK;
		track_update();
	}

	if (--pulse_quest <= 0) {
		pulse_quest = PULSE_QUEST;
		quest_update();
	}

	if (--pulse_point <= 0) {
		CHAR_DATA *ch;

		wiznet("CHAR TICK!", NULL, NULL, WIZ_TICKS, 0, 0);
		pulse_point = PULSE_TICK;
		weather_update();
		char_update();
		obj_update();
		/*clan_item_update();*/
		world_affect_update();
		wiznet("Saving Server Usage", NULL, NULL, WIZ_TICKS, 0, 0);
		server_update_usage();
		save_server_usage();
		check_reboot();

		/* room counting */
		for (ch = char_list; ch && !IS_NPC(ch); ch = ch->next)
			if (ch->in_room)
				ch->in_room->area->count++;
	}

	aggr_update();
	auction_update();
	tail_chain();

	return;
}

void light_update(void)
{   
	CHAR_DATA *ch;
	int dam_light;
	DESCRIPTOR_DATA *d;


	for (d = descriptor_list; d != NULL; d = d->next) {
		if (d->connected != CON_PLAYING)
			continue;

		ch = (d->original != NULL) ? d->original : d->character;

		if (!ch) {
			BUG("light_update: ch is NULL? host: %s", d->host);
			continue;	
		}

		if (IS_IMMORTAL(ch))
			continue;

		/* also checks vampireness */
		if ((dam_light = isn_dark_safe(ch)) == 0) 	
			continue;	

		if (number_percent() < get_skill(ch, gsn_light_resistance)) {
			check_improve(ch, gsn_light_resistance, TRUE, 32);
			continue;
		}

		if (dam_light == 1)
			char_puts("The light in the room disturbs you.\n", ch);
		else
			char_puts("Sun light disturbs you.\n",ch);

		dam_light = 1 + (ch->max_hit * 4)/ 100;
		damage(ch, ch, dam_light, TYPE_HUNGER, DAM_LIGHT_V, TRUE);

		if (ch->position == POS_STUNNED)
			update_pos(ch);

		if (number_percent() < 10)
			gain_condition(ch, COND_DRUNK,  -1);
	}
}

void room_update(void)
{   
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;

	for (room = top_affected_room; room; room = room_next) {
		AFFECT_DATA *paf;
		AFFECT_DATA *paf_next;

		room_next = room->aff_next;

		for (paf = room->affected; paf != NULL; paf = paf_next) {
			paf_next = paf->next;

			if (paf->duration > 0) {
				paf->duration--;
/*
 * spell strength shouldn't fade with time 
 * because of checks in safe_rspell with af->level 
				if (number_range(0,4) == 0 && paf->level > 0)
					paf->level--;
*/
			}
			else if (paf->duration == 0)
				affect_remove_room(room, paf);
		}
	}
}

void room_affect_update(void)
{   
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;

	for (room = top_affected_room; room ; room = room_next)
	{
	room_next = room->aff_next;

	    while (IS_ROOM_AFFECTED(room, RAFF_PLAGUE) && room->people != NULL)
	    {
	        AFFECT_DATA *af, plague;
	        CHAR_DATA *vch;

	        for (af = room->affected; af != NULL; af = af->next)
	        {
	        	if (af->type == gsn_black_death)
	                break;
	        }
	    
	        if (af == NULL)
	        {
	        	REMOVE_BIT(room->affected_by,RAFF_PLAGUE);
	        	break;
	        }

	        if (af->level == 1)
	        	af->level = 2;
	
	    plague.where		= TO_AFFECTS;
	        plague.type 		= gsn_plague;
	        plague.level 		= af->level - 1; 
	        plague.duration 		= number_range(1,((plague.level/2)+1));
	        plague.location		= APPLY_NONE;
	        plague.modifier 		= -5;
	        plague.bitvector 		= AFF_PLAGUE;
	    
	        for (vch = room->people; vch != NULL; vch = vch->next_in_room)
	        {
	            if (!saves_spell(plague.level ,vch,DAM_DISEASE) 
		&&  !IS_IMMORTAL(vch)
		&&  !is_safe_rspell(af->level,vch)
	        	&&  !IS_AFFECTED(vch,AFF_PLAGUE) && number_bits(3) == 0)
	        	{
	        	    char_puts("You feel hot and feverish.\n",vch);
	        	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
	        	    affect_join(vch,&plague);
	        	}
	        }
	 break;
	    }

	    while (IS_ROOM_AFFECTED(room, RAFF_POISON) && room->people != NULL)
	    {
	        AFFECT_DATA *af, paf;
	        CHAR_DATA *vch;

	        for (af = room->affected; af != NULL; af = af->next)
	        {
	        	if (af->type == gsn_deadly_venom)
	                break;
	        }
	    
	        if (af == NULL)
	        {
	        	REMOVE_BIT(room->affected_by,RAFF_POISON);
	        	break;
	        }

	        if (af->level == 1)
	        	af->level = 2;
	
	    paf.where		= TO_AFFECTS;
	        paf.type 		= gsn_poison;
	        paf.level 		= af->level - 1; 
	        paf.duration 	= number_range(1,((paf.level/5)+1));
	        paf.location	= APPLY_NONE;
	        paf.modifier 	= -5;
	        paf.bitvector 	= AFF_POISON;
	    
	        for (vch = room->people; vch != NULL; vch = vch->next_in_room)
	        {
	            if (!saves_spell(paf.level ,vch,DAM_POISON) 
		&&  !IS_IMMORTAL(vch)
		&&  !is_safe_rspell(af->level,vch)
	        	&&  !IS_AFFECTED(vch,AFF_POISON) && number_bits(3) == 0)
	        	{
	        	    char_puts("You feel very sick.\n",vch);
	        	    act("$n looks very ill.",vch,NULL,NULL,TO_ROOM);
	        	    affect_join(vch,&paf);
	        	}
	        }
	 break;
	    }

	    while (IS_ROOM_AFFECTED(room, RAFF_SLOW) && room->people != NULL)
	    {
	        AFFECT_DATA *af, paf;
	        CHAR_DATA *vch;

	        for (af = room->affected; af != NULL; af = af->next)
	        {
	        	if (af->type == gsn_lethargic_mist)
	                break;
	        }
	    
	        if (af == NULL)
	        {
	        	REMOVE_BIT(room->affected_by,RAFF_SLOW);
	        	break;
	        }

	        if (af->level == 1)
	        	af->level = 2;
	
	    paf.where		= TO_AFFECTS;
	        paf.type 		= gsn_slow;
	        paf.level 		= af->level - 1; 
	        paf.duration 	= number_range(1,((paf.level/5)+1));
	        paf.location	= APPLY_NONE;
	        paf.modifier 	= -5;
	        paf.bitvector 	= AFF_SLOW;
	    
	        for (vch = room->people; vch != NULL; vch = vch->next_in_room)
	        {
	            if (!saves_spell(paf.level ,vch,DAM_OTHER) 
		&&  !IS_IMMORTAL(vch)
		&&  !is_safe_rspell(af->level,vch)
	        	&&  !IS_AFFECTED(vch,AFF_SLOW) && number_bits(3) == 0)
	        	{
	        	    char_puts("You start to move less quickly.\n",vch);
	        	    act("$n is moving less quickly.",vch,NULL,NULL,TO_ROOM);
	        	    affect_join(vch,&paf);
	        	}
	        }
	 break;
	    }

	    while (IS_ROOM_AFFECTED(room, RAFF_SLEEP) && room->people != NULL)
	    {
	        AFFECT_DATA *af, paf;
	        CHAR_DATA *vch;

	        for (af = room->affected; af != NULL; af = af->next)
	        {
	        	if (af->type == gsn_mysterious_dream)
	                break;
	        }
	    
	        if (af == NULL)
	        {
	        	REMOVE_BIT(room->affected_by,RAFF_SLEEP);
	        	break;
	        }

	        if (af->level == 1)
	        	af->level = 2;
	
	    paf.where		= TO_AFFECTS;
	        paf.type 		= gsn_sleep;
	        paf.level 		= af->level - 2; 
	        paf.duration 	= number_range(1,((paf.level/10)+1));
	        paf.location	= APPLY_NONE;
	        paf.modifier 	= -5;
	        paf.bitvector 	= AFF_SLEEP;
	    
	        for (vch = room->people; vch != NULL; vch = vch->next_in_room)
	        {
	            if (!saves_spell(paf.level - 4,vch,DAM_CHARM) 
		&&  !IS_IMMORTAL(vch)
		&&  !is_safe_rspell(af->level,vch)
		&&  !(IS_NPC(vch) && IS_SET(vch->pIndexData->act, ACT_UNDEAD))
	        	&&  !IS_AFFECTED(vch,AFF_SLEEP) && number_bits(3) == 0)
	        	{
		  if (IS_AWAKE(vch))
		   {
	        	    char_puts("You feel very sleepy.......zzzzzz.\n",vch);
	        	    act("$n goes to sleep.",vch,NULL,NULL,TO_ROOM);
		    vch->position = POS_SLEEPING;
		   }		    
	      	  affect_join(vch,&paf);
	        	}
	        }
	 break;
	    }


	    while (IS_ROOM_AFFECTED(room, RAFF_ESPIRIT) && room->people != NULL)
	    {
	        AFFECT_DATA *af, paf;
	        CHAR_DATA *vch;

	        for (af = room->affected; af != NULL; af = af->next)
	        {
	        	if (af->type == gsn_evil_spirit)
	                break;
	        }
	    
	        if (af == NULL)
	        {
	        	REMOVE_BIT(room->affected_by,RAFF_ESPIRIT);
	        	break;
	        }

	        if (af->level == 1)
	        	af->level = 2;
	
	    paf.where		= TO_AFFECTS;
	        paf.type 		= gsn_evil_spirit;
	        paf.level 		= af->level; 
	        paf.duration 	= number_range(1,(paf.level/30));
	        paf.location	= APPLY_NONE;
	        paf.modifier 	= 0;
	        paf.bitvector 	= 0;
	    
	        for (vch = room->people; vch != NULL; vch = vch->next_in_room)
	        {
			if (!saves_spell(paf.level + 2,vch,DAM_MENTAL) 
			&&  !IS_IMMORTAL(vch)
			&&  !is_safe_rspell(af->level,vch)
	        	&&  !is_affected(vch,gsn_evil_spirit) 
			&& number_bits(3) == 0) {
				char_puts("You feel worse than ever.\n",vch);
				act("$n looks more evil.",vch,NULL,NULL,TO_ROOM);
				affect_join(vch,&paf);
	        	}
	        }
	 break;
	    }

/* new ones here
	    while (IS_ROOM_AFFECTED(room, RAFF_) && room->people != NULL)
	    {
	        AFFECT_DATA *af, paf;
	        CHAR_DATA *vch;

	        for (af = room->affected; af != NULL; af = af->next)
	        {
	        	if (af->type == gsn_)
	                break;
	        }
	    
	        if (af == NULL)
	        {
	        	REMOVE_BIT(room->affected_by,RAFF_);
	        	break;
	        }

	        if (af->level == 1)
	        	af->level = 2;
	
	    paf.where		= TO_AFFECTS;
	        paf.type 		= gsn_;
	        paf.level 		= af->level - 1; 
	        paf.duration 	= number_range(1,((paf.level/5)+1));
	        paf.location	= APPLY_NONE;
	        paf.modifier 	= -5;
	        paf.bitvector 	= AFF_;
	    
	        for (vch = room->people; vch != NULL; vch = vch->next_in_room)
	        {
	            if (!saves_spell(paf.level + 2,vch,DAM_) 
		&&  !IS_IMMORTAL(vch)
		&&  !is_safe_rspell(af->level,vch)
	        	&&  !IS_AFFECTED(vch,AFF_) && number_bits(3) == 0)
	        	{
	        	    char_puts("You feel hot and feverish.\n",vch);
	        	    act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
	        	    affect_join(vch,&paf);
	        	}
	        }
	 break;
	    }
*/
	}
	return;
}


void check_reboot(void)
{
	DESCRIPTOR_DATA *d;

	switch(reboot_counter) {
	case -1:
		break;
	case 0:
		reboot_mud();
		return;
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 10:
	case 15:
		for (d = descriptor_list; d != NULL; d = d->next) 
			if (d->character != NULL) {
				if (rebooter || !IS_IMMORTAL(d->character))
					char_printf(d->character, 
						    "{*{W*****{R REBOOT IN {W%d{R MIN. {W*****{x\n",
					    	    reboot_counter);
				else
					char_printf(d->character, 
						    "{*{W*****{R AUTOMAGIC REBOOT IN {W%d{R MIN. {W*****{x\n",
					    	    reboot_counter);
				}

		/* FALLTHRU */
	default: 
		reboot_counter--;
		break;
	}
}

void track_update(void)
{   
	CHAR_DATA *ch, *ch_next;

	for (ch = npc_list; ch; ch = ch_next) {
		CHAR_DATA *vch, *vch_next;

		ch_next = ch->next;
		if (IS_AFFECTED(ch, AFF_CALM | AFF_CHARM)
	        ||  ch->fighting
		||  !ch->in_room
	        ||  !IS_AWAKE(ch) 
	        ||  IS_SET(ch->pIndexData->act, ACT_NOTRACK)
		||  RIDDEN(ch)
		||  IS_AFFECTED(ch, AFF_SCREAM))
			continue;

		if (ch->last_fought != NULL
		&&  ch->in_room != ch->last_fought->in_room) {
			do_track(ch, ch->last_fought->name);
			continue;
		}

		if (ch->in_mind == NULL)
			continue;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

			if (IS_IMMORTAL(vch)
			||  !can_see(ch,vch)
			||  is_safe_nomessage(ch,vch)
			||  !is_name(vch->name,ch->in_mind))
				continue;
			doprintf(do_yell, ch,
			         "So we meet again, %s", vch->name);
			do_murder(ch, vch->name);
		}
	}
}

/*
 * deprecated code
 */
void clan_item_update(void)
{	
	clan_t *clan;
	OBJ_DATA *obj;
	bool put_back;
	int i;
	int j;

	if (time_info.hour != 0) 
		return;

	for (i=0; i<clans.nused; i++) {
		put_back = FALSE;
		if ((clan=clan_lookup(i))->obj_ptr == NULL) 
			continue;
		for (obj=clan->obj_ptr; obj->in_obj; obj=obj->in_obj);
		if (obj->carried_by) 
			put_back = TRUE;

		else if (obj->in_room) 
			for(j=0; j<clans.nused; j++) 
				if (!(put_back=(obj->in_room->vnum != clan_lookup(j)->altar_vnum)))
					break;

		if(put_back) {
			if(clan->obj_ptr->in_obj)
				obj_from_obj(clan->obj_ptr);
			if(clan->obj_ptr->carried_by)
				obj_from_char(clan->obj_ptr);
			if(clan->obj_ptr->in_room)
				obj_from_room(clan->obj_ptr);

			if(clan->altar_ptr) 
				obj_to_obj(clan->obj_ptr, clan->altar_ptr);
			else 
				bug("clan_item_update: no altar_ptr for clan %d", i);
		}
	}
}
