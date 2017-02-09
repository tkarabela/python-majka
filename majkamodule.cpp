/* Copyright, under GPL 2.0 2016 <petr.pulc@wolterskluwer.com> */

#include <Python.h>
#include <structmember.h>
#include <iostream>
#include "majka/majka.h"

#if PY_MAJOR_VERSION >= 3
  #define PY3K
#endif

typedef struct {
  PyObject_HEAD
  fsa* majka;
  int flags;
  bool tags;
} Majka;

static void Majka_dealloc(Majka* self) {
  delete self->majka;
  Py_TYPE(self)->tp_free(reinterpret_cast<Majka*>(self));
}

static PyObject* Majka_new(PyTypeObject* type,
                           PyObject* args,
                           PyObject* kwds) {
  Majka* self;
  self = reinterpret_cast<Majka*>(type->tp_alloc(type, 0));
  self->flags = 0;
  self->tags = true;
  return reinterpret_cast<PyObject*>(self);
}

static int Majka_init(Majka* self, PyObject* args, PyObject* kwds) {
  const char* file = NULL;
  static char* kwlist[] = {const_cast<char*>("file"), NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &file)) {
    return -1;
  }

  if (!file) {
      PyErr_SetString(PyExc_TypeError,
                      "Majka initialization needs a path to dictionary");
    return -1;
  }

  self->majka = new fsa(file);

  if (self->majka->state) {
      PyErr_SetString(PyExc_IOError,
                      "Majka dictionary is unreadable or invalid");
    return -1;
  }

  return 0;
}

static int stealing_PyList_Append(PyObject* list_obj, PyObject* item_obj) {
  int rv = PyList_Append(list_obj, item_obj);
  Py_DECREF(item_obj);
  return rv;
}

static int stealing_PyDict_SetItemString(PyObject* dict_obj, const char *key, PyObject* value_obj) {
  int rv = PyDict_SetItemString(dict_obj, key, value_obj);
  Py_DECREF(value_obj);
  return rv;
}

