
#include <Elementary.h>

#include <md4c.h>

#include "etui_md.h"

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

typedef struct MD_Cursors_ Md_Cursors;

struct MD_Cursors_
{
    Evas_Textblock_Cursor *begin;
    Evas_Textblock_Cursor *end;
};

typedef struct Md_ Md;

struct Md_
{
    Evas_Object *tb;
    Evas_Textblock_Cursor *cur;
    Md_Cursors *curs_current;
    Eina_List *cursors;
};

static Evas_Smart *_smart = NULL;
static Evas_Smart_Class _parent_sc = EVAS_SMART_CLASS_INIT_NULL;

static void
_etui_md_tb_resize(void *data, Evas *e EINA_UNUSED, Evas_Object *obj, void *event_info EINA_UNUSED)
{
    Evas_Object *o;
    Md *sd;
    Md_Cursors *cursors;
    Eina_List *l;
    int w, h;

    o = (Evas_Object *)data;
    sd = evas_object_smart_data_get(o);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    evas_object_textblock_size_formatted_get(sd->tb, &w, &h);
    printf("resized formatted : %d %d\n", w, h);
    evas_object_size_hint_min_set(sd->tb, w, h);
    evas_object_size_hint_min_set(o, w, h);

#if 0
    {
        int xb, yb, wb, hb;
        int xe, ye, we, he;
        evas_textblock_cursor_geometry_get(sd->cur_code_begin,
                                           &xb, &yb, &wb, &hb,
                                           NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE);
        evas_textblock_cursor_geometry_get(sd->cur_code_end,
                                           &xe, &ye, &we, &he,
                                           NULL, EVAS_TEXTBLOCK_CURSOR_BEFORE);
        printf(" ** beg : %d %d %d %d\n", xb, yb, wb, hb);
        printf(" ** end : %d %d %d %d\n", xe, ye, we, he);
        printf(" ** hau : %d\n", ye - yb);
        evas_textblock_cursor_line_geometry_get(sd->cur_code_begin, &xb, &yb, &wb, &hb);
        evas_textblock_cursor_line_geometry_get(sd->cur_code_end, &xe, &ye, &we, &he);
        printf(" ** beg : %d %d %d %d\n", xb, yb, wb, hb);
        printf(" ** end : %d %d %d %d\n", xe, ye, we, he);
        printf(" ** hau : %d\n", ye - yb);
    }
#endif

    EINA_LIST_FOREACH(sd->cursors, l, cursors)
    {
        int xb, yb, wb, hb;
        int xe, ye, we, he;
        /* Evas_Object *o; */
        /* o = evas_object_rectangle_add(evas_object_evas_get(obj)); */
        /* evas_object_move(o, 0, height->y1); */
        /* evas_object_resize(o, w, height->y2 - height->y1); */
        /* evas_object_color_set(o, 32, 32, 32, 255); */
        evas_textblock_cursor_line_geometry_get(cursors->begin, &xb, &yb, &wb, &hb);
        evas_textblock_cursor_line_geometry_get(cursors->end, &xe, &ye, &we, &he);
        printf(" ** beg : %d %d %d %d\n", xb, yb, wb, hb);
        printf(" ** end : %d %d %d %d\n", xe, ye, we, he);
        printf(" ** hau : %d\n", ye - yb);
        evas_object_show(o);
        //free(height);
    }
}

/******** Evas smart callbacks ********/

static void
_smart_add(Evas_Object *obj)
{
    char buf[128];
    Md *sd;
    Evas_Textblock_Style *st;

    INF(" * %s", __FUNCTION__);

    sd = calloc(1, sizeof(Md));
    EINA_SAFETY_ON_NULL_RETURN(sd);

    evas_object_smart_data_set(obj, sd);

    _parent_sc.add(obj);

    st = evas_textblock_style_new();
    snprintf(buf, sizeof(buf),
             "DEFAULT='font=Sans font_size=16 color=#a0a0a0'");
    evas_textblock_style_set(st, buf);

    sd->tb = evas_object_textblock_add(evas_object_evas_get(obj));
    evas_object_textblock_style_set(sd->tb, st);
    evas_object_scale_set(sd->tb, elm_config_scale_get());

    evas_object_smart_member_add(sd->tb, obj);

    sd->cur = evas_object_textblock_cursor_get(sd->tb);

    evas_object_event_callback_add(sd->tb, EVAS_CALLBACK_RESIZE,
                                   _etui_md_tb_resize, obj);
}

