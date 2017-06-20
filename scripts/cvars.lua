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

local cvar = require('config').cvar

cvar {
    name = 'system.process_priority',
    help = 'Sets the priority of the D2K process',
    default = 0,
    min = 0,
    max = 0,
}

cvar {
    name = 'system.render_smp',
    help = 'Use multiple processor to render',
    default = false
}

cvar {
    name = 'system.try_to_reduce_cpu_cache_misses',
    help = [[
        Run quick diagnostics to adjust the size of internal data structures
        to try and avoid CPU cache misses (slowdowns)
    ]],
    default = true
}

cvar {
    name = 'misc.default_compatibility_level',
    help = 'Default compatibility level',
    default = -1,
    min = -1,
    max = d2k.Compatibility.max
}

cvar {
    name = 'misc.realtic_clock_rate',
    help = 'Game speed (percentage)',
	default = 100,
	min = 0
}

cvar {
    name = 'misc.menu_background',
    help = 'Draw menu background',
	default = true
}

cvar {
    name = 'misc.max_player_corpse',
    help = 'Maximum player corpse count',
	default = 32,
	min = -1
}

cvar {
    name = 'misc.flashing_hom',
    help = 'Flash HoMs when detected',
	default = false
}

cvar {
    name = 'misc.demo_insurance',
    help = 'Demo insurance mode (0 = disabled)',
	default = 2,
	min = 0,
	max = 2
}

cvar {
    name = 'misc.endoom_mode',
    help = 'Used to affect endoom properties, no longer active',
	default = 5,
	min = 0,
	max = 7
}

cvar {
    name = 'misc.level_precache',
    help = 'Precache level data',
	default = true
}

cvar {
    name = 'misc.demo_smoothturns',
    help = 'Smooth out turns when recording demos',
	default = false
}

cvar {
    name = 'misc.demo_smoothturnsfactor',
    help = 'Turn smoothing strength',
    default = 6,
    min = 1,
    max = d2k.Demo.smooth_playing_max_factor
}

cvar {
    name = 'misc.showendoom',
    help = 'Enable endoom',
	default = false
}

cvar {
    name = 'misc.screenshot_dir',
    help = 'Folder to store screenshots in',
	default = ''
}

cvar {
    name = 'misc.health_bar',
    help = 'Show health bars over things with health',
	default = false
}

cvar {
    name = 'misc.health_bar_full_length',
    help = 'Show health bar outline when not full',
	default = true
}

cvar {
    name = 'misc.health_bar_red',
    help = 'Threshold at which healthbars turn red',
	default = 50,
	min = 0,
	max = 100
}

cvar {
    name = 'misc.health_bar_yellow',
    help = 'Threshold at which healthbars turn yellow',
	default = 99,
	min = 0,
	max = 100
}

cvar {
    name = 'misc.health_bar_green',
    help = 'Threshold at which healthbars turn green',
	default = 0,
	min = 0,
	max = 100
}

cvar {
    name = 'files.wadfile_1',
    help = 'WAD file to preload (1 of 2)',
	default = ''
}

cvar {
    name = 'files.wadfile_2',
    help = 'WAD file to preload (2 of 2)',
	default = ''
}

cvar {
    name = 'files.dehfile_1',
    help = 'DeHackEd file to preload (1 of 2)',
	default = ''
}

cvar {
    name = 'files.dehfile_2',
    help = 'DeHackEd file to preload (2 of 2)',
	default = ''
}


cvar {
    name = 'game.default_skill',
    help = "Default skill level, 1 = I'm Too Young To Die, 5 = Nightmare",
	default = 3,
	min = 1,
	max = 5
}

cvar {
    name = 'game.weapon_recoil',
    help = 'Enable weapon recoil',
	default = false
}

cvar {
    name = 'game.doom_weapon_toggles',
    help = 'Looks old, dunno',
	default = true
}

cvar {
    name = 'game.player_bobbing',
    help = 'Enable player movement bobbing',
	default = true
}

cvar {
    name = 'game.leave_weapons',
    help = 'Leave weapons if picked up',
	default = false
}

cvar {
    name = 'game.monsters_remember',
    help = 'Monsters will remember last target if no other target present',
	default = true
}

cvar {
    name = 'game.monster_infighting',
    help = 'Monsters will fight amongst themselves',
	default = true
}

cvar {
    name = 'game.monster_backing',
    help = 'Monsters will back away from melee attacks',
	default = false
}

cvar {
    name = 'game.monster_avoid_hazards',
    help = 'Monsters will avoid hazards',
	default = true
}

cvar {
    name = 'game.monsters_climb',
    help = 'Monsters will traverse steep stairs',
	default = false
}

cvar {
    name = 'game.monster_friction',
    help = 'Monsters will be affected by friction effects',
	default = true
}

cvar {
    name = 'game.help_friends',
    help = [[
        Friendly monsters will go to the aid of other dying friendly monsters
    ]],
	default = false
}

cvar {
    name = 'game.allow_pushers',
    help = 'Enable push effects (wind, current, etc.)',
	default = true
}

cvar {
    name = 'game.variable_friction',
    help = 'Enable friction effects (ice, mud, etc.)',
	default = true
}

cvar {
    name = 'game.player_helpers',
    help = 'Number of friendly monsters to spawn',
	default = 0,
    min = 0,
    max = 3
}

cvar {
    name = 'game.friend_distance',
    help = 'Minimum friendly monster distance',
	default = 128,
	min = 0,
	max = 999
}

cvar {
    name = 'game.dog_jumping',
    help = 'Allow friendly monsters to jump',
	default = true
}

cvar {
    name = 'game.sts_always_red',
    help = 'Always use red numbers on the status bar',
	default = true
}

cvar {
    name = 'game.sts_pct_always_gray',
    help = 'Always use gray percent marks on the status bar',
	default = false
}

cvar {
    name = 'game.sts_traditional_keys',
    help = 'Use traditional keys on the status bar',
	default = false
}

