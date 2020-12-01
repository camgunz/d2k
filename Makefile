.PHONY: help build releasebuild wad tags clean

help:
	@echo "Commands: help | build | releasebuild | wad | tags | clean"

build:
	@mkdir -p cbuild
	@cd cbuild && cmake .. -DPROFILE=1 -DCMAKE_BUILD_TYPE=Debug && make

releasebuild:
	@mkdir -p cbuild
	@cd cbuild && cmake .. -DCMAKE_BUILD_TYPE=Release && make

wad:
	@cd data && deutex -make d2k.txt d2k.wad

tags:
	@ctags --recurse=yes --tag-relative=yes src/*

clean:
	@rm -rf cbuild
