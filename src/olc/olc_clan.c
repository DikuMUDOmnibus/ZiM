/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: olc_clan.c 849 2006-04-22 13:08:54Z zsuzsu $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "olc.h"

#define EDIT_CLAN(ch, clan)	(clan = (clan_t*) ch->desc->pEdit)

DECLARE_OLC_FUN(claned_create		);
DECLARE_OLC_FUN(claned_edit		);
DECLARE_OLC_FUN(claned_touch		);
DECLARE_OLC_FUN(claned_show		);
DECLARE_OLC_FUN(claned_list		);

DECLARE_OLC_FUN(claned_name		);
DECLARE_OLC_FUN(claned_filename		);
DECLARE_OLC_FUN(claned_recall		);
DECLARE_OLC_FUN(claned_flags		);
DECLARE_OLC_FUN(claned_skill		);
DECLARE_OLC_FUN(claned_title		);
DECLARE_OLC_FUN(claned_item		);
DECLARE_OLC_FUN(claned_mark		);
DECLARE_OLC_FUN(claned_altar		);
DECLARE_OLC_FUN(claned_donate		);
DECLARE_OLC_FUN(claned_plist		);

DECLARE_OLC_FUN(claned_skill_add	);
DECLARE_OLC_FUN(claned_skill_del	);

static DECLARE_VALIDATE_FUN(validate_name);

olc_cmd_t olc_cmds_clan[] =
{
	{ 9, "create",	claned_create					},
	{ 5, "edit",	claned_edit					},
	{ 5, "touch",	claned_touch					},
	{ 0, "show",	claned_show					},
	{ 0, "list",	claned_list					},

	{ 5, "name",	claned_name,		validate_name	 	},
	{ 5, "filename",claned_filename,	validate_filename	},
	{ 5, "recall",	claned_recall,		validate_room_vnum	},
	{ 5, "flags",	claned_flags,		clan_flags		},
	{ 5, "skill",	claned_skill					},
	{ 5, "title",	claned_title					},
	{ 5, "item",	claned_item					},
	{ 5, "mark",	claned_mark					},
	{ 5, "altar", 	claned_altar					},
	{ 5, "donate",	claned_donate					},
	{ 5, "plist",	claned_plist					},

	{ 0, "commands",show_commands					},
	{ 0, NULL }
};