cvar {
    name = 'game.show_messages',
    help = 'Enable HUD messages',
	default = true
}

cvar {
    name = 'game.autorun',
    help = 'Always run',
	default = true
}

cvar {
    name = 'game.speed_step',
    help = 'Amount of speed to increase/decrease when changing speed',
	default = 0,
	min = 0,
	max = 1000
}

cvar {
    name = 'game.movement_strafe50',
    help = 'Always SR50',
	default = false
}

cvar {
    name = 'game.movement_strafe50onturns',
    help = 'SR50 on turns',
	default = false
}

cvar {
    name = 'game.movement_shorttics',
    help = 'Use shorttics even when not recording a demo',
	default = false
}

cvar {
    name = 'game.interpolation_maxobjects',
    help = [[
        Maximum number of objects to interpolate, for a given type (sector
        floors, sector ceilings, panning walls, panning floors, and panning
        ceilings)
    ]],
	default = 0,
	min = 0
}

cvar {
    name = 'dehacked.deh_apply_cheats',
    help = 'Accept cheats in DeHackEd files',
	default = true
}

cvar {
    name = 'compatibility.comp_zombie',
	default = true
}

cvar {
    name = 'compatibility.comp_infcheat',
	default = false
}

cvar {
    name = 'compatibility.comp_stairs',
	default = false
}

cvar {
    name = 'compatibility.comp_telefrag',
	default = false
}

cvar {
    name = 'compatibility.comp_dropoff',
	default = false
}

cvar {
    name = 'compatibility.comp_falloff',
	default = false
}

cvar {
    name = 'compatibility.comp_staylift',
	default = false
}

cvar {
    name = 'compatibility.comp_doorstuck',
	default = false
}

cvar {
    name = 'compatibility.comp_pursuit',
	default = false
}

cvar {
    name = 'compatibility.comp_vile',
	default = false
}

cvar {
    name = 'compatibility.comp_pain',
	default = false
}

cvar {
    name = 'compatibility.comp_skull',
	default = false
}

cvar {
    name = 'compatibility.comp_blazing',
	default = false
}

cvar {
    name = 'compatibility.comp_doorlight',
	default = false
}

cvar {
    name = 'compatibility.comp_god',
	default = false
}

cvar {
    name = 'compatibility.comp_skymap',
	default = false
}

cvar {
    name = 'compatibility.comp_floors',
	default = false
}

cvar {
    name = 'compatibility.comp_model',
	default = false
}

cvar {
    name = 'compatibility.comp_zerotags',
	default = false
}

cvar {
    name = 'compatibility.comp_moveblock',
	default = false
}

cvar {
    name = 'compatibility.comp_sound',
	default = false
}

cvar {
    name = 'compatibility.comp_666',
	default = false
}

cvar {
    name = 'compatibility.comp_soul',
	default = false
}

cvar {
    name = 'compatibility.comp_maskedanim',
	default = false
}

cvar {
    name = 'compatibility.comp_ouchface',
	default = false
}

cvar {
    name = 'compatibility.comp_maxhealth',
	default = false
}

cvar {
    name = 'compatibility.comp_translucency',
	default = false
}

cvar {
    name = 'compatibility.comperr_zerotag',
	default = false
}

cvar {
    name = 'compatibility.comperr_passuse',
	default = false
}

cvar {
    name = 'compatibility.comperr_hangsolid',
	default = false
}

cvar {
    name = 'compatibility.comperr_blockmap',
	default = false
}

cvar {
    name = 'compatibility.comperr_allowjump',
	default = false
}

cvar {
    name = 'sound.snd_pcspeaker',
	default = false
}

cvar {
    name = 'sound.sound_card',
	default = -1,
	min = -1,
	max = 7
}

cvar {
    name = 'sound.music_card',
	default = -1,
	min = -1,
	max = 9
}

cvar {
    name = 'sound.pitched_sounds',
	default = false
}

cvar {
    name = 'sound.samplerate',
	default = 22050,
	min = 11025,
	max = 48000
}

cvar {
    name = 'sound.sfx_volume',
	default = 8,
	min = 0,
	max = 15
}

cvar {
    name = 'sound.mus_pause_opt',
	default = 1,
	min = 0,
	max = 2
}

cvar {
    name = 'sound.snd_channels',
    default = d2k.Sound.max_channels,
    min = 1,
    max = d2k.Sound.max_channels
}

cvar {
    name = 'sound.snd_midiplayer',
	default = 'sdl'
}

cvar {
    name = 'sound.snd_soundfont',
	default = 'TimGM6mb.sf2'
}

cvar {
    name = 'sound.snd_mididev',
	default = ''
}

cvar {
    name = 'sound.mus_extend_volume',
	default = false
}

cvar {
    name = 'sound.mus_fluidsynth_gain',
	default = 50,
	min = 0,
	max = 1000
}

cvar {
    name = 'sound.mus_opl_gain',
	default = 50,
	min = 0,
	max = 1000
}

cvar {
    name = 'video.videomode',
	default = '32',
}

cvar {
    name = 'video.use_gl_surface',
	default = false
}

cvar {
    name = 'video.screen_resolution',
	default = '640x480'
}

cvar {
    name = 'video.use_fullscreen',
	default = false
}

cvar {
    name = 'video.use_doublebuffer',
	default = true
}

cvar {
    name = 'video.translucency',
	default = true
}

cvar {
    name = 'video.tran_filter_pct',
	default = 66,
	min = 0,
	max = 100
}

cvar {
    name = 'video.screenblocks',
	default = 10,
	min = 3,
	max = 11
}

cvar {
    name = 'video.usegamma',
	default = 3,
	min = 0,
	max = 4
}

cvar {
    name = 'video.uncapped_framerate',
	default = true
}

cvar {
    name = 'video.test_interpolation_method',
	default = false
}

