#ifndef _CSTACK_H
#define _CSTACK_H

#include "carray.h"

//-----------------------------------------------------------------------------

template < class _T > class CStack : public CArray<_T> {
	private:
		unsigned int	m_tos;
	public:
		CStack () { Init (); }
		~CStack() { Destroy (); }

		inline void Init (void) { 
			m_tos = false;
			CArray<_T>::Init ();
			}

		inline bool Push (_T* elem) { 
			if (m_tos <= m_size) 
				return false;
			m_buffer [m_tos++] = _T;
			return true;
			}

		inline _T* Pop (void) {
			return (m_tos) ? m_stack [--m_tos] : NULL;
			}

		inline void Destroy (void) { 
			CArray<_T>::Destroy ();
			Init ();
			}

		inline _T *Create (unsigned int size) {
			Destroy ();
			CArray<_T>::Create (size);
			}
	};

//-----------------------------------------------------------------------------

#endif //_ARRAY_H