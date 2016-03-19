-------------------------------------------------------------------------------
--                                                                           --
-- Place any custom console shortcuts in this file                           --
--                                                                           --
-------------------------------------------------------------------------------

function d2k.Shortcuts.enable_widget(widget_name)
  d2k.hud:add_interface(d2k.widgets[widget_name])
end

function d2k.Shortcuts.disable_widget(widget_name)
  d2k.hud:remove_interface(d2k.widgets[widget_name])
end

print('Loaded local console shortcuts')

-- vi: et ts=2 sw=2

