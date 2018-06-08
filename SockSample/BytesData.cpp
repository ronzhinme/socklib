#include "BytesData.h"
#include <string.h>

BytesData::~BytesData()
{
  delete[]Bytes;
}

BytesData::BytesData(const char * bytes, const int count)
{
  Count = count;
  Bytes = new char[Count];
  memset(Bytes, 0, Count);
  memcpy(Bytes, bytes, count);
}

BytesData::BytesData(const BytesData & src)
{
  Count = src.Count;
  Bytes = new char[Count];
  memset(Bytes, 0, Count);
  memcpy(Bytes, src.Bytes, Count);
}
