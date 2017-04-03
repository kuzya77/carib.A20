/*
    auto_ptr
*/

template <class T>
inline auto_ptr_ref<T>::auto_ptr_ref (T* p)
  : ptr (p)
  { }
  
template <class T>
inline auto_ptr<T>::auto_ptr (element_type* p) throw ()
  : ptr (p)
  { }

template <class T>
inline auto_ptr<T>::auto_ptr (auto_ptr& x) throw ()
  : ptr (x.release ())
  { }
  
template <class T> template <class T1> 
inline auto_ptr<T>::auto_ptr (auto_ptr<T1>& x) throw ()
  : ptr (x.release ())
  { }  

template <class T>
inline auto_ptr<T>::auto_ptr (auto_ptr_ref<T> x) throw () 
  : ptr (x.ptr)
  { }

template <class T>
inline auto_ptr<T>::~auto_ptr ()
{
  delete ptr;
}

template <class T>
inline auto_ptr<T>& auto_ptr<T>::operator = (auto_ptr& x) throw ()
{
  if (&x != this)
    reset (x.release ());

  return *this;
}

template <class T> template <class T1> 
inline auto_ptr<T>& auto_ptr<T>::operator = (auto_ptr<T1>& x) throw ()
{
  if (x.get () != ptr)
    reset (x.release ());  
    
  return *this;
}

template <class T>
inline auto_ptr<T>& auto_ptr<T>::operator = (auto_ptr_ref<T> x) throw ()
{
  reset (x.ptr);  
  return *this;
}

template <class T>
inline T& auto_ptr<T>::operator * () const throw ()
{
  return *ptr;
}

template <class T>
inline T* auto_ptr<T>::operator -> () const throw ()
{
  return ptr;
}

template <class T>
inline T* auto_ptr<T>::get () const throw ()
{
  return ptr;
}

template <class T>
inline T* get_pointer (const auto_ptr<T>& p)
{
  return p.get ();
}

template <class T>
inline bool auto_ptr<T>::operator ! () const throw ()
{
  return ptr == 0;
}

template <class T>
inline T* auto_ptr<T>::release () throw ()
{
  T* tmp = ptr;
  ptr    = 0;
  
  return tmp;
}

template <class T>
inline void auto_ptr<T>::reset (T* p) throw ()
{
  if (p == ptr)
    return;
    
  T* tmp = ptr;
  ptr    = p;    

  delete tmp;
}

template <class T> template <class T1> 
inline auto_ptr<T>::operator auto_ptr_ref<T1> () throw ()
{
  return release ();
}

template <class T> template <class T1> 
inline auto_ptr<T>::operator auto_ptr<T1> () throw ()
{
  return release ();
}

template <class T>
inline void auto_ptr<T>::swap (auto_ptr<T>& p)
{
  T* tmp = p.ptr;
  p.ptr  = ptr;
  ptr    = tmp;
}

template <class T>
inline void swap (auto_ptr<T>& p1, auto_ptr<T>& p2)
{
  p1.swap (p2);
}
