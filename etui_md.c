
#include <Elementary.h>

#include <md4c.h>

#include "etui_md.h"

/*
 * TODO
 *
 * Blocks
 *
 * [X] doc
 * [ ] quote
 * [X] ul
 * [X] ol
 * [X] li
 * [ ] hr
 * [X] h
 * [X] code
 * [ ] html
 * [X] p
 * [ ] table
 * [ ] thead
 * [ ] tbody
 * [ ] tr
 * [ ] th
 * [ ] td
 *
 * Spans
 *
 * [X] em
 * [X] strong
 * [ ] a
 * [ ] img
 * [X] code
 * [X] del
 * [ ] latexmath(display)
 * [ ] wikilink
 * [X] u
 *
 * Texts
 *
 * [X] normal
 * [ ] nullchar
 * [X] br
 * [X] softbr
 * [ ] entity
 * [X] code
 * [ ] html
 * [ ] latexmath
 *
 */

/*============================================================================*
 *                                  Local                                     *
 *============================================================================*/

typedef struct MD_List_ MD_List;
typedef struct MD_Cursors_ Md_Cursors;

struct MD_List_
{
    int is_tight;
    unsigned int start; /* for OL only*/
    unsigned int current_idx;
    MD_CHAR mark;
    char type; /* 0: OL, 1: UL */
};

struct MD_Cursors_
{
    Evas_Textblock_Cursor *begin;
    Evas_Textblock_Cursor *end;
    Evas_Object *backing;
};

typedef struct Md_ Md;

struct Md_
{
    Evas_Object_Smart_Clipped_Data __clipped_data;
    Evas_Object *tb;
    Evas_Textblock_Style *st;
    Evas_Textblock_Cursor *cur;
    Md_Cursors *curs_current;
    Eina_List *cursors;
    Eina_List *lists;
};

static char *_md_list_item[4] =
{
    "●",
    "○",
    "■",
    "□"
};

static Evas_Smart *_smart = NULL;
static Evas_Smart_Class _parent_sc = EVAS_SMART_CLASS_INIT_NULL;


/******** Evas smart callbacks ********/

static void
_smart_add(Evas_Object *obj)
{
   Md *sd;

   sd = calloc(1, sizeof(Md));
   EINA_SAFETY_ON_NULL_RETURN(sd);

   evas_object_smart_data_set(obj, sd);

   _parent_sc.add(obj);

   sd->tb = evas_object_textblock_add(evas_object_evas_get(obj));
   evas_object_scale_set(sd->tb, elm_config_scale_get());

   sd->st = evas_textblock_style_new();
   /* textblock API guards against NULL style, so no need to check twice */
   char buf[128];
   snprintf(buf, sizeof(buf),
            "DEFAULT='font=Sans font_size=10 color=#a0a0a0'");
   evas_textblock_style_set(sd->st, buf);
   evas_object_textblock_style_set(sd->tb, sd->st);

   evas_object_smart_member_add(sd->tb, obj);
   evas_object_show(sd->tb);

   sd->cur = evas_object_textblock_cursor_get(sd->tb);
}

static void
_smart_del(Evas_Object *obj)
{
   Md *sd;
   Md_Cursors *cursor;
   Eina_List *l;

   sd = evas_object_smart_data_get(obj);
   EINA_SAFETY_ON_NULL_RETURN(sd);

   EINA_LIST_FOREACH(sd->cursors, l, cursor)
   {
       evas_object_del(cursor->backing);
       evas_textblock_cursor_free(cursor->begin);
       evas_textblock_cursor_free(cursor->end);
   }
   evas_textblock_style_free(sd->st);
   evas_object_del(sd->tb);

   _parent_sc.del(obj);
   evas_object_smart_data_set(obj, NULL);

   memset(sd, 0, sizeof(*sd));
   free(sd);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w EINA_UNUSED, Evas_Coord h EINA_UNUSED)
{
   evas_object_smart_changed(obj);
}

static void
_smart_calculate(Evas_Object *obj)
{
   Md *sd = evas_object_smart_data_get(obj);
   Md_Cursors *cursor;
   Eina_List *l;
   Evas_Coord ox, oy, ow, oh, w, h;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   evas_object_geometry_get(obj, &ox, &oy, &ow, &oh);

   evas_object_resize(sd->tb, 0, 0);
   evas_object_textblock_size_formatted_get(sd->tb, &w, NULL);
   evas_object_geometry_set(sd->tb, ox, oy, ow, oh);
   evas_object_textblock_size_formatted_get(sd->tb, NULL, &h);
   evas_object_size_hint_min_set(obj, w, h);

   EINA_LIST_FOREACH(sd->cursors, l, cursor)
    {
       int xb, yb, wb, hb;
       int xe, ye, we, he;

       evas_textblock_cursor_line_geometry_get(cursor->begin, &xb, &yb, &wb, &hb);
       evas_textblock_cursor_line_geometry_get(cursor->end, &xe, &ye, &we, &he);

       if (!cursor->backing)
         {
            Evas_Object *r = evas_object_rectangle_add(evas_object_evas_get(obj));
            evas_object_color_set(r, 32, 32, 32, 255);
            evas_object_smart_member_add(r, obj);
            evas_object_stack_below(r, sd->tb);
            evas_object_show(r);
            cursor->backing = r;
         }
       evas_object_geometry_set(cursor->backing, ox, oy + yb, ow, ye - yb);
    }
}

