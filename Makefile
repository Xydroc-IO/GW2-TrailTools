# Cross-compile GW2-TrailTools.dll for Windows / Wine with MinGW-w64
CXX      = x86_64-w64-mingw32-g++
CC       = x86_64-w64-mingw32-gcc
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -MMD -MP
CXXFLAGS += -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_CRT_SECURE_NO_WARNINGS
CXXFLAGS += -Isrc -Isrc/app -Isrc/ui -Isrc/trailtools -Isrc/packedit -Isrc/pathing \
	-Isrc/pathing/packs -Isrc/pathing/world \
	-Ideps -Ideps/imgui -Ideps/miniz
CFLAGS   = -std=c11 -O2 -Wall -MMD -MP -DWIN32_LEAN_AND_MEAN -DNOMINMAX -Ideps/miniz
LDFLAGS  = -shared -static -static-libgcc -static-libstdc++ -Wl,--image-base,0x180000000
LIBS     = -ldxgi -ld3d11 -ld3dcompiler -lgdi32 -luser32 -lole32 -luuid -lshell32 -lcomdlg32 -ladvapi32 -lwinhttp -lcrypt32

SRC_CPP = \
	src/entry.cpp \
	src/entryLoad.cpp \
	src/entryUnload.cpp \
	src/app/AddonPaths.cpp \
	src/app/AspectLayout.cpp \
	src/app/CrashTrail.cpp \
	src/app/CrashTrailFiles.cpp \
	src/app/CrashTrailSnapshot.cpp \
	src/app/CrashTrailStack.cpp \
	src/app/Settings.cpp \
	src/ui/Gw2Ui.cpp \
	src/ui/Gw2UiPadScroll.cpp \
	src/ui/UI_Render.cpp \
	src/ui/TrailToolsQuickAccess.cpp \
	src/ui/WorldClick.cpp \
	src/pathing/PathingTrails.cpp \
	src/pathing/packs/PathingParse.cpp \
	src/pathing/packs/PathingParseXml.cpp \
	src/pathing/packs/PathingParseZip.cpp \
	src/pathing/packs/PathingRuntimeState.cpp \
	src/pathing/packs/PathingRuntimeIndex.cpp \
	src/pathing/packs/PathingRuntimeLoad.cpp \
	src/pathing/world/WorldGpsMath.cpp \
	src/pathing/world/WorldGpsD3dDevice.cpp \
	src/pathing/world/WorldGpsD3dDraw.cpp \
	src/pathing/world/WorldGpsImgui.cpp \
	src/packedit/PackEditAttrs.cpp \
	src/packedit/PackEditCanvas.cpp \
	src/packedit/PackEditDetails.cpp \
	src/packedit/PackEditLoad.cpp \
	src/packedit/PackEditLoadFinish.cpp \
	src/packedit/PackEditLoadFolder.cpp \
	src/packedit/PackEditPad.cpp \
	src/packedit/PackEditResources.cpp \
	src/packedit/PackEditSave.cpp \
	src/packedit/PackEditSaveXml.cpp \
	src/packedit/PackEditState.cpp \
	src/packedit/PackEditStyle.cpp \
	src/packedit/PackEditTick.cpp \
	src/packedit/PackEditTree.cpp \
	src/packedit/PackEditUndo.cpp \
	src/packedit/PackEditWindows.cpp \
	src/packedit/PackEditWorld.cpp \
	src/trailtools/TrailToolsAssets.cpp \
	src/trailtools/TrailToolsBinds.cpp \
	src/trailtools/TrailToolsBindsActions.cpp \
	src/trailtools/TrailToolsBindsChord.cpp \
	src/trailtools/TrailToolsBindsTrailEdit.cpp \
	src/trailtools/TrailToolsBuild.cpp \
	src/trailtools/TrailToolsDraftStyle.cpp \
	src/trailtools/TrailToolsEditUndo.cpp \
	src/trailtools/TrailToolsGround.cpp \
	src/trailtools/TrailToolsImport.cpp \
	src/trailtools/TrailToolsPad.cpp \
	src/trailtools/TrailToolsPadContent.cpp \
	src/trailtools/TrailToolsPadKeybinds.cpp \
	src/trailtools/TrailToolsPadLive.cpp \
	src/trailtools/TrailToolsPadLua.cpp \
	src/trailtools/TrailToolsPadMarkers.cpp \
	src/trailtools/TrailToolsPadMarkersFilters.cpp \
	src/trailtools/TrailToolsPadMarkersScript.cpp \
	src/trailtools/TrailToolsPadPack.cpp \
	src/trailtools/TrailToolsPadTrailAttrs.cpp \
	src/trailtools/TrailToolsPadTrailDesk.cpp \
	src/trailtools/TrailToolsPadTrailGeom.cpp \
	src/trailtools/TrailToolsPadTrailHelpers.cpp \
	src/trailtools/TrailToolsPadTrailRaw.cpp \
	src/trailtools/TrailToolsPadXmlDesk.cpp \
	src/trailtools/TrailToolsPadXmlEdit.cpp \
	src/trailtools/TrailToolsPersist.cpp \
	src/trailtools/TrailToolsPreview.cpp \
	src/trailtools/TrailToolsPreviewCompass.cpp \
	src/trailtools/TrailToolsState.cpp \
	src/trailtools/TrailToolsStateCategories.cpp \
	src/trailtools/TrailToolsStateEditors.cpp \
	src/trailtools/TrailToolsStateFs.cpp \
	src/trailtools/TrailToolsTrailGeom.cpp \
	src/trailtools/TrailToolsTrl.cpp \
	src/trailtools/TrailToolsUberTool.cpp \
	src/trailtools/TrailToolsUberToolDraw.cpp \
	src/trailtools/TrailToolsWorldPick.cpp \
	src/trailtools/TrailToolsXml.cpp \
	deps/imgui/imgui.cpp \
	deps/imgui/imgui_draw.cpp \
	deps/imgui/imgui_tables.cpp \
	deps/imgui/imgui_widgets.cpp

SRC_C = \
	deps/miniz/miniz.c \
	deps/miniz/miniz_tdef.c \
	deps/miniz/miniz_tinfl.c \
	deps/miniz/miniz_zip.c

OBJ = $(patsubst %.cpp,build/%.o,$(SRC_CPP)) $(patsubst %.c,build/%.o,$(SRC_C))
DEP = $(OBJ:.o=.d)
OUT = build/bin/GW2-TrailTools.dll

GW2_ADDONS ?= $(HOME)/.local/share/Steam/steamapps/common/Guild Wars 2/addons
INSTALL_DIR = $(GW2_ADDONS)/GW2-TrailTools

.PHONY: all clean install check-lines

all: $(OUT)

$(OUT): $(OBJ)
	@mkdir -p $(dir $@)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)
	@echo "Built $@"

build/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c -o $@ $<

-include $(DEP)

check-lines:
	@python3 tools/check_lines.py

clean:
	rm -rf build

install: all
	@mkdir -p "$(INSTALL_DIR)"
	/bin/cp -f "$(OUT)" "$(GW2_ADDONS)/GW2-TrailTools.dll"
	/bin/cp -f "$(OUT)" "$(INSTALL_DIR)/GW2-TrailTools.dll"
	@echo "Installed to $(GW2_ADDONS)/GW2-TrailTools.dll"
	@echo "Data folder: $(INSTALL_DIR)"
