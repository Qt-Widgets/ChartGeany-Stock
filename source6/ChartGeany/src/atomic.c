/* 
   Solaris Studio 12.6 C++ compiler, under Solaris 10, cannot find 
   q_atomic_increment and q_atomic_decrement in libQtCore, during linking. 
   We had to re-implement both here.
*/   

#ifdef __SunOS_5_10     /* Solaris 10 */
#ifdef __SUNPRO_C       /* Sun compiler */

int 
q_atomic_increment (volatile int *ptr)
{
  ++(*ptr);
  return *ptr ? !0 : 0;
}


int 
q_atomic_decrement (volatile int *ptr)
{
  --(*ptr);
  return *ptr ? !0 : 0;
}

#endif 		/* __SUNPRO_C */
#endif		/* __SunOS_5_10 */

