################################################################################
#
# lproc_queue
#
################################################################################

LPROC_QUEUE_VERSION = 0.1
LPROC_QUEUE_SITE = $(CURDIR)/package/0_platform_custom/lproc_queue/lproc_queue
LPROC_QUEUE_SITE_METHOD = local

LPROC_QUEUE_DEPENDENCIES = lua


define LUAMODBUS_CONSOLE_BUILD_CMDS
    $(MAKE) CC="$(TARGET_CC)" LD="$(TARGET_LD)" -C $(@D)
    #$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D)
endef

define LUAMODBUS_CONSOLE_INSTALL_TARGET_CMDS
    $(INSTALL) -D -m 0755 $(@D)/lproc_queue.so $(TARGET_DIR)/usr/lib/lua/5.4
    #$(TARGET_MAKE_ENV) $(MAKE1) -C $(@D) DESTDIR=$(STAGING_DIR) LDCONFIG=true install
endef

$(eval $(generic-package))