OLC_FUN(claned_create)
{
	int cln;
	clan_t *clan;
	char arg[MAX_STRING_LENGTH];

	if (ch->pcdata->security < SECURITY_CLAN) {
		char_puts("ClanEd: Insufficient security for creating clans\n",
			  ch);
		return FALSE;
	}

	first_arg(argument, arg, sizeof(arg), FALSE);
	if (arg[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((cln = cln_lookup(arg)) >= 0) {
		char_printf(ch, "ClanEd: %s: already exists.\n",
			    CLAN(cln)->name);
		return FALSE;
	}

	clan		= clan_new();
	clan->name	= str_dup(arg);
	clan->file_name	= str_printf("clan%02d.clan", clans.nused-1);

	ch->desc->pEdit	= (void *)clan;
	OLCED(ch)	= olced_lookup(ED_CLAN);
	touch_clan(clan);
	char_puts("Clan created.\n",ch);
	return FALSE;
}

OLC_FUN(claned_edit)
{
	int cln;
	char arg[MAX_STRING_LENGTH];

	if (ch->pcdata->security < SECURITY_CLAN) {
		char_puts("ClanEd: Insufficient security.\n", ch);
		return FALSE;
	}

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if ((cln = cln_lookup(arg)) < 0) {
		char_printf(ch, "ClanEd: %s: No such clan.\n", arg);
		return FALSE;
	}

	ch->desc->pEdit	= CLAN(cln);
	OLCED(ch)	= olced_lookup(ED_CLAN);
	return FALSE;
}

OLC_FUN(claned_touch)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return touch_clan(clan);
}

OLC_FUN(claned_show)
{
	char arg[MAX_STRING_LENGTH];
	int i;
	BUFFER *output;
	clan_t *clan;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_CLAN))
			EDIT_CLAN(ch, clan);
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		if ((i = cln_lookup(arg)) < 0) {
			char_printf(ch, "ClanEd: %s: No such clan.\n", arg);
			return FALSE;
		}
		clan = CLAN(i);
	}

	output = buf_new(-1);
	buf_printf(output,
		   "Name:        [%s]\n"
		   "Filename:    [%s]\n",
		   clan->name,
		   clan->file_name);
	if (clan->flags)
		buf_printf(output, "Flags:       [%s]\n",
			   flag_string(clan_flags, clan->flags));
	if (clan->recall_vnum)
		buf_printf(output, "Recall:      [%d]\n",
			   clan->recall_vnum);
	if (clan->obj_vnum)
		buf_printf(output, "Item:        [%d]\n",
			   clan->obj_vnum);
	if (clan->mark_vnum) 
		buf_printf(output, "Mark:	 [%d]\n",
			   clan->mark_vnum);
	if (clan->altar_vnum)
		buf_printf(output, "Altar:       [%d]\n",
			   clan->altar_vnum);
	if (clan->donate_vnum)
		buf_printf(output, "Donation:	 [%d]\n",
			   clan->donate_vnum);

	for (i = 0; i < clan->skills.nused; i++) {
		clskill_t *cs = VARR_GET(&clan->skills, i);
		skill_t *sk;

		if (cs->sn <= 0
		||  (sk = skill_lookup(cs->sn)) == NULL)
			continue;
		buf_printf(output, "Skill:       '%s' (level %d, %d%%)\n",
			   sk->name, cs->level, cs->percent);
	}

	for (i = MAX_CLAN_RANK-1; i >= 0; i--) {
		buf_printf(output, "Title: %2d %-6s %s\n", 
			i, "male", clan->rank_table[i].title[SEX_MALE]);
		buf_printf(output, "Title: %2d %-6s %s\n", 
			i, "female", clan->rank_table[i].title[SEX_FEMALE]);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(claned_list)
{
	int i;

	for (i = 0; i < clans.nused; i++)
		char_printf(ch, "[%d] %s\n", i, CLAN(i)->name);
	return FALSE;
}

OLC_FUN(claned_name)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, cmd, &clan->name);
}

OLC_FUN(claned_filename)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_str(ch, argument, cmd, &clan->file_name);
}

OLC_FUN(claned_recall)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->recall_vnum);
}

OLC_FUN(claned_item)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->obj_vnum);
}

OLC_FUN(claned_mark)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->mark_vnum);
}

OLC_FUN(claned_altar)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->altar_vnum);
}
OLC_FUN(claned_donate)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_number(ch, argument, cmd, &clan->donate_vnum);
}
OLC_FUN(claned_flags)
{
	clan_t *clan;
	EDIT_CLAN(ch, clan);
	return olced_flag32(ch, argument, cmd, &clan->flags);
}

OLC_FUN(claned_skill)
{
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_prefix(arg, "add")) 
		return claned_skill_add(ch, argument, cmd);
	else if (!str_prefix(arg, "delete"))
		return claned_skill_del(ch, argument, cmd);

	do_help(ch, "'OLC CLAN SKILL'");
	return FALSE;
}

/*
 * syntax: level sex title
 */
OLC_FUN(claned_title)
{
	char	arg1[MAX_STRING_LENGTH];
	char	arg2[MAX_STRING_LENGTH];
	clan_t *clan;
	int	rank =0;
	int	sex = 0;
	EDIT_CLAN(ch, clan);

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0') {
		do_help(ch, "'OLC CLAN TITLE'");
		return FALSE;
	}

	rank = atoi(arg1);
	sex = flag_value(sex_table, arg2);

        free_string(clan->rank_table[rank].title[sex]);
	clan->rank_table[rank].title[sex] = str_dup(argument);
	char_puts("Ok.\n", ch);

	return TRUE;
}

