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
        apply_cheats = true
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
        pcspeaker = false,
        sound_card = { default = -1, min = -1, max = 7 },
        music_card = { default = -1, min = -1, max = 9 },
        pitched_sounds = false,
        samplerate = { default = 22050, min = 11025, max = 48000 },
        sfx_volume = { default = 8, min = 0, max = 15 },
        pause_opt = { default = 1, min = 0, max = 2 },
        snd_channels = {
            default = d2k.Sound.max_channels,
            min = 1,
            max = d2k.Sound.max_channels
        },
        midiplayer = "sdl",
        soundfont = "TimGM6mb.sf2",
        mididev = "",
        extend_volume = false,
        fluidsynth_gain = { default = 50, min = 0, max = 1000 },
        opl_gain = { default = 50, min = 0, max = 1000 }
    },

    video = {
        mode = "32",
        use_gl_surface = false,
        screen_resolution = "640x480",
        fullscreen = false,
        doublebuffer = true,
        translucency = true,
        tran_filter_pct = { default = 66, min = 0, max = 100 },
        screenblocks = { default = 10, min = 3, max = 11 },
        gamma = { default = 3, min = 0, max = 4 },
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
    },

    opengl = {
        compatibility = false,
        arb_multitexture = true,
        arb_texture_compression = true,
        arb_texture_non_power_of_two = true,
        ext_arb_vertex_buffer_object = true,
        arb_pixel_buffer_object = true,
        arb_shader_objects = true,
        ext_blend_color = true,
        arb_framebuffer_object = true,
        ext_packed_depth_stencil = true,
        ext_texture_filter_anisotropic = true,
        use_stencil = true,
        use_display_lists = false,
        vsync = true,
        clear = false,
        ztrick = false,
        nearclip = { default = 5, min = 0 },
        colorbuffer_bits = { default = 32, min = 16, max = 32 },
        depthbuffer_bits = { default = 24, min = 16, max = 32 },
        texture_filter = {
            default = d2k.Renderer.gl_filter_linear_mipmap_linear,
            min = d2k.Renderer.gl_filter_nearest,
            max = d2k.Renderer.gl_filter_linear_mipmap_linear
        },
        sprite_filter = {
            default = d2k.Renderer.gl_filter_linear, 
            min = d2k.Renderer.gl_filter_nearest,
            max = d2k.Renderer.gl_filter_linear_mipmap_nearest
        },
        patch_filter = {
            default = d2k.Renderer.gl_filter_linear,
            min = d2k.Renderer.gl_filter_nearest,
            max = d2k.Renderer.gl_filter_linear
        },
        texture_filter_anisotropic = {
            default = d2k.Renderer.anisotropic_8x,
            min = d2k.Renderer.anisotropic_off,
            max = d2k.Renderer.anisotropic_16x
        },
        tex_format_string = "GL_RGBA",
        sprite_offset = { default = 0, min = 0, max = 5 },
        sprite_blend = false,
        mask_sprite_threshold = { default = 50, min = 0, max = 100 },
        skymode = {
            default = d2k.Renderer.skytype_auto,
            min = d2k.Renderer.skytype_auto,
            max = d2k.Renderer.skytype_screen
        },
        sky_detail = { default = 16, min = 1, max = 32 },
        use_paletted_texture = false,
        use_shared_texture_palette = false,
        allow_detail_textures = true,
        detail_maxdist = { default = 0, min = 0, max = 65535 },
        render_multisampling = { default = 0, min = 0, max = 8 },
        render_fov = { default = 90, min = 20, max = 160 },
        spriteclip = {
            default = d2k.Renderer.spriteclip_smart,
            min = d2k.Renderer.spriteclip_const,
            max = d2k.Renderer.spriteclip_smart
        },
        spriteclip_threshold = { default = 10, min = 0, max = 100 },
        sprites_frustum_culling = true,
        render_paperitems = false,
        boom_colormaps = true,
        hires_24bit_colormap = false,
        texture_internal_hires = true,
        texture_external_hires = false,
        hires_override_pwads = false,
        texture_hires_dir = "",
        texture_hqresize = false,
        texture_hqresize_textures = {
            default = d2k.Renderer.hq_scale_2x,
            min = d2k.Renderer.hq_scale_none,
            max = d2k.Renderer.hq_scale_4x
        },
        texture_hqresize_sprites = {
            default = d2k.Renderer.hq_scale_none,
            min = d2k.Renderer.hq_scale_none,
            max = d2k.Renderer.hq_scale_4x
        },
        texture_hqresize_patches = {
            default = d2k.Renderer.hq_scale_2x,
            min = d2k.Renderer.hq_scale_none,
            max = d2k.Renderer.hq_scale_4x
        },
        motionblur = false,
        motionblur_min_speed = 21.36,
        motionblur_min_angle = 20.0,
        motionblur_att_a = 55.0,
        motionblur_att_b = 1.8,
        motionblur_att_c = 0.9,
        lightmode = {
            default = d2k.Renderer.lightmode_glboom,
            min = d2k.Renderer.lightmode_glboom,
            max = d2k.Renderer.lightmode_shaders
        },
        light_ambient = { default = 20, min = 1, max = 255 },
        fog = true,
        fog_color = { default = 0, min = 0, max = 16777215 },
        useglgamma = { default = 6, min = 0, max = d2k.Renderer.max_gl_gamma },
        color_mip_levels = false,
        shadows = false,
        shadows_maxdist = { default = 1000, min = 0, max = 32767 },
        shadows_factor = { default = 128, min = 0, max = 255 },
        blend_animations = false
    },

    mouse = {
        enabled = true,
        sensitivity_horiz = { default = 10, min = 0 },
        sensitivity_vert = { default = 10, min = 0 },
        fire = {
            default = 0,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        strafe = {
            default = 1,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        forward = {
            default = 2,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        backward = {
            default = -1,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        use = {
            default = -1,
            min = -1,
            max = d2k.Mouse.max_buttons
        },
        acceleration = { default = 0, min = 0 },
        sensitivity_mlook = { default = 10, min = 0 },
        doubleclick_as_use = true,
        mouselook = false,
        maxviewpitch = {
            default = 90,
            min = 0,
            max = 90
        },
        invert = false
    },

    keybinds = {
        right = "right",
        left = "left",
        up = "w",
        down = "s",
        mlook = "\\",
        help = "f1",
        menu_toggle = "escape",
        menu_right = "right",
        menu_left = "left",
        menu_up = "up",
        menu_down = "down",
        menu_backspace = "backspace",
        menu_escape = "escape",
        menu_enter = "enter",
        setup = "0",
        strafeleft = "a",
        straferight = "d",
        flyup = ".",
        flydown = ",",
        fire = "left_control",
        use = " ",
        strafe = "left_alt",
        speed = "left_shift",
        savegame = "f2",
        loadgame = "f3",
        soundvolume = "f4",
        hud = "f5",
        quicksave = "f6",
        endgame = "f7",
        messages = "f8",
        quickload = "f9",
        quit = "f10",
        gamma = "f11",
        spy = "f12",
        pause = "pause",
        autorun = "caps_lock",
        chat = "t",
        backspace = "backspace",
        enter = "enter",
        map = "tab",
        map_right = "right",
        map_left = "left",
        map_up = "up",
        map_down = "down",
        map_zoomin = "=",
        map_zoomout = "-",
        map_gobig = "0",
        map_follow = "f",
        map_mark = "m",
        map_clear = "c",
        map_grid = "g",
        map_rotate = "r",
        map_overlay = "o",
        map_textured = "0",
        reverse = "/",
        zoomin = "=",
        zoomout = "-",
        chatplayer1 = "g",
        chatplayer2 = "i",
        chatplayer3 = "b",
        chatplayer4 = "r",
        weapontoggle = "0",
        weapon1 = "1",
        weapon2 = "2",
        weapon3 = "3",
        weapon4 = "4",
        weapon5 = "5",
        weapon6 = "6",
        weapon7 = "7",
        weapon8 = "8",
        weapon9 = "9",
        nextweapon = "wheel_up",
        prevweapon = "wheel_down",
        screenshot = "*",
        speedup = "kp8",
        speeddown = "kp2",
        speeddefault = "kp5",
        demo_skip = "kp6",
        level_restart = "kp8",
        demo_endlevel = "kp1",
        nextlevel = "kp6",
        demo_jointogame = "kp_enter",
        walkcamera = "kp_multiply",
        showalive = "kp_divide"
    },

    joystick = {
        mode = { default = 0, min = 0, max = 2 },
        left = 0,
        right = 0,
        up = 0,
        down = 0,
        fire = 0,
        strafe = { default = 1, min = 0 },
        speed = { default = 2, min = 0 },
        use = { default = 3, min = 0 },
        strafeleft = { default = 4, min = 0 },
        straferight = { default = 5, min = 0 }
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
        back_color = { default = 247, min = 0, max = 255 },
        grid_color = { default = 104, min = 0, max = 255 },
        wall_color = { default = 23, min = 0, max = 255 },
        fchg_color = { default = 55, min = 0, max = 255 },
        cchg_color = { default = 215, min = 0, max = 255 },
        clsd_color = { default = 208, min = 0, max = 255 },
        rkey_color = { default = 175, min = 0, max = 255 },
        bkey_color = { default = 204, min = 0, max = 255 },
        ykey_color = { default = 231, min = 0, max = 255 },
        rdor_color = { default = 175, min = 0, max = 255 },
        bdor_color = { default = 204, min = 0, max = 255 },
        ydor_color = { default = 231, min = 0, max = 255 },
        tele_color = { default = 119, min = 0, max = 255 },
        secr_color = { default = 252, min = 0, max = 255 },
        exit_color = { default = 0, min = 0, max = 255 },
        unsn_color = { default = 104, min = 0, max = 255 },
        flat_color = { default = 88, min = 0, max = 255 },
        sprt_color = { default = 112, min = 0, max = 255 },
        item_color = { default = 231, min = 0, max = 255 },
        hair_color = { default = 208, min = 0, max = 255 },
        sngl_color = { default = 208, min = 0, max = 255 },
        me_color = { default = 112, min = 0, max = 255 },
        enemy_color = { default = 177, min = 0, max = 255 },
        frnd_color = { default = 112, min = 0, max = 255 },
        secret_after = false,
        point_coord = false,
        level_stat = true,
        mode = { -- [CG] These are actually flags
            default = d2k.AutoMap.MODE_OFF,
            min = d2k.AutoMap.MODE_OFF,
            max = d2k.AutoMap.MODE_GRID
        },
        always_updates = true,
        grid_size = { default = 128, min = 8, max = 256 },
        scroll_speed = { default = 8, min = 1, max = 32 },
        wheel_zoom = true,
        use_multisamling = true,
        textured = true,
        textured_trans = { default = 100, min = 0, max = 100 },
        textured_overlay_trans = { default = 66, min = 0, max = 100 },
        lines_overlay_trans = { default = 100, min = 0, max = 100 },
        overlay_pos_x = { default = 0, min = 0, max = 319 },
        overlay_pos_y = { default = 0, min = 0, max = 199 },
        overlay_pos_width = { default = 320, min = 0, max = 320 },
        overlay_pos_height = { default = 200, min = 0, max = 200 },
        things_appearance = {
            default = d2k.AutoMap.map_things_appearance_icon,
            min = d2k.AutoMap.map_things_appearance_classic,
            max = d2k.AutoMap.map_things_appearance_icon
        }
    },

    hud = {
        titl_color = { default = 5, min = 0, max = 9 },
        xyco_color = { default = 3, min = 0, max = 9 },
        mapstat_title_color = { default = 6, min = 0, max = 9 },
        mapstat_value_color = { default = 2, min = 0, max = 9 },
        mapstat_time_color = { default = 2, min = 0, max = 9 },
        mesg_color = { default = 6, min = 0, max = 9 },
        chat_color = { default = 5, min = 0, max = 9 },
        list_color = { default = 5, min = 0, max = 9 },
        msg_lines = { default = 1, min = 1, max = 16 },
        list_bgon = false,
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
        num = { default = 6, min = 0, max = 100 },
        displayed = false,
        add_gamespeed = false,
        add_leveltime = false,
        add_demotime = false,
        add_secretarea = false,
        add_smarttotals = false,
        add_demoprogressbar = true,
        add_crosshair = {
            default = 0, min = 0, max = d2k.Video.crosshair_count - 1
        },
        add_crosshair_scale = false,
        add_crosshair_color = { default = 3, min = 0, max = 9 },
        add_crosshair_health = false,
        add_crosshair_target = false,
        add_crosshair_target_color = { default = 9, min = 0, max = 9 },
        add_crosshair_lock_target = false
    },

    demos = {
        extendedformat = true,
        demoex_filename = "",
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
        spechit_warn = false,
        spechit_emulate = true,
        reject_warn = false,
        reject_emulate = true,
        intercept_warn = false,
        intercept_emulate = true,
        playeringame_warn = false,
        playeringame_emulate = true,
        donut_warn = false,
        donut_emulate = false,
        missedbackside_warn = false,
        missedbackside_emulate = false
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
        e1m1   = "e1m1.mp3",
        e1m2   = "e1m2.mp3",
        e1m3   = "e1m3.mp3",
        e1m4   = "e1m4.mp3",
        e1m5   = "e1m5.mp3",
        e1m6   = "e1m6.mp3",
        e1m7   = "e1m7.mp3",
        e1m8   = "e1m8.mp3",
        e1m9   = "e1m9.mp3",
        e2m1   = "e2m1.mp3",
        e2m2   = "e2m2.mp3",
        e2m3   = "e2m3.mp3",
        e2m4   = "e2m4.mp3",
        e2m5   = "e1m7.mp3",
        e2m6   = "e2m6.mp3",
        e2m7   = "e2m7.mp3",
        e2m8   = "e2m8.mp3",
        e2m9   = "e3m1.mp3",
        e3m1   = "e3m1.mp3",
        e3m2   = "e3m2.mp3",
        e3m3   = "e3m3.mp3",
        e3m4   = "e1m8.mp3",
        e3m5   = "e1m7.mp3",
        e3m6   = "e1m6.mp3",
        e3m7   = "e2m7.mp3",
        e3m8   = "e3m8.mp3",
        e3m9   = "e1m9.mp3",
        inter  = "e2m3.mp3",
        intro  = "intro.mp3",
        bunny  = "bunny.mp3",
        victor = "victor.mp3",
        introa = "intro.mp3",
        runnin = "runnin.mp3",
        stalks = "stalks.mp3",
        countd = "countd.mp3",
        betwee = "betwee.mp3",
        doom   = "doom.mp3",
        the_da = "the_da.mp3",
        shawn  = "shawn.mp3",
        ddtblu = "ddtblu.mp3",
        in_cit = "in_cit.mp3",
        dead   = "dead.mp3",
        stlks2 = "stalks.mp3",
        theda2 = "the_da.mp3",
        doom2  = "doom.mp3",
        ddtbl2 = "ddtblu.mp3",
        runni2 = "runnin.mp3",
        dead2  = "dead.mp3",
        stlks3 = "stalks.mp3",
        romero = "romero.mp3",
        shawn2 = "shawn.mp3",
        messag = "messag.mp3",
        count2 = "countd.mp3",
        ddtbl3 = "ddtblu.mp3",
        ampie  = "ampie.mp3",
        theda3 = "the_da.mp3",
        adrian = "adrian.mp3",
        messg2 = "messag.mp3",
        romer2 = "romero.mp3",
        tense  = "tense.mp3",
        shawn3 = "shawn.mp3",
        openin = "openin.mp3",
        evil   = "evil.mp3",
        ultima = "ultima.mp3",
        read_m = "read_m.mp3",
        dm2ttl = "dm2ttl.mp3",
        dm2int = "dm2int.mp3"
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
