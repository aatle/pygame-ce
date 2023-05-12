#define PYGAMEAPI_WINDOW_INTERNAL

#include "pygame.h"

#include "pgcompat.h"

#include <SDL_syswm.h>

#include "doc/window_doc.h"

static PyTypeObject pgWindow_Type;

#define pgWindow_Check(x) \
    (PyObject_IsInstance((x), (PyObject *)&pgWindow_Type))

static PyObject *
get_grabbed_window(PyObject *self, PyObject *_null)
{
    SDL_Window *grabbed = SDL_GetGrabbedWindow();
    PyObject *win_obj = NULL;
    if (grabbed) {
        win_obj = SDL_GetWindowData(grabbed, "pg_window");
        if (!win_obj) {
            Py_RETURN_NONE;
        }
        Py_INCREF(win_obj);
        return win_obj;
    }
    Py_RETURN_NONE;
}

static PyObject *
window_destroy(pgWindowObject *self, PyObject *_null)
{
    if (self->_win) {
        SDL_DestroyWindow(self->_win);
        self->_win = NULL;
    }
    Py_RETURN_NONE;
}

static PyObject *
window_set_windowed(pgWindowObject *self, PyObject *_null)
{
    if (SDL_SetWindowFullscreen(self->_win, 0)) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }
    Py_RETURN_NONE;
}

static PyObject *
window_set_fullscreen(pgWindowObject *self, PyObject *args, PyObject *kwargs)
{
    SDL_bool desktop = SDL_FALSE;
    int flags = SDL_WINDOW_FULLSCREEN;
    char *kwids[] = {"desktop", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|p", kwids, &desktop)) {
        return NULL;
    }
    if (desktop) {
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    if (SDL_SetWindowFullscreen(self->_win, flags)) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }
    Py_RETURN_NONE;
}

static PyObject *
window_focus(pgWindowObject *self, PyObject *args, PyObject *kwargs)
{
    SDL_bool input_only = SDL_FALSE;
    char *kwids[] = {"input_only", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|p", kwids, &input_only)) {
        return NULL;
    }
    if (input_only) {
        if (SDL_SetWindowInputFocus(self->_win)) {
            return RAISE(pgExc_SDLError, SDL_GetError());
        }
    }
    else {
        SDL_RaiseWindow(self->_win);
    }
    Py_RETURN_NONE;
}

static PyObject *
window_hide(pgWindowObject *self, PyObject *_null)
{
    SDL_HideWindow(self->_win);
    Py_RETURN_NONE;
}

static PyObject *
window_show(pgWindowObject *self, PyObject *_null)
{
    SDL_ShowWindow(self->_win);
    Py_RETURN_NONE;
}

static PyObject *
window_restore(pgWindowObject *self, PyObject *_null)
{
    SDL_RestoreWindow(self->_win);
    Py_RETURN_NONE;
}

static PyObject *
window_maximize(pgWindowObject *self, PyObject *_null)
{
    SDL_MaximizeWindow(self->_win);
    Py_RETURN_NONE;
}

static PyObject *
window_minimize(pgWindowObject *self, PyObject *_null)
{
    SDL_MinimizeWindow(self->_win);
    Py_RETURN_NONE;
}

static PyObject *
window_set_modal_for(pgWindowObject *self, PyObject *arg)
{
    if (!pgWindow_Check(arg)) {
        return RAISE(PyExc_TypeError,
                     "Argument to set_modal_for must be a Window.");
    }
    if (!SDL_SetWindowModalFor(self->_win, ((pgWindowObject *)arg)->_win)) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }
    Py_RETURN_NONE;
}

static PyObject *
window_set_icon(pgWindowObject *self, PyObject *arg)
{
    if (!pgSurface_Check(arg)) {
        return RAISE(PyExc_TypeError,
                     "Argument to set_icon must be a Surface.");
    }
    SDL_SetWindowIcon(self->_win, pgSurface_AsSurface(arg));
    Py_RETURN_NONE;
}

