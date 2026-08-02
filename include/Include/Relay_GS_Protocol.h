/************************************************************************/
/* Relay和GS之间的协议定义					                            */
/* Copyright : Kingsoft 2004										    */
/* Author	 : Zhu Jianqiu												*/
/* History	 :															*/
/*		2004.12.09	Create												*/
/************************************************************************/
#ifndef _RELAY_GS_PROTOCOL_H_
#define _RELAY_GS_PROTOCOL_H_

#include "SO3ProtocolBasic.h"
#include "KRoleDBDataDef.h"
#include "KTongDef.h"
#include "KStatDataDef.h"
#include "KGameCardDef.h"
#include "KMailDef.h"
#include "KAuctionDef.h"
#include "KMentorDef.h"

#define PQ_PARAM_SYNC_COUNT 8
#define MAX_ROLE_DATA_PAK_SIZE  (1024 * 32)
#define MAX_ACCOUNT_DATA_SIZE   (1024 * 256)
#define MAX_ACCOUNT_DATA_PAK_SIZE (1024 * 32)

// 上行协议定义
// Target v2.5.2 KS2R IDs are explicit. Missing source routes remain named
// here so the enum itself cannot silently renumber later IDs.
enum KS2R_PROTOCOL
{
    s2r_protocol_begin = 0,

	s2r_handshake_request = 1,
    s2r_ping_signal = 2,
    s2r_update_performance = 3,

	s2r_create_map_respond = 4,

    s2r_player_login_respond = 5,
	s2r_search_map_request = 6,
	s2r_search_map_queue_request = 7,
    s2r_transfer_player_request = 8,
    s2r_transfer_player_respond = 9,
    s2r_confirm_player_login_request = 10,
	s2r_player_leave_gs = 11,

	s2r_update_fellowship = 12,

    // ------------- 组队相关协议 --------------------------------
	s2r_invite_player_join_team_request = 13,
	s2r_invite_player_join_team_respond = 14,
    s2r_apply_join_team_request = 15,
    s2r_apply_join_team_respond = 16,
    s2r_team_del_member_request = 17,
    s2r_team_change_authority_request = 18,
    s2r_team_set_loot_mode_request = 19,
	s2r_team_set_roll_quality_request = 20,
	s2r_team_set_formation_leader_request = 21,
    s2r_team_disband_request = 22,
    s2r_sync_team_member_max_lmr = 23,
    s2r_sync_team_member_current_lmr = 24,
    s2r_sync_team_member_misc = 25,
    s2r_sync_team_member_position = 26,
    s2r_team_set_mark_request = 27,
    s2r_team_level_up_raid_request = 28,
    s2r_team_change_member_group_request = 29,

    // ---------------- 聊天相关 --------------------------------------
    s2r_talk_message = 30,
    s2r_remote_talk_message = 31,
    s2r_player_talk_error = 32,

    // ---------------- 好友相关 --------------------------------------
    s2r_apply_fellowship_data_request = 33,
    s2r_update_fellowship_data = 34,
    s2r_add_fellowship_request = 35,
    s2r_get_fellowship_name_request = 36,
    s2r_apply_fellowship_player_fellow_info = 37,
    s2r_add_fellowship_notify = 38,
    s2r_sync_fellowship_player_mini_avatar = 39,

    // ---------------- 邮件系统 --------------------------------------
    s2r_send_mail_request = 40,
    s2r_send_global_mail_request = 41,
    s2r_get_maillist_request = 42,
    s2r_query_mail_content = 43,
    s2r_acquire_mail_money_request = 44,
    s2r_acquire_mail_item_request = 45,
    s2r_give_mail_money_to_player = 46,
    s2r_give_mail_item_to_player = 47,
    s2r_set_mail_read = 48,
    s2r_delete_mail = 49,
    s2r_return_mail = 50,
	s2r_sync_mid_map_mark = 51,
    
    // ---------------- 副本相关 --------------------------------------
    // Target v2.5.2 Relay IDs are explicit because the 2010 enum omits routes.
    s2r_save_scene_player_list = 52,
    s2r_save_scene_data = 53,
    s2r_reset_map_copy_request = 54,
    s2r_player_enter_scene_notify = 55,

    // ---------------- 交通相关 --------------------------------------
    s2r_sync_road_track_force = 56,
    
    // ---------------- 角色操作 --------------------------------------
    s2r_load_role_data_request = 57,
    s2r_load_account_data_request = 58,

    s2r_change_role_level_request = 59,
    s2r_change_role_forceid_request = 60,

    s2r_send_gm_message = 61,
    s2r_send_gm_chn = 62,

    s2r_send_gm_command_to_player_gs = 63,

    s2r_send_gm_command_gc = 64,
    s2r_send_gm_command_gs = 65,

    s2r_send_gm_command_all_gs = 66,
    s2r_send_gm_command_all_gc = 67,

    s2r_join_battle_field_queue_request = 68,
    s2r_leave_battle_field_queue_request = 69,

    s2r_accept_join_battle_field = 70,

    s2r_leave_battle_field_request = 71,

    s2r_join_tong_battle_field_queue_request = 72,
    s2r_leave_tong_battle_field_queue_request = 73,
    s2r_accept_join_tong_battle_field = 74,
    s2r_leave_tong_battle_field_request = 75,
    s2r_join_arena_visitor_queue_request = 76,
    s2r_leave_arena_visitor_queue_request = 77,
    s2r_join_arena_queue_request = 78,
    s2r_leave_arena_queue_request = 79,
    s2r_accept_join_arena = 80,
    s2r_leave_arena = 81,
    s2r_corps_create_request = 82,
    s2r_corps_destroy_request = 83,
    s2r_corps_del_member_request = 84,
    s2r_corps_change_leader_request = 85,
    s2r_invitation_join_corps_request = 86,
    s2r_apply_invitation_join_corps = 87,
    s2r_sync_corps_list_request = 88,
    s2r_sync_corps_rank_list_request = 89,
    s2r_sync_corps_base_data_request = 90,
    s2r_sync_corps_member_data_request = 91,
    s2r_sync_corps_change_data_request = 92,
    s2r_update_corps_value = 93,
    s2r_get_seanson_rank_info_request = 94,

    s2r_apply_create_pq_request = 95,
    s2r_apply_delete_pq_request = 96,
    s2r_apply_change_pq_value_request = 97,
    
    s2r_add_camp_score = 98,
    s2r_add_new_camp_fight_value = 99,
    s2r_set_new_camp_fight_value = 100,
    s2r_apply_set_camp_request = 101,
    s2r_sync_camp_active_change = 102,

    s2r_sync_role_data = 103,
    s2r_save_role_data = 104,
    s2r_sync_account_data = 105,
    s2r_save_account_data = 106,

    // ---------------- 帮会相关 --------------------------------------
    s2r_apply_tong_roster_request = 107,
    s2r_apply_tong_info_request = 108,
    s2r_apply_tong_repertory_page_request = 109,
    s2r_apply_create_tong_request = 110,
    s2r_invite_player_join_tong_request = 111,
    s2r_invite_player_join_tong_respond = 112,
    s2r_apply_kick_out_tong_member_request = 113,
    s2r_modify_tong_info_request = 114,
    s2r_modify_tong_schema_request = 115,
    s2r_apply_quit_tong_request = 116,
    s2r_change_tong_member_group_request = 117,
    s2r_change_tong_master_request = 118,
    s2r_cancel_change_tong_master = 119,
    s2r_change_tong_member_remark_request = 120,
    s2r_get_tong_description_request = 121,
    s2r_save_money_in_tong_request = 122,
    s2r_get_tong_salary_request = 123,
    s2r_get_tong_salary_fail_respond = 124,
    s2r_change_tong_camp_request = 125,

    s2r_apply_open_tong_repertory_request = 130,

    s2r_take_tong_repertory_item_request = 131,

    s2r_put_tong_repertory_item_request = 132,

    s2r_exchange_tong_repertory_item_pos_request = 133,

    s2r_apply_stack_tong_repertory_item_request = 104, /* source-only route; target mapping pending */
    s2r_stack_tong_repertory_item_request = 134,

    s2r_stack_item_in_tong_repertory_request = 135,
    s2r_tong_tech_tree_produce_item_respond = 136,
    s2r_return_tong_bind_item = 137,
    s2r_sync_tong_history_request = 138,
    s2r_sync_tong_simple_info_request = 139,
    s2r_update_tong_member_equip_score = 140,
    // ---------------- 寄卖行相关 --------------------------------------
    s2r_auction_lookup_request = 141,
    s2r_auction_bid_request = 142,
    s2r_auction_sell_request = 143,
    s2r_auction_cancel_request = 144,
    s2r_bm_sell_respond = 145,
    s2r_bm_lookup_request = 146,
    s2r_bm_bid_request = 147,
    s2r_bm_bid_cancel_request = 148,

    s2r_remote_lua_call = 149,
    
    s2r_achievement_global_announce_request = 150,

    s2r_designation_global_announce_request = 151,

    // ---------------- 统计数据相关 ------------------------------------
    s2r_update_stat_data_request = 152,
    s2r_query_stat_id_request = 153,
    s2r_map_copy_keep_player = 154,
    s2r_sync_new_ext_point_request = 155,

