/* Low-level functions for universes
 *
 * Written by Konrad Hinsen
 * last revision: 2006-5-30
 */

#define _UNIVERSE_MODULE

#include "MMTK/core.h"
#include "MMTK/universe.h"


#define DEBUG 0
#define THREAD_DEBUG 0

/*
 * Distance vector functions
 */
static void
distance_vector(vector3 d, vector3 r1, vector3 r2, double *data)
{
  distance_vector_1(d, r1, r2, data);
}

static void
orthorhombic_distance_vector(vector3 d, vector3 r1, vector3 r2, double *data)
{
  distance_vector_2(d, r1, r2, data);
}

/*
 * Position correction functions (to fold coordinates into the central box)
 */
static void
no_correction(vector3 *x, int natoms, double *data)
{
}

static void
orthorhombic_correction(vector3 *x, int natoms, double *data)
{
  double a = data[0];
  double b = data[1];
  double c = data[2];
  double ah = 0.5*a, bh = 0.5*b, ch = 0.5*c;
  int i;
#if DEBUG
  if (a > 0. && b > 0. && c > 0.)
    for (i = 0; i < natoms; i++) {
      if (x[i][0] > ah) x[i][0] -= a;
      while (x[i][0] > ah) {
	printf("x[%d]=%lf, a=%lf\n", i, x[i][0], a);
	x[i][0] -= a;
      }
      if (x[i][0] < -ah) x[i][0] += a;
      while (x[i][0] < -ah) {
	printf("x[%d]=%lf, -a=%lf\n", i, x[i][0], -a);
	x[i][0] += a;
      }

      if (x[i][1] > bh) x[i][1] -= b;
      while (x[i][1] > bh) {
	printf("x[%d]=%lf, b=%lf\n", i, x[i][1], b);
	x[i][1] -= b;
      }
      if (x[i][1] < -bh) x[i][1] += b;
      while (x[i][1] < -bh) {
	printf("x[%d]=%lf, -b=%lf\n", i, x[i][1], -b);
	x[i][1] += b;
      }

      if (x[i][2] > ch) x[i][2] -= c;
      while (x[i][2] > ch) {
	printf("x[%d]=%lf, c=%lf\n", i, x[i][2], c);
	x[i][2] -= c;
      }
      if (x[i][2] < -ch) x[i][2] += c;
      while (x[i][2] < -ch) {
	printf("x[%d]=%lf, -c=%lf\n", i, x[i][2], -c);
	x[i][2] += c;
      }
    }
#else
  if (a > 0. && b > 0. && c > 0.)
    for (i = 0; i < natoms; i++) {
      while (x[i][0] > ah) x[i][0] -= a;
      while (x[i][0] < -ah) x[i][0] += a;
      while (x[i][1] > bh) x[i][1] -= b;
      while (x[i][1] < -bh) x[i][1] += b;
      while (x[i][2] > ch) x[i][2] -= c;
      while (x[i][2] < -ch) x[i][2] += c;
    }
#endif
}

/*
 * Volume scaling functions
 */
static double
no_volume(double scale_factor, double *data)
{
  return -1.;
}

static double
orthorhombic_volume(double scale_factor, double *data)
{
  data[0] *= scale_factor;
  data[1] *= scale_factor;
  data[2] *= scale_factor;
  return data[0]*data[1]*data[2];
}

/*
 * Box coordinate transformation functions
 */
static void
no_box(vector3 *x, vector3 *b, int n, double *data, int to_box)
{
}

static void
orthorhombic_box(vector3 *x, vector3 *b, int n, double *data, int to_box)
{
  int i;
  if (to_box)
    for (i = 0; i < n; i++) {
      b[i][0] = x[i][0]/data[0];
      b[i][1] = x[i][1]/data[1];
      b[i][2] = x[i][2]/data[2];
    }
  else
    for (i = 0; i < n; i++) {
      x[i][0] = b[i][0]*data[0];
      x[i][1] = b[i][1]*data[1];
      x[i][2] = b[i][2]*data[2];
    }
}