cvar {
    name = 'video.filter_wall',
    default = d2k.Renderer.filter_point,
    min = d2k.Renderer.filter_point,
    max = d2k.Renderer.filter_rounded
}

cvar {
    name = 'video.filter_floor',
    default = d2k.Renderer.filter_point,
    min = d2k.Renderer.filter_point,
    max = d2k.Renderer.filter_rounded
}

cvar {
    name = 'video.filter_sprite',
    default = d2k.Renderer.filter_point,
    min = d2k.Renderer.filter_point,
    max = d2k.Renderer.filter_rounded
}

cvar {
    name = 'video.filter_z',
    default = d2k.Renderer.filter_point,
    min = d2k.Renderer.filter_point,
    max = d2k.Renderer.filter_linear
}

cvar {
    name = 'video.filter_patch',
    default = d2k.Renderer.filter_point,
    min = d2k.Renderer.filter_point,
    max = d2k.Renderer.filter_rounded
}

cvar {
    name = 'video.filter_threshold',
	default = 49152,
	min = 0
}

cvar {
    name = 'video.sprite_edges',
    default = d2k.Renderer.masked_column_edge_square,
    min = d2k.Renderer.masked_column_edge_square,
    max = d2k.Renderer.masked_column_edge_sloped
}

cvar {
    name = 'video.patch_edges',
    default = d2k.Renderer.masked_column_edge_square,
    min = d2k.Renderer.masked_column_edge_square,
    max = d2k.Renderer.masked_column_edge_sloped
}

cvar {
    name = 'video.sdl_videodriver',
	default = 'default'
}

cvar {
    name = 'video.sdl_video_window_pos',
	default = 'center'
}

cvar {
    name = 'video.palette_ondamage',
	default = true
}

cvar {
    name = 'video.palette_onbonus',
	default = true
}

cvar {
    name = 'video.palette_onpowers',
	default = true
}

cvar {
    name = 'video.render_wipescreen',
	default = true
}

cvar {
    name = 'video.render_screen_multiply',
	default = 1,
	min = 1,
	max = 4
}

cvar {
    name = 'video.render_interlaced_scanning',
	default = false
}

cvar {
    name = 'video.render_precise_high_quality',
	default = true
}

cvar {
    name = 'video.render_aspect',
	default = 0,
	min = 0,
	max = 4
}

cvar {
    name = 'video.render_doom_lightmaps',
	default = false
}

cvar {
    name = 'video.fake_contrast',
	default = true
}

cvar {
    name = 'video.render_stretch_hud',
    default = d2k.Renderer.patch_stretch_16x10,
    min = d2k.Renderer.patch_stretch_16x10,
    max = d2k.Renderer.patch_stretch_full
}

cvar {
    name = 'video.render_patches_scalex',
	default = 0,
	min = 0,
	max = 16
}

cvar {
    name = 'video.render_patches_scaley',
	default = 0,
	min = 0,
	max = 16
}

cvar {
    name = 'video.render_stretchsky',
	default = true
}

cvar {
    name = 'video.sprites_doom_order',
    default = d2k.Renderer.sprite_order_static,
    min = d2k.Renderer.sprite_order_none,
    max = d2k.Renderer.sprite_order_dynamic
}

cvar {
    name = 'video.movement_mouselook',
	default = false
}

cvar {
    name = 'video.movement_maxviewpitch',
	default = 90,
	min = 0,
	max = 90
}

cvar {
    name = 'video.movement_mouseinvert',
	default = false
}


cvar {
    name = 'opengl.gl_compatibility',
	default = false
}

cvar {
    name = 'opengl.gl_arb_multitexture',
	default = true
}

cvar {
    name = 'opengl.gl_arb_texture_compression',
	default = true
}

cvar {
    name = 'opengl.gl_arb_texture_non_power_of_two',
	default = true
}

cvar {
    name = 'opengl.gl_ext_arb_vertex_buffer_object',
	default = true
}

cvar {
    name = 'opengl.gl_arb_pixel_buffer_object',
	default = true
}

cvar {
    name = 'opengl.gl_arb_shader_objects',
	default = true
}

cvar {
    name = 'opengl.gl_ext_blend_color',
	default = true
}

cvar {
    name = 'opengl.gl_arb_framebuffer_object',
	default = true
}

cvar {
    name = 'opengl.gl_ext_packed_depth_stencil',
	default = true
}

cvar {
    name = 'opengl.gl_ext_texture_filter_anisotropic',
	default = true
}

cvar {
    name = 'opengl.gl_use_stencil',
	default = true
}

cvar {
    name = 'opengl.gl_use_display_lists',
	default = false
}

cvar {
    name = 'opengl.gl_vsync',
	default = true
}

cvar {
    name = 'opengl.gl_clear',
	default = false
}

cvar {
    name = 'opengl.gl_ztrick',
	default = false
}

cvar {
    name = 'opengl.gl_nearclip',
	default = 5,
	min = 0
}

cvar {
    name = 'opengl.gl_colorbuffer_bits',
	default = 32,
	min = 16,
	max = 32
}

cvar {
    name = 'opengl.gl_depthbuffer_bits',
	default = 24,
	min = 16,
	max = 32
}

cvar {
    name = 'opengl.gl_texture_filter',
    default = d2k.Renderer.gl_filter_linear_mipmap_linear,
    min = d2k.Renderer.gl_filter_nearest,
    max = d2k.Renderer.gl_filter_linear_mipmap_linear
}

cvar {
    name = 'opengl.gl_sprite_filter',
    default = d2k.Renderer.gl_filter_linear,
    min = d2k.Renderer.gl_filter_nearest,
    max = d2k.Renderer.gl_filter_linear_mipmap_nearest
}

cvar {
    name = 'opengl.gl_patch_filter',
    default = d2k.Renderer.gl_filter_linear,
    min = d2k.Renderer.gl_filter_nearest,
    max = d2k.Renderer.gl_filter_linear
}