    s2r_change_ext_point_request = 156,
    s2r_change_new_ext_point_request = 157,
    s2r_apply_gs_new_ext_point = 158,
    s2r_sync_gs_new_ext_point = 159,
    s2r_set_charge_flag_request = 160,
    s2r_active_present_code_request = 161,

    s2r_apex_protocol = 162,
    
    s2r_report_farmer_player_request = 163,
    s2r_apply_freeze_farmer_reuqest = 164,
    s2r_apply_coin_operating_flag = 165,

    s2r_game_card_sell_coin_request = 166,
    s2r_game_card_buy_coin_request = 167,
    s2r_game_card_lookup_request = 168,
    s2r_game_card_cancel_request = 169,
    s2r_game_card_sell_money_request = 170,
    s2r_game_card_buy_money_request = 171,
    s2r_coin_shop_buy_item_request = 172,
    s2r_coin_shop_buy_item_ex_request = 173,

    s2r_apply_mentor_data = 174,
    s2r_apply_direct_mentor_data = 175,
    s2r_update_max_apprentice_num = 176,
    s2r_add_mentor_value_request = 177,
    s2r_add_ta_equips_score_request = 178,
    s2r_pickup_ta_equips_score_request = 179,
    s2r_update_single_dungeon_score = 180,
    s2r_apply_single_dungeon_score_rank_player_info = 181,
    s2r_apply_single_dungeon_last_score = 182,
    s2r_transfer_log = 183,
    s2r_report_camp_npc_info = 184,
    s2r_del_camp_npc = 185,
    s2r_verify_trust_result = 186,
    s2r_send_clientfile_checksum = 187,
    s2r_mibao_verify_request = 188,

    
    s2r_protocol_end = 189
};

// Source-2010 names without target route/handler proof. These values preserve
// source compatibility only; they are not part of the target contract.
enum KS2R_LEGACY_SOURCE_ONLY_PROTOCOL
{
    s2r_apply_fellowship_player_level_and_forceid = 33,
    s2r_get_battle_field_list = 68,
    s2r_pay_tong_salary_request = 91,
    s2r_take_tong_repertory_item_to_pos_request = 96,
    s2r_take_tong_repertory_item_respond = 98,
    s2r_put_tong_repertory_item_to_pos_request = 99,
    s2r_put_tong_repertory_item_respond = 101,
    s2r_unlock_tong_repertory_grid = 102,
    s2r_game_card_sell_request = 119,
    s2r_game_card_buy_request = 120
};

