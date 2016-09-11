#ifndef TACS_OBJECT_H
#define TACS_OBJECT_H

/*!
  The following class implements a basic reference counting scheme
  for memory allocation/deallocation.

  Copyright (c) 2010 Graeme Kennedy. All rights reserved. 
  Not for commercial purposes.

  The following assumptions are made about references:
  - References returned from functions are always borrowed
  (except for constructors which own the new reference -- this is 
  the way I'd like to do it, but SWIG doesn't seem to support it easily)
  - References passed into functions have no effect - 
  references are never stolen.  
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <complex>
#include "mpi.h"

extern MPI_Op TACS_MPI_MIN;
extern MPI_Op TACS_MPI_MAX;

/*
  Use the cplx type for TacsComplex
*/
// typedef cplx TacsComplex;
typedef std::complex<double> TacsComplex;

/*
  Define the basic scalar type TacsScalar
*/
#ifdef TACS_USE_COMPLEX
#define TACS_MPI_TYPE MPI_DOUBLE_COMPLEX
typedef TacsComplex TacsScalar;
#else
#define TACS_MPI_TYPE MPI_DOUBLE
typedef double TacsScalar;
#endif

// Define the real part function for the complex data type
inline double RealPart( const TacsComplex& c ){
  return real(c);
}

// Define the imaginary part function for the complex data type
inline double ImagPart( const TacsComplex& c ){
  return imag(c);
}

// Dummy function for real part
inline double RealPart( const double& r ){
  return r;
}

// Compute the absolute value
inline TacsComplex fabs( const TacsComplex& c ){
  if (real(c) < 0.0){
    return -c;
  }
  return c;
}

/*
  Define the macro to add flop counts. This does not work for threaded
  implementations. Don't use it in threaded code!

  Note that this may only log FLOPs from certain parts of the code.
  I've tried to log all operations within the linear algebra portions of
  the code, but the analysis (e.g. residual/matrix computation) is much 
  more difficult.
*/
extern double tacs_local_flop_count;

// Zero the number of counted flops
void TacsZeroNumFlops(); 

// Retrieve the total number of counted flops
double TacsGetNumFlops();

// Macro to record the number of flops in an operation
#ifdef TACS_LOG_FLOPS
#define TacsAddFlops(flop) (tacs_local_flop_count += (flop));
#else
#define TacsAddFlops(flop) 
#endif

/*
  Set up the define statements for beginning/ending a namespace
  environment 
*/
#define TACS_BEGIN_NAMESPACE(a) namespace a {
#define TACS_END_NAMESPACE }

/*!
  Initialize some static variables related to MPI
*/
void TacsInitialize();

/*!
  Check if TacsInitialize has been called
*/
int TacsIsInitialized();

/*!
  Clean up from the Tacs initialization
*/
void TacsFinalize();

/*!
  TACSObject: The base class for all TACS objects to enable reference
  counting. In most cases this is sufficient to free any allocated 
  memory. 
*/
class TACSObject {
 public:
  TACSObject();
  virtual ~TACSObject();

  // Increase the reference count functions
  // --------------------------------------
  void incref();

  // Decrease the reference count
  // ----------------------------
  void decref();

  // Return the reference count
  // --------------------------
  int refcount();

  // Return the name of the object
  // -----------------------------
  virtual const char *TACSObjectName();

 private:
  int ref_count;
  static const char *tacsDefault;
};

/*!  
  TACSOptObject: Base class for objects that contain design
  variable information 
*/
class TACSOptObject : public TACSObject {
 public:
  TACSOptObject() : TACSObject() {}

  virtual ~TACSOptObject(){}

  /*!
    Set the design variable numbers owned by this object to those passed 
    into the object in dvs[]
    
    dvs:    An array of the design variable values 
    numDVs: The size of the array dvs
  */
  virtual void setDesignVars( const TacsScalar dvs[], int numDVs ){}

  /*!
    Get the current values of the design variables

    dvs:    An array of the design variable values 
    numDVs: The size of the array dvs
  */ 
  virtual void getDesignVars( TacsScalar dvs[], int numDVs ){}

  /*!
    Get the range of allowable design variable values
    
    lowerBound:  The design variable lower bounds
    upperBound:  The design variable upper bounds
    numDVs:      The size of the arrays lowerBound and upperBound
  */
  virtual void getDesignVarRange( TacsScalar lowerBound[], 
				  TacsScalar upperBound[], int numDVs ){}
};

/*!  
  Information about the number of threads to use in a computation.
  This should only be allocated by the TACSAssembler object. The
  number of threads is volitile in the sense that it can change
  between subsequent calls.
*/
class TACSThreadInfo : public TACSObject {
 public:
  static const int TACS_MAX_NUM_THREADS = 16;

  TACSThreadInfo( int _num_threads );
  ~TACSThreadInfo(){}

  void setNumThreads( int _num_threads );
  int getNumThreads();

 private:
  int num_threads;  
};

/*!
  The TACSSparseConObject object. 

  Evaluate sparse constraint information.
*/
class TACSSparseConObject : public TACSOptObject {
 public:
  virtual ~TACSSparseConObject(){}

  // Is this constraint linear?
  virtual int isLinear(){ return 0; }

  // Get the number of constraints to be added by this object
  virtual int getNumCon(){ return 0; }

  // Get the size of the CSR for this constraint
  virtual int getConCSRSize(){ return 0; }

  // Retreive the allowable range of values for this constraint
  virtual int getConRange( int offset, TacsScalar lb[], TacsScalar ub[] ){ 
    return 0; 
  }

  // Retrieve the CSR representation of the constraint
  virtual int addConCSR( int offset, int rowp[], int cols[] ){ return 0; }

  // Evaluate the constraints
  virtual int evalCon( int offset, TacsScalar con[] ){ return 0; }

  // Evaluate the gradient of the constraints
  virtual int evalConDVSens( int offset, TacsScalar Acol[], 
                             const int rowp[], const int cols[] ){
    return 0;
  }
};

#endif
