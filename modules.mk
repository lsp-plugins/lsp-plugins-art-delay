#
# Copyright (C) 2025 Linux Studio Plugins Project <https://lsp-plug.in/>
#           (C) 2025 Vladimir Sadovnikov <sadko4u@gmail.com>
#
# This file is part of lsp-plugins-art-delay
#
# lsp-plugins-art-delay is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# lsp-plugins-art-delay is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with lsp-plugins-art-delay.  If not, see <https://www.gnu.org/licenses/>.
#

# Variables that describe dependencies
LSP_3RD_PARTY_VERSION      := 1.0.22
LSP_3RD_PARTY_NAME         := lsp-3rd-party
LSP_3RD_PARTY_TYPE         := hdr
LSP_3RD_PARTY_INC_OPT      := -idirafter
LSP_3RD_PARTY_URL_RO       := https://github.com/lsp-plugins/$(LSP_3RD_PARTY_NAME).git
LSP_3RD_PARTY_URL_RW       := git@github.com:lsp-plugins/$(LSP_3RD_PARTY_NAME).git

LSP_COMMON_LIB_VERSION     := 1.0.42
LSP_COMMON_LIB_NAME        := lsp-common-lib
LSP_COMMON_LIB_TYPE        := src
LSP_COMMON_LIB_URL_RO      := https://github.com/lsp-plugins/$(LSP_COMMON_LIB_NAME).git
LSP_COMMON_LIB_URL_RW      := git@github.com:lsp-plugins/$(LSP_COMMON_LIB_NAME).git

LSP_DSP_LIB_VERSION        := 1.0.30
LSP_DSP_LIB_NAME           := lsp-dsp-lib
LSP_DSP_LIB_TYPE           := src
LSP_DSP_LIB_URL_RO         := https://github.com/lsp-plugins/$(LSP_DSP_LIB_NAME).git
LSP_DSP_LIB_URL_RW         := git@github.com:lsp-plugins/$(LSP_DSP_LIB_NAME).git

LSP_DSP_UNITS_VERSION      := 1.0.29
LSP_DSP_UNITS_NAME         := lsp-dsp-units
LSP_DSP_UNITS_TYPE         := src
LSP_DSP_UNITS_URL_RO       := https://github.com/lsp-plugins/$(LSP_DSP_UNITS_NAME).git
LSP_DSP_UNITS_URL_RW       := git@github.com:lsp-plugins/$(LSP_DSP_UNITS_NAME).git

LSP_LLTL_LIB_VERSION       := 1.0.25
LSP_LLTL_LIB_NAME          := lsp-lltl-lib
LSP_LLTL_LIB_TYPE          := src
LSP_LLTL_LIB_URL_RO        := https://github.com/lsp-plugins/$(LSP_LLTL_LIB_NAME).git
LSP_LLTL_LIB_URL_RW        := git@github.com:lsp-plugins/$(LSP_LLTL_LIB_NAME).git

LSP_R3D_BASE_LIB_VERSION   := 1.0.24
LSP_R3D_BASE_LIB_NAME      := lsp-r3d-base-lib
LSP_R3D_BASE_LIB_TYPE      := src
LSP_R3D_BASE_LIB_URL_RO    := https://github.com/lsp-plugins/$(LSP_R3D_BASE_LIB_NAME).git
LSP_R3D_BASE_LIB_URL_RW    := git@github.com:lsp-plugins/$(LSP_R3D_BASE_LIB_NAME).git

LSP_R3D_IFACE_VERSION      := 1.0.24
LSP_R3D_IFACE_NAME         := lsp-r3d-iface
LSP_R3D_IFACE_TYPE         := src
LSP_R3D_IFACE_URL_RO       := https://github.com/lsp-plugins/$(LSP_R3D_IFACE_NAME).git
LSP_R3D_IFACE_URL_RW       := git@github.com:lsp-plugins/$(LSP_R3D_IFACE_NAME).git

LSP_R3D_GLX_LIB_VERSION    := 1.0.24
LSP_R3D_GLX_LIB_NAME       := lsp-r3d-glx-lib
LSP_R3D_GLX_LIB_TYPE       := bin
LSP_R3D_GLX_LIB_URL_RO     := https://github.com/lsp-plugins/$(LSP_R3D_GLX_LIB_NAME).git
LSP_R3D_GLX_LIB_URL_RW     := git@github.com:lsp-plugins/$(LSP_R3D_GLX_LIB_NAME).git