// 下行协议定义
enum KR2S_PROTOCOL
{
	r2s_protocol_begin = 0,
	r2s_handshake_respond,	// v246 id=1 size=18 [src]
	r2s_quit_notify,	// v246 id=2 size=6 [src-unplaced] noop
	r2s_create_map_notify,	// v246 id=3 size=22 [src]
	r2s_finish_create_map_notify,	// v246 id=4 size=6 [src]
	r2s_delete_map_notify,	// v246 id=5 size=10 [src]
	r2s_player_login_request,	// v246 id=6 size=302 [src]
	r2s_search_map_respond,	// v246 id=7 size=46 [src]
	r2s_v246_unused_8,	// v246 id=8 size=30 [new] noop
	r2s_transfer_player_request,	// v246 id=9 size=323 [src]
	r2s_transfer_player_respond,	// v246 id=10 size=36 [src]
	r2s_confirm_player_login_respond,	// v246 id=11 size=10 [src]
	r2s_kick_account_notify,	// v246 id=12 size=14 [src-unplaced] noop
	r2s_invite_player_join_team_request,	// v246 id=13 size=6 [src-unplaced] noop
	r2s_apply_join_team_request,	// v246 id=14 size=45 [src-unplaced] noop
	r2s_v246_unused_15,	// v246 id=15 size=53 [new] noop
	r2s_team_create_notify,	// v246 id=16 size=104 [src]
	r2s_team_add_member_notify,	// v246 id=17 size=46 [src]
	r2s_team_del_member_notify,	// v246 id=18 size=14 [src]
	r2s_team_change_authority_notify,	// v246 id=19 size=11 [src]
	r2s_team_set_loot_mode_notify,	// v246 id=20 size=14 [src]
	r2s_team_disband_notify,	// v246 id=21 size=6 [src]
	r2s_sync_team_member_online_flag,	// v246 id=22 size=14 [src]
	r2s_sync_team_member_max_lmr,	// v246 id=23 size=34 [src-unplaced] noop
	r2s_sync_team_member_current_lmr,	// v246 id=24 size=16 [src-unplaced] noop
	r2s_sync_team_member_misc,	// v246 id=25 size=26 [src-unplaced] noop
	r2s_sync_team_member_position,	// v246 id=26 size=26 [src-unplaced] noop
	r2s_party_message_notify,	// v246 id=27 size=10 [src]
	r2s_team_set_formation_leader_notify,	// v246 id=28 size=14 [src]
	r2s_team_set_mark_respond,	// v246 id=29 size=14 [src-unplaced] noop
	r2s_team_camp_change,	// v246 id=30 size=7 [src]
	r2s_team_level_up_raid_notify,	// v246 id=31 size=10 [src]
	r2s_team_change_member_group_notify,	// v246 id=32 size=22 [src]
	r2s_player_talk_error,	// v246 id=33 size=10 [src]
	r2s_player_talk_whisper_success_respond,	// v246 id=34 size=10 [src-unplaced] noop
	r2s_talk_message,	// v246 id=35 size=75 [src-unplaced] noop
	r2s_v246_unused_36,	// v246 id=36 size=75 [new] noop
	r2s_apply_fellowship_data_respond,	// v246 id=37 size=6 [src]
	r2s_add_fellowship_respond,	// v246 id=38 size=43 [src-unplaced] noop
	r2s_get_fellowship_name_respond,	// v246 id=39 size=7 [src-unplaced] noop
	r2s_sync_fellowship_mapid,	// v246 id=40 size=11 [src-unplaced] noop
	r2s_sync_fellowship_player_level,	// v246 id=41 size=8 [src-unplaced] noop
	r2s_sync_fellowship_player_forceid,	// v246 id=42 size=7 [src-unplaced] noop
	r2s_add_fellowship_notify,	// v246 id=43 size=7 [src-unplaced] noop
	r2s_v246_unused_44,	// v246 id=44 size=46 [new] noop
	r2s_v246_unused_45,	// v246 id=45 size=6 [new] noop
	r2s_v246_unused_46,	// v246 id=46 size=10 [new] noop
	r2s_send_mail_respond,	// v246 id=47 size=40 [src]
	r2s_get_maillist_respond,	// v246 id=48 size=10 [src-unplaced] noop
	r2s_sync_mail_content,	// v246 id=49 size=11 [src]
	r2s_give_mail_money_to_player,	// v246 id=50 size=47 [src]
	r2s_give_mail_item_to_player,	// v246 id=51 size=54 [src]
	r2s_new_mail_notify,	// v246 id=52 size=118 [src-unplaced] noop
	r2s_mail_general_respond,	// v246 id=53 size=11 [src-unplaced] noop
	r2s_sync_mid_map_mark,	// v246 id=54 size=58 [src-unplaced] noop
	r2s_map_copy_do_clear_player_prepare,	// v246 id=55 size=18 [src]
	r2s_map_copy_do_clear_player,	// v246 id=56 size=10 [src]
	r2s_set_map_copy_owner,	// v246 id=57 size=14 [src]
	r2s_sync_road_track_info,	// v246 id=58 size=8 [src]
	r2s_save_role_data_respond,	// v246 id=59 size=14 [src]
	r2s_save_account_data_respond,	// v246 id=60 size=14 [target]
	r2s_sync_role_data,	// v246 id=61 size=10 [src]
	r2s_load_role_data,	// v246 id=62 size=14 [src]
	r2s_sync_account_data,	// v246 id=63 size=10 + data [target]
	r2s_load_account_data,	// v246 id=64 size=14 + data [target]
	r2s_gm_command,	// v246 id=65 size=38 [src]
	r2s_join_battle_field_queue_respond,	// v246 id=66 size=50 [src-unplaced] noop
	r2s_leave_battle_field_queue_respond,	// v246 id=67 size=14 [src-unplaced] noop
	r2s_battle_field_notify,	// v246 id=68 size=38 [src-unplaced] noop
	r2s_accept_join_battle_field_respond,	// v246 id=69 size=50 [src]
	r2s_leave_battle_field_respond,	// v246 id=70 size=10 [src]
	r2s_v246_unused_71,	// v246 id=71 size=46 [new] noop
	r2s_v246_unused_72,	// v246 id=72 size=10 [new] noop
	r2s_v246_unused_73,	// v246 id=73 size=46 [new] noop
	r2s_v246_unused_74,	// v246 id=74 size=6 [new] noop
	r2s_v246_unused_75,	// v246 id=75 size=30 [new] noop
	r2s_v246_unused_76,	// v246 id=76 size=6 [new] noop
	r2s_v246_unused_77,	// v246 id=77 size=6 [new] noop
	r2s_v246_unused_78,	// v246 id=78 size=46 [new] noop
	r2s_v246_unused_79,	// v246 id=79 size=6 [new] noop
	r2s_v246_unused_80,	// v246 id=80 size=50 [new] noop
	r2s_v246_unused_81,	// v246 id=81 size=10 [new] noop
	r2s_v246_unused_82,	// v246 id=82 size=46 [new] noop
	r2s_v246_unused_83,	// v246 id=83 size=10 [new] noop
	r2s_v246_unused_84,	// v246 id=84 size=126 [new] noop
	r2s_v246_unused_85,	// v246 id=85 size=38 [new] noop
	r2s_v246_unused_86,	// v246 id=86 size=22 [new] noop
	r2s_v246_unused_87,	// v246 id=87 size=54 [new] noop
	r2s_v246_unused_88,	// v246 id=88 size=94 [new] noop
	r2s_v246_unused_89,	// v246 id=89 size=26 [new] noop
	r2s_v246_unused_90,	// v246 id=90 size=154 [new] noop
	r2s_v246_unused_91,	// v246 id=91 size=74 [new] noop
	r2s_v246_unused_92,	// v246 id=92 size=54 [new] noop
	r2s_sync_pq,	// v246 id=93 size=60 [src]
	r2s_delete_pq,	// v246 id=94 size=6 [src]
	r2s_pq_value_change,	// v246 id=95 size=14 [src]
	r2s_pq_finish,	// v246 id=96 size=10 [src]
	r2s_sync_camp_info,	// v246 id=97 size=106 [src-unplaced] noop
	r2s_apply_set_camp_respond,	// v246 id=98 size=8 [src]
	r2s_sync_tong_member_info,	// v246 id=99 size=18 [src-unplaced] noop
	r2s_delete_tong_member_notify,	// v246 id=100 size=107 [src-unplaced] noop
	r2s_apply_tong_info_respond,	// v246 id=101 size=14 [src-unplaced] noop
	r2s_update_tong_client_data_version,	// v246 id=102 size=7 [src-unplaced] noop
	r2s_invite_player_join_tong_request,	// v246 id=103 size=15 [src-unplaced] noop
	r2s_v246_unused_104,	// v246 id=104 size=10 [new] noop
	r2s_v246_unused_105,	// v246 id=105 size=78 [new] noop
	r2s_change_player_tong_notify,	// v246 id=106 size=43 [src]
	r2s_tong_broadcast_message,	// v246 id=107 size=11 [src]
	r2s_get_tong_description_respond,	// v246 id=108 size=42 [src-unplaced] noop
	r2s_sync_tong_online_message,	// v246 id=109 size=7 [src]
	r2s_apply_open_tong_repertpry_respond,	// v246 id=110 size=40 [src]
	r2s_sync_tong_repertory_page_respond,	// v246 id=111 size=13 [src-unplaced] noop
	r2s_take_tong_repertory_item_respond,	// v246 id=112 size=9 [src]
	r2s_put_tong_repertory_item_respond,	// v246 id=113 size=9 [src-unplaced] noop
	r2s_apply_stack_tong_repertory_item_respond,	// v246 id=114 size=13 [src-unplaced] noop
	r2s_v246_unused_115,	// v246 id=115 size=15 [new] noop
	r2s_get_tong_salary_respond,	// v246 id=116 size=10 [src]
	r2s_sync_tong_history_respond,	// v246 id=117 size=12 [src]
	r2s_auction_lookup_respond,	// v246 id=118 size=6 [src-unplaced] noop
	r2s_v246_unused_119,	// v246 id=119 size=2 [new] noop
	r2s_v246_unused_120,	// v246 id=120 size=30 [new] noop
	r2s_v246_unused_121,	// v246 id=121 size=15 [new] noop
	r2s_v246_unused_122,	// v246 id=122 size=11 [new] noop
	r2s_v246_unused_123,	// v246 id=123 size=6 [new] noop
	r2s_v246_unused_124,	// v246 id=124 size=8 [new] noop
	r2s_v246_unused_125,	// v246 id=125 size=16 [new] noop
	r2s_auction_bid_respond,	// v246 id=126 size=19 [src]
	r2s_auction_sell_respond,	// v246 id=127 size=23 [src]
	r2s_auction_cancel_respond,	// v246 id=128 size=7 [src-unplaced] noop
	r2s_auction_message_notify,	// v246 id=129 size=75 [src-unplaced] noop
	r2s_v246_unused_130,	// v246 id=130 size=18 [new] noop
	r2s_v246_unused_131,	// v246 id=131 size=15 [new] noop
	r2s_v246_unused_132,	// v246 id=132 size=23 [new] noop
	r2s_v246_unused_133,	// v246 id=133 size=11 [new] noop
	r2s_remote_lua_call,	// v246 id=134 size=38 [src]
	r2s_achievement_global_announce_respond,	// v246 id=135 size=38 [src-unplaced] noop
	r2s_designation_global_announce_respond,	// v246 id=136 size=42 [src-unplaced] noop
	r2s_sync_global_system_value,	// v246 id=137 size=18 [src-unplaced] noop
	r2s_query_stat_id_respond,	// v246 id=138 size=134 [src-unplaced] noop
	r2s_sync_new_ext_point_respond,	// v246 id=139 size=19 [target]
	r2s_change_ext_point_respond,	// v246 id=140 size=18 [src]
	r2s_change_new_ext_point_respond,	// v246 id=141 size=26 [target]
	r2s_apply_gs_new_ext_point,	// v246 id=142 size=14 [target]
	r2s_sync_gs_new_ext_point,	// v246 id=143 size=15 [target]
	r2s_set_charge_flag_respond,	// v246 id=144 size=14 [src]
	r2s_sync_zone_charge_flag,	// v246 id=145 size=6 [src-unplaced] noop
	r2s_v246_unused_146,	// v246 id=146 size=18 [new] noop
	r2s_active_present_code_respond,	// v246 id=147 size=46 [src]
	r2s_apex_protocol,	// v246 id=148 size=0 [src-unplaced] noop
	r2s_set_farmer_limit_flag_request,	// v246 id=149 size=10 [src-unplaced] noop
	r2s_game_card_sell_respond,	// v246 id=150 size=3 [src-unplaced] noop
	r2s_game_card_buy_respond,	// v246 id=151 size=7 [src-unplaced] noop
	r2s_v246_unused_152,	// v246 id=152 size=2 [new] noop
	r2s_v246_unused_153,	// v246 id=153 size=7 [new] noop
	r2s_v246_unused_154,	// v246 id=154 size=11 [new] noop
	r2s_v246_unused_155,	// v246 id=155 size=19 [new] noop
	r2s_game_card_lookup_respond,	// v246 id=156 size=12 [src]
	r2s_v246_unused_157,	// v246 id=157 size=26 [new] noop
	r2s_v246_unused_158,	// v246 id=158 size=14 [new] noop
	r2s_v246_unused_159,	// v246 id=159 size=15 [new] noop
	r2s_update_mentor_record,	// v246 id=160 size=38 [src-unplaced] noop
	r2s_seek_mentor_yell,	// v246 id=161 size=34 [src-unplaced] noop
	r2s_seek_apprentice_yell,	// v246 id=162 size=6 [src-unplaced] noop
	r2s_v246_unused_163,	// v246 id=163 size=6 [new] noop
	r2s_v246_unused_164,	// v246 id=164 size=10 [new] noop
	r2s_v246_unused_165,	// v246 id=165 size=10 [new] noop
	r2s_v246_unused_166,	// v246 id=166 size=19 [new] noop
	r2s_v246_unused_167,	// v246 id=167 size=15 [new] noop
	r2s_v246_unused_168,	// v246 id=168 size=69 [new] noop
	r2s_v246_unused_169,	// v246 id=169 size=70 [new] noop
	r2s_v246_unused_170,	// v246 id=170 size=8 [new] noop
	r2s_v246_unused_171,	// v246 id=171 size=6 [new] noop
	r2s_v246_unused_172,	// v246 id=172 size=10 [new] noop
	r2s_v246_unused_173,	// v246 id=173 size=58 [new] noop
	r2s_v246_unused_174,	// v246 id=174 size=2 [new] noop
	r2s_v246_unused_175,	// v246 id=175 size=22 [new] noop
	r2s_v246_unused_176,	// v246 id=176 size=6 [new] noop
	r2s_v246_unused_177,	// v246 id=177 size=10 [new] noop
	r2s_sync_battle_field_list,	// orphan (2010, no v246 slot) id=178
	r2s_take_tong_repertory_item_to_pos_respond,	// orphan (2010, no v246 slot) id=179
	r2s_protocol_end
};



#pragma	pack(1)

// 上行协议结构定义

/*[drift 2.5.2] standalone (no bReserved): WORD@0, ver@2/6, +2 fields@10/14, time@18 = 22B (stock FUN_080d601c)*/
struct S2R_HANDSHAKE_REQUEST
{
    WORD    wProtocolID;
    int     nGameWorldLowerVersion;
    int     nGameWorldUpperVersion;
    int     nField10;
    int     nWorldIndex;
    time_t  nServerTime;
};