static void
_smart_del(Evas_Object *obj)
{
    Md *sd;

    INF(" * %s", __FUNCTION__);

    sd = evas_object_smart_data_get(obj);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    _parent_sc.del(obj);

    evas_object_del(sd->tb);

    evas_object_smart_data_set(obj, NULL);
    memset(sd, 0, sizeof(*sd));
    free(sd);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
    Md *sd;

    INF(" * %s", __FUNCTION__);

    sd = evas_object_smart_data_get(obj);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    evas_object_move(sd->tb, x, y);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
    Md *sd;

    INF(" * %s", __FUNCTION__);

    sd = evas_object_smart_data_get(obj);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    evas_object_resize(sd->tb, w, h);
}

static void
_smart_show(Evas_Object *obj)
{
    Md *sd;

    INF(" * %s", __FUNCTION__);

    sd = evas_object_smart_data_get(obj);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    evas_object_show(sd->tb);
}

static void
_smart_hide(Evas_Object *obj)
{
    Md *sd;

    INF(" * %s", __FUNCTION__);

    sd = evas_object_smart_data_get(obj);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    evas_object_hide(sd->tb);
}

static void
_smart_calculate(Evas_Object *obj)
{
    Md *sd;
    Evas_Coord ox;
    Evas_Coord oy;
    Evas_Coord ow;
    Evas_Coord oh;

    INF(" * %s", __FUNCTION__);

    sd = evas_object_smart_data_get(obj);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);

    evas_object_move(sd->tb, ox, oy);
    evas_object_resize(sd->tb, ow, oh);
}

static void
_smart_init(void)
{
    static Evas_Smart_Class sc;

    INF(" * %s", __FUNCTION__);

    evas_object_smart_clipped_smart_set(&_parent_sc);
    sc           = _parent_sc;
    sc.name      = "image";
    sc.version   = EVAS_SMART_CLASS_VERSION;
    sc.add       = _smart_add;
    sc.del       = _smart_del;
    sc.move    = _smart_move;
    sc.resize    = _smart_resize;
    sc.show      = _smart_show;
    sc.hide      = _smart_hide;
    sc.calculate = _smart_calculate;
    _smart = evas_smart_class_new(&sc);
}


/******** Markdown callbacks ********/

static int
_md_enter_block(MD_BLOCKTYPE type, void *detail, void *data)
{
    Md *sd;

    sd = (Md *)data;

    printf("%s : ", __FUNCTION__);

    switch (type)
    {
        case MD_BLOCK_DOC:
            evas_textblock_cursor_format_prepend(sd->cur, "+wrap=word");
            printf("body\n");
            break;
        case MD_BLOCK_QUOTE:
            printf("blockquote\n");
            break;
        case MD_BLOCK_UL:
            printf("ul\n");
            break;
        case MD_BLOCK_OL:
            printf("ol\n");
            break;
        case MD_BLOCK_LI:
            printf("li\n");
            break;
        case MD_BLOCK_HR:
            printf("hr\n");
            break;
        case MD_BLOCK_H:
        {
            char buf[64];
            MD_BLOCK_H_DETAIL *d;

            d = (MD_BLOCK_H_DETAIL *)detail;
            printf("h: %d\n", d->level);
            snprintf(buf, sizeof(buf), "+font_weight=bold font_size=%d",
                     22 - (d->level * 2));
            evas_textblock_cursor_format_prepend(sd->cur, buf);
            break;
        }
        case MD_BLOCK_CODE:
          {
            int x,y,w,h;

            printf("code\n");

            evas_textblock_cursor_line_geometry_get(sd->cur, &x, &y, &w, &h);
            printf("line code 1: %d %d %d %d\n", x,y,w,h);

            sd->curs_current = (Md_Cursors *)calloc(1, sizeof(Md_Cursors));
            if (sd->curs_current)
            {
                sd->curs_current->begin = evas_object_textblock_cursor_new(sd->tb);
            }

            evas_textblock_cursor_format_prepend(sd->cur, "+Monospace color=#bbbbbb wrap=none");
            break;
          }
        case MD_BLOCK_HTML:
            printf("raw html\n");
            break;
        case MD_BLOCK_P:
            //evas_textblock_cursor_format_prepend(sd->cur, "\n");
            printf("p\n");
            break;
        case MD_BLOCK_TABLE:
            printf("table\n");
            break;
        case MD_BLOCK_THEAD:
            printf("thead\n");
            break;
        case MD_BLOCK_TBODY:
            printf("tbody\n");
            break;
        case MD_BLOCK_TR:
            printf("tr\n");
            break;
        case MD_BLOCK_TH:
            printf("th\n");
            break;
        case MD_BLOCK_TD:
            printf("td\n");
            break;
    }

    return 0;
}