/*
 * Trajectory transformation functions
 */
static void
no_trajectory(vector3 *x, vector3 *b, int nsteps, double *data, int to_box)
{
}

static void
orthorhombic_trajectory(vector3 *x, vector3 *b, int nsteps,
			double *data, int to_box)
{
  int i;
  if (to_box)
    for (i = 0; i < nsteps; i++) {
      b[i][0] = x[i][0]/data[3*i];
      b[i][1] = x[i][1]/data[3*i+1];
      b[i][2] = x[i][2]/data[3*i+2];
    }
  else
    for (i = 0; i < nsteps; i++) {
      x[i][0] = b[i][0]*data[3*i];
      x[i][1] = b[i][1]*data[3*i+1];
      x[i][2] = b[i][2]*data[3*i+2];
    }
}

/*
 * Bounding box functions
 */
static void
infinite_bounding_box(vector3 *box1, vector3 *box2, vector3 *x,
		      int n, double *data)
{
  int i;
  vector_copy(*box1, x[0]);
  vector_copy(*box2, x[0]);
  for (i = 1; i < n; i++) {
    if (x[i][0] < (*box1)[0]) (*box1)[0] = x[i][0]; 
    if (x[i][1] < (*box1)[1]) (*box1)[1] = x[i][1]; 
    if (x[i][2] < (*box1)[2]) (*box1)[2] = x[i][2]; 
    if (x[i][0] > (*box2)[0]) (*box2)[0] = x[i][0]; 
    if (x[i][1] > (*box2)[1]) (*box2)[1] = x[i][1]; 
    if (x[i][2] > (*box2)[2]) (*box2)[2] = x[i][2]; 
  }
}

static void
orthorhombic_bounding_box(vector3 *box1, vector3 *box2, vector3 *x,
			  int n, double *data)
{
  (*box2)[0] = 0.5*data[0];
  (*box2)[1] = 0.5*data[1];
  (*box2)[2] = 0.5*data[2];
  (*box1)[0] = -(*box2)[0];
  (*box1)[1] = -(*box2)[1];
  (*box1)[2] = -(*box2)[2];
}

/*
 *  Type "universe specification"
 *
 * Objects of this type provide a low-level description of universes,
 * in particular their geometry.
 *
 * The structure declaration is in mmtk_universe.h.
 */

/* Allocation and deallocation */