struct S2R_PING_SIGNAL : INTERNAL_PROTOCOL_HEADER
{
	DWORD dwReserved;
};

struct S2R_UPDATE_PERFORMANCE : INTERNAL_PROTOCOL_HEADER
{
    int     nIdleFrame;
    size_t  uMemory;
    int     nPlayerCount;
    int     nConnectionCount;
    size_t  uNetworkFlux;
    int     nGSResourceVer;  /*[R10 s2r] +4B: v246 grew this struct*/
};

struct S2R_CREATE_MAP_RESPOND : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwMapID;			
	int		    nMapCopyIndex;		
	BOOL		bResult;			
};

// GameServer对登录请求的应答消息
struct S2R_PLAYER_LOGIN_RESPOND : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwPlayerID;
	union
	{
		int			nGatewayPlayerIndex;
		DWORD		dwPacketIdentity;
	};
	BOOL		bPermit;
	GUID		Guid;
	DWORD       dwGSAddr;
	int			nGSPort;
};

struct S2R_CONFIRM_PLAYER_LOGIN_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwIP;
};

struct S2R_PLAYER_LEAVE_GS : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
};

struct S2R_SEARCH_MAP_REQUEST : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwPlayerID;
	DWORD		dwMapID;
    int         nMapCopyIndex;
    int         nPosX;
    int         nPosY;
    int         nPosZ;
    int     nField246;  /*[R10 s2r] +4B: v246 grew this struct*/
};

// 玩家跨服务器,源服务器发出跨服请求(附带角色基本数据)
struct S2R_TRANSFER_PLAYER_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwRoleID;
    BOOL            bChargeFlag;
	tagExtPointInfo	ExtPointInfo;			// 可用的附送点
    BOOL            bExtPointLock;
    int             nLastExtPointIndex;
    short           nLastExtPointValue;
    time_t          nEndTimeOfFee;
    int             nCoin;
    KRoleBaseInfo   RoleBaseInfo;
    int             nBattleFieldSide;
};

// 玩家跨服时,目标服务器的响应消息
struct S2R_TRANSFER_PLAYER_RESPOND : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwPlayerID;
	BOOL		bSucceed;
	DWORD	    dwAddress;
	WORD		wPort;
	GUID		Guid;
};

// ------------------ 组队相关协议结构定义 ---------------------------

// 从GS转发过来的玩家组队邀请
struct S2R_INVITE_PLAYER_JOIN_TEAM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
	DWORD dwInviteSrc;
	char  szInviteRole[_NAME_LEN];
};

// 被邀请玩家的应答消息GS==>Relay
struct S2R_INVITE_PLAYER_JOIN_TEAM_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    char  szInviteSrc[_NAME_LEN];
    DWORD dwInviteDst;
    BOOL  bAgreeFlag;
};

// 转发玩家的入队申请到Relay
struct S2R_APPLY_JOIN_TEAM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwApplySrc; // 发出入队申请的玩家
    char  szApplyDst[_NAME_LEN]; // 申请加入这个玩家的队伍
};

// 将队长对入队申请的应答转发到Relay
struct S2R_APPLY_JOIN_TEAM_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    char  szApplySrc[_NAME_LEN];
    DWORD dwApplyDst;
    BOOL  bAgreeFlag;
};

// 队伍解散请求
struct S2R_TEAM_DISBAND_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwLeaderID;
    DWORD dwTeamID;
};

// 队伍删除成员请求
struct S2R_TEAM_DEL_MEMBER_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    DWORD dwRequestSrc; // 请求发起者的ID
    DWORD dwMemberID;
};

// 设置队伍队长请求
struct S2R_TEAM_CHANGE_AUTHORITY_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    DWORD dwRequestSrc; // 请求发起者的ID
    BYTE  byType;
    DWORD dwTargetID;
};

// 设置队伍物品分配方式
struct S2R_TEAM_SET_LOOT_MODE_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    DWORD dwRequestSrc;
    int   nLootMode;
};

struct S2R_TEAM_SET_ROLL_QUALITY_REQUEST : INTERNAL_PROTOCOL_HEADER
{
	DWORD dwTeamID;
	DWORD dwRequestSrc;
	int   nRollQuality;
};

struct S2R_TEAM_SET_FORMATION_LEADER_REQUEST : INTERNAL_PROTOCOL_HEADER
{
	DWORD dwTeamID;
	DWORD dwRequestSrc;
	DWORD dwNewFormationLeader;
    int   nGroupIndex;
};

struct S2R_SYNC_TEAM_MEMBER_MAX_LMR : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;
    int   	nMaxLife; 
    int   	nMaxMana; 
    int     nMaxRage;
};

struct S2R_SYNC_TEAM_MEMBER_CURRENT_LMR : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;
    BYTE   	byLifePercent; 
    BYTE   	byManaPercent; 
    BYTE   	byRagePercent; 
};

struct S2R_SYNC_TEAM_MEMBER_MISC : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;

    // 头像相关数据
    KPORTRAIT_INFO PortraitInfo;
    
    DWORD   dwForceID;
    BYTE    byLevel;
    BYTE    byCamp;
	BYTE    byRoleType;
    bool    bDeathFlag;
};

struct S2R_SYNC_TEAM_MEMBER_POSITION : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;
    DWORD   dwMapID;
    int     nMapCopyIndex;
    int     nPosX;
    int     nPosY;
};

struct S2R_TEAM_SET_MARK_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwTeamID;
    DWORD   dwMemberID;
    int     nMarkType;
    DWORD   dwTargetID;
};

struct S2R_TEAM_LEVEL_UP_RAID_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    DWORD dwRequestSrc;
};

struct S2R_TEAM_CHANGE_MEMBER_GROUP_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    DWORD dwMemberID;
    int   nDstGroupIndex;
    DWORD dwDstMemberID;
    DWORD dwRequestSrc;
};

// -------------- 聊天相关协议结构定义 ------------------------

struct S2R_TALK_MESSAGE : INTERNAL_PROTOCOL_HEADER
{
    BYTE  byMsgType;
    DWORD dwTalkerID;
    char  szTalker[_NAME_LEN];
    DWORD dwReceiverID;
    char  szReceiver[_NAME_LEN];
    BYTE  byTalkData[0];
};

struct S2R_PLAYER_TALK_ERROR : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwReceiverID;
    int   nErrorCode;
};

// -------------- 好友相关协议结构定义 -------------------------

struct S2R_APPLY_FELLOWSHIP_DATA_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
};

struct S2R_UPDATE_FELLOWSHIP_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byData[0];
};

struct S2R_ADD_FELLOWSHIP_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    char    szAlliedPlayerName[_NAME_LEN];
    BYTE    byType;
};

struct S2R_GET_FELLOWSHIP_NAME_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byCount;
    DWORD   dwAlliedPlayerIDArray[0];
};

struct S2R_APPLY_FELLOWSHIP_PLAYER_LEVEL_AND_FORCEID : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
};

struct S2R_ADD_FELLOWSHIP_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwAlliedPlayerID;
    char    szAlliedPlayerName[_NAME_LEN];
    int     nType;
};

// -------------- 邮件系统相关　------------------------------->

struct S2R_SEND_MAIL_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    BYTE            byRequestID;
    char            szReceiverName[ROLE_NAME_LEN];
    BYTE            byData[0];                      // KMail
};

struct S2R_SEND_GLOBAL_MAIL_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    int             nLifeTime;
    BYTE            byData[0];                      // KMail
};

struct S2R_GET_MAILLIST_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwStartID;
};

struct S2R_QUERY_MAIL_CONTENT : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
    BYTE            byMailType;
};

struct S2R_ACQUIRE_MAIL_MONEY_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
};

struct S2R_ACQUIRE_MAIL_ITEM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
    BYTE            byIndex;
};

struct S2R_GIVE_MAIL_MONEY_TO_PLAYER_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
    BYTE            byResult;
};

struct S2R_GIVE_MAIL_ITEM_TO_PLAYER_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
    BYTE            byIndex;
    BYTE            byResult;
};

struct S2R_SET_MAIL_READ : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
};

struct S2R_DELETE_MAIL : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
};

struct S2R_RETURN_MAIL : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    DWORD           dwMailID;
};

// <------------- 邮件系统相关　--------------------------------

struct S2R_SYNC_MID_MAP_MARK : INTERNAL_PROTOCOL_HEADER
{
	DWORD   dwTeamID;
	DWORD   dwSrcPlayerID;
	int     nMapID;
	int     nX;
	int     nY;
	int     nType;
	char    szComment[MAX_MIDMAP_MARK_COMMENT_LEN];
};

// <--------------------- 副本相关 ---------------------------------------
struct S2R_SAVE_SCENE_PLAYER_LIST : INTERNAL_PROTOCOL_HEADER
{
	DWORD    dwMapID;
    int      nMapCopyIndex;
    int      nPlayerCount;
    DWORD    dwPlayerList[0];
};

struct S2R_SAVE_SCENE_DATA : INTERNAL_PROTOCOL_HEADER
{
	DWORD    dwMapID;
    int      nMapCopyIndex;
    size_t   uDataLen;
    BYTE     byData[0];
};

