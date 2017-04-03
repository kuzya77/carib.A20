/*
    ���� ���஢����
*/

template <class T>
inline hash_key<T>::hash_key (const value_type& value)
  : hash (hash_fun<T> ()(value))
  {}

template <class T>
inline size_t hash_key<T>::get_hash () const
{
  return hash;
}

template <class T>
inline bool hash_key<T>::operator == (const hash_key& key) const
{
  return hash == key.hash;
}

template <class T>
inline bool hash_key<T>::operator != (const hash_key& key) const
{
  return hash != key.hash;
}

/*
    �㭪樨 ���஢����
*/

inline size_t hash (int x)
{
  return x;
}

inline size_t hash (long x)
{
  return static_cast<size_t> (x);
}

inline size_t hash (unsigned int x)
{
  return x;
}

inline size_t hash (unsigned long x)
{
  return static_cast<size_t> (x);
}

template <class T> inline size_t hash (const T* p)
{
  return (size_t)p;
}

template <class T> inline size_t hash (const hash_key<T>& key)
{
  return key.get_hash ();
}

inline size_t hash (const unsigned char* s)
{
  return hash ((const char*)s);
}

inline size_t hash (const char* s)
{
  size_t h = 0;

  while (*s) h = 5 * h + *s++;

  return h;
}

inline size_t hash (const wchar_t* s)
{
  size_t h = 0;

  while (*s) h = 5 * h + *s++;

  return h;
}

//����祭�� ��᫥����⥫쭮�� ���
template <class T>
inline size_t hash (const T& object, size_t previous_hash)
{
  return hash (object) ^ previous_hash;
}

//����祭�� ��� ����
template <class T1, class T2>
inline size_t hash (const pair<T1, T2>& p)
{
  return hash (p.second, hash (p.first));
}
