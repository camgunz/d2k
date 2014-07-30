#!/usr/bin/env python

import os, sys

HEADER_EXCEPTIONS = (
    'src/MUSIC/dbopl.c',
    'src/MUSIC/dbopl.h',
    'src/MUSIC/midifile.c',
    'src/MUSIC/midifile.h',
    'src/MUSIC/opl_queue.c',
    'src/MUSIC/opl_queue.h',
    'src/MUSIC/opl.c',
    'src/MUSIC/opl.h',
    'src/MUSIC/oplplayer.c',
    'src/MUSIC/oplplayer.h',
    'src/PCSOUND/*',
    'src/PCSOUND/Makefile.am',
    'src/PCSOUND/pcsound.c',
    'src/PCSOUND/pcsound.h',
    'src/PCSOUND/pcsound_linux.c',
    'src/PCSOUND/pcsound_sdl.c',
    'src/PCSOUND/pcsound_win32.c',
    'src/TEXTSCREEN/doomkeys.h',
    'src/TEXTSCREEN/textscreen.h',
    'src/TEXTSCREEN/txt_button.c',
    'src/TEXTSCREEN/txt_button.h',
    'src/TEXTSCREEN/txt_checkbox.c',
    'src/TEXTSCREEN/txt_checkbox.h',
    'src/TEXTSCREEN/txt_desktop.c',
    'src/TEXTSCREEN/txt_desktop.h',
    'src/TEXTSCREEN/txt_dropdown.c',
    'src/TEXTSCREEN/txt_dropdown.h',
    'src/TEXTSCREEN/txt_font.h',
    'src/TEXTSCREEN/txt_gui.c',
    'src/TEXTSCREEN/txt_gui.h',
    'src/TEXTSCREEN/txt_inputbox.c',
    'src/TEXTSCREEN/txt_inputbox.h',
    'src/TEXTSCREEN/txt_io.c',
    'src/TEXTSCREEN/txt_io.h',
    'src/TEXTSCREEN/txt_label.c',
    'src/TEXTSCREEN/txt_label.h',
    'src/TEXTSCREEN/txt_main.h',
    'src/TEXTSCREEN/txt_radiobutton.c',
    'src/TEXTSCREEN/txt_radiobutton.h',
    'src/TEXTSCREEN/txt_scrollpane.c',
    'src/TEXTSCREEN/txt_scrollpane.h',
    'src/TEXTSCREEN/txt_sdl.c',
    'src/TEXTSCREEN/txt_sdl.h',
    'src/TEXTSCREEN/txt_separator.c',
    'src/TEXTSCREEN/txt_separator.h',
    'src/TEXTSCREEN/txt_smallfont.h',
    'src/TEXTSCREEN/txt_spinctrl.c',
    'src/TEXTSCREEN/txt_spinctrl.h',
    'src/TEXTSCREEN/txt_strut.c',
    'src/TEXTSCREEN/txt_strut.h',
    'src/TEXTSCREEN/txt_table.c',
    'src/TEXTSCREEN/txt_table.h',
    'src/TEXTSCREEN/txt_widget.c',
    'src/TEXTSCREEN/txt_widget.h',
    'src/TEXTSCREEN/txt_window.c',
    'src/TEXTSCREEN/txt_window.h',
    'src/TEXTSCREEN/txt_window_action.c',
    'src/TEXTSCREEN/txt_window_action.h'
)

EXTENSIONS = (
    '.c',
    '.h',
    '.inl',
    '.m'
)

OLD_HEADER = ''
HEADER = ''
OLD_FOOTER = ''
FOOTER = ''

def read_file(path):
    data = ''
    with open(path, 'rb') as fobj:
        data = fobj.read().decode('utf8')
    return data

def write_file(data, path):
    with open(path, 'wb') as fobj:
        fobj.write(data.encode('utf8'))

def strip_old_header(filedata):
    if OLD_HEADER and filedata.startswith(OLD_HEADER):
        return filedata[len(OLD_HEADER):]
    return filedata

def strip_old_footer(filedata):
    if OLD_FOOTER and filedata.endswith(OLD_FOOTER):
        return filedata[:len(OLD_FOOTER):]
    return filedata

def main():
    global HEADER, FOOTER, OLD_HEADER, OLD_FOOTER

    HEADER = read_file('header.txt')
    FOOTER = read_file('footer.txt')
    if os.path.isfile('old_header.txt'):
        OLD_HEADER = read_file('old_header.txt')
    if os.path.isfile('old_footer.txt'):
        OLD_FOOTER = read_file('old_footer.txt')
    for root, dirs, files in os.walk('src'):
        for f in files:
            for extension in EXTENSIONS:
                if f.endswith(extension):
                    break
            else:
                continue
            filepath = os.path.join(root, f)
            print("Processing %s" % (filepath))
            filedata = read_file(filepath).strip()
            if filepath not in HEADER_EXCEPTIONS:
                filedata = strip_old_header(filedata)
                filedata = '\n'.join((HEADER, filedata))
            filedata = strip_old_footer(filedata)
            filedata = '\n'.join((filedata, FOOTER))
            write_file(filedata, filepath)

main()

