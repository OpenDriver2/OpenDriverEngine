//////////////////////////////////////////////////////////////////////////////////
// Copyright © Inspiration Byte
// 2009-2020
//////////////////////////////////////////////////////////////////////////////////
// Description: EqEngine mutex storage
//////////////////////////////////////////////////////////////////////////////////

#ifndef REFCOUNTED_H
#define REFCOUNTED_H

/*
	usage:

	1. Inherit it in your class
	------------------------------------------------------------------------------

	class CBlaBlaObject : public RefCountedObject

	2. Make it's removal function
	------------------------------------------------------------------------------

	void CBlaBlaObject::Ref_DeleteObject()
	{
		assignedFactory->Free( this );
	}

	3. Creating object. First you need to grab it
	------------------------------------------------------------------------------
	CBlaBlaObject* pObject = assignedFactory->CreateNew();

	pObject->Ref_Grab();

	4. Removal. Object will be removed automatically as Ref_DeleteObject provides
	------------------------------------------------------------------------------
	pObject->Ref_Drop();

	Don't do Ref_Drop inside assignedFactory->Free();
*/


class RefCountedObject
{
public:
			RefCountedObject()	: m_numRefs(0) {}
	virtual ~RefCountedObject() {}

	void	Ref_Grab()			{m_numRefs++;}
	bool	Ref_Drop();

	int		Ref_Count() const	{return m_numRefs;}

private:
	void	Ref_CheckRemove()
	{
		// ASSERT(m_numRefs < 0);

		if(m_numRefs <= 0)
			Ref_DeleteObject();
	}

	// deletes object when no references
	// example of usage: 
	// void Ref_DeleteObject()
	// {
	//		assignedRemover->Free(this);
	// }

	virtual void Ref_DeleteObject() = 0;

private:
	mutable int	m_numRefs;
};

inline bool	RefCountedObject::Ref_Drop()
{
	m_numRefs--;

	if(m_numRefs == 0)
	{
		Ref_DeleteObject();
		return true;
	}
	else if(m_numRefs < 0)
	{
		//ASSERTMSG(false, varargs("Ref_Drop NOT VALID (RefCount=%d)!!!", m_numRefs));
	}

	return false;
}

//-----------------------------------------------------------------------------

template< class TYPE >
class CRefPointer
{
public:
						CRefPointer() : m_ptrObj(nullptr) {}

						CRefPointer( const TYPE& pObject );
						CRefPointer( const CRefPointer<TYPE>& refptr );
	virtual				~CRefPointer();

	// frees object (before scope, if you econom-guy)
	void				Free();
	void				Assign( const TYPE& obj );
	void				Unassign(bool deref = true);

	operator const		TYPE& () const				{ return (TYPE)m_ptrObj; }
	operator			TYPE& ()					{ return (TYPE)m_ptrObj; }
	TYPE				p() const					{ return (TYPE)m_ptrObj; }
	TYPE				operator->() const			{ return (TYPE)m_ptrObj; }
	void				operator=( const TYPE& obj );
	void				operator=( const CRefPointer<TYPE>& refptr );

protected:
	TYPE				m_ptrObj;
};

//------------------------------------------------------------

template< class TYPE >
inline CRefPointer<TYPE>::CRefPointer( const TYPE& pObject ) : m_ptrObj(nullptr)
{
	Assign(pObject);
}

template< class TYPE >
inline CRefPointer<TYPE>::CRefPointer( const CRefPointer<TYPE>& refptr ) : m_ptrObj(nullptr)
{
	Assign(refptr.m_ptrObj);
}

template< class TYPE >
inline CRefPointer<TYPE>::~CRefPointer()
{
	Free();
}

// frees object (before scope, if you econom-guy)
template< class TYPE >
inline void CRefPointer<TYPE>::Free()
{
	if(m_ptrObj &&
		((RefCountedObject*)m_ptrObj)->Ref_Drop())
	m_ptrObj = nullptr;
}

template< class TYPE >
inline void CRefPointer<TYPE>::Assign( const TYPE& obj )
{
	if(m_ptrObj == obj)
		return;

	// del old ref
	RefCountedObject* oldObj = (RefCountedObject*)m_ptrObj;
	
	if(m_ptrObj = obj)
		((RefCountedObject*)obj)->Ref_Grab();

	if(oldObj != nullptr)
		oldObj->Ref_Drop();
}

template< class TYPE >
inline void CRefPointer<TYPE>::Unassign(bool deref)
{
	// del old ref
	RefCountedObject* oldObj = (RefCountedObject*)m_ptrObj;
	
	m_ptrObj = NULL;

	if(oldObj != nullptr && deref)
		oldObj->Ref_Drop();
}

template< class TYPE >
inline void CRefPointer<TYPE>::operator=( const TYPE& obj )
{
	Assign( obj );
}

template< class TYPE >
inline void CRefPointer<TYPE>::operator=( const CRefPointer<TYPE>& refptr )
{
	Assign( refptr.m_ptrObj);
}

#endif // REFCOUNTED_H