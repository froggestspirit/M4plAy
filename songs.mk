STD_REVERB = 50

$(MID_BUILDDIR)/%.o: $(MID_SUBDIR)/%.s
	$(ASM_PSEUDO_OP_CONV) $< | $(AS) $(ASFLAGS) -I sound -o $@

$(MID_SUBDIR)/mus_aqua_magma_hideout.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G076 -V084

$(MID_SUBDIR)/mus_encounter_aqua.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G065 -V086

$(MID_SUBDIR)/mus_route111.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G055 -V076

$(MID_SUBDIR)/mus_encounter_suspicious.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G069 -V078

$(MID_SUBDIR)/mus_b_arena.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G104 -V090

$(MID_SUBDIR)/mus_b_dome.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G111 -V090

$(MID_SUBDIR)/mus_b_dome_lobby.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G111 -V056

$(MID_SUBDIR)/mus_b_factory.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G113 -V100

$(MID_SUBDIR)/mus_b_frontier.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G103 -V094

$(MID_SUBDIR)/mus_b_palace.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G108 -V105

$(MID_SUBDIR)/mus_b_tower_rs.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G035 -V080

$(MID_SUBDIR)/mus_b_pike.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G112 -V092

$(MID_SUBDIR)/mus_vs_trainer.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G119 -V080 -P1

$(MID_SUBDIR)/mus_vs_wild.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G117 -V080 -P1

$(MID_SUBDIR)/mus_vs_aqua_magma_leader.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G126 -V080 -P1

$(MID_SUBDIR)/mus_vs_aqua_magma.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G118 -V080 -P1

$(MID_SUBDIR)/mus_vs_gym_leader.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G120 -V080 -P1

$(MID_SUBDIR)/mus_vs_champion.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G121 -V080 -P1

$(MID_SUBDIR)/mus_vs_kyogre_groudon.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G123 -V080 -P1

$(MID_SUBDIR)/mus_vs_rival.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G124 -V080 -P1

$(MID_SUBDIR)/mus_vs_regi.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G122 -V080 -P1

$(MID_SUBDIR)/mus_vs_elite_four.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G125 -V080 -P1

$(MID_SUBDIR)/mus_roulette.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G038 -V080

$(MID_SUBDIR)/mus_lilycove_museum.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G020 -V080

$(MID_SUBDIR)/mus_encounter_brendan.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G067 -V078

$(MID_SUBDIR)/mus_encounter_male.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G028 -V080

$(MID_SUBDIR)/mus_victory_road.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G075 -V076

$(MID_SUBDIR)/mus_game_corner.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G072 -V072

$(MID_SUBDIR)/mus_contest_winner.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G085 -V100

$(MID_SUBDIR)/mus_contest_results.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G092 -V080

$(MID_SUBDIR)/mus_contest_lobby.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G098 -V060

$(MID_SUBDIR)/mus_contest.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G086 -V088

$(MID_SUBDIR)/mus_cycling.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G049 -V083

$(MID_SUBDIR)/mus_encounter_champion.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G100 -V076

$(MID_SUBDIR)/mus_petalburg_woods.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G018 -V080

$(MID_SUBDIR)/mus_abandoned_ship.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G030 -V080

$(MID_SUBDIR)/mus_cave_of_origin.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G037 -V080

$(MID_SUBDIR)/mus_underwater.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G057 -V094

$(MID_SUBDIR)/mus_intro.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G060 -V090

$(MID_SUBDIR)/mus_hall_of_fame.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G082 -V078

$(MID_SUBDIR)/mus_route110.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G010 -V080

$(MID_SUBDIR)/mus_route120.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G014 -V080

$(MID_SUBDIR)/mus_route122.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G021 -V080

$(MID_SUBDIR)/mus_route101.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G011 -V080

$(MID_SUBDIR)/mus_dummy.s: %.s: %.mid
	$(MID) $< $@ -E -R40

$(MID_SUBDIR)/mus_hall_of_fame_room.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G093 -V080

$(MID_SUBDIR)/mus_end.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G102 -V036

$(MID_SUBDIR)/mus_help.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G056 -V078

