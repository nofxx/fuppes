include $(top_srcdir)/PreRules.am

basedir = $(sysconfdir)/fuppes
dist_base_DATA = \
	fuppes.cfg

devicesdir = $(sysconfdir)/fuppes/devices
dist_devices_DATA = \
	devices/default.cfg \
	devices/default-transcoding-enabled.cfg \
  devices/xbox.cfg \
  devices/ps3.cfg

if HAVE_VFOLDER

vfoldersdir = $(sysconfdir)/fuppes/vfolders
dist_vfolders_DATA = \
	vfolders/default.cfg \
  vfolders/xbox.cfg

endif