static void
_smart_init(void)
{
   static Evas_Smart_Class sc;

   ERR(" * %s", __FUNCTION__);

   evas_object_smart_clipped_smart_set(&_parent_sc);
   sc           = _parent_sc;
   sc.name      = "md";
   sc.version   = EVAS_SMART_CLASS_VERSION;
   sc.add       = _smart_add;
   sc.del       = _smart_del;
   sc.resize    = _smart_resize;
   sc.calculate = _smart_calculate;
   _smart = evas_smart_class_new(&sc);
}


/******** Markdown callbacks ********/

static int
_md_enter_block(MD_BLOCKTYPE type, void *detail, void *data)
{
    Md *sd = data;

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
        {
            MD_List *l;
            MD_BLOCK_UL_DETAIL *d;

            printf("ul (%p)\n", sd->lists);
            fflush(stdout);

            if (!sd->lists)
                evas_textblock_cursor_format_prepend(sd->cur, "\n");

            d = (MD_BLOCK_UL_DETAIL *)detail;
            l = (MD_List *)calloc(1, sizeof(MD_List));
            if (!l)
                return -10;

            l->type = 1;
            l->is_tight = d->is_tight;
            l->mark = d->mark;

            sd->lists = eina_list_append(sd->lists, l);
            break;
        }
        case MD_BLOCK_OL:
        {
            MD_List *l;
            MD_BLOCK_OL_DETAIL *d;

            if (!sd->lists)
                evas_textblock_cursor_format_prepend(sd->cur, "\n");

            d = (MD_BLOCK_OL_DETAIL *)detail;
            l = (MD_List *)calloc(1, sizeof(MD_List));
            if (!l)
                return -10;

            l->type = 0;
            l->is_tight = d->is_tight;
            l->mark = d->mark_delimiter;
            l->start = d->start;
            l->current_idx = d->start;

            printf("ol : start : %d\n", l->start);
            fflush(stdout);

            sd->lists = eina_list_append(sd->lists, l);
            break;
        }
        case MD_BLOCK_LI:
        {
            MD_List *l;
            unsigned int i;
            unsigned int count;

            l = eina_list_last_data_get(sd->lists);
            if (!l)
                return -10;

            count = eina_list_count(sd->lists);
            if (count == 0)
                return -10;

            for (i = 0; i < count; i++)
                evas_textblock_cursor_format_prepend(sd->cur, "\t");
            if (l->type == 1)
            {
                if (count >= 4)
                    evas_textblock_cursor_text_prepend(sd->cur, _md_list_item[3]);
                else
                    evas_textblock_cursor_text_prepend(sd->cur, _md_list_item[count - 1]);
            }
            else
            {
                char buf[256];

                snprintf(buf, sizeof(buf), "%d%c", l->current_idx, l->mark);
                evas_textblock_cursor_text_prepend(sd->cur, buf);
                l->current_idx++;
            }
            evas_textblock_cursor_text_prepend(sd->cur, " ");
            printf("li\n");
            fflush(stdout);
            break;
        }
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

            evas_textblock_cursor_format_prepend(sd->cur, "\n");
            printf("code\n");

            evas_textblock_cursor_line_geometry_get(sd->cur, &x, &y, &w, &h);
            printf("line code 1: %d %d %d %d\n", x,y,w,h);

            sd->curs_current = (Md_Cursors *)calloc(1, sizeof(Md_Cursors));
            if (sd->curs_current)
            {
                sd->curs_current->begin = evas_object_textblock_cursor_new(sd->tb);
                evas_textblock_cursor_copy(sd->cur, sd->curs_current->begin);
            }

            evas_textblock_cursor_format_prepend(sd->cur, "+Monospace color=#bbbbbb wrap=none");
            break;
          }
        case MD_BLOCK_HTML:
            printf("raw html\n");
            break;
        case MD_BLOCK_P:
            if (!sd->lists)
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
    fflush(stdout);

    return 0;
}