static PyObject *
window_set_grab(pgWindowObject *self, PyObject *arg, void *v)
{
    int enable = PyObject_IsTrue(arg);
    if (enable == -1)
        return NULL;

    SDL_SetWindowGrab(self->_win, enable);

    Py_RETURN_NONE;
}

static PyObject *
window_get_grab(pgWindowObject *self, void *v)
{
    return PyBool_FromLong(SDL_GetWindowGrab(self->_win));
}

static PyObject *
window_set_title(pgWindowObject *self, PyObject *arg, void *v)
{
    const char *title;
    if (!PyUnicode_Check(arg)) {
        return RAISE(PyExc_TypeError, "Argument to set_title must be a str.");
    }
    title = PyUnicode_AsUTF8(arg);
    SDL_SetWindowTitle(self->_win, title);
    Py_RETURN_NONE;
}

static PyObject *
window_get_title(pgWindowObject *self, void *v)
{
    const char *title = SDL_GetWindowTitle(self->_win);
    return PyUnicode_FromString(title);
}

static PyObject *
window_set_resizable(pgWindowObject *self, PyObject *arg, void *v)
{
    int enable = PyObject_IsTrue(arg);
    if (enable == -1)
        return NULL;

    SDL_SetWindowResizable(self->_win, enable);

    Py_RETURN_NONE;
}

static PyObject *
window_get_resizable(pgWindowObject *self, void *v)
{
    return PyBool_FromLong(SDL_GetWindowFlags(self->_win) &
                           SDL_WINDOW_RESIZABLE);
}

static PyObject *
window_set_borderless(pgWindowObject *self, PyObject *arg, void *v)
{
    int enable = PyObject_IsTrue(arg);
    if (enable == -1)
        return NULL;

    SDL_SetWindowBordered(self->_win, !enable);

    Py_RETURN_NONE;
}

static PyObject *
window_get_borderless(pgWindowObject *self, void *v)
{
    return PyBool_FromLong(SDL_GetWindowFlags(self->_win) &
                           SDL_WINDOW_BORDERLESS);
}

static PyObject *
window_get_window_id(pgWindowObject *self, PyObject *_null)
{
    Uint32 window_id = SDL_GetWindowID(self->_win);
    if (!window_id) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }
    return PyLong_FromLong(window_id);
}

static PyObject *
window_set_size(pgWindowObject *self, PyObject *arg, void *v)
{
    int w, h;

    if (!pg_TwoIntsFromObj(arg, &w, &h)) {
        return RAISE(PyExc_TypeError, "invalid size argument");
    }

    if (w <= 0 || h <= 0) {
        return RAISE(
            PyExc_ValueError,
            "width or height should not be less than or equal to zero.");
    }

    SDL_SetWindowSize(self->_win, w, h);

    Py_RETURN_NONE;
}

static PyObject *
window_get_size(pgWindowObject *self, void *v)
{
    int w, h;
    PyObject *out = NULL;

    SDL_GetWindowSize(self->_win, &w, &h);
    out = Py_BuildValue("(i,i)", w, h);

    if (!out)
        return NULL;

    return out;
}

static PyObject *
window_set_position(pgWindowObject *self, PyObject *arg, void *v)
{
    int x, y;

    if (Py_TYPE(arg) == &PyLong_Type) {
        x = y = PyLong_AsLong(arg);
        if (x != SDL_WINDOWPOS_CENTERED && x != SDL_WINDOWPOS_UNDEFINED) {
            return RAISE(PyExc_TypeError, "invalid position argument");
        }
    }
    else if (!pg_TwoIntsFromObj(arg, &x, &y)) {
        return RAISE(PyExc_TypeError, "invalid position argument");
    }

    SDL_SetWindowPosition(self->_win, x, y);

    Py_RETURN_NONE;
}

static PyObject *
window_get_position(pgWindowObject *self, void *v)
{
    int x, y;
    PyObject *out = NULL;

    SDL_GetWindowPosition(self->_win, &x, &y);
    out = Py_BuildValue("(i,i)", x, y);

    if (!out)
        return NULL;

    return out;
}

