/*
 * SmartPointerObject.h
 *
 *  Created on: Jan 18, 2016
 *      Author: z.j
 */
#ifndef SMART_POINTER_OBJECT_H
#define SMART_POINTER_OBJECT_H

namespace cm {

    /*
     * The class is the base class for all not thread-safe smart pointer
     * objects.
     */
    class SmartPointerObject
    {
    public:

        /**
        * Increment the reference count for the object. 
        * This method should be called for every new copy of a pointer 
        * to the given object. 
        */
        void 
        AddRef() const;

        /**
        * Decrements the reference count for the object. 
        * If the reference count on the object falls 
        * to 0, the object is deleted.
        */
        void 
        Release() const;

        protected:

        /**
        * Creates a reference counted object and sets its
        * reference counter to zero.
        */
        SmartPointerObject();

        /**
        * Destructor.
        */
        virtual
        ~SmartPointerObject();

    private:

        mutable unsigned long referenceCounterM;
    };

    // ------------------------------------------------------------------------

    inline
    void 
    SmartPointerObject::AddRef() const
    {
      referenceCounterM++;
    }

    // ------------------------------------------------------------------------

    inline
    void 
    SmartPointerObject::Release() const
    {
      referenceCounterM--;
      if (referenceCounterM == 0)
      {
         delete this;
      }
    }

    // ------------------------------------------------------------------------

    inline
    SmartPointerObject::SmartPointerObject()
    :  referenceCounterM(0)
    {
      // empty
    }

    // ------------------------------------------------------------------------

    inline
    SmartPointerObject::~SmartPointerObject()
    {
      // empty
    }

} // end of namespace cm

#endif
