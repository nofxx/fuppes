
#include "../../include/fuppes_plugin.h"

void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "libavformat metadata");
	info->plugin_type = PT_METADATA;

	/*add_extension(info, "avi");
	add_extension(info, "mpeg");
	add_extension(info, "mkv");*/
}

int init_fuppes_plugin(plugin_info* info)
{
	return 0;
}

void unregister_fuppes_plugin(plugin_info* info)
{
}
