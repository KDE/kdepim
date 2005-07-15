#ifndef __KLEO__CHIASMUS_CONFIG_DATA_H__
#define __KLEO__CHIASMUS_CONFIG_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

struct kleo_chiasmus_config_data {
  const char * name;
  const char * description;
  int level;
  int type;
  union {
    const char * path; /* must be first, see config_data.c */
    const char * string;
    const char * url;
    struct { unsigned int value : 1; unsigned int numTimesSet : 31; } boolean;
    int integer;
    unsigned int unsigned_integer;
  } defaults;
  unsigned int is_optional : 1;
  unsigned int is_list : 1;
  unsigned int is_runtime : 1;
};

extern const struct kleo_chiasmus_config_data kleo_chiasmus_config_entries[];
extern const unsigned int kleo_chiasmus_config_entries_dim;

#ifdef __cplusplus
}
#endif

#endif /* __KLEO__CHIASMUS_CONFIG_DATA_H__ */