$(MID_SUBDIR)/mus_level_up.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_obtain_item.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_evolved.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_gsc_route38.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -V080

$(MID_SUBDIR)/mus_slateport.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G079 -V070

$(MID_SUBDIR)/mus_poke_mart.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G050 -V085

$(MID_SUBDIR)/mus_oceanic_museum.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G023 -V080

$(MID_SUBDIR)/mus_gym.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G013 -V080

$(MID_SUBDIR)/mus_encounter_may.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G061 -V078

$(MID_SUBDIR)/mus_encounter_female.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G053 -V072

$(MID_SUBDIR)/mus_verdanturf.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G044 -V090

$(MID_SUBDIR)/mus_rustboro.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G045 -V085

$(MID_SUBDIR)/mus_route119.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G048 -V096

$(MID_SUBDIR)/mus_encounter_intense.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G062 -V078

$(MID_SUBDIR)/mus_weather_groudon.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G090 -V050

$(MID_SUBDIR)/mus_dewford.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G073 -V078

$(MID_SUBDIR)/mus_encounter_twins.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G095 -V075

$(MID_SUBDIR)/mus_encounter_interviewer.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G099 -V062

$(MID_SUBDIR)/mus_victory_trainer.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G058 -V091

$(MID_SUBDIR)/mus_victory_wild.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G025 -V080

$(MID_SUBDIR)/mus_victory_gym_leader.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G024 -V080

$(MID_SUBDIR)/mus_victory_aqua_magma.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G070 -V088

$(MID_SUBDIR)/mus_victory_league.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G029 -V080

$(MID_SUBDIR)/mus_caught.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G025 -V080

$(MID_SUBDIR)/mus_encounter_cool.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G063 -V086

$(MID_SUBDIR)/mus_trick_house.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G094 -V070

$(MID_SUBDIR)/mus_route113.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G064 -V084

$(MID_SUBDIR)/mus_sailing.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G077 -V086

$(MID_SUBDIR)/mus_mt_pyre.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G078 -V088

$(MID_SUBDIR)/mus_sealed_chamber.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G084 -V100

$(MID_SUBDIR)/mus_petalburg.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G015 -V080

$(MID_SUBDIR)/mus_fortree.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G032 -V080

$(MID_SUBDIR)/mus_oldale.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G019 -V080

$(MID_SUBDIR)/mus_mt_pyre_exterior.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G080 -V080

$(MID_SUBDIR)/mus_heal.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_slots_jackpot.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_slots_win.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_obtain_badge.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_obtain_berry.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_obtain_b_points.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G103 -V090 -P5

$(MID_SUBDIR)/mus_rg_photo.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G180 -V100 -P5

$(MID_SUBDIR)/mus_evolution_intro.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G026 -V080

$(MID_SUBDIR)/mus_obtain_symbol.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G103 -V100 -P5

$(MID_SUBDIR)/mus_awaken_legend.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_register_match_call.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G105 -V090 -P5

$(MID_SUBDIR)/mus_move_deleted.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_obtain_tmhm.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_too_bad.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G012 -V090 -P5

$(MID_SUBDIR)/mus_encounter_magma.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G087 -V072

$(MID_SUBDIR)/mus_lilycove.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G054 -V085

$(MID_SUBDIR)/mus_littleroot.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G051 -V100

$(MID_SUBDIR)/mus_surf.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G017 -V080

$(MID_SUBDIR)/mus_route104.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G047 -V097

$(MID_SUBDIR)/mus_gsc_pewter.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -V080

$(MID_SUBDIR)/mus_birch_lab.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G033 -V080

$(MID_SUBDIR)/mus_abnormal_weather.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G089 -V080

$(MID_SUBDIR)/mus_school.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G081 -V100

$(MID_SUBDIR)/mus_c_comm_center.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -V080

$(MID_SUBDIR)/mus_poke_center.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G046 -V092

$(MID_SUBDIR)/mus_b_pyramid.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G106 -V079

$(MID_SUBDIR)/mus_b_pyramid_top.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G107 -V077

$(MID_SUBDIR)/mus_ever_grande.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G068 -V086

$(MID_SUBDIR)/mus_rayquaza_appears.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G109 -V090