struct S2R_PLAYER_ENTER_SCENE_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD    dwPlayerID;
    DWORD    dwMapID;
    int      nMapCopyIndex;
};

struct S2R_LOAD_ROLE_DATA_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
};

struct S2R_LOAD_ACCOUNT_DATA_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
};

struct S2R_SYNC_ACCOUNT_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwRoleID;
    size_t  uOffset;
    BYTE    byData[0];
};

struct S2R_SAVE_ACCOUNT_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwRoleID;
    int     nUserValue;
    size_t  uAccountDataLen;
};

struct S2R_CHANGE_ROLE_LEVEL_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    BYTE  byLevel;
};

struct S2R_CHANGE_ROLE_FORCEID_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    BYTE  byForceID;
};

// ---------------------- 交通相关 ------------------------------------------
struct S2R_SYNC_ROAD_TRACK_INFO : INTERNAL_PROTOCOL_HEADER
{
    int     nNodeID;
    BYTE    byCamp;
    BYTE    byEnable;
};

struct S2R_SEND_GM_MESSAGE : INTERNAL_PROTOCOL_HEADER 
{
    char    szGmName[_NAME_LEN];
    char    szMessage[0];
};

struct S2R_SEND_GM_CHN : INTERNAL_PROTOCOL_HEADER 
{
    char    szAccount[_NAME_LEN];
    char    szRole[_NAME_LEN];
    char    szMessage[0];
};

struct S2R_SEND_GM_COMMAND_GS : INTERNAL_PROTOCOL_HEADER
{
    char    szRoleName[_NAME_LEN];
    char    szCommand[0];
};

struct S2R_SEND_GM_COMMAND_GC : INTERNAL_PROTOCOL_HEADER
{
    char    szCommand[0];
};

struct S2R_JOIN_BATTLE_FIELD_QUEUE_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD       dwMapID;
    int         nCopyIndex;
    DWORD       dwGroupID;
    BOOL        bTeamJoin;
    DWORD       dwRoleID;
    DWORD       dwJoinValue;
};

struct S2R_LEAVE_BATTLE_FIELD_QUEUE_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwMapID;
    DWORD dwRoleID;
};

struct S2R_ACCEPT_JOIN_BATTLE_FIELD : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwMapID;
    int     nCopyIndex;
    DWORD   dwRoleID;
};

struct S2R_GET_BATTLE_FIELD_LIST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwMapID;
};

struct S2R_LEAVE_BATTLE_FIELD_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
};

struct S2R_APPLY_CREATE_PQ_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD dwPQTemplateID;
};

struct S2R_APPLY_DELETE_PQ_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD dwPQID;
};

struct S2R_APPLY_CHANGE_PQ_VALUE_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPQID;
    int     nIndex;
    int     nAddValue;
};

struct S2R_ADD_CAMP_SCORE : INTERNAL_PROTOCOL_HEADER
{
    int     nAddCampScore;
};

struct S2R_APPLY_SET_CAMP_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD dwPlayerID;
    BYTE  byNewCamp;
};

// ---------------------- 帮会相关 ------------------------------------------
struct S2R_APPLY_TONG_ROSTER_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    int     nLastUpdateFrame;
    int     nMemberCount;
    DWORD   dwMemberList[0];
};

struct S2R_APPLY_TONG_INFO_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    int     nLastUpdateFrame;
};

struct S2R_APPLY_TONG_REPERTORY_PAGE_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byPageIndex;
    int     nLastUpdateCounter;
};

struct S2R_APPLY_CREATE_TONG_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    char  szTongName[_NAME_LEN];
    BYTE  byTemplateIndex;
};

struct S2R_INVITE_PLAYER_JOIN_TONG_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    char  szNewMemberName[_NAME_LEN];
};

struct S2R_INVITE_PLAYER_JOIN_TONG_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwInviterID;
    DWORD dwTongID;
    BYTE  byAccept;
};

struct S2R_APPLY_KICK_OUT_TONG_MEMBER : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwMemberID;
};

struct S2R_MODIFY_TONG_INFO_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byInfoType;
    WORD    wDataLen;
    BYTE    byData[0];
};

struct S2R_MODIFY_TONG_SCHEMA_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byGroupIndex;
    BYTE    byModifyType;
    BYTE    byData[0];
};

struct S2R_APPLY_QUIT_TONG_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
};

struct S2R_CHANGE_TONG_MEMBER_GROUP_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwTargetMemberID;
    BYTE  byTargetGroup;
};

struct S2R_CHANGE_TONG_MASTER_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwNewMasterID;
};

struct S2R_CHANGE_TONG_MEMBER_REMARK_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwTargetMemberID;
    char  szRemark[_NAME_LEN];
};

struct S2R_GET_TONG_DESCRIPTION_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    DWORD dwTongID;
};

struct S2R_SAVE_MONEY_IN_TONG_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    int     nMoney;
};

struct S2R_PAY_TONG_SALARY_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    int     nTotalSalary;
};

struct S2R_GET_TONG_SALARY_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
};

struct S2R_GET_TONG_SALARY_FAIL_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    int     nSilver;
};

struct S2R_CHANGE_TONG_CAMP_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    BYTE  byCamp;
};

struct S2R_APPLY_OPEN_TONG_REPERTORY_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
};

struct S2R_TAKE_TONG_REPERTORY_ITEM_TO_POS_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byPageIndex;
    BYTE    byRepertoryPos;
    BYTE    byBoxIndex;
    BYTE    byBoxPos;
    int     nLastUpdateCounter;
};

struct S2R_TAKE_TONG_REPERTORY_ITEM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byPageIndex;
    BYTE    byRepertoryPos;
    int     nLastUpdateCounter;
};

struct S2R_TAKE_TONG_REPERTORY_ITEM_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byRepertoryPos;
    bool    bSuccess;
};

struct S2R_PUT_TONG_REPERTORY_ITEM_TO_POS_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwServerItemID;
    BYTE    byBoxIndex;
    BYTE    byBoxPos;
    BYTE    byPageIndex;
    BYTE    byPagePos;
};

struct S2R_PUT_TONG_REPERTORY_ITEM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwServerItemID;
    BYTE    byBoxIndex;
    BYTE    byBoxPos;
    BYTE    byPageIndex;
};

struct S2R_PUT_TONG_REPERTORY_ITEM_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byPagePos;
    BYTE    byDataSize;
    BYTE    byData[0];
};

struct S2R_UNLOCK_TONG_REPERTORY_GRID : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byPagePos;
};

struct S2R_EXCHANGE_TONG_REPERTORY_ITEM_POS_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    bySrcPageIndex;
    BYTE    byDstPageIndex;
    BYTE    bySrcPagePos;
    BYTE    byDstPagePos;
    int     nSrcUpdateCounter;
    int     nDstUpdateCounter;
};

struct S2R_APPLY_STACK_TONG_REPERTORY_ITEM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byPagePos;
    BYTE    byBoxIndex;
    BYTE    byBoxPos;
    DWORD   dwItemID;
    BYTE    byTabType;
    WORD    wTabIndex;
    WORD    wStackNum;
    WORD    wMaxStackNum;
    int     nLastUpdateCounter;
};

struct S2R_STACK_TONG_REPERTORY_ITEM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byPagePos;
    BYTE    byTabType;
    WORD    wTabIndex;
    WORD    wStackNum;
    WORD    wMaxStackNum;
    int     nLastUpdateCounter;
};
// ---------------------- 寄卖行相关 ------------------------------------------
struct S2R_AUCTION_LOOKUP_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    BYTE                    byRequestID;
    DWORD                   dwPlayerID;
    KAUCTION_LOOKUP_PARAM   Param;
};

struct S2R_AUCTION_BID_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwBidderID;
    DWORD   dwSaleID;
    DWORD   dwCRC;
    int     nPrice;
};

struct S2R_AUCTION_SELL_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwSellerID;
    char    szSaleName[SALE_NAME_LEN];
    int     nAucGenre;
    int     nAucSub;
    int     nRequireLevel;
    int     nQuality;
    int     nStartPrice;
    int     nBuyItNowPrice;
    int     nCustodyCharges;
    int     nLeftTime;
    size_t  uItemDataLen;
    BYTE    byItemData[0];
};

struct S2R_AUCTION_CANCEL_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    DWORD   dwSaleID;
};

struct S2R_REMOTE_LUA_CALL :  INTERNAL_PROTOCOL_HEADER
{
    char   szFunction[_NAME_LEN];
    size_t uParamLen;
    BYTE   byParam[0];
};

struct S2R_ACHIEVEMENT_ANNOUNCE_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    int     nAchievementID;
};

struct S2R_CHANGE_EXT_POINT_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    WORD    wExtPointIndex;
    WORD    wChangeValue;
};

struct S2R_SET_CHARGE_FLAG_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    DWORD   dwLastLoginIP;
    int     nChargeFlag;
};

struct S2R_DESIGNATION_GLOBAL_ANNOUNCE_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    int   nPrefix;
    int   nPostfix;
};