OLC_FUN(claned_plist)
{
	const char **nl;
	const char *name;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	clan_t *clan;
	int rank = 0;
	EDIT_CLAN(ch, clan);
	
	if (ch->pcdata->security < SECURITY_CLAN_PLIST) {
		char_puts("ClanEd: Insufficient security.\n", ch);
		return FALSE;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
		   one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || !isdigit(arg1[0])) {
		do_help(ch, "'OLC CLAN PLIST'");
		return FALSE;
	}

	rank = atoi(arg1);
	if (rank > MAX_CLAN_RANK || rank < 0) {
		do_help(ch, "'OLC CLAN PLIST'");
		return FALSE;
	}
	
	nl = &clan->member_list[rank];
	name = clan->rank_table[rank].title[1];

	if (arg2[0] == '\0') {
		char_printf(ch, "List of %s of %s: [%s]\n",
			    name, clan->name, *nl);
		return FALSE;
	}
			    
	if (!pc_name_ok(arg2)) {
		char_printf(ch, "ClanEd: %s: Illegal name\n", arg2);
		return FALSE;
	}

	name_toggle(nl, arg2, ch, "ClanEd");
	return TRUE;
}

OLC_FUN(claned_skill_add)
{
	int sn;
	int percent;
	clskill_t *clsk;
	char	arg1[MAX_STRING_LENGTH];
	char	arg2[MAX_STRING_LENGTH];
	char	arg3[MAX_STRING_LENGTH];
	clan_t *clan;
	EDIT_CLAN(ch, clan);

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
		   one_argument(argument, arg3, sizeof(arg3));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		do_help(ch, "'OLC CLAN SKILL'");
		return FALSE;
	}

	if ((sn = sn_lookup(arg1)) <= 0) {
		char_printf(ch, "ClanEd: %s: unknown skill.\n", arg1);
		return FALSE;
	}

	if (!IS_SET(SKILL(sn)->flags, SKILL_CLAN)) {
		char_printf(ch, "ClanEd: %s: not a clan skill.\n",
			    SKILL(sn)->name);
		return FALSE;
	}

	if ((clsk = clskill_lookup(clan, sn))) {
		char_printf(ch, "ClanEd: %s: already there.\n",
			    SKILL(sn)->name);
		return FALSE;
	}

	percent = atoi(arg3);
	if (percent < 1 || percent > 100) {
		char_puts("ClanEd: percent value must be in range 1..100.\n",
			  ch);
		return FALSE;
	}

	clsk = varr_enew(&clan->skills);
	clsk->sn = sn;
	clsk->level = atoi(arg2);
	clsk->percent = percent;
	varr_qsort(&clan->skills, cmpint);

	return TRUE;
}

OLC_FUN(claned_skill_del)
{
	char	arg[MAX_STRING_LENGTH];
	clskill_t *clsk;
	clan_t *clan;
	EDIT_CLAN(ch, clan);

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC CLAN SKILL'");
		return FALSE;
	}

	if ((clsk = skill_vlookup(&clan->skills, arg)) == NULL) {
		char_printf(ch, "ClanEd: %s: not found in clan skill list.\n",
			    arg);
		return FALSE;
	}
	clsk->sn = 0;
	varr_qsort(&clan->skills, cmpint);
	return TRUE;
}

bool touch_clan(clan_t *clan)
{
	SET_BIT(clan->flags, CLAN_CHANGED);
	return FALSE;
}

static VALIDATE_FUN(validate_name)
{
	int i;
	clan_t *clan;
	EDIT_CLAN(ch, clan);

	for (i = 0; i < clans.nused; i++)
		if (CLAN(i) != clan
		&&  !str_cmp(CLAN(i)->name, arg)) {
			char_printf(ch, "ClanEd: %s: duplicate clan name.\n",
				    arg);
			return FALSE;
		}

	return TRUE;
}