static PyObject *
window_set_opacity(pgWindowObject *self, PyObject *arg, void *v)
{
    float opacity;
    opacity = (float)PyFloat_AsDouble(arg);
    if (PyErr_Occurred()) {
        return NULL;
    }
    if (SDL_SetWindowOpacity(self->_win, opacity)) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }
    Py_RETURN_NONE;
}

static PyObject *
window_get_opacity(pgWindowObject *self, void *v)
{
    float opacity;
    if (SDL_GetWindowOpacity(self->_win, &opacity)) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }
    return PyFloat_FromDouble((double)opacity);
}

static PyObject *
window_get_display_index(pgWindowObject *self, PyObject *_null)
{
    int index = SDL_GetWindowDisplayIndex(self->_win);
    if (index < 0) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }
    return PyLong_FromLong(index);
}

static void
window_dealloc(pgWindowObject *self, PyObject *_null)
{
    PyObject_Free(self);
}

static int
window_init(pgWindowObject *self, PyObject *args, PyObject *kwargs)
{
    char *title = "pygame window";
    PyObject *size = NULL;
    int size_w = 640, size_h = 480;
    PyObject *position = NULL;
    int pos_x = SDL_WINDOWPOS_UNDEFINED;
    int pos_y = SDL_WINDOWPOS_UNDEFINED;
    Uint32 flags = 0;
    SDL_Window *_win = NULL;

    Py_ssize_t dict_pos = 0;
    PyObject *_key, *_value, *_kw;
    const char *_key_str;
    char _exc_str[64];
    int _value_bool;

    _kw = PyDict_New();
    if (!_kw)
        return -1;

    if (kwargs) {
        while (PyDict_Next(kwargs, &dict_pos, &_key, &_value)) {
            if (!PyUnicode_Check(_key)) {
                PyErr_SetString(PyExc_TypeError, "keywords must be strings");
                return -1;
            }

            _key_str = PyUnicode_AsUTF8(_key);
            if (!_key_str)
                return -1;

            if (!strcmp(_key_str, "title") || !strcmp(_key_str, "size") ||
                !strcmp(_key_str, "position")) {
                PyDict_SetItem(_kw, _key, _value);
            }

            // handle **flags
            else {
                _value_bool = PyObject_IsTrue(_value);
                if (_value_bool == -1)
                    return -1;

                if (!strcmp(_key_str, "opengl")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_OPENGL;
                }
                else if (!strcmp(_key_str, "fullscreen")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_FULLSCREEN;
                }
                else if (!strcmp(_key_str, "fullscreen_desktop")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
                }
                else if (!strcmp(_key_str, "hidden")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_HIDDEN;
                }
                else if (!strcmp(_key_str, "borderless")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_BORDERLESS;
                }
                else if (!strcmp(_key_str, "resizable")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_RESIZABLE;
                }
                else if (!strcmp(_key_str, "minimized")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_MINIMIZED;
                }
                else if (!strcmp(_key_str, "maximized")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_MAXIMIZED;
                }
                else if (!strcmp(_key_str, "input_grabbed")) {
                    if (_value_bool)
#if SDL_VERSION_ATLEAST(2, 0, 16)
                        flags |= SDL_WINDOW_MOUSE_GRABBED;
#else
                        flags |= SDL_WINDOW_INPUT_GRABBED;
#endif
                }
                else if (!strcmp(_key_str, "input_focus")) {
                    if (_value_bool) {
                        flags |= SDL_WINDOW_INPUT_FOCUS;
                    }
                }
                else if (!strcmp(_key_str, "mouse_focus")) {
                    if (_value_bool) {
                        flags |= SDL_WINDOW_MOUSE_FOCUS;
                    }
                }
                else if (!strcmp(_key_str, "foreign")) {
                    if (_value_bool) {
                        flags |= SDL_WINDOW_FOREIGN;
                    }
                }
                else if (!strcmp(_key_str, "allow_high_dpi")) {
                    if (_value_bool) {
                        flags |= SDL_WINDOW_ALLOW_HIGHDPI;
                    }
                }
                else if (!strcmp(_key_str, "mouse_capture")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_MOUSE_CAPTURE;
                }
                else if (!strcmp(_key_str, "always_on_top")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_ALWAYS_ON_TOP;
                }
                else if (!strcmp(_key_str, "skip_taskbar")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_SKIP_TASKBAR;
                }
                else if (!strcmp(_key_str, "utility")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_UTILITY;
                }
                else if (!strcmp(_key_str, "tooltip")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_TOOLTIP;
                }
                else if (!strcmp(_key_str, "popup_menu")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_POPUP_MENU;
                }
                else if (!strcmp(_key_str, "vulkan")) {
                    if (_value_bool)
                        flags |= SDL_WINDOW_VULKAN;
                }
                else {
                    sprintf(_exc_str, "__init__ got an unexpected flag \'%s\'",
                            _key_str);
                    PyErr_SetString(PyExc_TypeError, _exc_str);
                    return -1;
                }
            }
        }
    }

    char *kwids[] = {"title", "size", "position", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, _kw, "|sOO", kwids, &title, &size,
                                     &position)) {
        return -1;
    }

    if (size) {
        if (!pg_TwoIntsFromObj(size, &size_w, &size_h)) {
            PyErr_SetString(PyExc_TypeError, "invalid size argument");
            return -1;
        }
    }

    if (size_w <= 0 || size_h <= 0) {
        PyErr_SetString(
            PyExc_ValueError,
            "width or height should not be less than or equal to zero.");
        return -1;
    }

    if (position) {
        if (Py_TYPE(position) == &PyLong_Type) {
            pos_x = pos_y = PyLong_AsLong(position);
            if (pos_x != SDL_WINDOWPOS_CENTERED &&
                pos_x != SDL_WINDOWPOS_UNDEFINED) {
                PyErr_SetString(PyExc_TypeError, "invalid positon argument");
                return -1;
            }
        }
        else if (!pg_TwoIntsFromObj(position, &pos_x, &pos_y)) {
            PyErr_SetString(PyExc_TypeError, "invalid positon argument");
            return -1;
        }
    }

    _win = SDL_CreateWindow(title, pos_x, pos_y, size_w, size_h, flags);
    if (!_win) {
        PyErr_SetString(pgExc_SDLError, SDL_GetError());
        return -1;
    }
    self->_win = _win;
    self->_is_borrowed = SDL_FALSE;

    SDL_SetWindowData(_win, "pg_window", self);

    return 0;
}