struct S2R_SYNC_ROLE_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwRoleID;
    size_t  uOffset;
    BYTE    byData[0];
};

struct S2R_SAVE_ROLE_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwRoleID;
    int             nUserValue;
    KRoleBaseInfo   BaseInfo;
    size_t          uRoleDataLen;
};

struct S2R_UPDATE_STAT_DATA_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    int               	nCount;
    KSTAT_DATA_MODIFY 	Values[0];
};

struct S2R_QUERY_STAT_ID_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    char            	szName[STAT_DATA_NAME_LEN];
};

struct S2R_MAP_COPY_KEEP_PLAYER : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwMapID;
    int   nCopyIndex;
    DWORD dwPlayerID;
};

struct S2R_ACTIVE_PRESENT_CODE_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    char    szPresentCode[_NAME_LEN];
    DWORD   dwActiveIP;
};

struct S2R_APEX_PROTOCOL : INTERNAL_PROTOCOL_HEADER
{
    BYTE   byIsUserData;
    DWORD  dwRoleID;
    size_t nSize;
    BYTE   byData[0];
};

struct S2R_REPORT_FARMER_PLAYER_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    time_t  nPunishTime;
};

struct S2R_GAME_CARD_SELL_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    int     nCoin;
    int     nGameTime;
    BYTE    byType;
    int     nPrice;
    int     nDurationTime;
};

struct S2R_GAME_CARD_BUY_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwBuyerID;
    DWORD   dwID;
    int     nBuyPrice;
};

struct S2R_GAME_CARD_LOOKUP_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCardType;
    int     nStartIndex;
    BYTE    byOrderType;
    BOOL    bDesc;
};

struct S2R_GAME_CARD_CANCEL_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    DWORD   dwID;
};

// ---------------------- 师徒相关 --------------------------------------------
struct S2R_APPLY_MENTOR_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwMentorID;
    DWORD dwApprenticeID;
};

struct S2R_UPDATE_MAX_APPRENTICE_NUM : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwMentorID;
    int     nMaxApprenticeNum;
};

struct S2R_ADD_MENTOR_VALUE_REQUEST : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwMentorID;
    DWORD   dwApprenticeID;
    int     nDeltaVlue;
};
// --------------------------------------------------------------------------

// ---------------------- 下行协议结构定义 ---------------------------------------

/*[cce58b6 GLM] standalone packed: WORD@0, worldIndex@2, baseTime@6, startFrame@10, fastBoot@14 = 18B (stock FUN_080d3872)*/
struct R2S_HANDSHAKE_RESPOND
{
    WORD    wProtocolID;
    int     nWorldIndex;
    time_t  nBaseTime;
    int     nStartFrame;
    int     nFastBootMode;
};

struct R2S_QUIT_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    int nNothing;
};

// 地图管理相关协议结构
struct R2S_CREATE_MAP_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwMapID;		
	int		    nMapCopyIndex;
    size_t      uDataLen;
    BYTE        byData[0];
};

struct R2S_FINISH_CREATE_MAP_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
};

struct R2S_DELETE_MAP_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwMapID;				
	int		    nMapCopyIndex;
};

// 玩家登录请求
struct R2S_PLAYER_LOGIN_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwRoleID;
    union
    {
        int             nGatewayPlayerIndex;
        DWORD           dwPacketIdentity;
    };
    char            szRoleName[_NAME_LEN];
    char            szAccount[_NAME_LEN];
    tagExtPointInfo ExtPointInfo;
    DWORD           nEndTimeOfFee;
    DWORD           nCoin;
    int             nLastLoginTime;
    BYTE            nChargeFlag;
    BYTE            byMibaoMode;
    BYTE            byFreeIP;
    BYTE            byReserved;
    DWORD           dwSystemTeamID;
    DWORD           dwTeamID;
    KRoleBaseInfo   BaseInfo;
    int             nBattleSide;
    DWORD           dwTongID;
    BYTE            byFarmerLimit;
    BYTE            byAccountMaxLevel;
};

/* Target v2.5.2 wire contract: packed, no inherited-header tail padding. */
struct __attribute__((packed)) S2R_SYNC_NEW_EXT_POINT_REQUEST
{
    WORD    wProtocolID;
    DWORD   dwPlayerID;
    int     nGatewayPlayerIndex;
    int     nBoundKey;
};

struct __attribute__((packed)) S2R_CHANGE_NEW_EXT_POINT_REQUEST
{
    WORD  wProtocolID;
    DWORD dwPlayerID;
    int   nKey;
    int   nChangeValue;
    int   nOldValue;
    int   nChangeMethod;
};

struct __attribute__((packed)) SYNC_NEP_INFO
{
    int     nKey;
    int     nValue;
};

struct __attribute__((packed)) R2S_SYNC_NEW_EXT_POINT_RESPOND
{
    WORD            wProtocolID;
    int             nCenterIndex;
    DWORD           dwPlayerID;
    int             nGatewayIndex;
    BYTE            bySyncFinish;
    int             nCount;
    SYNC_NEP_INFO   SyncNEPInfo[0];
};

struct __attribute__((packed)) KSyncGSNEPInfo
{
    int  nKey;
    int  nValue;
    BYTE byLocked;
};

struct __attribute__((packed)) R2S_SYNC_GS_NEW_EXT_POINT
{
    WORD              wProtocolID;
    DWORD             dwPlayerID;
    int               nRespondCenterIndex;
    BYTE              bySyncFinish;
    int               nCount;
    KSyncGSNEPInfo    SyncGSNEPInfo[0];
};

struct __attribute__((packed)) R2S_APPLY_GS_NEW_EXT_POINT
{
    WORD  wProtocolID;
    DWORD dwPlayerID;
    int   nRespondCenterIndex;
    int   nBoundKey;
};

struct __attribute__((packed)) R2S_CHANGE_NEW_EXT_POINT_RESPOND
{
    WORD  wProtocolID;
    DWORD dwPlayerID;
    int   nKey;
    int   nOldValue;
    int   nChangeValue;
    int   nCurrentValue;
    int   nActionCode;
};

struct R2S_CONFIRM_PLAYER_LOGIN_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    BOOL  bPermit;
};

struct R2S_SEARCH_MAP_RESPOND : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwPlayerID;
	int         nResultCode;
    BOOL        bSwitchServer;
	DWORD		dwMapID;
	int		    nMapCopyIndex;
    int         nPosX;
    int         nPosY;
    int         nPosZ;
    int         nBattleFieldSide;
};

// 玩家跨服务器,GameCenter转发源服务器发出的跨服请求(附带角色基本数据)
struct R2S_TRANSFER_PLAYER_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwRoleID;
    char            szRoleName[_NAME_LEN];
    char            szAccount[_NAME_LEN];
    BOOL            bChargeFlag;
	tagExtPointInfo	ExtPointInfo;
    BOOL            bExtPointLock;
    int             nLastExtPointIndex;
    short           nLastExtPointValue;
    time_t          nEndTimeOfFee;
    int             nCoin;
    DWORD           dwSystemTeamID;
    DWORD           dwTeamID;
    KRoleBaseInfo   BaseInfo;
    int             nBattleFieldSide;
    DWORD           dwTongID;
    BYTE            byFarmerLimit;
};

// 玩家跨服时,GameCenter转发目标服务器的响应消息
struct R2S_TRANSFER_PLAYER_RESPOND : INTERNAL_PROTOCOL_HEADER
{
	DWORD		dwPlayerID;
	BOOL		bSucceed;
	DWORD	    dwAddress;
	WORD		wPort;
	GUID		Guid;
};

struct R2S_KICK_ACCOUNT_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
	DWORD       dwPlayerID;
};

// ------------------ 组队相关协议结构定义 ---------------------------

// Relay将组队邀请转发到被邀请者所在的服务器
struct R2S_INVITE_PLAYER_JOIN_TEAM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwInviteSrc;
	char  szInviteSrc[_NAME_LEN];
	DWORD dwInviteDst;
    BYTE  bySrcCamp;
};

// Relay对入队申请稍加处理后，转发到目标玩家所在的服务器
struct R2S_APPLY_JOIN_TEAM_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwApplyDst;
    DWORD dwApplySrc;
    char  szApplySrc[_NAME_LEN];
    BYTE  bySrcLevel;
    BYTE  bySrcCamp;
    BYTE  bySrcForceID;
    DWORD dwSrcTongID;
    DWORD dwMapID;
};

// 队伍解散消息广播
struct R2S_TEAM_DISBAND_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
};

// 队员上下线通知
struct R2S_SYNC_TEAM_MEMBER_ONLINE_FLAG : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    DWORD dwMemberID;
    BOOL  bOnlineFlag;
};

struct R2S_PARTY_MESSAGE_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    int   nCode;
    BYTE  byData[0];
};

struct R2S_TEAM_SET_FORMATION_LEADER_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    int   nGroupIndex;
    DWORD dwNewFormationLeader;
};

struct R2S_TEAM_SET_MARK_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    int   nMarkType;
    DWORD dwTarget;
};

struct R2S_TEAM_CAMP_CHANGE : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    BYTE  byCamp;
};

struct R2S_TEAM_LEVEL_UP_RAID_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    int   nGroupNum;
};

