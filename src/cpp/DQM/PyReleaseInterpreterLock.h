#ifndef DQM_PYRELEASEINTERPRETERLOCK_H
#define DQM_PYRELEASEINTERPRETERLOCK_H

class PyReleaseInterpreterLock {
  PyThreadState *save_;
 public:
  PyReleaseInterpreterLock(void) { save_ = PyEval_SaveThread(); }
  ~PyReleaseInterpreterLock(void) { PyEval_RestoreThread(save_); }
};

#endif // DQM_PYRELEASEINTERPRETERLOCK_H