static int
_md_leave_block(MD_BLOCKTYPE type, void *detail, void *data)
{
    Md *sd;

    sd = (Md *)data;

    printf("%s : ", __FUNCTION__);

    switch (type)
    {
        case MD_BLOCK_DOC:
            //evas_textblock_cursor_format_prepend(sd->cur, "-");
            printf("body\n");
            break;
        case MD_BLOCK_QUOTE:
            printf("blockquote\n");
            break;
        case MD_BLOCK_UL:
            printf("ul\n");
            break;
        case MD_BLOCK_OL:
            printf("ol\n");
            break;
        case MD_BLOCK_LI:
            printf("li\n");
            break;
        case MD_BLOCK_HR:
            printf("hr\n");
            break;
        case MD_BLOCK_H:
            evas_textblock_cursor_format_prepend(sd->cur, "-");
            evas_textblock_cursor_format_prepend(sd->cur, "\n");
            printf("h\n");
            break;
        case MD_BLOCK_CODE:
        {
            int x,y,w,h;

            evas_textblock_cursor_format_prepend(sd->cur, "-");

            if (sd->curs_current)
            {
                sd->curs_current->end = evas_object_textblock_cursor_new(sd->tb);
                sd->cursors  = eina_list_append(sd->cursors, sd->curs_current);
            }

            evas_textblock_cursor_line_geometry_get(sd->cur, &x, &y, &w, &h);

            printf("line code 2: %d %d %d %d\n", x,y,w,h);
            break;
        }
        case MD_BLOCK_HTML:
            printf("raw html\n");
            break;
        case MD_BLOCK_P:
            evas_textblock_cursor_format_prepend(sd->cur, "\n");
            printf("p\n");
            break;
        case MD_BLOCK_TABLE:
            printf("table\n");
            break;
        case MD_BLOCK_THEAD:
            printf("thead\n");
            break;
        case MD_BLOCK_TBODY:
            printf("tbody\n");
            break;
        case MD_BLOCK_TR:
            printf("tr\n");
            break;
        case MD_BLOCK_TH:
            printf("th\n");
            break;
        case MD_BLOCK_TD:
            printf("td\n");
            break;
    }

    return 0;
}

static int
_md_enter_span(MD_SPANTYPE type, void *detail, void *data)
{
    Md *sd;

    sd = (Md *)data;

    printf("%s : ", __FUNCTION__);

    switch (type)
    {
        case MD_SPAN_EM:
            printf("emphasis\n");
            evas_textblock_cursor_format_prepend(sd->cur, "+font_style=italic");
            break;
        case MD_SPAN_STRONG:
            printf("strong\n");
            evas_textblock_cursor_format_prepend(sd->cur, "+font_weight=bold");
            break;
        case MD_SPAN_A:
            printf("a\n");
            break;
        case MD_SPAN_IMG:
            printf("img\n");
            break;
        case MD_SPAN_CODE:
            printf("code\n");
            /* fg */
            evas_textblock_cursor_format_prepend(sd->cur, "+Monospace color=#bbbbbb wrap=none");
            break;
        case MD_SPAN_DEL:
            printf("del\n");
            break;
        case MD_SPAN_LATEXMATH:
        case MD_SPAN_LATEXMATH_DISPLAY:
            printf("latex\n");
            break;
        case MD_SPAN_WIKILINK:
            printf("wiki link\n");
            break;
        case MD_SPAN_U:
            printf("underline\n");
            break;
    }

    return 0;
}