struct R2S_TEAM_CHANGE_MEMBER_GROUP_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    DWORD dwMemberID;
    int   nOldGroupIndex;
    DWORD dwDstMemberID;
    int   nNewGroupIndex;
};

struct R2S_TEAM_CREATE_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    BOOL  bSystem;
    BYTE  byCamp;
    int   nLootMode;
    int   nRollQuality;
    int   nGroupNum;
    DWORD dwAuthority[tatTotal];
    DWORD dwTeamMark[PARTY_MARK_COUNT];
    BYTE  byMemberCount;
    struct KMEMBER
    {
        int     nGroupIndex;
        DWORD   dwMemberID;
        char    szMemberName[_NAME_LEN];
        BYTE    byOnLineFlag;
        BYTE    byLevel;
    } Member[0];
};

// 队伍添加成员消息广播
struct R2S_TEAM_ADD_MEMBER_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    int   nGroupIndex;
    DWORD dwMemberID;
    char  szMemberName[_NAME_LEN];
};

// 队伍删除成员消息广播
struct R2S_TEAM_DEL_MEMBER_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    int   nGroupIndex;
    DWORD dwMemberID; 
};

// 队伍队长变更通知
struct R2S_TEAM_CHANGE_AUTHORITY_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    BYTE  byType;
    DWORD dwNewID;
};

// 物品分配模式变更通知
struct R2S_TEAM_SET_LOOT_MODE_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTeamID;
    int   nLootMode;
	int   nRollQuality;
};

struct R2S_SYNC_TEAM_MEMBER_MAX_LMR : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;
    int   	nMaxLife; 
    int   	nMaxMana; 
    int     nMaxRage;
};

struct R2S_SYNC_TEAM_MEMBER_CURRENT_LMR : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;
    BYTE   	byLifePercent; 
    BYTE   	byManaPercent; 
    BYTE    byRagePercent;
};

struct R2S_SYNC_TEAM_MEMBER_MISC : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;
    
    // 头像相关数据
    KPORTRAIT_INFO PortraitInfo;

    DWORD   dwForceID;
    BYTE    byCamp;
    BYTE    byLevel;
	BYTE    byRoleType;
    bool    bDeathFlag;
};

struct R2S_SYNC_TEAM_MEMBER_POSITION : INTERNAL_PROTOCOL_HEADER
{
    DWORD 	dwTeamID;
    DWORD 	dwMemberID;
    DWORD   dwMapID;
    int     nMapCopyIndex;
    int     nPosX;
    int     nPosY;
};

// -------------- 聊天相关协议结构定义 ------------------------
struct R2S_PLAYER_TALK_ERROR : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwReceiverID;
    int   nErrorCode;
};

struct R2S_PLAYER_TALK_WHISPER_SUCCESS_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwTalkerID;
    DWORD dwReceiverID;
};

struct R2S_TALK_MESSAGE : INTERNAL_PROTOCOL_HEADER
{
    BYTE  byMsgType;
    DWORD dwTalkerID;
    char  szTalker[_NAME_LEN];
    DWORD dwReceiverID;
    char  szReceiver[_NAME_LEN];
    BYTE  byTalkData[0];
};

// -------------- 好友相关协议结构定义 -------------------------

struct R2S_APPLY_FELLOWSHIP_DATA_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byData[0];
};

struct R2S_ADD_FELLOWSHIP_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwAlliedPlayerID;
    char    szAlliedPlayerName[_NAME_LEN];
    BYTE    byType; // 1: Friend, 2: Foe, 3: BlackList
};

struct R2S_GET_FELLOWSHIP_NAME_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byCount;
    struct _AlliedPlayer 
    {
        DWORD   dwID;
        DWORD   dwMapID;
        char    szName[_NAME_LEN];
    } AlliedPlayerInfo[0];
};

struct R2S_SYNC_FELLOWSHIP_MAPID : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwMapID;
};

struct R2S_SYNC_FELLOWSHIP_PLAYER_LEVEL : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byLevel;
    BYTE    bNotifyClient;
};

struct R2S_SYNC_FELLOWSHIP_PLAYER_FORCEID : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byForceID;
};

struct R2S_ADD_FELLOWSHIP_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwAlliedPlayerID;
    char    szAlliedPlayerName[_NAME_LEN];
    int     nType;
};

// -------------- 邮件系统相关　------------------------------->
struct R2S_SYNC_MAIL_COUNT_INFO : INTERNAL_PROTOCOL_HEADER
{
    DWORD               dwPlayerID;
    int                 nReadCount;
    int                 nTotalCount;
};

struct R2S_SEND_MAIL_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    BYTE                byRespondID;
    DWORD               dwSrcID;
    BYTE                byResult;
    char                szReceiver[ROLE_NAME_LEN];
    BYTE                byData[0];        // KMail
};

struct R2S_GET_MAILLIST_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD               dwPlayerID;
    int                 nMailCount;
    KMailListInfo       MailList[0];
};

struct R2S_SYNC_MAIL_CONTENT : INTERNAL_PROTOCOL_HEADER
{
    DWORD               dwPlayerID;
    DWORD               dwMailID;
    BYTE                byResult;
    BYTE                byData[0];
};

struct R2S_GIVE_MAIL_MONEY_TO_PLAYER : INTERNAL_PROTOCOL_HEADER
{
    DWORD               dwPlayerID;
    DWORD               dwMailID;
    BYTE                byMailType;
    int                 nMoney;
    char                szSender[ROLE_NAME_LEN];
};

struct R2S_GIVE_MAIL_ITEM_TO_PLAYER : INTERNAL_PROTOCOL_HEADER
{
    DWORD               dwPlayerID;
    DWORD               dwMailID;
    BYTE                byMailType;
    BYTE                byResult;
    BYTE                byItemIndex;
    int                 nItemPrice;
    BYTE                byItemLen;
    char                szSender[ROLE_NAME_LEN];
    BYTE                byData[0];
};

struct R2S_NEW_MAIL_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD               dwPlayerID;
    KMailListInfo       NewMailListInfo;
};

struct R2S_MAIL_GENERAL_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD               dwPlayerID;
    DWORD               dwMailID;
    BYTE                byRespond;
};

// <------------- 邮件系统相关　--------------------------------

struct R2S_SYNC_MID_MAP_MARK : INTERNAL_PROTOCOL_HEADER
{
	DWORD dwTeamID;
	DWORD dwSrcPlayerID;
	int nMapID;
	int nX;
	int nY;
	int nType;
	char szComment[MAX_MIDMAP_MARK_COMMENT_LEN];
};

// <--------------------- 副本相关 ---------------------------------------
struct R2S_MAP_COPY_DO_CLEAR_PLAYER_PREPARE : INTERNAL_PROTOCOL_HEADER
{
    DWORD    dwMapID;
    int      nMapCopyIndex;
    int      nType;
    int      nDelayTime;
};

struct R2S_MAP_COPY_DO_CLEAR_PLAYER : INTERNAL_PROTOCOL_HEADER
{
    DWORD    dwMapID;
    int      nMapCopyIndex;
};

struct R2S_SET_MAP_COPY_OWNER : INTERNAL_PROTOCOL_HEADER
{
    DWORD    dwMapID;
    int      nMapCopyIndex;
    DWORD    dwOwnerID;
};

// ---------------------- 交通相关 ------------------------------------

struct R2S_SYNC_ROAD_TRACK_INFO : INTERNAL_PROTOCOL_HEADER
{
    int     nNodeID;
    BYTE    byCamp;
    BYTE    byEnable;
};

// --------------------- 角色操作相关 -----------------------------

struct R2S_SAVE_ROLE_DATA_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    BOOL  bSucceed;
    int   nUserValue;
};

struct R2S_SAVE_ACCOUNT_DATA_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BOOL    bSucceed;
    int     nUserValue;
};

struct R2S_GM_COMMAND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    char    szGmName[_NAME_LEN];
    char	szGmCommand[0];
};

// --------------------- 战场排队相关 -------------------------------

struct R2S_JOIN_BATTLE_FIELD_QUEUE_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwMapID;
    int     nErrorCode;
};

struct R2S_LEAVE_BATTLE_FIELD_QUEUE_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwMapID;
    int     nErrorCode;
};

struct R2S_BATTLE_FIELD_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD                       dwPlayerID;
    time_t                      nAvgQueueTime;
    int                         nPassTime;
    DWORD                       dwMapID;
    int                         nCopyIndex;
    BATTLE_FIELD_NOTIFY_TYPE    eType;
};

struct R2S_SYNC_BATTLE_FIELD_LIST : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwMapID;
    int     nCopyIndex;
};

struct R2S_ACCEPT_JOIN_BATTLE_FIELD_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwEnterMapID;
    int     nEnterCopyIndex;
    int     nEnterX;
    int     nEnterY;
    int     nEnterZ;
    DWORD   dwLeaveMapID;
    int     nLeaveCopyIndex;
    int     nLeaveX;
    int     nLeaveY;
    int     nLeaveZ;
};

struct R2S_LEAVE_BATTLE_FIELD_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
};

// ---------------------   帮会相关   -------------------------------

