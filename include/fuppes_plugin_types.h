
#ifdef __cplusplus
extern "C" {
#endif

typedef enum tagPLUGIN_TYPE {
	PT_NONE,
	PT_METADATA,
	PT_AUDIO_DECODER,
	PT_AUDIO_ENCODER,
	PT_TRANSCODER,
	PT_THREADED_TRANSCODER
} PLUGIN_TYPE;

typedef struct {
	char		ext[10];
	void*		next;
} file_ext;

typedef struct {
	char					plugin_name[200];
	PLUGIN_TYPE		plugin_type;
	void*					user_data;
} plugin_info;


#ifdef __cplusplus
}
#endif
