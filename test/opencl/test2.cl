int get_global_id(int a);

void VectorAdd( const long * a, const long * b, long * c,int iNumElements)
{
  int tGID = get_global_id(0);
  if ( tGID < iNumElements )
  {
    c[tGID] = a[tGID] + b[tGID];
  }
}