LSP_R3D_WGL_LIB_VERSION    := 1.0.19
LSP_R3D_WGL_LIB_NAME       := lsp-r3d-wgl-lib
LSP_R3D_WGL_LIB_TYPE       := bin
LSP_R3D_WGL_LIB_URL_RO     := https://github.com/lsp-plugins/$(LSP_R3D_WGL_LIB_NAME).git
LSP_R3D_WGL_LIB_URL_RW     := git@github.com:lsp-plugins/$(LSP_R3D_WGL_LIB_NAME).git

LSP_RUNTIME_LIB_VERSION    := 1.0.28
LSP_RUNTIME_LIB_NAME       := lsp-runtime-lib
LSP_RUNTIME_LIB_TYPE       := src
LSP_RUNTIME_LIB_URL_RO     := https://github.com/lsp-plugins/$(LSP_RUNTIME_LIB_NAME).git
LSP_RUNTIME_LIB_URL_RW     := git@github.com:lsp-plugins/$(LSP_RUNTIME_LIB_NAME).git

LSP_TEST_FW_VERSION        := 1.0.31
LSP_TEST_FW_NAME           := lsp-test-fw
LSP_TEST_FW_TYPE           := src
LSP_TEST_FW_URL_RO         := https://github.com/lsp-plugins/$(LSP_TEST_FW_NAME).git
LSP_TEST_FW_URL_RW         := git@github.com:lsp-plugins/$(LSP_TEST_FW_NAME).git

LSP_TK_LIB_VERSION         := 1.0.28
LSP_TK_LIB_NAME            := lsp-tk-lib
LSP_TK_LIB_TYPE            := src
LSP_TK_LIB_URL_RO          := https://github.com/lsp-plugins/$(LSP_TK_LIB_NAME).git
LSP_TK_LIB_URL_RW          := git@github.com:lsp-plugins/$(LSP_TK_LIB_NAME).git

LSP_WS_LIB_VERSION         := 1.0.28
LSP_WS_LIB_NAME            := lsp-ws-lib
LSP_WS_LIB_TYPE            := src
LSP_WS_LIB_URL_RO          := https://github.com/lsp-plugins/$(LSP_WS_LIB_NAME).git
LSP_WS_LIB_URL_RW          := git@github.com:lsp-plugins/$(LSP_WS_LIB_NAME).git

# Plugin-related module dependencies
LSP_PLUGIN_FW_VERSION      := 1.0.30
LSP_PLUGIN_FW_NAME         := lsp-plugin-fw
LSP_PLUGIN_FW_TYPE         := src
LSP_PLUGIN_FW_URL_RO       := https://github.com/lsp-plugins/$(LSP_PLUGIN_FW_NAME).git
LSP_PLUGIN_FW_URL_RW       := git@github.com:lsp-plugins/$(LSP_PLUGIN_FW_NAME).git

LSP_PLUGINS_SHARED_VERSION := 1.0.29
LSP_PLUGINS_SHARED_NAME    := lsp-plugins-shared
LSP_PLUGINS_SHARED_TYPE    := src
LSP_PLUGINS_SHARED_URL_RO  := https://github.com/lsp-plugins/$(LSP_PLUGINS_SHARED_NAME).git
LSP_PLUGINS_SHARED_URL_RW  := git@github.com:lsp-plugins/$(LSP_PLUGINS_SHARED_NAME).git

# System libraries
LIBAUDIOTOOLBOX_VERSION    := system
LIBAUDIOTOOLBOX_NAME       := libaudiotoolbox
LIBAUDIOTOOLBOX_TYPE       := opt
LIBAUDIOTOOLBOX_LDFLAGS    := -framework AudioToolbox

LIBCOREFOUNDATION_VERSION  := system
LIBCOREFOUNDATION_NAME     := libcorefoundation
LIBCOREFOUNDATION_TYPE     := opt
LIBCOREFOUNDATION_LDFLAGS  := -framework CoreFoundation

LIBADVAPI_VERSION          := system
LIBADVAPI_NAME             := libadvapi32
LIBADVAPI_TYPE             := opt
LIBADVAPI_LDFLAGS          := -ladvapi32

