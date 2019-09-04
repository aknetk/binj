rwc = $(foreach d, $(wildcard $1*), $(call rwc,$d/,$2) $(filter $(subst *,%,$2),$d))

TARGET    = Binj
# $(notdir $(CURDIR))
TARGETDIR = builds/osx/$(TARGET)
OBJS      = main.o

SRC_C   = $(call rwc, source/, *.c)
SRC_CPP = $(call rwc, source/, *.cpp)

OBJ_DIRS := $(sort \
			$(addprefix out/osx/, $(dir $(SRC_C:source/%.c=%.o))) \
			$(addprefix out/osx/, $(dir $(SRC_CPP:source/%.cpp=%.o))) )

OBJS     := $(addprefix out/osx/, $(SRC_C:source/%.c=%.o)) \
			$(addprefix out/osx/, $(SRC_CPP:source/%.cpp=%.o))

INCLUDES  =	-Wall -Wno-deprecated \
			-Isource \
			-Iinclude \
			-I/usr/local/include/ \
			-I/Library/Frameworks/SDL2.framework/Headers/ \
			-I/Library/Frameworks/SDL2_image.framework/Headers/ \
			-I/Library/Frameworks/SDL2_mixer.framework/Headers/ \
			-I/Library/Frameworks/SDL2_ttf.framework/Headers/

LIBS      = -lfreetype -lass
			# FFmpeg Libraries
LIBS     += -lavcodec -lavformat -lavutil -lswscale -lswresample
			# CURL
LIBS 	 +=  -lcurl -lpthread -lcrypto -lssl
			# OGG Audio
LIBS	 +=	-logg -lvorbis -lvorbisfile
			# zlib Compression
LIBS	 +=	-lz
			# Discord RPC
LIBS	 +=	-ldiscord-rpc

FRMWORKS  = -F/Library/Frameworks/ \
			-F/System/Library/Frameworks/
FRMWORKS +=	-framework SDL2 \
			-framework SDL2_image \
			-framework SDL2_mixer \
			-framework SDL2_ttf \
			-framework OpenGL \
			-framework CoreFoundation \
			-framework CoreServices \
			-framework Foundation \
			-framework Cocoa


DEFINES   =	-Ofast -DMACOSX -DUSING_FRAMEWORK -DOGG_MUSIC -DDEBUG

all:
	@mkdir -p $(OBJ_DIRS)
	@./tools/makeheaders source
	@make build
	@./"$(TARGETDIR)"

package:
	@rm -rf "$(TARGETDIR).app"
	@mkdir -p "$(TARGETDIR).app"
	@mkdir -p "$(TARGETDIR).app/Contents"
	@mkdir -p "$(TARGETDIR).app/Contents/Frameworks"
	@mkdir -p "$(TARGETDIR).app/Contents/MacOS"
	@mkdir -p "$(TARGETDIR).app/Contents/Resources"
	@rm -rf $(OBJS)
	@mkdir -p $(OBJ_DIRS)
	@./tools/makeheaders source
	@make build
	@cp "$(TARGETDIR)" "$(TARGETDIR).app/Contents/MacOS/$(TARGET)"
	@# @[ -f "source/Data.iedat" ] && cp "source/Data.iedat" "$(TARGETDIR).app/Contents/Resources/Data.iedat" || true
	@[ -f "meta/icon.icns" ] && cp "meta/icon.icns" "$(TARGETDIR).app/Contents/Resources/icon.icns" || true
	@# Making PkgInfo
	@echo "APPL????" > "$(TARGETDIR).app/Contents/PkgInfo"
	@# Making Info.plist
	@echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" > "$(TARGETDIR).app/Contents/Info.plist"
	@echo "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "<plist version=\"1.0\">" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "<dict>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <key>CFBundleExecutable</key>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <string>$(TARGET)</string>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <key>CFBundleDisplayName</key>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <string>$(TARGET)</string>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@[ -f "meta/icon.icns" ] && echo "    <key>CFBundleIconFile</key>" >> "$(TARGETDIR).app/Contents/Info.plist" && echo "    <string>icon</string>" >> "$(TARGETDIR).app/Contents/Info.plist" || true
	@# GCSupportsControllerUserInteraction
	@echo "    <key>CFBundlePackageType</key>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <string>APPL</string>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <key>CFBundleShortVersionString</key>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <string>1.0.0</string>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <key>CFBundleVersion</key>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <string>1.0.0</string>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <key>NSHighResolutionCapable</key>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "    <string>true</string>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "</dict>" >> "$(TARGETDIR).app/Contents/Info.plist"
	@echo "</plist>" >> "$(TARGETDIR).app/Contents/Info.plist"

nogen:
	@mkdir -p $(OBJ_DIRS)
	@make build
	@./"$(TARGETDIR)"

debug:
	@mkdir -p $(OBJ_DIRS)
	@./tools/makeheaders source
	@make build
	lldb -o r -f ./"$(TARGETDIR)"

clean:
	@@rm -rf $(OBJS)
	@make all

cleannogen:
	@@rm -rf $(OBJS)
	@make build
	@./"$(TARGETDIR)"

build: $(OBJS)
	@g++ $^ $(INCLUDES) $(LIBS) $(FRMWORKS) -o "$(TARGETDIR)" -std=c++11

$(OBJ_DIRS):
	@mkdir -p $@

out/osx/%.o: source/%.cpp
	@g++ -c -g $(INCLUDES) $(DEFINES) -o "$@" "$<" -std=c++11

out/osx/%.o: source/%.c
	@gcc -c -g $(INCLUDES) $(DEFINES) -o "$@" "$<" -std=c11