$(MID_SUBDIR)/mus_rg_rocket_hideout.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G133 -V090

$(MID_SUBDIR)/mus_rg_follow_me.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G131 -V068

$(MID_SUBDIR)/mus_rg_victory_road.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G154 -V090

$(MID_SUBDIR)/mus_rg_cycling.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G141 -V090

$(MID_SUBDIR)/mus_rg_intro_fight.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G136 -V090

$(MID_SUBDIR)/mus_rg_hall_of_fame.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G145 -V079

$(MID_SUBDIR)/mus_rg_encounter_deoxys.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G184 -V079

$(MID_SUBDIR)/mus_rg_credits.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G149 -V090

$(MID_SUBDIR)/mus_rg_encounter_gym_leader.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G144 -V090

$(MID_SUBDIR)/mus_rg_dex_rating.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G175 -V070 -P5

$(MID_SUBDIR)/mus_rg_obtain_key_item.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G178 -V077 -P5

$(MID_SUBDIR)/mus_rg_caught_intro.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G179 -V094 -P5

$(MID_SUBDIR)/mus_rg_caught.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G170 -V100

$(MID_SUBDIR)/mus_rg_cinnabar.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G138 -V090

$(MID_SUBDIR)/mus_rg_gym.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G134 -V090

$(MID_SUBDIR)/mus_rg_fuchsia.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G167 -V090

$(MID_SUBDIR)/mus_rg_poke_jump.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G132 -V090

$(MID_SUBDIR)/mus_rg_heal.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G140 -V090

$(MID_SUBDIR)/mus_rg_oak_lab.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G160 -V075

$(MID_SUBDIR)/mus_rg_berry_pick.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G132 -V090

$(MID_SUBDIR)/mus_rg_vermillion.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G172 -V090

$(MID_SUBDIR)/mus_rg_route1.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G150 -V079

$(MID_SUBDIR)/mus_rg_route3.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G152 -V083

$(MID_SUBDIR)/mus_rg_route11.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G153 -V090

$(MID_SUBDIR)/mus_rg_pallet.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G159 -V100

$(MID_SUBDIR)/mus_rg_surf.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G164 -V071

$(MID_SUBDIR)/mus_rg_sevii_45.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G188 -V084

$(MID_SUBDIR)/mus_rg_sevii_67.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G189 -V084

$(MID_SUBDIR)/mus_rg_sevii_123.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G173 -V084

$(MID_SUBDIR)/mus_rg_sevii_cave.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G147 -V090

$(MID_SUBDIR)/mus_rg_sevii_dungeon.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G146 -V090

$(MID_SUBDIR)/mus_rg_sevii_route.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G187 -V080

$(MID_SUBDIR)/mus_rg_net_center.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G162 -V096

$(MID_SUBDIR)/mus_rg_pewter.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G173 -V084

$(MID_SUBDIR)/mus_rg_oak.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G161 -V086

$(MID_SUBDIR)/mus_rg_mystery_gift.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G183 -V100

$(MID_SUBDIR)/mus_rg_route24.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G151 -V086

$(MID_SUBDIR)/mus_rg_teachy_tv_show.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G131 -V068

$(MID_SUBDIR)/mus_rg_mt_moon.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G147 -V090

$(MID_SUBDIR)/mus_rg_poke_tower.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G165 -V090

$(MID_SUBDIR)/mus_rg_poke_center.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G162 -V096

$(MID_SUBDIR)/mus_rg_poke_flute.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G165 -V048 -P5

$(MID_SUBDIR)/mus_rg_poke_mansion.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G148 -V090

$(MID_SUBDIR)/mus_rg_jigglypuff.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G135 -V068 -P5

$(MID_SUBDIR)/mus_rg_encounter_rival.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G174 -V079

$(MID_SUBDIR)/mus_rg_rival_exit.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G174 -V079

$(MID_SUBDIR)/mus_rg_encounter_rocket.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G142 -V096

$(MID_SUBDIR)/mus_rg_ss_anne.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G163 -V090

$(MID_SUBDIR)/mus_rg_new_game_exit.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G182 -V088