static PyObject *
window_from_display_module(PyTypeObject *cls, PyObject *_null)
{
    SDL_Window *window;
    pgWindowObject *self;
    window = pg_GetDefaultWindow();
    if (!window) {
        return RAISE(pgExc_SDLError, SDL_GetError());
    }

    self = (pgWindowObject *)(cls->tp_new(cls, NULL, NULL));
    self->_win = window;
    self->_is_borrowed = SDL_TRUE;
    SDL_SetWindowData(window, "pg_window", self);
    return (PyObject *)self;
}

static PyMethodDef window_methods[] = {
    {"destroy", (PyCFunction)window_destroy, METH_NOARGS,
     DOC_WINDOW_WINDOW_DESTROY},
    {"set_windowed", (PyCFunction)window_set_windowed, METH_NOARGS,
     DOC_WINDOW_WINDOW_SETWINDOWED},
    {"set_fullscreen", (PyCFunction)window_set_fullscreen,
     METH_VARARGS | METH_KEYWORDS, DOC_WINDOW_WINDOW_SETFULLSCREEN},
    {"focus", (PyCFunction)window_focus, METH_VARARGS | METH_KEYWORDS,
     DOC_WINDOW_WINDOW_FOCUS},
    {"hide", (PyCFunction)window_hide, METH_NOARGS, DOC_WINDOW_WINDOW_HIDE},
    {"show", (PyCFunction)window_show, METH_NOARGS, DOC_WINDOW_WINDOW_SHOW},
    {"restore", (PyCFunction)window_restore, METH_NOARGS,
     DOC_WINDOW_WINDOW_RESTORE},
    {"maximize", (PyCFunction)window_maximize, METH_NOARGS,
     DOC_WINDOW_WINDOW_MAXIMIZE},
    {"minimize", (PyCFunction)window_minimize, METH_NOARGS,
     DOC_WINDOW_WINDOW_MINIMIZE},
    {"set_modal_for", (PyCFunction)window_set_modal_for, METH_O,
     DOC_WINDOW_WINDOW_SETMODALFOR},
    {"set_icon", (PyCFunction)window_set_icon, METH_O,
     DOC_WINDOW_WINDOW_SETICON},
    {"from_display_module", (PyCFunction)window_from_display_module,
     METH_CLASS | METH_NOARGS, DOC_WINDOW_WINDOW_FROMDISPLAYMODULE},
    {NULL, NULL, 0, NULL}};

