
#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include "fuppes_plugin_types.h"

//static void register_fuppes_plugin(plugin_info* info) = 0;

/*static inline void add_extension(file_ext* info, char* ext)
{
	file_ext* tmp  = info->extension;
	file_ext* next = NULL;
	
	if(tmp) {
		next = (file_ext*)tmp->next;
	}
	else {
		tmp = (file_ext*)malloc(sizeof(file_ext));
		tmp->next = NULL;
		next = tmp;
	}
	
	while(next != NULL) {
		tmp = next;
		next = (file_ext*)next->next;
	}
	
	strcpy(tmp->ext, ext);
	tmp->next = NULL;	
}*/

#ifdef __cplusplus
}
#endif