$(MID_SUBDIR)/mus_rg_new_game_intro.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G182 -V088

$(MID_SUBDIR)/mus_rg_lavender.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G139 -V090

$(MID_SUBDIR)/mus_rg_silph.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G166 -V076

$(MID_SUBDIR)/mus_rg_encounter_girl.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G143 -V051

$(MID_SUBDIR)/mus_rg_encounter_boy.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G144 -V090

$(MID_SUBDIR)/mus_rg_game_corner.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G132 -V090

$(MID_SUBDIR)/mus_rg_slow_pallet.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G159 -V092

$(MID_SUBDIR)/mus_rg_new_game_instruct.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G182 -V085

$(MID_SUBDIR)/mus_rg_viridian_forest.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G146 -V090

$(MID_SUBDIR)/mus_rg_trainer_tower.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G134 -V090

$(MID_SUBDIR)/mus_rg_celadon.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G168 -V070

$(MID_SUBDIR)/mus_rg_title.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G137 -V090

$(MID_SUBDIR)/mus_rg_game_freak.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G181 -V075

$(MID_SUBDIR)/mus_rg_teachy_tv_menu.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G186 -V059

$(MID_SUBDIR)/mus_rg_union_room.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G132 -V090

$(MID_SUBDIR)/mus_rg_vs_legend.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G157 -V090

$(MID_SUBDIR)/mus_rg_vs_deoxys.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G185 -V080

$(MID_SUBDIR)/mus_rg_vs_gym_leader.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G155 -V090

$(MID_SUBDIR)/mus_rg_vs_champion.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G158 -V090

$(MID_SUBDIR)/mus_rg_vs_mewtwo.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G157 -V090

$(MID_SUBDIR)/mus_rg_vs_trainer.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G156 -V090

$(MID_SUBDIR)/mus_rg_vs_wild.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G157 -V090

$(MID_SUBDIR)/mus_rg_victory_gym_leader.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G171 -V090

$(MID_SUBDIR)/mus_rg_victory_trainer.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G169 -V089

$(MID_SUBDIR)/mus_rg_victory_wild.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G170 -V090

$(MID_SUBDIR)/mus_cable_car.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G071 -V078

$(MID_SUBDIR)/mus_sootopolis.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G091 -V062

$(MID_SUBDIR)/mus_safari_zone.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G074 -V082

$(MID_SUBDIR)/mus_b_tower.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G110 -V100

$(MID_SUBDIR)/mus_evolution.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G026 -V080

$(MID_SUBDIR)/mus_encounter_elite_four.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G096 -V078

$(MID_SUBDIR)/mus_c_vs_legend_beast.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -V080

$(MID_SUBDIR)/mus_encounter_swimmer.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G036 -V080

$(MID_SUBDIR)/mus_encounter_girl.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G027 -V080

$(MID_SUBDIR)/mus_intro_battle.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G088 -V088

$(MID_SUBDIR)/mus_encounter_rich.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G043 -V094

$(MID_SUBDIR)/mus_link_contest_p1.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G039 -V079

$(MID_SUBDIR)/mus_link_contest_p2.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G040 -V090

$(MID_SUBDIR)/mus_link_contest_p3.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G041 -V075

$(MID_SUBDIR)/mus_link_contest_p4.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G042 -V090

$(MID_SUBDIR)/mus_littleroot_test.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G034 -V099

$(MID_SUBDIR)/mus_credits.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G101 -V100

$(MID_SUBDIR)/mus_title.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G059 -V090

$(MID_SUBDIR)/mus_fallarbor.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G083 -V100

$(MID_SUBDIR)/mus_mt_chimney.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G052 -V078

$(MID_SUBDIR)/mus_follow_me.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G066 -V074

$(MID_SUBDIR)/mus_vs_frontier_brain.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G115 -V090 -P1

$(MID_SUBDIR)/mus_vs_mew.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G116 -V090

$(MID_SUBDIR)/mus_vs_rayquaza.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G114 -V080 -P1

$(MID_SUBDIR)/mus_encounter_hiker.s: %.s: %.mid
	$(MID) $< $@ -E -R$(STD_REVERB) -G097 -V076