struct R2S_SYNC_TONG_MEMBER_INFO : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    KTongMemberInfo MemberInfo;
};

struct R2S_DELETE_TONG_MEMBER_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwMemberID;
};

struct R2S_APPLY_TONG_INFO_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byInfoType;
    BYTE    byData[0];
};

struct R2S_UPDATE_TONG_CLIENT_DATA_VERSION : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byFrameType;
    int     nUpdateFrame;
};

struct R2S_INVITE_PLAYER_JOIN_TONG_REQUEST: INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwInviterID;
    DWORD   dwTongID;
    char    szInviterName[_NAME_LEN];
    char    szTongName[_NAME_LEN];
};

struct R2S_CHANGE_PLAYER_TONG_NOTIFY : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byReason;
    char    szTongName[_NAME_LEN];
};

struct R2S_TONG_BROADCAST_MESSAGE : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byCode;
    BYTE    byData[0];
};

struct R2S_GET_TONG_DESCRIPTION_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    char    szTongName[_NAME_LEN];
};

struct R2S_SYNC_TONG_ONLINE_MESSAGE : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    BYTE  byDataLen;
    char  szOnlineMessage[0];
};

struct R2S_SYNC_TONG_REPERTORY_PAGE_RESPOND: INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byPageIndex;
    int     nServerCounter;
    BYTE    byGetRight;
    BYTE    byPutRight;
    BYTE    byUpdateItemCount;
    BYTE    byInvailedItemCount;
    BYTE    byData[0];
};

struct R2S_TAKE_TONG_REPERTORY_ITEM_TO_POS_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byRepertoryPos;
    BYTE    byBoxIndex;
    BYTE    byBoxPos;
    BYTE    bySize;
    BYTE    byData[0];
};

struct R2S_TAKE_TONG_REPERTORY_ITEM_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byRepertoryPos;
    BYTE    bySize;
    BYTE    byData[0];
};

struct R2S_PUT_TONG_REPERTORY_ITEM_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    DWORD   dwServerItemID;  // server上的道具ID 用来在异步过程中验证是否是同一道具
    BYTE    byBoxIndex;
    BYTE    byBoxPos;
    BYTE    byPageIndex;
    BYTE    byPagePos;
};

struct R2S_APPLY_STACK_TONG_REPERTORY_ITEM_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    BYTE    byPageIndex;
    BYTE    byPagePos;
    BYTE    byBoxIndex;
    BYTE    byBoxPos;
    DWORD   dwItemID;
    WORD    wStackNum;
};

struct R2S_APPLY_OPEN_TONG_REPERTPRY_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    char    szTongName[_NAME_LEN];
    BYTE    byRepertoryPageNum;
};

struct R2S_GET_TONG_SALARY_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    DWORD   dwTongID;
    int     nSilver;
};

struct R2S_SYNC_TONG_HISTORY_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPlayerID;
    BYTE    byType;
    DWORD   dwStartIndex;
    BYTE    byCount;
    BYTE    byData[0];
};

// ---------------------   寄卖行相关   -------------------------------

struct R2S_AUCTION_LOOKUP_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    BYTE                byRespondID;
    DWORD               dwPlayerID;
    BYTE                byCode;
    KAUCTION_PAGE_HEAD  Page;
};

struct R2S_AUCTION_BID_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCode;
    int     nBidPrice;
};

struct R2S_AUCTION_SELL_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCode;
    int     nStartPrice;
    int     nBuyItNowPrice;
    int     nCustodyCharges;
    size_t  uItemDataLen;
    BYTE    byItemData[0];
};

struct R2S_AUCTION_CANCEL_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCode;
};

struct R2S_AUCTION_MESSAGE_NOTIFY : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCode;
    char    szSaleName[SALE_NAME_LEN];
    int     nPrice;
};

// ------------------------------------------------------------------
struct R2S_SYNC_PQ : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPQID;
    DWORD   dwPQTemplateID;
    time_t  nBeginTime;
    int     nPQValue[PQ_PARAM_SYNC_COUNT];
    BYTE    bFinished;
    BYTE    byOnwerType;
    DWORD   dwOnwerID1;
    DWORD   dwOnwerID2;
    DWORD   dwOnwerID3;
};

struct R2S_DELETE_PQ : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPQID;
};

struct R2S_PQ_VALUE_CHANGE : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPQID;
    int     nIndex;
    int     nValue;
};

struct R2S_PQ_FINISH : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwPQID;
};

struct R2S_SYNC_CAMP_INFO : INTERNAL_PROTOCOL_HEADER
{
    int     nCampLevel;
    int     nCampScore;
    int     nActivePlayerCount[cTotal];
};

struct R2S_APPLY_SET_CAMP_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwPlayerID;
    BYTE  byCamp;
    BYTE  byResult;
};

struct R2S_REMOTE_LUA_CALL :  INTERNAL_PROTOCOL_HEADER
{
    char   szFunction[_NAME_LEN];
    size_t uParamLen;
    BYTE   byParam[0];
};

struct R2S_ACHIEVEMENT_GLOBAL_ANNOUNCE_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    char    szRoleName[ROLE_NAME_LEN];
    int     nAchievementID;
};

struct R2S_DESIGNATION_GLOBAL_ANNOUNCE_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    char    szRoleName[ROLE_NAME_LEN];
    int     nPrefix;
    int     nPostfix;
};

struct R2S_SYNC_GLOBAL_SYSTEM_VALUE : INTERNAL_PROTOCOL_HEADER 
{
    int     nGameWorldStartTime;
    int     nMaxPlayerLevel;
    int     nGameWorldChargeTime;
};

struct R2S_SYNC_ROLE_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwRoleID;
    size_t  uOffset;
    BYTE    byData[0];
};

struct R2S_SYNC_ACCOUNT_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwRoleID;
    size_t  uOffset;
    BYTE    byData[0];
};

struct R2S_LOAD_ROLE_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwRoleID;
    BOOL    bSucceed;
    size_t  uRoleDataLen;
    BYTE    byRoleData[0];
};

struct R2S_LOAD_ACCOUNT_DATA : INTERNAL_PROTOCOL_HEADER
{
    DWORD   dwRoleID;
    BOOL    bSucceed;
    size_t  uAccountDataLen;
    BYTE    byAccountData[0];
};

struct R2S_QUERY_STAT_ID_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    char    szName[STAT_DATA_NAME_LEN];
    int     nID;
};

struct R2S_CHANGE_EXT_POINT_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BOOL    bSucceed;
};

struct R2S_SET_CHARGE_FLAG_RESPOND : INTERNAL_PROTOCOL_HEADER
{
    DWORD           dwPlayerID;
    int             nChargeFlag;
    tagExtPointInfo ExtPointInfo;
    DWORD           dwEndTimeOfFee;
};

struct R2S_SYNC_ZONE_CHARGE_FLAG : INTERNAL_PROTOCOL_HEADER 
{
    int     nZoneChargeFlag;
};

struct R2S_ACTIVE_PRESENT_CODE_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BOOL    bSucceed;
    char    szPresentCode[_NAME_LEN];
    DWORD   dwPresentType;
};

struct R2S_APEX_PROTOCOL : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwRoleID;
    size_t  nSize;
    BYTE    byData[0];
};

struct R2S_SET_FARMER_LIMIT_FLAG_REQUEST : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwRoleID;
    BOOL  bLimit;
};

struct R2S_GAME_CARD_SELL_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCode;
    int     nCoin;
};

struct R2S_GAME_CARD_BUY_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCode;
    int     nBuyPrice;
};

struct R2S_GAME_CARD_LOOKUP_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD       dwPlayerID;
    int         nTotalCount;
    int         nCount;
    KGAME_CARD  CardList[0];
};

struct R2S_GAME_CARD_CANCEL_RESPOND : INTERNAL_PROTOCOL_HEADER 
{
    DWORD   dwPlayerID;
    BYTE    byCode;
};

struct R2S_SYNC_MENTOR_DATA : INTERNAL_PROTOCOL_HEADER 
{
    int             nRecordCount;
    KMentorSyncInfo Data[0];
};

struct R2S_DELETE_MENTOR_RECORD : INTERNAL_PROTOCOL_HEADER 
{
    uint64_t uMKey;
};

struct R2S_UPDATE_MENTOR_RECORD : INTERNAL_PROTOCOL_HEADER
{
    DWORD dwMentor;
    DWORD dwApprentice;
    int   nValue;
    BYTE  byState;
};

struct R2S_SEEK_MENTOR_YELL : INTERNAL_PROTOCOL_HEADER
{
    char szRoleName[_NAME_LEN];
    char szTongName[_NAME_LEN];
    BYTE byForceID;
    BYTE byRoleLevel;
    BYTE byRoleType;
};

struct R2S_SEEK_APPRENTICE_YELL : INTERNAL_PROTOCOL_HEADER
{
    char szRoleName[_NAME_LEN];
    char szTongName[_NAME_LEN];
    BYTE byForceID;
    BYTE byRoleLevel;
    BYTE byRoleType;
};
#pragma pack()

#endif	//_RELAY_GS_PROTOCOL_H