static PyUniverseSpecObject *
universe_new(void)
{
  PyUniverseSpecObject *self;
  int i, error;

  self = PyObject_NEW(PyUniverseSpecObject, &PyUniverseSpec_Type);
  if (self == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  self->geometry = NULL;
  self->geometry_data = NULL;
  self->distance_function = NULL;
  self->correction_function = NULL;
  self->volume_function = NULL;
  self->box_function = NULL;
  self->trajectory_function = NULL;
  self->bounding_box_function = NULL;
  self->is_periodic = 0;
#ifdef WITH_THREAD
  error = 0;
  self->main_state_lock = PyThread_allocate_lock();
  if (self->main_state_lock == NULL)
    error = 1;
  if (!error) {
    self->configuration_change_lock = PyThread_allocate_lock();
    if (self->configuration_change_lock == NULL)
      error = 1;
  }
  for (i = 0; i < MMTK_MAX_THREADS && !error; i++) {
    self->state_wait_lock[i] = PyThread_allocate_lock();
    if (self->state_wait_lock[i] == NULL)
      error = 1;
    else
      PyThread_acquire_lock(self->state_wait_lock[i], 1);
    self->state_access_type[i] = 0;
  }
  if (error) {
      PyErr_SetString(PyExc_OSError, "couldn't allocate lock");
      PyObject_Del(self);
      return NULL;
  }
  self->state_access = 0;
  self->waiting_threads = 0;
#endif
  return self;
}

static void
universe_dealloc(PyUniverseSpecObject *self)
{
  Py_XDECREF(self->geometry);
  PyObject_Del(self);
}

/* Methods */

static PyObject*
call_correction_function_py(PyObject *self, PyObject *args)
{
  PyUniverseSpecObject *universe = (PyUniverseSpecObject *)self;
  PyArrayObject *configuration;

  vector3 *x;
  int natoms;
  double *data;

  if (!PyArg_ParseTuple(args, "O!",
			&PyArray_Type, &configuration))
    return NULL;

  x = (vector3 *)configuration->data;
  natoms = configuration->dimensions[0];
  data = (double *)universe->geometry->data;
  universe->correction_function(x, natoms, data);

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject*
distance_vector_py(PyObject *self, PyObject *args)
{
  PyUniverseSpecObject *universe = (PyUniverseSpecObject *)self;
  PyArrayObject *r1, *r2;
  PyArrayObject *geometry_data = NULL;

  PyArrayObject *d;
  int three = 3;

  if (!PyArg_ParseTuple(args, "O!O!|O!",
			&PyArray_Type, &r1,
			&PyArray_Type, &r2,
			&PyArray_Type, &geometry_data))
    return NULL;

  d = (PyArrayObject *)PyArray_FromDims(1, &three, PyArray_DOUBLE);
  if (d == NULL)
    return NULL;
  if (geometry_data != NULL)
    universe->distance_function(*(vector3 *)d->data,
				*(vector3 *)r1->data, *(vector3 *)r2->data,
				(double *)geometry_data->data);
  else
    universe->distance_function(*(vector3 *)d->data,
				*(vector3 *)r1->data, *(vector3 *)r2->data,
				(double *)universe->geometry->data);
  return (PyObject *)d;
}

/* State lock management */

int
PyUniverseSpec_StateLock(PyUniverseSpecObject *universe, int action)
{
  /* action = 1: acquire read access
     action = 2: release read access
     action = -1: acquire write access
     action = -2: release write access
  */
#ifdef WITH_THREAD
  int error;
  int i;
#ifdef THREAD_DEBUG
  int id;
#endif

  id = PyThread_get_thread_ident();
  PyThread_acquire_lock(universe->main_state_lock, 1);
#if THREAD_DEBUG
  printf("thread %d entering; state is %d, %d threads waiting\n",
	 id, universe->state_access, universe->waiting_threads);
  for (i = 0; i < MMTK_MAX_THREADS; i++) {
    if (universe->state_access_type[i] == 1)
      printf("  lock %d waiting for read access\n", i);
    else if (universe->state_access_type[i] == -1)
      printf("  lock %d waiting for write access\n", i);
  }
#endif
  error = 0;
  switch(action) {
  case 1:
    while (universe->state_access < 0) {
      if (universe->waiting_threads == MMTK_MAX_THREADS) {
	PyErr_SetString(PyExc_OSError, "too many threads");
	error = 1;
      }
      for (i = 0; i < MMTK_MAX_THREADS; i++) {
	if (universe->state_access_type[i] == 0)
	  break;
      }
#if THREAD_DEBUG
      printf("thread %d waiting for read access using lock %d\n", id, i);
#endif
      universe->state_access_type[i] = 1;
      universe->waiting_threads++;
      PyThread_release_lock(universe->main_state_lock);
      PyThread_acquire_lock(universe->state_wait_lock[i], 1);
      PyThread_acquire_lock(universe->main_state_lock, 1);
      universe->waiting_threads--;
      universe->state_access_type[i] = 0;
    }
#if THREAD_DEBUG
    printf("thread %d has read access\n", id);
#endif
    universe->state_access++;
    break;
  case 2:
#if THREAD_DEBUG
    if (universe->state_access <= 0)
      printf("thread %d found state < 0 after read access\n", id);
    printf("thread %d gives up read access; %d threads waiting\n",
	   id, universe->waiting_threads);
#endif
    universe->state_access--;
    if (universe->state_access == 0 && universe->waiting_threads > 0) {
      for (i = 0; i < MMTK_MAX_THREADS; i++) {
	if (universe->state_access_type[i] == -1) {
#if THREAD_DEBUG
	  printf("thread %d releases lock %d\n", id, i);
#endif
	  PyThread_release_lock(universe->main_state_lock);
	  PyThread_release_lock(universe->state_wait_lock[i]);
	  PyThread_acquire_lock(universe->main_state_lock, 1);
	  break;
	}
      }
#if THREAD_DEBUG
      if (i == MMTK_MAX_THREADS)
	printf("Error: no thread waiting for write access!\n");
#endif
    }
    break;
  case -1:
    while (universe->state_access != 0) {
      if (universe->waiting_threads == MMTK_MAX_THREADS) {
	PyErr_SetString(PyExc_OSError, "too many threads");
	error = 1;
      }
      for (i = 0; i < MMTK_MAX_THREADS; i++) {
	if (universe->state_access_type[i] == 0)
	  break;
      }
#if THREAD_DEBUG
      printf("thread %d waiting for write access using lock %d\n", id, i);
#endif
      universe->state_access_type[i] = -1;
      universe->waiting_threads++;
      PyThread_release_lock(universe->main_state_lock);
      PyThread_acquire_lock(universe->state_wait_lock[i], 1);
      PyThread_acquire_lock(universe->main_state_lock, 1);
      universe->waiting_threads--;
      universe->state_access_type[i] = 0;
    }
#if THREAD_DEBUG
    printf("thread %d has write access\n", id);
#endif
    universe->state_access = -1;
    break;
  case -2:
#if THREAD_DEBUG
    printf("thread %d gives up write access; %d threads waiting\n",
	   id, universe->waiting_threads);
#endif
    universe->state_access = 0;
    if (universe->waiting_threads > 0) {
      for (i = 0; i < MMTK_MAX_THREADS; i++) {
	if (universe->state_access_type[i] == -1) {
#if THREAD_DEBUG
	  printf("thread %d releases lock %d\n", id, i);
#endif
	  PyThread_release_lock(universe->main_state_lock);
	  PyThread_release_lock(universe->state_wait_lock[i]);
	  PyThread_acquire_lock(universe->main_state_lock, 1);
	  break;
	}
      }
      if (i == MMTK_MAX_THREADS) {
	for (i = 0; i < MMTK_MAX_THREADS; i++) {
	  if (universe->state_access_type[i] == 1) {
#if THREAD_DEBUG
	    printf("thread %d releases lock %d\n", id, i);
#endif
	    PyThread_release_lock(universe->main_state_lock);
	    PyThread_release_lock(universe->state_wait_lock[i]);
	    PyThread_acquire_lock(universe->main_state_lock, 1);
	  }
	}
#if THREAD_DEBUG
	if (i == MMTK_MAX_THREADS)
	  printf("Error: no thread waiting for read access!\n");
#endif
      }
    }
    break;
  }
  PyThread_release_lock(universe->main_state_lock);
  return !error;
#else
  return 1;
#endif
}

static PyObject*
state_lock_py(PyObject *object, PyObject *args)
{
  PyUniverseSpecObject *self = (PyUniverseSpecObject *)object;
  int action;
  int ok;
  if (!PyArg_ParseTuple(args, "i", &action))
    return NULL;
  Py_BEGIN_ALLOW_THREADS;
  ok = PyUniverseSpec_StateLock(self, action);
  Py_END_ALLOW_THREADS;
  if (ok) {
    Py_INCREF(Py_None);
    return Py_None;
  }
  else
    return NULL;
}

static PyObject*
configuration_change_lock_py(PyObject *object, PyObject *args)
{
  PyUniverseSpecObject *self = (PyUniverseSpecObject *)object;
  int action;
  int success;
  if (!PyArg_ParseTuple(args, "i", &action))
    return NULL;
  /* action = 0: acquire lock non-blocking
     action = 1: acquire lock blocking
     action = 2: release lock
  */
#ifdef WITH_THREAD
  Py_BEGIN_ALLOW_THREADS;
  switch(action) {
  case 0:
    success = PyThread_acquire_lock(self->configuration_change_lock, 0);
    break;
  case 1:
    success = PyThread_acquire_lock(self->configuration_change_lock, 1);
    break;
  case 2:
    PyThread_release_lock(self->configuration_change_lock);
    success = 1;
    break;
  }
  Py_END_ALLOW_THREADS;
  return PyInt_FromLong((long)success);
#else
  return PyInt_FromLong(1L);
#endif
}

/* Documentation string */

static char PyUniverseSpec_Type__doc__[] = 
  "low-level universe specification";

/* Method table */

static struct PyMethodDef universe_methods[] = {
  {"distanceVector", distance_vector_py, 1},
  {"foldCoordinatesIntoBox", call_correction_function_py, 1},
  {"stateLock", state_lock_py, 1},
  {"configurationChangeLock", configuration_change_lock_py, 1},
  {NULL, NULL} /* sentinel */
};

/* Attribute access. */

static PyObject *
universe_getattr(PyUniverseSpecObject *self, char *name)
{
  return Py_FindMethod(universe_methods, (PyObject *)self, name);
}

/* Type object */

PyTypeObject PyUniverseSpec_Type = {
  PyObject_HEAD_INIT(NULL)
  0,			          /*ob_size*/
  "UniverseSpec",	          /*tp_name*/
  sizeof(PyUniverseSpecObject),    /*tp_basicsize*/
  0,			          /*tp_itemsize*/
  /* methods */
  (destructor)universe_dealloc,   /*tp_dealloc*/
  0,			          /*tp_print*/
  (getattrfunc)universe_getattr,  /*tp_getattr*/
  0, 			          /*tp_setattr*/
  0,			          /*tp_compare*/
  0,                              /*tp_repr*/
  0,                              /*tp_as_number*/
  0,			          /*tp_as_sequence*/
  0,			          /*tp_as_mapping*/
  0,			          /*tp_hash*/
  0,	                          /*tp_call*/
  0,                              /*tp_str*/
  0,                              /*tp_getattro*/
  0,                              /*tp_setattro*/
  /* Space for future expansion */
  0L,0L,
  /* Documentation string */
  PyUniverseSpec_Type__doc__
};


/*
 * Creators for infinite and periodic universes
 */
static PyObject *
InfiniteUniverseSpec(PyObject *dummy, PyObject *args)
{
  PyUniverseSpecObject *new;
  if (!PyArg_ParseTuple(args, ""))
    return NULL;
  new = universe_new();
  if (new == NULL)
    return NULL;
  new->geometry = (PyArrayObject *)PyArray_FromDims(0, NULL, PyArray_DOUBLE);
  new->geometry_data = NULL;
  new->geometry_data_length = 0;
  new->distance_function = distance_vector;
  new->correction_function = no_correction;
  new->volume_function = no_volume;
  new->box_function = no_box;
  new->trajectory_function = no_trajectory;
  new->bounding_box_function = infinite_bounding_box;
  new->is_periodic = 0;
  return (PyObject *)new;
}

static PyObject *
OrthorhombicPeriodicUniverseSpec(PyObject *dummy, PyObject *args)
{
  PyUniverseSpecObject *new;
  PyArrayObject *geometry;
  if (!PyArg_ParseTuple(args, "O!", &PyArray_Type, &geometry))
    return NULL;
  new = universe_new();
  if (new == NULL)
    return NULL;
  new->geometry = geometry;
  Py_INCREF(geometry);
  new->geometry_data = (double *)new->geometry->data;
  new->geometry_data_length = 3;
  new->distance_function = orthorhombic_distance_vector;
  new->correction_function = orthorhombic_correction;
  new->volume_function = orthorhombic_volume;
  new->box_function = orthorhombic_box;
  new->trajectory_function = orthorhombic_trajectory;
  new->bounding_box_function = orthorhombic_bounding_box;
  new->is_periodic = 1;
  return (PyObject *)new;
}

/*
 * Find offset to be added to each atom position to make all atoms
 * contiguous in spite of periodic boundary conditions.
 */
static PyObject *
contiguous_object_offset(PyObject *dummy, PyObject *args)
{
  PyUniverseSpecObject *spec;
  PyArrayObject *pairs, *conf, *offsets;
  PyArrayObject *geometry = NULL;
  double *geometry_data;
  long *p;
  vector3 *x, *o;
  int npairs, box_coor_flag;
  int i;

  if (!PyArg_ParseTuple(args, "O!O!O!O!i|O!",
			&PyUniverseSpec_Type, &spec,
			&PyArray_Type, &pairs,
			&PyArray_Type, &conf,
                        &PyArray_Type, &offsets,
			&box_coor_flag,
			&PyArray_Type, &geometry))
    return NULL;

  if (geometry == NULL)
    geometry_data = spec->geometry_data;
  else
    geometry_data = (double *)geometry->data;
  npairs = pairs->dimensions[0];
  p = (long *)pairs->data;
  x = (vector3 *)conf->data;
  o = (vector3 *)offsets->data;
  for (i = 0; i < npairs; i++) {
    int a1 = p[2*i], a2=p[2*i+1];
    vector3 pos1, d;
    vector_copy(pos1, x[a1]);
    vector_add(pos1, o[a1], 1.);
    spec->distance_function(d, pos1, x[a2], geometry_data);
    o[a2][0] = d[0] + pos1[0] - x[a2][0];
    o[a2][1] = d[1] + pos1[1] - x[a2][1];
    o[a2][2] = d[2] + pos1[2] - x[a2][2];
  }
  if (box_coor_flag)
    spec->box_function(o, o, offsets->dimensions[0], geometry_data, 1);
  Py_INCREF(Py_None);
  return Py_None;
}

#if 0
static PyObject *
contiguous_object_offset(PyObject *dummy, PyObject *args)
{
  PyUniverseSpecObject *spec;
  PyArrayObject *atoms, *conf, *masses, *offsets;
  PyArrayObject *geometry = NULL;
  double *geometry_data;
  vector3 center, cms;
  long *a;
  vector3 *x, *o;
  double *m;
  int natoms;
  double mass = 0.;
  int i;

  if (!PyArg_ParseTuple(args, "O!O!O!O!O!|O!",
			&PyUniverseSpec_Type, &spec,
			&PyArray_Type, &atoms,
			&PyArray_Type, &conf,
			&PyArray_Type, &masses,
                        &PyArray_Type, &offsets,
			&PyArray_Type, &geometry))
    return NULL;

  if (geometry == NULL)
    geometry_data = spec->geometry_data;
  else
    geometry_data = (double *)geometry->data;
  natoms = atoms->dimensions[0];
  a = (long *)atoms->data;
  m = (double *)masses->data;
  x = (vector3 *)conf->data;
  o = (vector3 *)offsets->data;
  cms[0] = m[a[0]]*x[a[0]][0];
  cms[1] = m[a[0]]*x[a[0]][1];
  cms[2] = m[a[0]]*x[a[0]][2];
  mass = m[a[0]];
  for (i = 1; i < natoms; i++) {
    int ai = a[i];
    vector3 d;
    center[0] = cms[0]/mass;
    center[1] = cms[1]/mass;
    center[2] = cms[2]/mass;
    spec->distance_function(d, center, x[ai], geometry_data);
    o[ai][0] = d[0] + center[0] - x[ai][0];
    o[ai][1] = d[1] + center[1] - x[ai][1];
    o[ai][2] = d[2] + center[2] - x[ai][2];
    cms[0] += m[ai]*(x[ai][0]+o[ai][0]);
    cms[1] += m[ai]*(x[ai][1]+o[ai][1]);
    cms[2] += m[ai]*(x[ai][2]+o[ai][2]);
    mass += m[ai];
  }
  center[0] = cms[0]/mass;
  center[1] = cms[1]/mass;
  center[2] = cms[2]/mass;
  vector_copy(cms, center);
  spec->correction_function(&center, 1, geometry_data);
  if (cms[0] != center[0] || cms[1] != center[1] || cms[2] != center[2]) {
    for (i = 0; i < natoms; i++) {
      int ai = a[i];
      o[ai][0] += center[0]-cms[0];
      o[ai][1] += center[1]-cms[1];
      o[ai][2] += center[2]-cms[2];
    }
  }
  Py_INCREF(Py_None);
  return Py_None;
}
#endif

/*
 * Module method table
 */
static PyMethodDef universe_module_methods[] = {
  {"InfiniteUniverseSpec", InfiniteUniverseSpec, 1},
  {"OrthorhombicPeriodicUniverseSpec", OrthorhombicPeriodicUniverseSpec, 1},
  {"contiguous_object_offset", contiguous_object_offset, 1},
  {NULL, NULL}		/* sentinel */
};

/*
 * Initialization function for the module
 */
DL_EXPORT(void)
initMMTK_universe(void)
{
  PyObject *m, *d;
  static void *PyUniverse_API[PyUniverse_API_pointers];

  /* Patch object type */
#ifdef EXTENDED_TYPES
  if (PyType_Ready(&PyUniverseSpec_Type) < 0)
    return;
#else
  PyUniverseSpec_Type.ob_type = &PyType_Type;
#endif

  /* Create the module */
  m = Py_InitModule("MMTK_universe", universe_module_methods);
  d = PyModule_GetDict(m);

  /* Import the array module */
#ifdef import_array
  import_array();
#endif

  /* Add C API pointer array */ 
  PyUniverse_API[PyUniverseSpec_Type_NUM] = (void *)&PyUniverseSpec_Type;
  PyUniverse_API[PyUniverseSpec_StateLock_NUM] =
                                    (void *)&PyUniverseSpec_StateLock;
  PyDict_SetItemString(d, "_C_API",
		       PyCObject_FromVoidPtr((void *)PyUniverse_API, NULL));

  /* Add function pointer objects */
  PyDict_SetItemString(d, "infinite_universe_distance_function",
		       PyCObject_FromVoidPtr((void *) distance_vector, NULL));
  PyDict_SetItemString(d, "infinite_universe_correction_function",
		       PyCObject_FromVoidPtr((void *) no_correction, NULL));
  PyDict_SetItemString(d, "infinite_universe_volume_function",
		       PyCObject_FromVoidPtr((void *) no_volume, NULL));
  PyDict_SetItemString(d, "orthorhombic_universe_distance_function",
		       PyCObject_FromVoidPtr((void *)
					     orthorhombic_distance_vector,
					     NULL));
  PyDict_SetItemString(d, "orthorhombic_universe_correction_function",
		       PyCObject_FromVoidPtr((void *) orthorhombic_correction,
					     NULL));
  PyDict_SetItemString(d, "orthorhombic_universe_volume_function",
		       PyCObject_FromVoidPtr((void *) orthorhombic_volume,
					     NULL));
  PyDict_SetItemString(d, "orthorhombic_universe_box_transformation",
		       PyCObject_FromVoidPtr((void *) orthorhombic_box,
					     NULL));

  /* Check for errors */
  if (PyErr_Occurred())
    Py_FatalError("can't initialize module MMTK_universe");
}