LIBCAIRO_VERSION           := system
LIBCAIRO_NAME              := cairo
LIBCAIRO_TYPE              := pkg

LIBD2D1_VERSION            := system
LIBD2D1_NAME               := libd2d1
LIBD2D1_TYPE               := opt
LIBD2D1_LDFLAGS            := -ld2d1

LIBDL_VERSION              := system
LIBDL_NAME                 := libdl
LIBDL_TYPE                 := opt
LIBDL_LDFLAGS              := -ldl

LIBDWRITE_VERSION          := system
LIBDWRITE_NAME             := libdwrite
LIBDWRITE_TYPE             := opt
LIBDWRITE_LDFLAGS          := -ldwrite

LIBFONTCONFIG_VERSION      := system
LIBFONTCONFIG_NAME         := fontconfig
LIBFONTCONFIG_TYPE         := pkg

LIBFREETYPE_VERSION        := system
LIBFREETYPE_NAME           := freetype2
LIBFREETYPE_TYPE           := pkg

LIBGDI32_VERSION           := system
LIBGDI32_NAME              := libgid32
LIBGDI32_TYPE              := opt
LIBGDI32_LDFLAGS           := -lgdi32

LIBGL_VERSION              := system
LIBGL_NAME                 := gl
LIBGL_TYPE                 := pkg

LIBGSTREAMER_AUDIO_VERSION := system
LIBGSTREAMER_AUDIO_NAME    := gstreamer-audio-1.0
LIBGSTREAMER_AUDIO_TYPE    := pkg

LIBICONV_VERSION           := system
LIBICONV_NAME              := libiconv
LIBICONV_TYPE              := opt
LIBICONV_LDFLAGS           := -liconv

LIBJACK_VERSION            := system
LIBJACK_NAME               := jack
LIBJACK_TYPE               := pkg

LIBMPR_VERSION             := system
LIBMPR_NAME                := libmpr
LIBMPR_TYPE                := opt
LIBMPR_LDFLAGS             := -lmpr

LIBMSACM_VERSION           := system
LIBMSACM_NAME              := libmsacm
LIBMSACM_TYPE              := opt
LIBMSACM_LDFLAGS           := -lmsacm32

LIBOLE_VERSION             := system
LIBOLE_NAME                := libole
LIBOLE_TYPE                := opt
LIBOLE_LDFLAGS             := -lole32

LIBOPENGL32_VERSION        := system
LIBOPENGL32_NAME           := libopengl32
LIBOPENGL32_TYPE           := opt
LIBOPENGL32_LDFLAGS        := -lopengl32

LIBPTHREAD_VERSION         := system
LIBPTHREAD_NAME            := libpthread
LIBPTHREAD_TYPE            := opt
LIBPTHREAD_LDFLAGS         := -lpthread

LIBRT_VERSION              := system
LIBRT_NAME                 := librt
LIBRT_TYPE                 := opt
LIBRT_LDFLAGS              := -lrt

LIBSNDFILE_VERSION         := system
LIBSNDFILE_NAME            := sndfile
LIBSNDFILE_TYPE            := pkg

LIBSHLWAPI_VERSION         := system
LIBSHLWAPI_NAME            := libshlwapi
LIBSHLWAPI_TYPE            := opt
LIBSHLWAPI_LDFLAGS         := -lshlwapi

LIBUUID_VERSION            := system
LIBUUID_NAME               := libuuid
LIBUUID_TYPE               := opt
LIBUUID_LDFLAGS            := -luuid

LIBWINCODEC_VERSION        := system
LIBWINCODEC_NAME           := libwincodec
LIBWINCODEC_TYPE           := opt
LIBWINCODEC_LDFLAGS        := -lwindowscodecs

LIBWINMM_VERSION           := system
LIBWINMM_NAME              := libwinmm
LIBWINMM_TYPE              := opt
LIBWINMM_LDFLAGS           := -lwinmm

LIBX11_VERSION             := system
LIBX11_NAME                := x11
LIBX11_TYPE                := pkg

LIBXRANDR_VERSION          := system
LIBXRANDR_NAME             := xrandr
LIBXRANDR_TYPE             := pkg
