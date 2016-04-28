-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- Copyright (C) 2014: See COPYRIGHT file                                    --
--                                                                           --
-- This file is part of D2K.                                                 --
--                                                                           --
-- D2K is free software: you can redistribute it and/or modify it under the  --
-- terms of the GNU General Public License as published by the Free Software --
-- Foundation, either version 2 of the License, or (at your option) any      --
-- later version.                                                            --
--                                                                           --
-- D2K is distributed in the hope that it will be useful, but WITHOUT ANY    --
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS --
-- FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    --
-- details.                                                                  --
--                                                                           --
-- You should have received a copy of the GNU General Public License along   --
-- with D2K.  If not, see <http://www.gnu.org/licenses/>.                    --
--                                                                           --
-------------------------------------------------------------------------------

ConfigSchema = {
    system = {
        process_priority = { default = 0, min = 0, max = 2 },
        render_smp = false,
        try_to_reduce_cpu_cache_misses = true
    },

    misc = {
        default_compatibility_level = {
            default = -1,
            min = -1,
            max = d2k.Compatibility.max
        },
        realtic_clock_rate = { default = 100, min = 0 },
        menu_background = true,
        max_player_corpse = { default = 32, min = -1 },
        flashing_hom = false,
        demo_insurance = { default = 2, min = 0, max = 2 },
        endoom_mode = { default = 5, min = 0, max = 7 },
        level_precache = true,
        demo_smoothturns = false,
        demo_smoothturnsfactor = {
            default = 6, min = 1, max = d2k.Demo.smooth_playing_max_factor
        },
        showendoom = false,
        screenshot_dir = "",
        health_bar = false,
        health_bar_full_length = true,
        health_bar_red = { default = 50, min = 0, max = 100 },
        health_bar_yellow = { default = 99, min = 0, max = 100 },
        health_bar_green = { default = 0, min = 0, max = 100 }
    },

    files = {
        wadfile_1 = "",
        wadfile_2 = "",
        dehfile_1 = "",
        dehfile_2 = ""
    },

    game = {
        default_skill = { default = 3, min = 1, max = 5 },
        weapon_recoil = false,
        doom_weapon_toggles = true,
        player_bobbing = true,
        leave_weapons = false,
        monsters_remember = true,
        monster_infighting = true,
        monster_backing = false,
        monster_avoid_hazards = true,
        monkeys = false,
        monster_friction = true,
        help_friends = false,
        allow_pushers = true,
        variable_friction = true,
        player_helpers = false,
        friend_distance = { default = 128, min = 0, max = 999 },
        dog_jumping = true,
        sts_always_red = true,
        sts_pct_always_gray = false,
        sts_traditional_keys = false,
        show_messages = true,
        autorun = true,
        speed_step = { default = 0, min = 0, max = 1000 },
        movement_strafe50 = false,
        movement_strafe50onturns = false,
        movement_shorttics = false,
        interpolation_maxobjects = { default = 0, min = 0 }
    },

    dehacked = {
        deh_apply_cheats = true
    },

    compatibility = {
        comp_zombie = true,
        comp_infcheat = false,
        comp_stairs = false,
        comp_telefrag = false,
        comp_dropoff = false,
        comp_falloff = false,
        comp_staylift = false,
        comp_doorstuck = false,
        comp_pursuit = false,
        comp_vile = false,
        comp_pain = false,
        comp_skull = false,
        comp_blazing = false,
        comp_doorlight = false,
        comp_god = false,
        comp_skymap = false,
        comp_floors = false,
        comp_model = false,
        comp_zerotags = false,
        comp_moveblock = false,
        comp_sound = false,
        comp_666 = false,
        comp_soul = false,
        comp_maskedanim = false,
        comp_ouchface = false,
        comp_maxhealth = false,
        comp_translucency = false,
        comperr_zerotag = false,
        comperr_passuse = false,
        comperr_hangsolid = false,
        comperr_blockmap = false,
        comperr_allowjump = false
    },

    sound = {
        snd_pcspeaker = false,
        sound_card = { default = -1, min = -1, max = 7 },
        music_card = { default = -1, min = -1, max = 9 },
        pitched_sounds = false,
        samplerate = { default = 22050, min = 11025, max = 48000 },
        sfx_volume = { default = 8, min = 0, max = 15 },
        mus_pause_opt = { default = 1, min = 0, max = 2 },
        snd_channels = {
            default = d2k.Sound.max_channels,
            min = 1,
            max = d2k.Sound.max_channels
        },
        snd_midiplayer = "sdl",
        snd_soundfont = "TimGM6mb.sf2",
        snd_mididev = "",
        mus_extend_volume = false,
        mus_fluidsynth_gain = { default = 50, min = 0, max = 1000 },
        mus_opl_gain = { default = 50, min = 0, max = 1000 }
    },

    video = {
        videomode = "32",
        use_gl_surface = false,
        screen_resolution = "640x480",
        use_fullscreen = false,
        use_doublebuffer = true,
        translucency = true,
        tran_filter_pct = { default = 66, min = 0, max = 100 },
        screenblocks = { default = 10, min = 3, max = 11 },
        usegamma = { default = 3, min = 0, max = 4 },
        uncapped_framerate = true,
        test_interpolation_method = false,
        filter_wall = {
            default = d2k.Renderer.filter_point,
            min = d2k.Renderer.filter_point,
            max = d2k.Renderer.filter_rounded
        },
        filter_floor = {
            default = d2k.Renderer.filter_point,
            min = d2k.Renderer.filter_point,
            max = d2k.Renderer.filter_rounded
        },
        filter_sprite = {
            default = d2k.Renderer.filter_point,
            min = d2k.Renderer.filter_point,
            max = d2k.Renderer.filter_rounded
        },
        filter_z = {
            default = d2k.Renderer.filter_point,
            min = d2k.Renderer.filter_point,
            max = d2k.Renderer.filter_linear
        },
        filter_patch = {
            default = d2k.Renderer.filter_point,
            min = d2k.Renderer.filter_point,
            max = d2k.Renderer.filter_rounded
        },
        filter_threshold = {
            default = 49152,
            min = 0
        },
        sprite_edges = {
            default = d2k.Renderer.masked_column_edge_square,
            min = d2k.Renderer.masked_column_edge_square,
            max = d2k.Renderer.masked_column_edge_sloped
        },
        patch_edges = {
            default = d2k.Renderer.masked_column_edge_square,
            min = d2k.Renderer.masked_column_edge_square,
            max = d2k.Renderer.masked_column_edge_sloped
        },
        sdl_videodriver = "default",
        sdl_video_window_pos = "center",
        palette_ondamage = true,
        palette_onbonus = true,
        palette_onpowers = true,
        render_wipescreen = true,
        render_screen_multiply = { default = 1, min = 1, max = 4 },
        render_interlaced_scanning = false,
        render_precise_high_quality = true,
        render_aspect = { default = 0, min = 0, max = 4 },
        render_doom_lightmaps = false,
        fake_contrast = true,
        render_stretch_hud = {
            default = d2k.Renderer.patch_stretch_16x10,
            min = d2k.Renderer.patch_stretch_16x10,
            max = d2k.Renderer.patch_stretch_full
        },
        render_patches_scalex = { default = 0, min = 0, max = 16 },
        render_patches_scaley = { default = 0, min = 0, max = 16 },
        render_stretchsky = true,
        sprites_doom_order = {
            default = d2k.Renderer.sprite_order_static,
            min = d2k.Renderer.sprite_order_none,
            max = d2k.Renderer.sprite_order_dynamic
        },
        movement_mouselook = false,
        movement_maxviewpitch = {
            default = 90,
            min = 0,
            max = 90
        },
        movement_mouseinvert = false
    },

    opengl = {
        gl_compatibility = false,
        gl_arb_multitexture = true,
        gl_arb_texture_compression = true,
        gl_arb_texture_non_power_of_two = true,
        gl_ext_arb_vertex_buffer_object = true,
        gl_arb_pixel_buffer_object = true,
        gl_arb_shader_objects = true,
        gl_ext_blend_color = true,
        gl_arb_framebuffer_object = true,
        gl_ext_packed_depth_stencil = true,
        gl_ext_texture_filter_anisotropic = true,
        gl_use_stencil = true,
        gl_use_display_lists = false,
        gl_vsync = true,
        gl_clear = false,
        gl_ztrick = false,
        gl_nearclip = { default = 5, min = 0 },
        gl_colorbuffer_bits = { default = 32, min = 16, max = 32 },
        gl_depthbuffer_bits = { default = 24, min = 16, max = 32 },
        gl_texture_filter = {
            default = d2k.Renderer.gl_filter_linear_mipmap_linear,
            min = d2k.Renderer.gl_filter_nearest,
            max = d2k.Renderer.gl_filter_linear_mipmap_linear
        },
        gl_sprite_filter = {
            default = d2k.Renderer.gl_filter_linear, 
            min = d2k.Renderer.gl_filter_nearest,
            max = d2k.Renderer.gl_filter_linear_mipmap_nearest
        },
        gl_patch_filter = {
            default = d2k.Renderer.gl_filter_linear,
            min = d2k.Renderer.gl_filter_nearest,
            max = d2k.Renderer.gl_filter_linear
        },
        gl_texture_filter_anisotropic = {
            default = d2k.Renderer.anisotropic_8x,
            min = d2k.Renderer.anisotropic_off,
            max = d2k.Renderer.anisotropic_16x
        },
        gl_tex_format_string = "GL_RGBA",
        gl_sprite_offset = { default = 0, min = 0, max = 5 },
        gl_sprite_blend = false,
        gl_mask_sprite_threshold = { default = 50, min = 0, max = 100 },
        gl_skymode = {
            default = d2k.Renderer.skytype_auto,
            min = d2k.Renderer.skytype_auto,
            max = d2k.Renderer.skytype_screen
        },
        gl_sky_detail = { default = 16, min = 1, max = 32 },
        gl_use_paletted_texture = false,
        gl_use_shared_texture_palette = false,
        gl_allow_detail_textures = true,
        gl_detail_maxdist = { default = 0, min = 0, max = 65535 },
        render_multisampling = { default = 0, min = 0, max = 8 },
        render_fov = { default = 90, min = 20, max = 160 },
        gl_spriteclip = {
            default = d2k.Renderer.spriteclip_smart,
            min = d2k.Renderer.spriteclip_const,
            max = d2k.Renderer.spriteclip_smart
        },
        gl_spriteclip_threshold = { default = 10, min = 0, max = 100 },
        gl_sprites_frustum_culling = true,
        render_paperitems = false,
        gl_boom_colormaps = true,
        gl_hires_24bit_colormap = false,
        gl_texture_internal_hires = true,
        gl_texture_external_hires = false,
        gl_hires_override_pwads = false,
        gl_texture_hires_dir = "",
        gl_texture_hqresize = false,
        gl_texture_hqresize_textures = {
            default = d2k.Renderer.hq_scale_2x,
            min = d2k.Renderer.hq_scale_none,
            max = d2k.Renderer.hq_scale_4x
        },
        gl_texture_hqresize_sprites = {
            default = d2k.Renderer.hq_scale_none,
            min = d2k.Renderer.hq_scale_none,
            max = d2k.Renderer.hq_scale_4x
        },
        gl_texture_hqresize_patches = {
            default = d2k.Renderer.hq_scale_2x,
            min = d2k.Renderer.hq_scale_none,
            max = d2k.Renderer.hq_scale_4x
        },
        gl_motionblur = false,
        gl_motionblur_min_speed = 21.36,
        gl_motionblur_min_angle = 20.0,
        gl_motionblur_att_a = 55.0,
        gl_motionblur_att_b = 1.8,
        gl_motionblur_att_c = 0.9,
        gl_lightmode = {
            default = d2k.Renderer.lightmode_glboom,
            min = d2k.Renderer.lightmode_glboom,
            max = d2k.Renderer.lightmode_shaders
        },
        gl_light_ambient = { default = 20, min = 1, max = 255 },
        gl_fog = true,
        gl_fog_color = { default = 0, min = 0, max = 16777215 },
        useglgamma = { default = 6, min = 0, max = d2k.Renderer.max_gl_gamma },
        gl_color_mip_levels = false,
        gl_shadows = false,
        gl_shadows_maxdist = { default = 1000, min = 0, max = 32767 },
        gl_shadows_factor = { default = 128, min = 0, max = 255 },
        gl_blend_animations = false
    },

    mouse = {
        use_mouse = true,
        mouse_sensitivity_horiz = { default = 10, min = 0 },
        mouse_sensitivity_vert = { default = 10, min = 0 },
        mouseb_fire = {
            default = 0,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        mouseb_strafe = {
            default = 1,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        mouseb_forward = {
            default = 2,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        mouseb_backward = {
            default = -1,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        mouseb_use = {
            default = -1,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        mouse_acceleration = { default = 0, min = 0 },
        mouse_sensitivity_mlook = { default = 10, min = 0 },
        mouse_doubleclick_as_use = true
    },

    key_bindings = {
        key_right = "right",
        key_left = "left",
        key_up = "w",
        key_down = "s",
        key_mlook = "\\",
        key_help = "f1",
        key_menu_toggle = "escape",
        key_menu_right = "right",
        key_menu_left = "left",
        key_menu_up = "up",
        key_menu_down = "down",
        key_menu_backspace = "backspace",
        key_menu_escape = "escape",
        key_menu_enter = "enter",
        key_setup = "0",
        key_strafeleft = "a",
        key_straferight = "d",
        key_flyup = ".",
        key_flydown = ",",
        key_fire = "left_control",
        key_use = " ",
        key_strafe = "left_alt",
        key_speed = "left_shift",
        key_savegame = "f2",
        key_loadgame = "f3",
        key_soundvolume = "f4",
        key_hud = "f5",
        key_quicksave = "f6",
        key_endgame = "f7",
        key_messages = "f8",
        key_quickload = "f9",
        key_quit = "f10",
        key_gamma = "f11",
        key_spy = "f12",
        key_pause = "pause",
        key_autorun = "caps_lock",
        key_chat = "t",
        key_backspace = "backspace",
        key_enter = "enter",
        key_map = "tab",
        key_map_right = "right",
        key_map_left = "left",
        key_map_up = "up",
        key_map_down = "down",
        key_map_zoomin = "=",
        key_map_zoomout = "-",
        key_map_gobig = "0",
        key_map_follow = "f",
        key_map_mark = "m",
        key_map_clear = "c",
        key_map_grid = "g",
        key_map_rotate = "r",
        key_map_overlay = "o",
        key_map_textured = "0",
        key_reverse = "/",
        key_zoomin = "=",
        key_zoomout = "-",
        key_chatplayer1 = "g",
        key_chatplayer2 = "i",
        key_chatplayer3 = "b",
        key_chatplayer4 = "r",
        key_weapontoggle = "0",
        key_weapon1 = "1",
        key_weapon2 = "2",
        key_weapon3 = "3",
        key_weapon4 = "4",
        key_weapon5 = "5",
        key_weapon6 = "6",
        key_weapon7 = "7",
        key_weapon8 = "8",
        key_weapon9 = "9",
        key_nextweapon = "wheel_up",
        key_prevweapon = "wheel_down",
        key_screenshot = "*",
        key_speedup = "kp8",
        key_speeddown = "kp2",
        key_speeddefault = "kp5",
        key_demo_skip = "kp6",
        key_level_restart = "kp8",
        key_demo_endlevel = "kp1",
        key_nextlevel = "kp6",
        key_demo_jointogame = "kp_enter",
        key_walkcamera = "kp_multiply",
        key_showalive = "kp_divide"
    },

    joystick = {
        use_joystick = { default = 0, min = 0, max = 2 },
        joy_left = 0,
        joy_right = 0,
        joy_up = 0,
        joy_down = 0,
        joyb_fire = 0,
        joyb_strafe = { default = 1, min = 0 },
        joyb_speed = { default = 2, min = 0 },
        joyb_use = { default = 3, min = 0 },
        joyb_strafeleft = { default = 4, min = 0 },
        joyb_strafeleft = { default = 5, min = 0 }
    },

    chat_macros = {
        "No",
        "I'm ready to kick butt!",
        "I'm OK.",
        "I'm not looking too good!",
        "Help!",
        "You suck!",
        "Next time, scumbag...",
        "Come here!",
        "I'll take care of it.",
        "Yes"
    },

    automap = {
        mapcolor_back = { default = 247, min = 0, max = 255 },
        mapcolor_grid = { default = 104, min = 0, max = 255 },
        mapcolor_wall = { default = 23, min = 0, max = 255 },
        mapcolor_fchg = { default = 55, min = 0, max = 255 },
        mapcolor_cchg = { default = 215, min = 0, max = 255 },
        mapcolor_clsd = { default = 208, min = 0, max = 255 },
        mapcolor_rkey = { default = 175, min = 0, max = 255 },
        mapcolor_bkey = { default = 204, min = 0, max = 255 },
        mapcolor_ykey = { default = 231, min = 0, max = 255 },
        mapcolor_rdor = { default = 175, min = 0, max = 255 },
        mapcolor_bdor = { default = 204, min = 0, max = 255 },
        mapcolor_ydor = { default = 231, min = 0, max = 255 },
        mapcolor_tele = { default = 119, min = 0, max = 255 },
        mapcolor_secr = { default = 252, min = 0, max = 255 },
        mapcolor_exit = { default = 0, min = 0, max = 255 },
        mapcolor_unsn = { default = 104, min = 0, max = 255 },
        mapcolor_flat = { default = 88, min = 0, max = 255 },
        mapcolor_sprt = { default = 112, min = 0, max = 255 },
        mapcolor_item = { default = 231, min = 0, max = 255 },
        mapcolor_hair = { default = 208, min = 0, max = 255 },
        mapcolor_sngl = { default = 208, min = 0, max = 255 },
        mapcolor_me = { default = 112, min = 0, max = 255 },
        mapcolor_enemy = { default = 177, min = 0, max = 255 },
        mapcolor_frnd = { default = 112, min = 0, max = 255 },
        map_secret_after = false,
        map_point_coord = false,
        map_level_stat = true,
        automapmode = false,
        map_always_updates = true,
        map_grid_size = { default = 128, min = 8, max = 256 },
        map_scroll_speed = { default = 8, min = 1, max = 32 },
        map_wheel_zoom = true,
        map_use_multisamling = true,
        map_textured = true,
        map_textured_trans = { default = 100, min = 0, max = 100 },
        map_textured_overlay_trans = { default = 66, min = 0, max = 100 },
        map_lines_overlay_trans = { default = 100, min = 0, max = 100 },
        map_overlay_pos_x = { default = 0, min = 0, max = 319 },
        map_overlay_pos_y = { default = 0, min = 0, max = 199 },
        map_overlay_pos_width = { default = 320, min = 0, max = 320 },
        map_overlay_pos_height = { default = 200, min = 0, max = 200 },
        map_things_appearance = {
            default = d2k.AutoMap.map_things_appearance_icon,
            min = d2k.AutoMap.map_things_appearance_classic,
            max = d2k.AutoMap.map_things_appearance_icon
        }
    },

    hud = {
        hudcolor_titl = { default = 5, min = 0, max = 9 },
        hudcolor_xyco = { default = 3, min = 0, max = 9 },
        hudcolor_mapstat_title = { default = 6, min = 0, max = 9 },
        hudcolor_mapstat_value = { default = 2, min = 0, max = 9 },
        hudcolor_mapstat_time = { default = 2, min = 0, max = 9 },
        hudcolor_mesg = { default = 6, min = 0, max = 9 },
        hudcolor_chat = { default = 5, min = 0, max = 9 },
        hudcolor_list = { default = 5, min = 0, max = 9 },
        hud_msg_lines = { default = 1, min = 1, max = 16 },
        hud_list_bgon = false,
        health_red = { default = 25, min = 0, max = 200 },
        health_yellow = { default = 50, min = 0, max = 200 },
        health_green = { default = 100, min = 0, max = 200 },
        armor_red = { default = 25, min = 0, max = 200 },
        armor_yellow = { default = 50, min = 0, max = 200 },
        armor_green = { default = 100, min = 0, max = 200 },
        ammo_red = { default = 25, min = 0, max = 100 },
        ammo_yellow = { default = 50, min = 0, max = 100 },
        ammo_colour_behaviour = {
            default = d2k.StatusBar.ammo_colour_behaviour_yes,
            min = d2k.StatusBar.ammo_colour_behaviour_no,
            max = d2k.StatusBar.ammo_colour_behaviour_yes
        },
        hud_num = { default = 6, min = 0, max = 100 },
        hud_displayed = false,
        hudadd_gamespeed = false,
        hudadd_leveltime = false,
        hudadd_demotime = false,
        hudadd_secretarea = false,
        hudadd_smarttotals = false,
        hudadd_demoprogressbar = true,
        hudadd_crosshair = {
            default = 0, min = 0, max = d2k.Video.crosshair_count - 1
        },
        hudadd_crosshair_scale = false,
        hudadd_crosshair_color = { default = 3, min = 0, max = 9 },
        hudadd_crosshair_health = false,
        hudadd_crosshair_target = false,
        hudadd_crosshair_target_color = { default = 9, min = 0, max = 9 },
        hudadd_crosshair_lock_target = false
    },

    demos = {
        demo_extendedformat = true,
        demo_demoex_filename = "",
        getwad_cmdline = "",
        demo_overwriteexisting = true
    },

    video_recording = {
        cap_soundcommand = "oggenc2 -r -R %s -q 5 - -o output.ogg",
        cap_videocommand = "x264 -o output.mp4 --crf 22 --muxer mp4 --demuxer raw --input-csp rgb --input-depth 8 --input-res %wx%h --fps 35 -",
        cap_muxcommand = "mkvmerge -o %f output.mp4 output.ogg",
        cap_tempfile1 = "output.ogg",
        cap_tempfile2 = "output.mp4",
        cap_remove_tempfiles = true
    },

    overrun = {
        overrun_spechit_warn = false,
        overrun_spechit_emulate = true,
        overrun_reject_warn = false,
        overrun_reject_emulate = true,
        overrun_intercept_warn = false,
        overrun_intercept_emulate = true,
        overrun_playeringame_warn = false,
        overrun_playeringame_emulate = true,
        overrun_donut_warn = false,
        overrun_donut_emulate = false,
        overrun_missedbackside_warn = false,
        overrun_missedbackside_emulate = false
    },

    demo_patterns = {
        "DOOM 2 = Hell on Earth/((lv)|(nm)|(pa)|(ty))\\d\\d.\\d\\d\\d\\.lmp/doom2.wad",
        "DOOM 2 = Plutonia Experiment/p(c|f|l|n|p|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|plutonia.wad",
        "DOOM 2 = TNT - Evilution/((e(c|f|v|p|r|s|t))|(tn))\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|tnt.wad",
        "The Ultimate DOOM/(((e|f|n|p|r|t|u)\\dm\\d)|(n\\ds\\d)).\\d\\d\\d\\.lmp/doom.wad",
        "Alien Vendetta/a(c|f|n|p|r|s|t|v)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|av.wad|av.deh",
        "Requiem/r(c|f|n|p|q|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|requiem.wad|req21fix.wad|reqmus.wad",
        "Hell Revealed/h(c|e|f|n|p|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|hr.wad|hrmus.wad",
        "Memento Mori/mm\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|mm.wad|mmmus.wad",
        "Memento Mori 2/m2\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|mm2.wad|mm2mus.wad"
    },

    pwo = {
        "plasma",
        "super_shotgun",
        "chaingun",
        "shotgun",
        "pistol",
        "chainsaw",
        "rocket_launcher",
        "bfg",
        "fist"
    },

    music = {
        mus_e1m1   = "e1m1.mp3",
        mus_e1m2   = "e1m2.mp3",
        mus_e1m3   = "e1m3.mp3",
        mus_e1m4   = "e1m4.mp3",
        mus_e1m5   = "e1m5.mp3",
        mus_e1m6   = "e1m6.mp3",
        mus_e1m7   = "e1m7.mp3",
        mus_e1m8   = "e1m8.mp3",
        mus_e1m9   = "e1m9.mp3",
        mus_e2m1   = "e2m1.mp3",
        mus_e2m2   = "e2m2.mp3",
        mus_e2m3   = "e2m3.mp3",
        mus_e2m4   = "e2m4.mp3",
        mus_e2m5   = "e1m7.mp3",
        mus_e2m6   = "e2m6.mp3",
        mus_e2m7   = "e2m7.mp3",
        mus_e2m8   = "e2m8.mp3",
        mus_e2m9   = "e3m1.mp3",
        mus_e3m1   = "e3m1.mp3",
        mus_e3m2   = "e3m2.mp3",
        mus_e3m3   = "e3m3.mp3",
        mus_e3m4   = "e1m8.mp3",
        mus_e3m5   = "e1m7.mp3",
        mus_e3m6   = "e1m6.mp3",
        mus_e3m7   = "e2m7.mp3",
        mus_e3m8   = "e3m8.mp3",
        mus_e3m9   = "e1m9.mp3",
        mus_inter  = "e2m3.mp3",
        mus_intro  = "intro.mp3",
        mus_bunny  = "bunny.mp3",
        mus_victor = "victor.mp3",
        mus_introa = "intro.mp3",
        mus_runnin = "runnin.mp3",
        mus_stalks = "stalks.mp3",
        mus_countd = "countd.mp3",
        mus_betwee = "betwee.mp3",
        mus_doom   = "doom.mp3",
        mus_the_da = "the_da.mp3",
        mus_shawn  = "shawn.mp3",
        mus_ddtblu = "ddtblu.mp3",
        mus_in_cit = "in_cit.mp3",
        mus_dead   = "dead.mp3",
        mus_stlks2 = "stalks.mp3",
        mus_theda2 = "the_da.mp3",
        mus_doom2  = "doom.mp3",
        mus_ddtbl2 = "ddtblu.mp3",
        mus_runni2 = "runnin.mp3",
        mus_dead2  = "dead.mp3",
        mus_stlks3 = "stalks.mp3",
        mus_romero = "romero.mp3",
        mus_shawn2 = "shawn.mp3",
        mus_messag = "messag.mp3",
        mus_count2 = "countd.mp3",
        mus_ddtbl3 = "ddtblu.mp3",
        mus_ampie  = "ampie.mp3",
        mus_theda3 = "the_da.mp3",
        mus_adrian = "adrian.mp3",
        mus_messg2 = "messag.mp3",
        mus_romer2 = "romero.mp3",
        mus_tense  = "tense.mp3",
        mus_shawn3 = "shawn.mp3",
        mus_openin = "openin.mp3",
        mus_evil   = "evil.mp3",
        mus_ultima = "ultima.mp3",
        mus_read_m = "read_m.mp3",
        mus_dm2ttl = "dm2ttl.mp3",
        mus_dm2int = "dm2int.mp3"
    },

    server = {
        limit_player_commands = true
    },

    client = {
        extrapolate_player_positions = true
    }

}

return {
    ConfigSchema = ConfigSchema
}

-- vi: et ts=4 sw=4