static int
_md_leave_block(MD_BLOCKTYPE type, void *detail, void *data)
{
    Md *sd = data;

    printf("%s : ", __FUNCTION__);

    switch (type)
    {
        case MD_BLOCK_DOC:
            printf("body\n");
            break;
        case MD_BLOCK_QUOTE:
            printf("blockquote\n");
            break;
        case MD_BLOCK_UL:
        {
            MD_List *l;

            l = eina_list_last_data_get(sd->lists);
            if (!l)
                return -10;

            sd->lists = eina_list_remove(sd->lists, l);
            free(l);
            if (eina_list_count(sd->lists) == 0)
            {
                eina_list_free(sd->lists);
                sd->lists = NULL;
            }

            printf("ul\n");
            fflush(stdout);
            break;
        }
        case MD_BLOCK_OL:
        {
            MD_List *l;

            l = eina_list_last_data_get(sd->lists);
            if (!l)
                return -10;

            sd->lists = eina_list_remove(sd->lists, l);
            free(l);
            if (eina_list_count(sd->lists) == 0)
            {
                eina_list_free(sd->lists);
                sd->lists = NULL;
            }

            printf("ol\n");
            fflush(stdout);
            break;
        }
        case MD_BLOCK_LI:
        {
            MD_List *l;

            l = eina_list_last_data_get(sd->lists);
            if (!l)
                return -10;

            /* if (l->is_tight) */
            /*     evas_textblock_cursor_format_prepend(sd->cur, "\n"); */

            printf("li %d\n", l->is_tight);
            fflush(stdout);
            break;
        }
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
                evas_textblock_cursor_copy(sd->cur, sd->curs_current->end);
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
        {
            evas_textblock_cursor_format_prepend(sd->cur, "\n");
            if (sd->lists)
            {
                MD_List *l;

                l = eina_list_last_data_get(sd->lists);
                if (!l)
                    return -10;

                if (!l->is_tight)
                    evas_textblock_cursor_format_prepend(sd->cur, "\n");
            }
            printf("p\n");
            break;
        }
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
    fflush(stdout);

    return 0;
}

static int
_md_enter_span(MD_SPANTYPE type, void *detail, void *data)
{
    Md *sd = data;

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
        {
            MD_SPAN_A_DETAIL *d;

            d = (MD_SPAN_A_DETAIL *)detail;
            printf("a : %s (%s)\n", d->href.text, d->title.text);
            break;
        }
        case MD_SPAN_IMG:
        {
            char *buf;
            MD_SPAN_IMG_DETAIL *d;

            d = (MD_SPAN_IMG_DETAIL *)detail;
            buf = alloca(d->src.size + 1);
            memcpy(buf, d->src.text, d->src.size);
            buf[d->src.size] = '\0';
            printf("img : %s (%s)\n", buf, d->title.text);
            break;
        }
        case MD_SPAN_CODE:
            printf("code\n");
            /* fg */
            evas_textblock_cursor_format_prepend(sd->cur, "+Monospace color=#bbbbbb wrap=none");
            break;
        case MD_SPAN_DEL:
            printf("del\n");
            evas_textblock_cursor_format_prepend(sd->cur, "+strikethrough=on strikethrough_color=#a0a0a0");
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
            evas_textblock_cursor_format_prepend(sd->cur, "+underline=single underline_color=#a0a0a0");
            break;
    }
    fflush(stdout);

    return 0;
}

static int
_md_leave_span(MD_SPANTYPE type, void *detail, void *data)
{
    Md *sd = data;

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
            evas_textblock_cursor_format_prepend(sd->cur, "-");
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
            evas_textblock_cursor_format_prepend(sd->cur, "-");
            break;
    }
    fflush(stdout);

    return 0;
}

static int
_md_text(MD_TEXTTYPE type, const MD_CHAR *text, MD_SIZE size, void *data)
{
    Md *sd = data;
    char *tmp;
    char *str = alloca(size+1);

    memcpy(str, text, size);
    str[size]=0;
    printf("text (%d) : '%s'\n", type, str);
    if (size == 1)
        printf("text (char) : '%c' '%d'\n", *text, *text);
    switch (type)
    {
        case MD_TEXT_NORMAL:
        {
            printf("text (%d) normal : '%s' (list: %p)\n", type, str, sd->lists);
            fflush(stdout);
            tmp = alloca(size + 1);
            memcpy(tmp, text, size);
            tmp[size] = '\0';
            evas_textblock_cursor_text_prepend(sd->cur, tmp);

            if (sd->lists)
            {
                MD_List *l;

                l = eina_list_last_data_get(sd->lists);
                if (!l)
                    return -10;

                printf("text normal is_tight : %d\n", l->is_tight);
                fflush(stdout);
                if (l->is_tight)
                    evas_textblock_cursor_format_prepend(sd->cur, "\n");
            }

            break;
        }
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
    fflush(stdout);

    return 0;
}

/*============================================================================*
 *                                 Global                                     *
 *============================================================================*/


Evas_Object *etui_md_add(Evas_Object *win)
{
   EINA_SAFETY_ON_NULL_RETURN_VAL(win, NULL);

   if (!_smart) _smart_init();
   return evas_object_smart_add(evas_object_evas_get(win), _smart);
}

void etui_md_file_set(Evas_Object *obj, const char *filename)
{
   Md *sd = evas_object_smart_data_get(obj);
   Eina_File *file;
   MD_PARSER parser;
   MD_CHAR *data;
   MD_SIZE size;
   int ret;

   EINA_SAFETY_ON_NULL_RETURN(sd);

   if (!filename || !*filename)
     {
        ERR("filename is NULL or empty");
        return;
     }

   // FIXME: delete cursors + backing objects in them
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
   parser.flags =
       MD_DIALECT_COMMONMARK |
       MD_FLAG_UNDERLINE |
       MD_FLAG_STRIKETHROUGH;
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
