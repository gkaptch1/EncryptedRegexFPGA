__kernel void VectorAdd( __global const long * a, __global const long * b, __global long * c,int iNumElements)
{
  int tGID = get_global_id(0);
  if ( tGID < iNumElements )
  {
    c[tGID] = a[tGID] + b[tGID];
  }
}
