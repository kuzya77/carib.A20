#ifndef __MYSTL_ALLOC__
#define __MYSTL_ALLOC__

#include <new>

#include <stl/utility>

namespace stl
{

//forward declarations
template <class T>                            class allocator;
template <class T,class AllocFn,class FreeFn> class simple_allocator;

///////////////////////////////////////////////////////////////////////////////////////////////////
///������� ���������
///////////////////////////////////////////////////////////////////////////////////////////////////
template <class T>
class allocator
{
  public:
    typedef typename non_const_type<T>::value_type  value_type;
    typedef size_t                                  size_type;
    typedef ptrdiff_t                               difference_type;
    typedef value_type*                             pointer;
    typedef value_type&                             reference;
    typedef const value_type*                       const_pointer;
    typedef const value_type&                       const_reference;

///////////////////////////////////////////////////////////////////////////////////////////////////
///�������������� ���������� ���� � ���������� ���� Other
///////////////////////////////////////////////////////////////////////////////////////////////////   
    template <class Other> struct rebind
    { 
      typedef typename default_allocator<Other>::allocator_type other; 
    };

///////////////////////////////////////////////////////////////////////////////////////////////////
///������������ � ������������
///////////////////////////////////////////////////////////////////////////////////////////////////           
    allocator () throw () {}
    allocator (const allocator&) throw () {}
    
    template <class Other> allocator (const allocator<Other>&) throw () {}
    
    template <class Other> allocator& operator = (const allocator<Other>&);

///////////////////////////////////////////////////////////////////////////////////////////////////
///��������� ������ / ����������� ��������� ���������� ����������� ��������3
///////////////////////////////////////////////////////////////////////////////////////////////////       
    pointer       address  (reference) const;
    const_pointer address  (const_reference) const;
    size_type     max_size () const throw ();

///////////////////////////////////////////////////////////////////////////////////////////////////
///��������� / ������������ ������
///////////////////////////////////////////////////////////////////////////////////////////////////
    pointer allocate   (size_type count,const void* hint = 0);
    void    deallocate (pointer ptr,size_type count);

///////////////////////////////////////////////////////////////////////////////////////////////////
///��������������� / ���������� ��������
///////////////////////////////////////////////////////////////////////////////////////////////////    
    void construct (pointer ptr,const value_type& val);
    void destroy   (pointer ptr);
    
///////////////////////////////////////////////////////////////////////////////////////////////////
///��������� ���� ����������� 
///  - ������ ���������� ����� allocator<T> ����� ������������� ����� allocator<Other>
///////////////////////////////////////////////////////////////////////////////////////////////////
    template <class Other> bool operator == (const allocator<Other>&) const { return true; }
    template <class Other> bool operator != (const allocator<Other>&) const { return false; }
    
///////////////////////////////////////////////////////////////////////////////////////////////////
///�������� ���������� ������� ����
///���������� ��� ������ C2064 � MSVC71 (�������� � ��������� ���������� ������������� �����������)
///////////////////////////////////////////////////////////////////////////////////////////////////               
    static allocator create () { return allocator (); }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///������������� ���������� ��� ���� void
///////////////////////////////////////////////////////////////////////////////////////////////////    
template <> class allocator<void>
{
  public:
    typedef void              value_type;
    typedef value_type*       pointer;
    typedef const value_type* const_pointer;

    template <class Other> struct rebind  { typedef allocator<Other> other; };

    allocator () throw () {}
    allocator (const allocator&) throw () {}

    template <class Other> allocator (const allocator<Other>&) throw () {}

    template <class Other> allocator& operator = (const allocator<Other>&) { return *this; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////
///���������� ������ ����������������� ����������
///////////////////////////////////////////////////////////////////////////////////////////////////
template <class T,class AllocFn=void* (*)(size_t),class FreeFn=void (*)(void*)>
class simple_allocator: public allocator<T>
{
  template <class T1,class AllocFn1,class FreeFn1> friend class simple_allocator;
  public:
    typedef typename allocator<T>::size_type        size_type;
    typedef typename allocator<T>::difference_type  difference_type;
    typedef typename allocator<T>::pointer          pointer;
  
///////////////////////////////////////////////////////////////////////////////////////////////////
///�������������� ���������� ���� � ���������� ���� Other
///////////////////////////////////////////////////////////////////////////////////////////////////   
    template <class Other> struct rebind
    { 
      typedef simple_allocator<Other,AllocFn,FreeFn> other;
    };  

///////////////////////////////////////////////////////////////////////////////////////////////////
///������������ � ������������
///////////////////////////////////////////////////////////////////////////////////////////////////           
    simple_allocator (AllocFn alloc,FreeFn free) throw ();
    simple_allocator (const simple_allocator&) throw ();
    
    template <class Other> 
    simple_allocator (const simple_allocator<Other,AllocFn,FreeFn>&) throw ();
    
    template <class Other> 
    simple_allocator& operator = (const simple_allocator<Other,AllocFn,FreeFn>&);

///////////////////////////////////////////////////////////////////////////////////////////////////
///��������� / ������������ ������
///////////////////////////////////////////////////////////////////////////////////////////////////        
    pointer allocate   (size_type count,const void* hint = 0);
    void    deallocate (pointer ptr,size_type count);
    
///////////////////////////////////////////////////////////////////////////////////////////////////
///��������� ���� ����������� 
///////////////////////////////////////////////////////////////////////////////////////////////////
    template <class Other> bool operator == (const simple_allocator<Other,AllocFn,FreeFn>&) const;
    template <class Other> bool operator != (const simple_allocator<Other,AllocFn,FreeFn>&) const;
    
///////////////////////////////////////////////////////////////////////////////////////////////////
///�������� ���������� ������� ���� (������� �������� ������ ��. � ������ allocator<T>)
///////////////////////////////////////////////////////////////////////////////////////////////////               
    static simple_allocator create () { return simple_allocator (); }
  
  private:
    AllocFn alloc;
    FreeFn  free;
};

#include <stl/detail/alloc.inl>
  
}

#endif