cvar {
    name = 'opengl.gl_texture_filter_anisotropic',
    default = d2k.Renderer.anisotropic_8x,
    min = d2k.Renderer.anisotropic_off,
    max = d2k.Renderer.anisotropic_16x
}

cvar {
    name = 'opengl.gl_tex_format_string',
	default = 'GL_RGBA'
}

cvar {
    name = 'opengl.gl_sprite_offset',
	default = 0,
	min = 0,
	max = 5
}

cvar {
    name = 'opengl.gl_sprite_blend',
	default = false
}

cvar {
    name = 'opengl.gl_mask_sprite_threshold',
	default = 50,
	min = 0,
	max = 100
}

cvar {
    name = 'opengl.gl_skymode',
    default = d2k.Renderer.skytype_auto,
    min = d2k.Renderer.skytype_auto,
    max = d2k.Renderer.skytype_screen
}

cvar {
    name = 'opengl.gl_sky_detail',
	default = 16,
	min = 1,
	max = 32
}

cvar {
    name = 'opengl.gl_use_paletted_texture',
	default = false
}

cvar {
    name = 'opengl.gl_use_shared_texture_palette',
	default = false
}

cvar {
    name = 'opengl.gl_allow_detail_textures',
	default = true
}

cvar {
    name = 'opengl.gl_detail_maxdist',
	default = 0,
	min = 0,
	max = 65535
}

cvar {
    name = 'opengl.render_multisampling',
	default = 0,
	min = 0,
	max = 8
}

cvar {
    name = 'opengl.render_fov',
	default = 90,
	min = 20,
	max = 160
}

cvar {
    name = 'opengl.gl_spriteclip',
    default = d2k.Renderer.spriteclip_smart,
    min = d2k.Renderer.spriteclip_const,
    max = d2k.Renderer.spriteclip_smart
}

cvar {
    name = 'opengl.gl_spriteclip_threshold',
	default = 10,
	min = 0,
	max = 100
}

cvar {
    name = 'opengl.gl_sprites_frustum_culling',
	default = true
}

cvar {
    name = 'opengl.render_paperitems',
	default = false
}

cvar {
    name = 'opengl.gl_boom_colormaps',
	default = true
}

cvar {
    name = 'opengl.gl_hires_24bit_colormap',
	default = false
}

cvar {
    name = 'opengl.gl_texture_internal_hires',
	default = true
}

cvar {
    name = 'opengl.gl_texture_external_hires',
	default = false
}

cvar {
    name = 'opengl.gl_hires_override_pwads',
	default = false
}

cvar {
    name = 'opengl.gl_texture_hires_dir',
	default = ''
}

cvar {
    name = 'opengl.gl_texture_hqresize',
	default = false
}

cvar {
    name = 'opengl.gl_texture_hqresize_textures',
    default = d2k.Renderer.hq_scale_2x,
    min = d2k.Renderer.hq_scale_none,
    max = d2k.Renderer.hq_scale_4x
}

cvar {
    name = 'opengl.gl_texture_hqresize_sprites',
    default = d2k.Renderer.hq_scale_none,
    min = d2k.Renderer.hq_scale_none,
    max = d2k.Renderer.hq_scale_4x
}

cvar {
    name = 'opengl.gl_texture_hqresize_patches',
    default = d2k.Renderer.hq_scale_2x,
    min = d2k.Renderer.hq_scale_none,
    max = d2k.Renderer.hq_scale_4x
}

cvar {
    name = 'opengl.gl_motionblur',
	default = false
}

cvar {
    name = 'opengl.gl_motionblur_min_speed',
	default = 21.36
}

cvar {
    name = 'opengl.gl_motionblur_min_angle',
	default = 20.0
}

cvar {
    name = 'opengl.gl_motionblur_att_a',
	default = 55.0
}

cvar {
    name = 'opengl.gl_motionblur_att_b',
	default = 1.8
}

cvar {
    name = 'opengl.gl_motionblur_att_c',
	default = 0.9
}

cvar {
    name = 'opengl.gl_lightmode',
    default = d2k.Renderer.lightmode_glboom,
    min = d2k.Renderer.lightmode_glboom,
    max = d2k.Renderer.lightmode_shaders
}

cvar {
    name = 'opengl.gl_light_ambient',
	default = 20,
	min = 1,
	max = 255
}

cvar {
    name = 'opengl.gl_fog',
	default = true
}

cvar {
    name = 'opengl.gl_fog_color',
	default = 0,
	min = 0,
	max = 16777215
}

cvar {
    name = 'opengl.useglgamma',
    default = 6,
    pmin = 0,
    max = d2k.Renderer.max_gl_gamma
}

cvar {
    name = 'opengl.gl_color_mip_levels',
	default = false
}

cvar {
    name = 'opengl.gl_shadows',
	default = false
}

cvar {
    name = 'opengl.gl_shadows_maxdist',
	default = 1000,
	min = 0,
	max = 32767
}

cvar {
    name = 'opengl.gl_shadows_factor',
	default = 128,
	min = 0,
	max = 255
}

cvar {
    name = 'opengl.gl_blend_animations',
	default = false
}

cvar {
    name = 'mouse.enabled',
	default = true
}

cvar {
    name = 'mouse.horizontal_sensitivity_horiz',
	default = 10,
	min = 0
}

cvar {
    name = 'mouse.vertical_sensitivity',
	default = 10,
	min = 0
}

cvar {
    name = 'mouse.fire',
    default = 0,
    min = -1,
    max = d2k.Mouse.max_buttons
}

cvar {
    name = 'mouse.strafe',
    default = 1,
    min = -1,
    max = d2k.Mouse.max_buttons
}

cvar {
    name = 'mouse.forward',
    default = 2,
    min = -1,
    max = d2k.Mouse.max_buttons
}

cvar {
    name = 'mouse.backward',
    default = -1,
    min = -1,
    max = d2k.Mouse.max_buttons
}

