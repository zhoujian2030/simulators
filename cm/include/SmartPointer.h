/*
 * SmartPointer.h
 *
 *  Created on: Jan 18, 2016
 *      Author: z.j
 */

#ifndef SMART_POINTER_H
#define SMART_POINTER_H

namespace cm {

template <class T> class SmartPointer {

public:

    /**
    * Constructor.
    */
    SmartPointer();

    /**
    * Constructor. Increases reference count.
    * @param theOther The pointer.
    */
    SmartPointer(const T* theOther);

    /**
    * Copy constructor. Increases reference count.
    * @param theOther The pointer.
    */
    SmartPointer(const SmartPointer<T>& theOther);

    /**
    * Destructor. Decreases the reference count.
    */
    virtual
    ~SmartPointer();

    /**
    * Return the object (dereferenced pointer).
    */ 
    operator T*();

    /**
    * Return the object (dereferenced pointer).
    */ 
    operator const T*() const;

    /**
    * Return reference to pointer.
    */
    T& 
    operator*();

    /**
    * Return reference to pointer.
    */
    const T& 
    operator*() const;

    /**
    * Returns the pointer.
    */
    T* 
    operator->();

    /**
    * Returns the pointer.
    */
    const T* 
    operator->() const;

    /**
    * Copy operator.
    * @param theOther Reference to the object to be copied.
    */
    SmartPointer& 
    operator=(const SmartPointer<T>& theOther);

    /**
    * Copy operator.
    * @param theOther Copies from a pointer.
    */
    SmartPointer& 
    operator=(const T* theOther);

private:
    
    T* pointerM;
};

    // ------------------------------------------------------------------------

    template <class T> 
    SmartPointer<T>::SmartPointer()
    :  pointerM(0)
    {
      // empty
    }
        
    // ------------------------------------------------------------------------

    template <class T> 
    SmartPointer<T>::SmartPointer(const SmartPointer<T> &theOther)
    :  pointerM(theOther.pointerM)
    {
      if (pointerM != 0)
      {
         pointerM->AddRef(); 
      }
    }
    
    // ------------------------------------------------------------------------

    template <class T> 
    SmartPointer<T>::SmartPointer(const T* theOther) 
    :  pointerM(const_cast<T*>(theOther)) 
    { 
      if (pointerM != 0)
      {
         pointerM->AddRef(); 
      }
    }

    // ------------------------------------------------------------------------

    template <class T>
    SmartPointer<T>::~SmartPointer() 
    { 
      if (pointerM != 0)
      {
         pointerM->Release(); 
      }
    }

    // ------------------------------------------------------------------------

    template <class T>
    SmartPointer<T>::operator T*()
    { 
      return pointerM; 
    }

    // ------------------------------------------------------------------------

    template <class T>
    SmartPointer<T>::operator const T*() const
    { 
      return pointerM; 
    }

    // ------------------------------------------------------------------------

    template <class T>
    T&
    SmartPointer<T>::operator*() 
    { 
      return *pointerM; 
    }

    // ------------------------------------------------------------------------

    template <class T>
    const T&
    SmartPointer<T>::operator*() const
    { 
      return *pointerM; 
    }

    // ------------------------------------------------------------------------

    template <class T>
    T* 
    SmartPointer<T>::operator->() 
    { 
      return pointerM; 
    }

    // ------------------------------------------------------------------------

    template <class T>
    const T* 
    SmartPointer<T>::operator->() const
    { 
      return pointerM; 
    }

    // ------------------------------------------------------------------------

    template <class T>
    SmartPointer<T>& 
    SmartPointer<T>::operator=(const SmartPointer<T>& theOther)
    {
      if (pointerM != 0)
      {
         pointerM->Release(); 
      }
      pointerM = const_cast<T*>(theOther.pointerM); 
      if (pointerM != 0)
      {
         pointerM->AddRef(); 
      }
      return *this;
    }

    // ------------------------------------------------------------------------

    template <class T>
    SmartPointer<T>& 
    SmartPointer<T>::operator=(const T* theOther) 
    {
      if (pointerM != 0)
      {
         pointerM->Release(); 
      }
      pointerM = const_cast<T*>(theOther);
      if (pointerM != 0)
      {
         pointerM->AddRef(); 
      }
      return *this;
    }

} // end of namespace

#endif //
