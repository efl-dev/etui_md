#include <Elementary.h>

#include "etui_md.h"

int etui_log_dom_global = -1;


EAPI_MAIN int
elm_main(int argc, char **argv)
{
    Evas_Object *win;
    Evas_Object *scroller;
    Evas_Object *o;

    if (argc < 2)
    {
        printf("Usage:  %s file\n", argv[0]);
        return 0;
    }

    etui_log_dom_global = eina_log_domain_register("etui-md",
                                                   ETUI_DEFAULT_LOG_COLOR);
    if (etui_log_dom_global < 0)
    {
        EINA_LOG_ERR("Etui: Could not register log domain 'etui'.");
        return 0;
    }

    elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
    elm_app_name_set("markdown");

    win = elm_win_add(NULL, "markdown", ELM_WIN_BASIC);
    elm_win_title_set(win, "markdown");
    elm_win_autodel_set(win, EINA_TRUE);
    evas_object_resize(win,
                       480 * elm_config_scale_get(),
                       640 * elm_config_scale_get());

    /* background */
    o = elm_bg_add(win);
    evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_style_set(o, "grad_vert_focus_title_match");
    elm_win_resize_object_add(win, o);
    evas_object_show(o);

    /* scroller */
    o = elm_scroller_add(win);
    elm_scroller_policy_set(o,
                            ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
    elm_scroller_bounce_set(o, EINA_TRUE, EINA_TRUE);
    evas_object_size_hint_weight_set(o, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(o, EVAS_HINT_FILL, EVAS_HINT_FILL);
    elm_win_resize_object_add(win, o);
    evas_object_show(o);
    scroller = o;

    o = etui_md_add(win);
    evas_object_move(o, 0, 0);
    evas_object_resize(o, 1, 1);
    elm_object_content_set(scroller, o);
    evas_object_show(o);

    etui_md_file_set(o, argv[1]);

    evas_object_show(win);

    elm_run();

    return 0;
}
ELM_MAIN()