cvar {
    name = 'mouse.use',
    default = -1,
    min = -1,
    max = d2k.Mouse.max_buttons
}

cvar {
    name = 'mouse.acceleration',
	default = 0,
	min = 0
}

cvar {
    name = 'mouse.mouselook_sensitivity',
	default = 10,
	min = 0
}

cvar {
    name = 'mouse.doubleclick_as_use',
	default = true
}

cvar {
    name = 'keybindings.right',
	default = 'right'
}

cvar {
    name = 'keybindings.left',
	default = 'left'
}

cvar {
    name = 'keybindings.up',
	default = 'w'
}

cvar {
    name = 'keybindings.down',
	default = 's'
}

cvar {
    name = 'keybindings.mlook',
	default = '\\'
}

cvar {
    name = 'keybindings.help',
	default = 'f1'
}

cvar {
    name = 'keybindings.menu.toggle',
	default = 'escape'
}

cvar {
    name = 'keybindings.menu.right',
	default = 'right'
}

cvar {
    name = 'keybindings.menu.left',
	default = 'left'
}

cvar {
    name = 'keybindings.menu.up',
	default = 'up'
}

cvar {
    name = 'keybindings.menu.down',
	default = 'down'
}

cvar {
    name = 'keybindings.menu.backspace',
	default = 'backspace'
}

cvar {
    name = 'keybindings.menu.escape',
	default = 'escape'
}

cvar {
    name = 'keybindings.menu.enter',
	default = 'enter'
}

cvar {
    name = 'keybindings.setup',
	default = '0'
}

cvar {
    name = 'keybindings.strafeleft',
	default = 'a'
}

cvar {
    name = 'keybindings.straferight',
	default = 'd'
}

cvar {
    name = 'keybindings.flyup',
	default = '.'
}

cvar {
    name = 'keybindings.flydown',
	default = ','
}

cvar {
    name = 'keybindings.fire',
	default = 'left_control'
}

cvar {
    name = 'keybindings.use',
	default = ' '
}

cvar {
    name = 'keybindings.strafe',
	default = 'left_alt'
}

cvar {
    name = 'keybindings.speed',
	default = 'left_shift'
}

cvar {
    name = 'keybindings.savegame',
	default = 'f2'
}

cvar {
    name = 'keybindings.loadgame',
	default = 'f3'
}

cvar {
    name = 'keybindings.soundvolume',
	default = 'f4'
}

cvar {
    name = 'keybindings.hud',
	default = 'f5'
}

cvar {
    name = 'keybindings.quicksave',
	default = 'f6'
}

cvar {
    name = 'keybindings.endgame',
	default = 'f7'
}

cvar {
    name = 'keybindings.messages',
	default = 'f8'
}

cvar {
    name = 'keybindings.quickload',
	default = 'f9'
}

cvar {
    name = 'keybindings.quit',
	default = 'f10'
}

cvar {
    name = 'keybindings.gamma',
	default = 'f11'
}

cvar {
    name = 'keybindings.spy',
	default = 'f12'
}

cvar {
    name = 'keybindings.pause',
	default = 'pause'
}

cvar {
    name = 'keybindings.autorun',
	default = 'caps_lock'
}

cvar {
    name = 'keybindings.chat',
	default = 't'
}

cvar {
    name = 'keybindings.backspace',
	default = 'backspace'
}

cvar {
    name = 'keybindings.enter',
	default = 'enter'
}

cvar {
    name = 'keybindings.map.toggle',
	default = 'tab'
}

cvar {
    name = 'keybindings.map.right',
	default = 'right'
}

cvar {
    name = 'keybindings.map.left',
	default = 'left'
}

cvar {
    name = 'keybindings.map.up',
	default = 'up'
}

cvar {
    name = 'keybindings.map.down',
	default = 'down'
}

cvar {
    name = 'keybindings.map.zoomin',
	default = '='
}

cvar {
    name = 'keybindings.map.zoomout',
	default = '-'
}

cvar {
    name = 'keybindings.map.gobig',
	default = '0'
}

cvar {
    name = 'keybindings.map.follow',
	default = 'f'
}

cvar {
    name = 'keybindings.map.mark',
	default = 'm'
}

cvar {
    name = 'keybindings.map.clear',
	default = 'c'
}

cvar {
    name = 'keybindings.map.grid',
	default = 'g'
}

cvar {
    name = 'keybindings.map.rotate',
	default = 'r'
}

cvar {
    name = 'keybindings.map.overlay',
	default = 'o'
}

cvar {
    name = 'keybindings.map.textured',
	default = '0'
}

cvar {
    name = 'keybindings.reverse',
	default = '/'
}

cvar {
    name = 'keybindings.zoomin',
	default = '='
}

cvar {
    name = 'keybindings.zoomout',
	default = '-'
}

cvar {
    name = 'keybindings.chat.player1',
	default = 'g'
}

cvar {
    name = 'keybindings.chat.player2',
	default = 'i'
}

cvar {
    name = 'keybindings.chat.player3',
	default = 'b'
}

cvar {
    name = 'keybindings.chat.player4',
	default = 'r'
}

cvar {
    name = 'keybindings.weapontoggle',
	default = '0'
}

cvar {
    name = 'keybindings.weapon1',
	default = '1'
}

cvar {
    name = 'keybindings.weapon2',
	default = '2'
}

cvar {
    name = 'keybindings.weapon3',
	default = '3'
}

cvar {
    name = 'keybindings.weapon4',
	default = '4'
}

cvar {
    name = 'keybindings.weapon5',
	default = '5'
}

cvar {
    name = 'keybindings.weapon6',
	default = '6'
}

cvar {
    name = 'keybindings.weapon7',
	default = '7'
}

cvar {
    name = 'keybindings.weapon8',
	default = '8'
}

cvar {
    name = 'keybindings.weapon9',
	default = '9'
}

cvar {
    name = 'keybindings.nextweapon',
	default = 'wheel_up'
}

