#pragma once
class BytesData
{
public:
  ~BytesData();
  BytesData(const char *bytes, const int count);
  BytesData(const BytesData &src);
  char* Bytes;
  int Count;
};