static PyGetSetDef _window_getset[] = {
    {"grab", (getter)window_get_grab, (setter)window_set_grab, NULL, NULL},
    {"title", (getter)window_get_title, (setter)window_set_title, NULL, NULL},
    {"resizable", (getter)window_get_resizable, (setter)window_set_resizable,
     NULL, NULL},
    {"borderless", (getter)window_get_borderless,
     (setter)window_set_borderless, NULL, NULL},
    {"size", (getter)window_get_size, (setter)window_set_size, NULL, NULL},
    {"position", (getter)window_get_position, (setter)window_set_position,
     NULL, NULL},
    {"opacity", (getter)window_get_opacity, (setter)window_set_opacity, NULL,
     NULL},
    {"display_index", (getter)window_get_display_index, NULL, NULL, NULL},
    {"id", (getter)window_get_window_id, NULL, NULL, NULL},
    {NULL, 0, NULL, NULL, NULL} /* Sentinel */
};

static PyTypeObject pgWindow_Type = {
    PyVarObject_HEAD_INIT(NULL, 0).tp_name = "pygame.window.Window",
    .tp_basicsize = sizeof(pgWindowObject),
    .tp_dealloc = (destructor)window_dealloc,
    .tp_doc = DOC_WINDOW_WINDOW,
    .tp_methods = window_methods,
    .tp_init = (initproc)window_init,
    .tp_new = PyType_GenericNew,
    .tp_getset = _window_getset};

static PyMethodDef _window_methods[] = {
    {"get_grabbed_window", (PyCFunction)get_grabbed_window, METH_NOARGS,
     DOC_WINDOW_GETGRABBEDWINDOW},
    {NULL, NULL, 0, NULL}};

MODINIT_DEFINE(window)
{
    PyObject *module, *apiobj;
    static void *c_api[PYGAMEAPI_WINDOW_NUMSLOTS];

    static struct PyModuleDef _module = {PyModuleDef_HEAD_INIT,
                                         "window",
                                         DOC_WINDOW,
                                         -1,
                                         _window_methods,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL};

    /* imported needed apis; Do this first so if there is an error
       the module is not loaded.
    */
    import_pygame_base();
    if (PyErr_Occurred()) {
        return NULL;
    }

    import_pygame_surface();
    if (PyErr_Occurred()) {
        return NULL;
    }

    import_pygame_rect();
    if (PyErr_Occurred()) {
        return NULL;
    }

    if (PyType_Ready(&pgWindow_Type) < 0) {
        return NULL;
    }

    /* create the module */
    module = PyModule_Create(&_module);
    if (module == 0) {
        return NULL;
    }

    Py_INCREF(&pgWindow_Type);
    if (PyModule_AddObject(module, "Window", (PyObject *)&pgWindow_Type)) {
        Py_DECREF(&pgWindow_Type);
        Py_DECREF(module);
        return NULL;
    }
    Py_INCREF(&pgWindow_Type);
    if (PyModule_AddObject(module, "WindowType", (PyObject *)&pgWindow_Type)) {
        Py_DECREF(&pgWindow_Type);
        Py_DECREF(module);
        return NULL;
    }

    c_api[0] = &pgWindow_Type;
    apiobj = encapsulate_api(c_api, "window");
    if (PyModule_AddObject(module, PYGAMEAPI_LOCAL_ENTRY, apiobj)) {
        Py_XDECREF(apiobj);
        Py_DECREF(module);
        return NULL;
    }

    return module;
}