cvar {
    name = 'keybindings.prevweapon',
	default = 'wheel_down'
}

cvar {
    name = 'keybindings.screenshot',
	default = '*'
}

cvar {
    name = 'keybindings.speedup',
	default = 'kp8'
}

cvar {
    name = 'keybindings.speeddown',
	default = 'kp2'
}

cvar {
    name = 'keybindings.speeddefault',
	default = 'kp5'
}

cvar {
    name = 'keybindings.demo_skip',
	default = 'kp6'
}

cvar {
    name = 'keybindings.level_restart',
	default = 'kp8'
}

cvar {
    name = 'keybindings.demo_endlevel',
	default = 'kp1'
}

cvar {
    name = 'keybindings.nextlevel',
	default = 'kp6'
}

cvar {
    name = 'keybindings.demo_jointogame',
	default = 'kp_enter'
}

cvar {
    name = 'keybindings.walkcamera',
	default = 'kp_multiply'
}

cvar {
    name = 'keybindings.showalive',
	default = 'kp_divide'
}

cvar {
    name = 'joystick.mode',
	default = 0,
	min = 0,
	max = 2
}

cvar {
    name = 'joystick.left',
	default = 0
}

cvar {
    name = 'joystick.right',
	default = 0
}

cvar {
    name = 'joystick.up',
	default = 0
}

cvar {
    name = 'joystick.down',
	default = 0
}

cvar {
    name = 'joystick.fire',
	default = 0
}

cvar {
    name = 'joystick.strafe',
	default = 1,
	min = 0
}

cvar {
    name = 'joystick.speed',
	default = 2,
	min = 0
}

cvar {
    name = 'joystick.use',
	default = 3,
	min = 0
}

cvar {
    name = 'joystick.strafeleft',
	default = 4,
	min = 0
}

cvar {
    name = 'joystick.straferight',
	default = 5,
	min = 0
}

cvar {
    name = 'chat_macros',
    default = {
        'No',
        "I'm ready to kick butt!",
        "I'm OK.",
        "I'm not looking too good!",
        'Help!',
        'You suck!',
        'Next time, scumbag...',
        'Come here!',
        "I'll take care of it.",
        'Yes'
    }
}

cvar {
    name = 'automap.color.background',
	default = 247,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.grid',
	default = 104,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.wall',
	default = 23,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.floor_height_change',
	default = 55,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.ceiling_height_change',
    default = 215,
    pmin = 0,
    max = 255
}

cvar {
    name = 'automap.color.floor_equals_ceiling',
    default = 208,
    min = 0,
    max = 255
}

cvar {
    name = 'automap.color.red_key',
	default = 175,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.blue_key',
	default = 204,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.yellow_key',
	default = 231,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.red_door',
	default = 175,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.blue_door',
	default = 204,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.yellow_door',
	default = 231,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.teleporter',
	default = 119,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.secret',
	default = 252,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.exit',
	default = 0,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.unseen',
	default = 104,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.no_floor_or_ceiling_changes',
    default = 88,
    min = 0,
    max = 255
}

cvar {
    name = 'automap.color.sprite',
	default = 112,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.item',
	default = 231,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.crosshair',
	default = 208,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.singleplayer_arrow',
	default = 208,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.multiplayer_self_color',
    default = 112,
    min = 0,
    max = 255
}

cvar {
    name = 'automap.color.enemy_sprite',
	default = 177,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.color.friends',
	default = 112,
	min = 0,
	max = 255
}

cvar {
    name = 'automap.only_show_secrets_after_entering',
	default = false
}

cvar {
    name = 'automap.show_coordinates',
	default = false
}

cvar {
    name = 'automap.show_stats',
	default = true
}

cvar {
    name = 'automap.mode',
	default = false
}

cvar {
    name = 'automap.always_updates',
	default = true
}

cvar {
    name = 'automap.grid_size',
	default = 128,
	min = 8,
	max = 256
}

cvar {
    name = 'automap.scroll_speed',
	default = 8,
	min = 1,
	max = 32
}

cvar {
    name = 'automap.wheel_zoom',
	default = true
}

cvar {
    name = 'automap.use_multisampling',
	default = true
}

cvar {
    name = 'automap.textured.enabled',
	default = true
}

cvar {
    name = 'automap.textured.translucency',
	default = 100,
	min = 0,
	max = 100
}

cvar {
    name = 'automap.textured.overlay_translucency',
    default = 66,
    min = 0,
    max = 100
}

cvar {
    name = 'automap.textured.overlay.line_translucency',
	default = 100,
	min = 0,
	max = 100
}

cvar {
    name = 'automap.textured.overlay.pos_x',
	default = 0,
	min = 0,
	max = 319
}

cvar {
    name = 'automap.textured.overlay.pos_y',
	default = 0,
	min = 0,
	max = 199
}

cvar {
    name = 'automap.textured.overlay.pos_width',
	default = 320,
	min = 0,
	max = 320
}

cvar {
    name = 'automap.textured.overlay.pos_height',
	default = 200,
	min = 0,
	max = 200
}

cvar {
    name = 'automap.textured.map_things_appearance',
    default = d2k.AutoMap.map_things_appearance_icon,
    min = d2k.AutoMap.map_things_appearance_classic,
    max = d2k.AutoMap.map_things_appearance_icon
}


cvar {
    name = 'hud.color.automap_title',
	default = 5,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.color.automap_coordinates',
	default = 3,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.color.mapstat_title',
	default = 6,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.color.mapstat_value',
	default = 2,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.color.mapstat_time',
	default = 2,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.color.messages',
	default = 6,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.color.chat',
	default = 5,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.message_review.size',
	default = 1,
	min = 1,
	max = 16
}

cvar {
    name = 'hud.message_review.color',
	default = 5,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.message_review.display_background',
	default = false
}

cvar {
    name = 'hud.health.threshold.red',
	default = 25,
	min = 0,
	max = 200
}

