# Copyright (c) 2010 Wind River Systems, Inc.
#
# The right to copy, distribute, modify, or otherwise make use
# of this software may be licensed only pursuant to the terms
# of an applicable Wind River license agreement.

LOCAL_PATH:= $(call my-dir)

xload_PATH := $(LOCAL_PATH)
xload_MAKE_CMD := make -C $(LOCAL_PATH) CROSS_COMPILE=$(ANDROID_BUILD_TOP)/$(TARGET_KERNEL_CROSSCOMPILE)

#Common routines.
ifneq ($(TARGET_PRODUCT), kunlun_n710e)
.PHONY: xload.clean
xload.clean:
	$(hide) echo "Cleaning x-loader.."
	$(hide) $(xload_MAKE_CMD) clean

.PHONY: xload.distclean
xload.distclean:
	$(hide) echo "Distcleaning x-loader.."
	$(hide) $(xload_MAKE_CMD) distclean

$(LOCAL_PATH)/include/config.h:
	$(hide) echo "Configuring x-loader with $(TARGET_XLOAD_CONFIG).."
	$(hide) $(xload_MAKE_CMD) $(TARGET_XLOAD_CONFIG)

.PHONY: xload.config
xload.config: $(LOCAL_PATH)/include/config.h

$(LOCAL_PATH)/MLO: $(LOCAL_PATH)/include/config.h
	$(hide) echo "Building x-loader.."
	$(hide) $(xload_MAKE_CMD) ift

.PHONY: xload
xload: $(LOCAL_PATH)/MLO

else
#For Haier N710E.
#Now we got two versions of Haier N710E.
#REV1 - NAND 4G + DDR 2G. REV2 - NAND 4G + DDR 4G.
#We build two versions on N710E platform temporarily.
#Clean up this when all our boards is REV2.

.PHONY: xload.clean
xload.clean:
	$(hide) echo "Cleaning x-loader.."
	$(hide) $(xload_MAKE_CMD) clean
	$(hide) rm -rf $(xload_PATH)/MLO_2G $(xload_PATH)/MLO_4G

.PHONY: xload.distclean
xload.distclean:
	$(hide) echo "Distcleaning x-loader.."
	$(hide) $(xload_MAKE_CMD) distclean
	$(hide) rm -rf $(xload_PATH)/MLO_2G $(xload_PATH)/MLO_4G

$(LOCAL_PATH)/MLO_2G:
	$(hide) echo "Distcleaning x-loader.."
	$(hide) $(xload_MAKE_CMD) distclean
	$(hide) echo "Configuring x-loader with $(TARGET_XLOAD_REV1_CONFIG).."
	$(hide) $(xload_MAKE_CMD) $(TARGET_XLOAD_REV1_CONFIG)
	$(hide) echo "Building x-loader.."
	$(hide) $(xload_MAKE_CMD) ift
	$(hide) mv $(xload_PATH)/MLO $(xload_PATH)/MLO_2G

#Don't break parallel build.
$(LOCAL_PATH)/MLO_4G: $(LOCAL_PATH)/MLO_2G
	$(hide) echo "Distcleaning x-loader.."
	$(hide) $(xload_MAKE_CMD) distclean
	$(hide) echo "Configuring x-loader with $(TARGET_XLOAD_REV2_CONFIG).."
	$(hide) $(xload_MAKE_CMD) $(TARGET_XLOAD_REV2_CONFIG)
	$(hide) echo "Building x-loader.."
	$(hide) $(xload_MAKE_CMD) ift
	$(hide) mv $(xload_PATH)/MLO $(xload_PATH)/MLO_4G

.PHONY: xload
xload: $(LOCAL_PATH)/MLO_2G $(LOCAL_PATH)/MLO_4G
endif

.PHONY: xload.rebuild
xload.rebuild: xload.clean xload