static PyObject* Majka_tags(char * tag_string) {
  PyObject* tags = PyDict_New();
  char category = ' ';
  char tmp[] = {'\0', '\0'};
  PyObject* type = PyList_New(0);

  if (*tag_string == 'k') {
    ++tag_string;
    category = *tag_string;
    switch (category) {
      case '1':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("substantive"));
        break;
      case '2':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("adjective"));
        break;
      case '3':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("pronomina"));
        break;
      case '4':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("numeral"));
        break;
      case '5':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("verb"));
        break;
      case '6':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("adverb"));
        break;
      case '7':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("preposition"));
        break;
      case '8':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("conjuction"));
        break;
      case '9':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("particle"));
        break;
      case '0':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("interjection"));
        break;
      case 'I':
        stealing_PyDict_SetItemString(tags, "pos",
                                      PyUnicode_FromString("punctuation"));
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'e') {
    ++tag_string;
    switch (*tag_string) {
      case 'A':
        stealing_PyDict_SetItemString(tags, "negation", Py_False);
        break;
      case 'N':
        stealing_PyDict_SetItemString(tags, "negation", Py_True);
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'a') {
    ++tag_string;
    switch (*tag_string) {
      case 'P':
        stealing_PyDict_SetItemString(tags, "aspect",
                                      PyUnicode_FromString("perfect"));
        break;
      case 'I':
        stealing_PyDict_SetItemString(tags, "aspect",
                                      PyUnicode_FromString("imperfect"));
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'm') {
    ++tag_string;
    switch (*tag_string) {
      case 'F':
        stealing_PyDict_SetItemString(tags, "mode",
                                      PyUnicode_FromString("infinitive"));
        break;
      case 'I':
        stealing_PyDict_SetItemString(tags, "mode",
                                      PyUnicode_FromString("present indicative"));
        break;
      case 'R':
        stealing_PyDict_SetItemString(tags, "mode",
                                      PyUnicode_FromString("imperative"));
        break;
      case 'A':
        stealing_PyDict_SetItemString(tags, "mode",
                                      PyUnicode_FromString("active participle"));
        break;
      case 'N':
        stealing_PyDict_SetItemString(tags, "mode",
                                      PyUnicode_FromString("passive participle"));
        break;
      case 'S':
        stealing_PyDict_SetItemString(tags, "mode",
                        PyUnicode_FromString("adverbium participle, present"));
        break;
      case 'D':
        stealing_PyDict_SetItemString(tags, "mode",
                           PyUnicode_FromString("adverbium participle, past"));
        break;
      case 'B':
        stealing_PyDict_SetItemString(tags, "mode",
                                      PyUnicode_FromString("futureu indicative"));
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'p') {
    ++tag_string;
    tmp[0] = *tag_string;
    stealing_PyDict_SetItemString(tags, "person", PyLong_FromString(tmp, NULL, 10));
    ++tag_string;
  }

  if (*tag_string == 'g') {
    ++tag_string;
    switch (*tag_string) {
      case 'M':
        stealing_PyDict_SetItemString(tags, "gender",
                                      PyUnicode_FromString("masculine"));
        stealing_PyDict_SetItemString(tags, "animate", Py_True);
        break;
      case 'I':
        stealing_PyDict_SetItemString(tags, "gender",
                                      PyUnicode_FromString("masculine"));
        stealing_PyDict_SetItemString(tags, "animate", Py_False);
        break;
      case 'F':
        stealing_PyDict_SetItemString(tags, "gender",
                                      PyUnicode_FromString("feminine"));
        break;
      case 'N':
        stealing_PyDict_SetItemString(tags, "gender",
                                      PyUnicode_FromString("neuter"));
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'n') {
    ++tag_string;
    switch (*tag_string) {
      case 'S':
        stealing_PyDict_SetItemString(tags, "singular", Py_True);
        break;
      case 'P':
        stealing_PyDict_SetItemString(tags, "plural", Py_True);
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'c') {
    ++tag_string;
    tmp[0] = *tag_string;
    stealing_PyDict_SetItemString(tags, "case", PyLong_FromString(tmp, NULL, 10));
    ++tag_string;
  }

  if (*tag_string == 'p') {  // Duplicate
    ++tag_string;
    tmp[0] = *tag_string;
    stealing_PyDict_SetItemString(tags, "person", PyLong_FromString(tmp, NULL, 10));
    ++tag_string;
  }

  if (*tag_string == 'd') {
    ++tag_string;
    tmp[0] = *tag_string;
    stealing_PyDict_SetItemString(tags, "degree", PyLong_FromString(tmp, NULL, 10));
    ++tag_string;
  }

  if (*tag_string == 'x') {
    ++tag_string;
    switch (category) {
      case '1':
        switch (*tag_string) {
          case 'P':
            stealing_PyList_Append(type, PyUnicode_FromString("half"));
            break;
          case 'F':
            stealing_PyList_Append(type, PyUnicode_FromString("family surname"));
            break;
        }
        break;
      case '3':
        switch (*tag_string) {
          case 'P':
            stealing_PyList_Append(type, PyUnicode_FromString("personal"));
            break;
          case 'O':
            stealing_PyList_Append(type, PyUnicode_FromString("possessive"));
            break;
          case 'D':
            stealing_PyList_Append(type, PyUnicode_FromString("demonstrative"));
            break;
          case 'T':
            stealing_PyList_Append(type, PyUnicode_FromString("deliminative"));
            break;
        }
        break;
      case '4':
        switch (*tag_string) {
          case 'C':
            stealing_PyList_Append(type, PyUnicode_FromString("cardinal"));
            break;
          case 'O':
            stealing_PyList_Append(type, PyUnicode_FromString("ordinal"));
            break;
          case 'R':
            stealing_PyList_Append(type, PyUnicode_FromString("reproductive"));
            break;
        }
        break;
      case '6':
        switch (*tag_string) {
          case 'D':
            stealing_PyList_Append(type, PyUnicode_FromString("demonstrative"));
            break;
          case 'T':
            stealing_PyList_Append(type, PyUnicode_FromString("delimitative"));
            break;
        }
        break;
      case '8':
        switch (*tag_string) {
          case 'C':
            stealing_PyList_Append(type, PyUnicode_FromString("coordinate"));
            break;
          case 'S':
            stealing_PyList_Append(type, PyUnicode_FromString("subordinate"));
            break;
        }
        break;
      case 'I':
        switch (*tag_string) {
          case '.':
            stealing_PyList_Append(type, PyUnicode_FromString("stop"));
            break;
          case ',':
            stealing_PyList_Append(type, PyUnicode_FromString("semi-stop"));
            break;
          case '"':
            stealing_PyList_Append(type, PyUnicode_FromString("parenthesis"));
            break;
          case '(':
            stealing_PyList_Append(type, PyUnicode_FromString("opening"));
            break;
          case ')':
            stealing_PyList_Append(type, PyUnicode_FromString("closing"));
            break;
          case '~':
            stealing_PyList_Append(type, PyUnicode_FromString("other"));
            break;
        }
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'y') {
    ++tag_string;
    switch (*tag_string) {
      case 'F':
        stealing_PyList_Append(type, PyUnicode_FromString("reflective"));
        break;
      case 'Q':
        stealing_PyList_Append(type, PyUnicode_FromString("interrogative"));
        break;
      case 'R':
        stealing_PyList_Append(type, PyUnicode_FromString("relative"));
        break;
      case 'N':
        stealing_PyList_Append(type, PyUnicode_FromString("negative"));
        break;
      case 'I':
        stealing_PyList_Append(type, PyUnicode_FromString("indeterminate"));
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 't') {
    ++tag_string;
    switch (*tag_string) {
        case 'S':
            stealing_PyList_Append(type, PyUnicode_FromString("status"));
            break;
        case 'D':
            stealing_PyList_Append(type, PyUnicode_FromString("modal"));
            break;
        case 'T':
            stealing_PyList_Append(type, PyUnicode_FromString("time"));
            break;
        case 'A':
            stealing_PyList_Append(type, PyUnicode_FromString("respect"));
            break;
        case 'C':
            stealing_PyList_Append(type, PyUnicode_FromString("reason"));
            break;
        case 'L':
            stealing_PyList_Append(type, PyUnicode_FromString("place"));
            break;
        case 'M':
            stealing_PyList_Append(type, PyUnicode_FromString("manner"));
            break;
        case 'Q':
            stealing_PyList_Append(type, PyUnicode_FromString("extent"));
            break;
    }
    ++tag_string;
  }

  if (*tag_string == 'z') {
    ++tag_string;
    switch (*tag_string) {
      case 'S':
        stealing_PyDict_SetItemString(tags, "subclass",
                                      PyUnicode_FromString("-s enclictic"));
        break;
      case 'Y':
        stealing_PyDict_SetItemString(tags, "subclass",
                                      PyUnicode_FromString("conditional"));
        break;
      case 'A':
        stealing_PyDict_SetItemString(tags, "subclass",
                                      PyUnicode_FromString("abbreviation"));
        break;
    }
    ++tag_string;
  }

  if (*tag_string == 'w') {
    ++tag_string;
    switch (*tag_string) {
      case 'B':
        stealing_PyDict_SetItemString(tags, "style",
                                      PyUnicode_FromString("poeticism"));
        break;
      case 'H':
        stealing_PyDict_SetItemString(tags, "style",
                                      PyUnicode_FromString("conversational"));
        break;
      case 'N':
        stealing_PyDict_SetItemString(tags, "style",
                                      PyUnicode_FromString("dialectal"));
        break;
      case 'R':
        stealing_PyDict_SetItemString(tags, "style",
                                      PyUnicode_FromString("rare"));
        break;
      case 'Z':
        stealing_PyDict_SetItemString(tags, "style",
                                      PyUnicode_FromString("obsolete"));
        break;
    }
    ++tag_string;
  }

  if (*tag_string == '~') {
    ++tag_string;
    tmp[0] = *tag_string;
    stealing_PyDict_SetItemString(tags, "frequency", PyLong_FromString(tmp, NULL, 10));
    ++tag_string;
  }

  if (PyList_Size(type)) {
    stealing_PyDict_SetItemString(tags, "type", type);
  } else {
    Py_DECREF(type);
  }

  if (*tag_string) {
    stealing_PyDict_SetItemString(tags, "other", PyUnicode_FromString(tag_string));
  }

  return tags;
}

static PyObject* Majka_find(Majka* self, PyObject* args, PyObject* kwds) {
  const char* word = NULL;
  char* results = new char[self->majka->max_results_size];
  char* entry, * colon;
  char tmp_lemma[300];
  PyObject* ret = PyList_New(0);
  PyObject* option;
  PyObject* lemma;
  int rc, i;


  static char* kwlist[] = {const_cast<char*>("word"), NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwds, "|s", kwlist, &word)) {
    delete [] results;
    Py_DECREF(ret);
    return NULL;
  }

  rc = self->majka->find(word, results, self->flags);

  if (rc == 0) {
    delete [] results;
    return ret;
  }

  for (entry = results, i=0; i < rc; i++, entry += strlen(entry) + 1) {
    colon = strchr(entry, ':');
    memcpy(tmp_lemma, entry, colon-entry);
    tmp_lemma[colon-entry] = '\0';
    lemma = PyUnicode_FromString(tmp_lemma);
    option = Py_BuildValue("{s:O}", "lemma", lemma);
    Py_DECREF(lemma);
    if (self->tags) {
      stealing_PyDict_SetItemString(option, "tags", Majka_tags(colon+1));
    }
    stealing_PyList_Append(ret, option);
  }
  delete [] results;
  return ret;
}

static PyMethodDef Majka_methods[] = {
  {"find", (PyCFunction)Majka_find, METH_VARARGS | METH_KEYWORDS,
   "Get results for given word."
  },
  {NULL}  /* Sentinel */
};

static PyMemberDef Majka_members[] = {
  {const_cast<char*>("flags"), T_INT, offsetof(Majka, flags), 0,
   const_cast<char*>("Flags to run Majka with.")},
  {const_cast<char*>("tags"), T_BOOL, offsetof(Majka, tags), 0,
   const_cast<char*>("If tags should be extracted and converted.")},
  {NULL}
};

static PyTypeObject MajkaType = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "majka.Majka",             /* tp_name */
  sizeof(Majka),             /* tp_basicsize */
  0,                         /* tp_itemsize */
  (destructor)Majka_dealloc, /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_reserved */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT |
      Py_TPFLAGS_BASETYPE,   /* tp_flags */
  "Majka object",            /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  Majka_methods,             /* tp_methods */
  Majka_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)Majka_init,      /* tp_init */
  0,                         /* tp_alloc */
  Majka_new,                 /* tp_new */
};

#ifdef PY3K
static PyModuleDef majkamodule = {
  PyModuleDef_HEAD_INIT,
  "majka",
  "Majka module.",
  -1,
  NULL, NULL, NULL, NULL, NULL
};

#define init_function PyInit_majka
#define init_return(value) return (value)
#else
#define init_function initmajka
#define init_return(unused) return
#endif

PyMODINIT_FUNC init_function(void) {
  PyObject* m;

  if (PyType_Ready(&MajkaType) < 0)
    init_return(NULL);
#ifdef PY3K
  m = PyModule_Create(&majkamodule);
#else
  m = Py_InitModule3("majka", NULL, NULL);
#endif
  if (m == NULL)
    init_return(NULL);

  Py_INCREF(&MajkaType);
  PyModule_AddObject(m, "Majka",
                     reinterpret_cast<PyObject*>(&MajkaType));
  PyModule_AddObject(m, "ADD_DIACRITICS",
                     PyLong_FromLong(ADD_DIACRITICS));
  PyModule_AddObject(m, "IGNORE_CASE",
                     PyLong_FromLong(IGNORE_CASE));
  PyModule_AddObject(m, "DISALLOW_LOWERCASE",
                     PyLong_FromLong(DISALLOW_LOWERCASE));
  init_return(m);
}