cvar {
    name = 'hud.health.threshold.yellow',
	default = 50,
	min = 0,
	max = 200
}

cvar {
    name = 'hud.health.threshold.green',
	default = 100,
	min = 0,
	max = 200
}

cvar {
    name = 'hud.armor.threshold.red',
	default = 25,
	min = 0,
	max = 200
}

cvar {
    name = 'hud.armor.threshold.yellow',
	default = 50,
	min = 0,
	max = 200
}

cvar {
    name = 'hud.armor.threshold.green',
	default = 100,
	min = 0,
	max = 200
}

cvar {
    name = 'hud.ammo.threshold.red',
	default = 25,
	min = 0,
	max = 100
}

cvar {
    name = 'hud.ammo.threshold.yellow',
	default = 50,
	min = 0,
	max = 100
}

cvar {
    name = 'hud.ammo.threshold.colour_behaviour',
    default = d2k.StatusBar.ammo_colour_behaviour_yes,
    min = d2k.StatusBar.ammo_colour_behaviour_no,
    max = d2k.StatusBar.ammo_colour_behaviour_yes
}

cvar {
    name = 'hud.num',
	default = 6,
	min = 0,
	max = 100
}

cvar {
    name = 'hud.enabled',
	default = false
}

cvar {
    name = 'hud.show_gamespeed',
	default = false
}

cvar {
    name = 'hud.show_leveltime',
	default = false
}

cvar {
    name = 'hud.show_demotime',
	default = false
}

cvar {
    name = 'hud.show_secretarea',
	default = false
}

cvar {
    name = 'hud.show_smarttotals',
	default = false
}

cvar {
    name = 'hud.show_demoprogressbar',
	default = true
}

cvar {
    name = 'hud.crosshair.mode',
    default = 0,
    min = 0,
    max = d2k.Video.crosshair_count - 1
}

cvar {
    name = 'hud.crosshair.scale',
	default = false
}

cvar {
    name = 'hud.crosshair.color',
	default = 3,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.crosshair.health',
	default = false
}

cvar {
    name = 'hud.crosshair.target',
	default = false
}

cvar {
    name = 'hud.crosshair.target_color',
	default = 9,
	min = 0,
	max = 9
}

cvar {
    name = 'hud.crosshair.lock_target',
	default = false
}


cvar {
    name = 'demos.extendedformat',
	default = true
}

cvar {
    name = 'demos.demoex_filename',
	default = ''
}

cvar {
    name = 'demos.getwad_cmdline',
	default = ''
}

cvar {
    name = 'demos.overwriteexisting',
	default = true
}

cvar {
    name = 'video_recording.command.sound',
	default = 'oggenc2 -r -R %s -q 5 - -o output.ogg'
}

cvar {
    name = 'video_recording.command.video',
    default = [[
        x264 -o output.mp4 --crf 22 --muxer mp4 --demuxer raw --input-csp rgb
        --input-depth 8 --input-res %wx%h --fps 35 -
    ]]
}

cvar {
    name = 'video_recording.command.mux',
	default = 'mkvmerge -o %f output.mp4 output.ogg'
}

cvar {
    name = 'video_recording.cap_tempfile1',
	default = 'output.ogg'
}

cvar {
    name = 'video_recording.cap_tempfile2',
	default = 'output.mp4'
}

cvar {
    name = 'video_recording.cap_remove_tempfiles',
	default = true
}

cvar {
    name = 'overrun.spechit_warn',
	default = false
}

cvar {
    name = 'overrun.spechit_emulate',
	default = true
}

cvar {
    name = 'overrun.reject_warn',
	default = false
}

cvar {
    name = 'overrun.reject_emulate',
	default = true
}

cvar {
    name = 'overrun.intercept_warn',
	default = false
}

cvar {
    name = 'overrun.intercept_emulate',
	default = true
}

cvar {
    name = 'overrun.playeringame_warn',
	default = false
}

cvar {
    name = 'overrun.playeringame_emulate',
	default = true
}

cvar {
    name = 'overrun.donut_warn',
	default = false
}

cvar {
    name = 'overrun.donut_emulate',
	default = false
}

cvar {
    name = 'overrun.missedbackside_warn',
	default = false
}

cvar {
    name = 'overrun.missedbackside_emulate',
	default = false
}

cvar {
    name = 'demo_patterns',
    default = {
        'DOOM 2 = Hell on Earth/((lv)|(nm)|(pa)|(ty))\\d\\d.\\d\\d\\d\\.lmp/doom2.wad',
        'DOOM 2 = Plutonia Experiment/p(c|f|l|n|p|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|plutonia.wad',
        'DOOM 2 = TNT - Evilution/((e(c|f|v|p|r|s|t))|(tn))\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|tnt.wad',
        'The Ultimate DOOM/(((e|f|n|p|r|t|u)\\dm\\d)|(n\\ds\\d)).\\d\\d\\d\\.lmp/doom.wad',
        'Alien Vendetta/a(c|f|n|p|r|s|t|v)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|av.wad|av.deh',
        'Requiem/r(c|f|n|p|q|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|requiem.wad|req21fix.wad|reqmus.wad',
        'Hell Revealed/h(c|e|f|n|p|r|s|t)\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|hr.wad|hrmus.wad',
        'Memento Mori/mm\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|mm.wad|mmmus.wad',
        'Memento Mori 2/m2\\d\\d.\\d\\d\\d\\.lmp/doom2.wad|mm2.wad|mm2mus.wad'
    }
}


cvar {
    name = 'pwo',
	default = {
		'plasma',
		'super shotgun',
		'chaingun',
		'shotgun',
		'pistol',
		'chainsaw',
		'rocket launcher',
		'bfg',
		'fist'
	}
}

cvar {
    name = 'music.e1m1',
	default = "e1m1.mp3"
}

cvar {
    name = 'music.e1m2',
	default = "e1m2.mp3"
}

cvar {
    name = 'music.e1m3',
	default = "e1m3.mp3"
}