static int
_md_leave_span(MD_SPANTYPE type, void *detail, void *data)
{
    Md *sd;

    sd = (Md *)data;

    printf("%s : ", __FUNCTION__);

    switch (type)
    {
        case MD_SPAN_EM:
            printf("emphasis\n");
            evas_textblock_cursor_format_prepend(sd->cur, "-");
            break;
        case MD_SPAN_STRONG:
            printf("strong\n");
            evas_textblock_cursor_format_prepend(sd->cur, "-");
            break;
        case MD_SPAN_A:
            printf("a\n");
            break;
        case MD_SPAN_IMG:
            printf("img\n");
            break;
        case MD_SPAN_CODE:
            printf("code\n");
            evas_textblock_cursor_format_prepend(sd->cur, "-");
            break;
        case MD_SPAN_DEL:
            printf("del\n");
            break;
        case MD_SPAN_LATEXMATH:
        case MD_SPAN_LATEXMATH_DISPLAY:
            printf("latex\n");
            break;
        case MD_SPAN_WIKILINK:
            printf("wiki link\n");
            break;
        case MD_SPAN_U:
            printf("underline\n");
            break;
    }

    return 0;
}

static int
_md_text(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size, void *data)
{
    Md *sd;
    char *tmp;

    sd = (Md *)data;

    char*str = alloca(size+1);
    memcpy(str, text, size);
    str[size]=0;
    printf("text (%d) : '%s'\n", type, str);
    if (size == 1)
        printf("text (char) : '%c' '%d'\n", *text, *text);
    switch (type)
    {
        case MD_TEXT_NORMAL:
            printf("text (%d) normal : '%s'\n", type, str);
            tmp = alloca(size + 1);
            memcpy(tmp, text, size);
            tmp[size] = '\0';
            evas_textblock_cursor_text_prepend(sd->cur, tmp);
            break;
        case MD_TEXT_NULLCHAR:
            printf("text (%d) null char: '%s'\n", type, str);
            break;
        case MD_TEXT_BR:
            printf("text (%d) hard br : '%s'\n", type, str);
            evas_textblock_cursor_format_prepend(sd->cur, "\n");
            break;
        case MD_TEXT_SOFTBR:
            printf("text (%d) soft br: '%s'\n", type, str);
            evas_textblock_cursor_format_prepend(sd->cur, "\n");
            break;
        case MD_TEXT_ENTITY:
            printf("text (%d) entity : '%s'\n", type, str);
            break;
        case MD_TEXT_CODE:
            printf("text (%d) code : '%s'\n", type, str);
            if ((size == 1) && (*text == 10))
            {
                evas_textblock_cursor_format_prepend(sd->cur, "\n");
            }
            else
            {
                tmp = alloca(size + 1);
                memcpy(tmp, text, size);
                tmp[size] = '\0';
                evas_textblock_cursor_text_prepend(sd->cur, tmp);
            }
            break;
        case MD_TEXT_HTML:
            printf("text (%d) html: '%s'\n", type, str);
            break;
        case MD_TEXT_LATEXMATH:
            printf("text (%d) latexmath: '%s'\n", type, str);
            break;
    }

    return 0;
}

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


Evas_Object *etui_md_add(Evas_Object *win)
{
    Evas *evas;
    Evas_Object *obj;

    EINA_SAFETY_ON_NULL_RETURN_VAL(win, NULL);

    evas = evas_object_evas_get(win);
    if (!evas)
    {
        ERR("can not get evas");
        return NULL;
    }

    if (!_smart) _smart_init();
    obj = evas_object_smart_add(evas, _smart);

    return obj;
}

void etui_md_file_set(Evas_Object *obj, const char *filename)
{
    Md *sd;
    Eina_File *file;
    MD_PARSER parser;
    MD_CHAR *data;
    MD_SIZE size;
    int ret;

    sd = evas_object_smart_data_get(obj);
    EINA_SAFETY_ON_NULL_RETURN(sd);

    if (!filename || !*filename)
    {
        ERR("filename is NULL or empty");
        return;
    }

    file = eina_file_open(filename, EINA_FALSE);
    if (!file)
    {
        ERR("filename %s can not be opened", filename);
        return;
    }

    data = eina_file_map_all(file, EINA_FILE_POPULATE);
    size = eina_file_size_get(file);

    /* parse markdown file */
    memset(&parser, 0, sizeof(parser));
    parser.flags = MD_DIALECT_COMMONMARK;
    parser.enter_block = _md_enter_block;
    parser.leave_block = _md_leave_block;
    parser.enter_span = _md_enter_span;
    parser.leave_span = _md_leave_span;
    parser.text = _md_text;
    ret = md_parse(data, size, &parser, sd);

    eina_file_close(file);

    if (ret != 0)
    {
        ERR("Can not parse file %s", filename);
    }
}
