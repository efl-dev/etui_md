#ifndef ETUI_MD_H
#define ETUI_MD_H

extern int etui_log_dom_global;

#ifdef ETUI_DEFAULT_LOG_COLOR
# undef ETUI_DEFAULT_LOG_COLOR
#endif
#define ETUI_DEFAULT_LOG_COLOR EINA_COLOR_CYAN

#ifdef ERR
# undef ERR
#endif
#define ERR(...)  EINA_LOG_DOM_ERR(etui_log_dom_global, __VA_ARGS__)

#ifdef DBG
# undef DBG
#endif
#define DBG(...)  EINA_LOG_DOM_DBG(etui_log_dom_global, __VA_ARGS__)

#ifdef INF
# undef INF
#endif
#define INF(...)  EINA_LOG_DOM_INFO(etui_log_dom_global, __VA_ARGS__)

#ifdef WRN
# undef WRN
#endif
#define WRN(...)  EINA_LOG_DOM_WARN(etui_log_dom_global, __VA_ARGS__)

#ifdef CRIT
# undef CRIT
#endif
#define CRIT(...) EINA_LOG_DOM_CRIT(etui_log_dom_global, __VA_ARGS__)



Evas_Object *etui_md_add(Evas_Object *win);

void etui_md_file_set(Evas_Object *obj, const char *filename);

#endif /* ETUI_MD_H */