cvar {
    name = 'music.e1m4',
	default = "e1m4.mp3"
}

cvar {
    name = 'music.e1m5',
	default = "e1m5.mp3"
}

cvar {
    name = 'music.e1m6',
	default = "e1m6.mp3"
}

cvar {
    name = 'music.e1m7',
	default = "e1m7.mp3"
}

cvar {
    name = 'music.e1m8',
	default = "e1m8.mp3"
}

cvar {
    name = 'music.e1m9',
	default = "e1m9.mp3"
}

cvar {
    name = 'music.e2m1',
	default = "e2m1.mp3"
}

cvar {
    name = 'music.e2m2',
	default = "e2m2.mp3"
}

cvar {
    name = 'music.e2m3',
	default = "e2m3.mp3"
}

cvar {
    name = 'music.e2m4',
	default = "e2m4.mp3"
}

cvar {
    name = 'music.e2m5',
	default = "e1m7.mp3"
}

cvar {
    name = 'music.e2m6',
	default = "e2m6.mp3"
}

cvar {
    name = 'music.e2m7',
	default = "e2m7.mp3"
}

cvar {
    name = 'music.e2m8',
	default = "e2m8.mp3"
}

cvar {
    name = 'music.e2m9',
	default = "e3m1.mp3"
}

cvar {
    name = 'music.e3m1',
	default = "e3m1.mp3"
}

cvar {
    name = 'music.e3m2',
	default = "e3m2.mp3"
}

cvar {
    name = 'music.e3m3',
	default = "e3m3.mp3"
}

cvar {
    name = 'music.e3m4',
	default = "e1m8.mp3"
}

cvar {
    name = 'music.e3m5',
	default = "e1m7.mp3"
}

cvar {
    name = 'music.e3m6',
	default = "e1m6.mp3"
}

cvar {
    name = 'music.e3m7',
	default = "e2m7.mp3"
}

cvar {
    name = 'music.e3m8',
	default = "e3m8.mp3"
}

cvar {
    name = 'music.e3m9',
	default = "e1m9.mp3"
}

cvar {
    name = 'music.inter',
	default = "e2m3.mp3"
}

cvar {
    name = 'music.intro',
	default = "intro.mp3"
}

cvar {
    name = 'music.bunny',
	default = "bunny.mp3"
}

cvar {
    name = 'music.victor',
	default = "victor.mp3"
}

cvar {
    name = 'music.introa',
	default = "intro.mp3"
}

cvar {
    name = 'music.runnin',
	default = "runnin.mp3"
}

cvar {
    name = 'music.stalks',
	default = "stalks.mp3"
}

cvar {
    name = 'music.countd',
	default = "countd.mp3"
}

cvar {
    name = 'music.betwee',
	default = "betwee.mp3"
}

cvar {
    name = 'music.doom',
	default = "doom.mp3"
}

cvar {
    name = 'music.the_da',
	default = "the_da.mp3"
}

cvar {
    name = 'music.shawn',
	default = "shawn.mp3"
}

cvar {
    name = 'music.ddtblu',
	default = "ddtblu.mp3"
}

cvar {
    name = 'music.in_cit',
	default = "in_cit.mp3"
}

cvar {
    name = 'music.dead',
	default = "dead.mp3"
}

cvar {
    name = 'music.stlks2',
	default = "stalks.mp3"
}

cvar {
    name = 'music.theda2',
	default = "the_da.mp3"
}

cvar {
    name = 'music.doom2',
	default = "doom.mp3"
}

cvar {
    name = 'music.ddtbl2',
	default = "ddtblu.mp3"
}

cvar {
    name = 'music.runni2',
	default = "runnin.mp3"
}

cvar {
    name = 'music.dead2',
	default = "dead.mp3"
}

cvar {
    name = 'music.stlks3',
	default = "stalks.mp3"
}

cvar {
    name = 'music.romero',
	default = "romero.mp3"
}

cvar {
    name = 'music.shawn2',
	default = "shawn.mp3"
}

cvar {
    name = 'music.messag',
	default = "messag.mp3"
}

cvar {
    name = 'music.count2',
	default = "countd.mp3"
}

cvar {
    name = 'music.ddtbl3',
	default = "ddtblu.mp3"
}

cvar {
    name = 'music.ampie',
	default = "ampie.mp3"
}

cvar {
    name = 'music.theda3',
	default = "the_da.mp3"
}

cvar {
    name = 'music.adrian',
	default = "adrian.mp3"
}

cvar {
    name = 'music.messg2',
	default = "messag.mp3"
}

cvar {
    name = 'music.romer2',
	default = "romero.mp3"
}

cvar {
    name = 'music.tense',
	default = "tense.mp3"
}

cvar {
    name = 'music.shawn3',
	default = "shawn.mp3"
}

cvar {
    name = 'music.openin',
	default = "openin.mp3"
}

cvar {
    name = 'music.evil',
	default = "evil.mp3"
}

cvar {
    name = 'music.ultima',
	default = "ultima.mp3"
}

cvar {
    name = 'music.read_m',
	default = "read_m.mp3"
}

cvar {
    name = 'music.dm2ttl',
	default = "dm2ttl.mp3"
}

cvar {
    name = 'music.dm2int',
	default = "dm2int.mp3"
}

cvar {
    name = 'server.limit_player_commands',
	default = true
}

cvar {
    name = 'server.max_connections',
    help = 'Maximum number of allowed connections',
    default = 32,
    min = 1,
    max = 2000
}

cvar {
    name = 'server.max_clients',
    help = 'Maximum number of allowed player clients',
    default = 32,
    min = 1,
    max = 2000
}

cvar {
    name = 'server.max_players',
    help = 'Maximum number of allowed in-game players',
    default = 32,
    min = 1,
    max = 2000
}

cvar {
    name = 'client.extrapolate_player_positions',
	default = true
}

-- vi: et ts=4 sw=4
